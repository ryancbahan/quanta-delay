#include "DampManager.h"
#include <random>
#include <cmath>

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0), smoothedDamp(0.0f), lastUpdatedDamp(0.0f),
      smoothedDamping(0.001f), roomSize(1.0f), reflectionGain(0.7f),
      decayTime(1.5f), modulationRate(0.5f), modulationDepth(0.1f), modulationPhase(0.0f),
      initialCutoff(20000.0f), cutoffDecayRate(0.5f),
      numActiveEchoes(0), numActiveReflections(0)
{
    // Seed RNG with a unique value
    std::random_device rd;
    rng.seed(rd());
}

void DampManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();

    echoBuffer.setSize(2, static_cast<int>(MAX_ECHO_TIME * sampleRate) + 1);
    echoBuffer.clear();

    generateReflectionPattern();
    updateEchoParameters();
    precalculateValues();

    // Prepare stereo managers
    int totalDelays = MAX_ECHOES + MAX_REFLECTIONS;
    for (int i = 0; i < totalDelays; ++i) {
        stereoManagers[i].prepare(spec);
    }

    // Set the initial cutoff frequency higher if needed
    initialCutoff = 10000.0f; // Increase cutoff to 10kHz
    lowpassCoeffsLeft = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, initialCutoff);
    lowpassFilterLeft.coefficients = lowpassCoeffsLeft;

    lowpassCoeffsRight = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, initialCutoff);
    lowpassFilterRight.coefficients = lowpassCoeffsRight;

    // Calculate integer modulation rate for efficient computation
    modulationRateInt = static_cast<int>(modulationRate * MODULATION_TABLE_SIZE / sampleRate);
}

void DampManager::reset()
{
    echoBuffer.clear();
    writePos = 0;
    smoothedDamp = damp;
    lastUpdatedDamp = damp;
    smoothedDamping.reset(sampleRate, 0.1);
    modulationPhase = 0.0f;
    lowpassFilterLeft.reset();
    lowpassFilterRight.reset();
}

void DampManager::setDamp(float newDamp)
{
    damp = juce::jlimit(0.0f, 1.0f, newDamp);
    smoothedDamping.setTargetValue(damp);
}

void DampManager::precalculateValues()
{
    // Precompute the modulation table
    for (int i = 0; i < MODULATION_TABLE_SIZE; ++i)
    {
        float phase = 2.0f * juce::MathConstants<float>::pi * i / MODULATION_TABLE_SIZE;
        modulationTable[i] = 1.0f + modulationDepth * std::sin(phase);
    }

    float cumulativeLeftGain = 0.0f;
    float cumulativeRightGain = 0.0f;

    // Recalculate decay gains for echoes and compute cumulative gains
    for (int i = 0; i < numActiveEchoes; ++i)
    {
        float time = echoDelays[i] / sampleRate;
        float decayGain = echoGains[i] * std::exp(-time / decayTime);

        // Get panning gains
        float leftGain = stereoManagers[i].getLeftGain();
        float rightGain = stereoManagers[i].getRightGain();

        // Store individual left and right decay gains
        decayGainsLeft[i] = decayGain * leftGain;
        decayGainsRight[i] = decayGain * rightGain;

        cumulativeLeftGain += decayGainsLeft[i];
        cumulativeRightGain += decayGainsRight[i];
    }

    // Recalculate decay gains for reflections and compute cumulative gains
    reflectionDecayGainsLeft.resize(reflectionDelays.size());
    reflectionDecayGainsRight.resize(reflectionDelays.size());

    for (size_t i = 0; i < reflectionDelays.size(); ++i)
    {
        float time = reflectionDelays[i] / sampleRate;
        float decayGain = reflectionGains[i] * std::exp(-2.0f * time / decayTime);

        int index = numActiveEchoes + static_cast<int>(i);
        float leftGain = stereoManagers[index].getLeftGain();
        float rightGain = stereoManagers[index].getRightGain();

        // Store individual left and right decay gains
        reflectionDecayGainsLeft[i] = decayGain * leftGain;
        reflectionDecayGainsRight[i] = decayGain * rightGain;

        cumulativeLeftGain += reflectionDecayGainsLeft[i];
        cumulativeRightGain += reflectionDecayGainsRight[i];
    }

    // Normalize left channel gains if necessary
    if (cumulativeLeftGain > 0.99f)
    {
        float normalizationFactorLeft = 0.99f / cumulativeLeftGain;

        for (int i = 0; i < numActiveEchoes; ++i)
        {
            decayGainsLeft[i] *= normalizationFactorLeft;
        }

        for (size_t i = 0; i < reflectionDecayGainsLeft.size(); ++i)
        {
            reflectionDecayGainsLeft[i] *= normalizationFactorLeft;
        }
    }

    // Normalize right channel gains if necessary
    if (cumulativeRightGain > 0.99f)
    {
        float normalizationFactorRight = 0.99f / cumulativeRightGain;

        for (int i = 0; i < numActiveEchoes; ++i)
        {
            decayGainsRight[i] *= normalizationFactorRight;
        }

        for (size_t i = 0; i < reflectionDecayGainsRight.size(); ++i)
        {
            reflectionDecayGainsRight[i] *= normalizationFactorRight;
        }
    }
}

void DampManager::generateReflectionPattern()
{
    reflectionDelays.clear();
    reflectionGains.clear();

    int preDelaySamples = static_cast<int>((PRE_DELAY_MS / 1000.0f) * sampleRate);

    int maxReflectionDelayMs = 100; // Maximum reflection delay in ms
    int maxDelay = static_cast<int>((maxReflectionDelayMs / 1000.0f) * sampleRate); // Convert to samples

    float totalReflectionGain = 0.0f;

    for (int i = 0; i < MAX_REFLECTIONS; ++i)
    {
        int delay = preDelaySamples + static_cast<int>(maxDelay * (i + 1) / (MAX_REFLECTIONS + 1));

        delay = std::min(delay, echoBuffer.getNumSamples() - 1);

        // Adjust the gain calculation to have higher initial values
        float gain = std::pow(1.5f, i);
        totalReflectionGain += gain;

        reflectionDelays.push_back(delay);
        reflectionGains.push_back(gain);
    }

    // Normalize reflection gains
    if (totalReflectionGain > 0.0f)
    {
        float normalizationFactor = 1.0f / totalReflectionGain;
        for (size_t i = 0; i < reflectionGains.size(); ++i)
        {
            reflectionGains[i] *= normalizationFactor;
        }
    }

    reflectionDecayGainsLeft.resize(reflectionDelays.size(), 1.0f);
    reflectionDecayGainsRight.resize(reflectionDelays.size(), 1.0f);

    // Recalculate positions for reflections
    numActiveReflections = static_cast<int>(reflectionDelays.size());
    for (size_t j = 0; j < reflectionDelays.size(); ++j)
    {
        int i = MAX_ECHOES + static_cast<int>(j);
        stereoManagers[i].calculateAndSetPosition(static_cast<int>(j), numActiveReflections);
    }
}

void DampManager::updateEchoParameters()
{
    smoothedDamp = smoothedDamping.getNextValue();

    numActiveEchoes = static_cast<int>(smoothedDamp * MAX_ECHOES) + 1;
    numActiveEchoes = juce::jlimit(2, MAX_ECHOES, numActiveEchoes);

    // Ensure even number of echoes for symmetry
    if (numActiveEchoes % 2 != 0)
        numActiveEchoes -= 1;

    // Use the class member RNG
    std::uniform_real_distribution<float> delayJitter(-0.02f, 0.02f); // ±20ms jitter

    for (int i = 0; i < numActiveEchoes; ++i)
    {
        // Assign equal base gain
        echoGains[i] = 1.0f / numActiveEchoes;

        // Modify delayFactor with jitter
        float t = (static_cast<float>(i) / static_cast<float>(numActiveEchoes - 1));
        float delayFactor = 0.05f + 0.4f * t + delayJitter(rng);
        delayFactor = juce::jlimit(0.0f, 1.0f, delayFactor);

        echoDelays[i] = static_cast<int>(delayFactor * MAX_ECHO_TIME * sampleRate);
        echoDelays[i] = std::min(echoDelays[i], echoBuffer.getNumSamples() - 1);

        // Calculate and set stereo position for this echo
        stereoManagers[i].calculateAndSetPosition(i, numActiveEchoes);
    }

    // Recalculate decay gains with the new echo gains
    precalculateValues();
}

void DampManager::process(float& sampleLeft, float& sampleRight)
{
    smoothedDamp = smoothedDamping.getNextValue();

    if (smoothedDamp < 0.01f) {
        writePos = (writePos + 1) % echoBuffer.getNumSamples();
        return;
    }

    // Write current samples to the echo buffer
    echoBuffer.setSample(0, writePos, sampleLeft);
    echoBuffer.setSample(1, writePos, sampleRight);

    float outputLeft = 0.0f;
    float outputRight = 0.0f;

    int modulationIndex = (writePos * modulationRateInt) % MODULATION_TABLE_SIZE;
    float modulationFactor = modulationTable[modulationIndex];

    // Process echoes
    for (int i = 0; i < numActiveEchoes; ++i)
    {
        int delay = echoDelays[i];
        int readPos = (writePos - delay + echoBuffer.getNumSamples()) % echoBuffer.getNumSamples();

        float delayedSampleLeft = echoBuffer.getSample(0, readPos) * modulationFactor;
        float delayedSampleRight = echoBuffer.getSample(1, readPos) * modulationFactor;

        // Combine delayed samples to mono
//        float delayedSample = (delayedSampleLeft + delayedSampleRight) * 0.5f;

        // Apply precomputed decay gains
        outputLeft += delayedSampleLeft;
        outputRight += delayedSampleRight;
    }

    // Process reflections
    for (size_t i = 0; i < reflectionDelays.size(); ++i)
    {
        int delay = reflectionDelays[i];
        int readPos = (writePos - delay + echoBuffer.getNumSamples()) % echoBuffer.getNumSamples();

        float delayedSampleLeft = echoBuffer.getSample(0, readPos) * modulationFactor;
        float delayedSampleRight = echoBuffer.getSample(1, readPos) * modulationFactor;

        // Combine delayed samples to mono
        float delayedSample = (delayedSampleLeft + delayedSampleRight) * 0.5f;

        // Apply precomputed decay gains
        outputLeft += delayedSample * reflectionDecayGainsLeft[i];
        outputRight += delayedSample * reflectionDecayGainsRight[i];
    }

    // Apply low-pass filter
    outputLeft = lowpassFilterLeft.processSample(outputLeft);
    outputRight = lowpassFilterRight.processSample(outputRight);

    // Mix the processed signal with the dry signal
    sampleLeft = sampleLeft * (1.0f - smoothedDamp) + outputLeft * smoothedDamp;
    sampleRight = sampleRight * (1.0f - smoothedDamp) + outputRight * smoothedDamp;

    writePos = (writePos + 1) % echoBuffer.getNumSamples();

    if (std::abs(smoothedDamp - lastUpdatedDamp) > 0.01f)
    {
        updateEchoParameters();
        lastUpdatedDamp = smoothedDamp;
    }
}
