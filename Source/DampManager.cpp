#include "DampManager.h"
#include <random>

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0), smoothedDamp(0.0f), lastUpdatedDamp(0.0f),
      smoothedDamping(0.001f), roomSize(1.0f), reflectionGain(0.7f),
      decayTime(1.5f), modulationRate(0.5f), modulationDepth(0.1f), modulationPhase(0.0f),
      initialCutoff(20000.0f), cutoffDecayRate(0.5f),
      numActiveEchoes(0), numActiveReflections(0)
{
    // Initialize the echo buffer size
    echoBuffer.setSize(2, static_cast<int>(MAX_ECHO_TIME * sampleRate) + 1);
    echoDelays.fill(0);
    echoGains.fill(0.0f);
    decayGains.fill(1.0f);

    // Seed RNG with a unique value
    std::random_device rd;
    rng.seed(rd());
}

void DampManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
    generateReflectionPattern(); // Generate reflections first
    updateEchoParameters();      // Update echoes and positions
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

    // Recalculate decay gains for echoes
    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (echoDelays[i] > 0)
        {
            float time = echoDelays[i] / sampleRate;
            decayGains[i] = echoGains[i] * std::exp(-1.0f * time / decayTime);
        }
        else
        {
            decayGains[i] = 0.0f;
        }
    }

    // Ensure reflectionDecayGains has the correct size
    reflectionDecayGains.resize(reflectionDelays.size());

    // Recalculate decay gains for reflections
    for (size_t i = 0; i < reflectionDelays.size(); ++i)
    {
        float time = reflectionDelays[i] / sampleRate;
        reflectionDecayGains[i] = reflectionGains[i] * std::exp(-2.0f * time / decayTime);
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

    // Normalize reflection gains to a higher total gain
    float desiredTotalReflectionGain = 2.0f; // Increase to make reflections more pronounced
    if (totalReflectionGain > 0.0f)
    {
        for (size_t i = 0; i < reflectionGains.size(); ++i)
        {
            reflectionGains[i] *= (desiredTotalReflectionGain / totalReflectionGain);
        }
    }

    reflectionDecayGains.resize(reflectionDelays.size(), 1.0f);

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
    numActiveEchoes = static_cast<int>(smoothedDamp * MAX_ECHOES) + 1;
    numActiveEchoes = juce::jlimit(1, MAX_ECHOES, numActiveEchoes); // Ensure at least one echo
    float totalEchoGain = 0.0f;

    // Use the class member RNG
    std::uniform_real_distribution<float> delayJitter(-0.02f, 0.02f); // ±20ms jitter
    std::uniform_real_distribution<float> gainJitter(0.9f, 1.1f);     // ±10% gain variation

    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (i < numActiveEchoes)
        {
            // Avoid division by zero
            float t = (numActiveEchoes > 1) ? (static_cast<float>(i) / static_cast<float>(numActiveEchoes - 1)) : 0.0f;
            echoGains[i] = std::pow(1.0f - t, 1.5f);

            // Apply gain jitter
            echoGains[i] *= gainJitter(rng);

            totalEchoGain += echoGains[i];

            // Modify delayFactor with jitter
            float delayFactor = 0.05f + 0.4f * t + delayJitter(rng);
            delayFactor = juce::jlimit(0.0f, 1.0f, delayFactor);

            echoDelays[i] = static_cast<int>(delayFactor * MAX_ECHO_TIME * sampleRate);
            echoDelays[i] = std::min(echoDelays[i], echoBuffer.getNumSamples() - 1);

            // Calculate and set stereo position for this echo
            stereoManagers[i].calculateAndSetPosition(i, numActiveEchoes);
        }
        else
        {
            echoGains[i] = 0.0f;
            echoDelays[i] = 0;
            decayGains[i] = 0.0f;
        }
    }

    // Normalize echo gains
    if (totalEchoGain > 0.0f)
    {
        for (int i = 0; i < numActiveEchoes; ++i)
        {
            echoGains[i] *= (0.7f / totalEchoGain);
        }
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

    echoBuffer.setSample(0, writePos, sampleLeft);
    echoBuffer.setSample(1, writePos, sampleRight);

    float outputLeft = 0.0f;
    float outputRight = 0.0f;

    int modulationIndex = (writePos * modulationRateInt) % MODULATION_TABLE_SIZE;
    float modulationFactor = modulationTable[modulationIndex];

    int totalDelays = numActiveEchoes + numActiveReflections;

    // Process echoes and reflections
    for (int i = 0; i < totalDelays; ++i)
    {
        int delay;
        float gain;

        if (i < numActiveEchoes) {
            delay = echoDelays[i];
            gain = decayGains[i];
        } else {
            int j = i - numActiveEchoes;
            if (j >= numActiveReflections) break;
            delay = reflectionDelays[j];
            gain = reflectionDecayGains[j];
        }

        if (gain < 0.0001f) continue;

        int readPos = (writePos - delay + echoBuffer.getNumSamples()) % echoBuffer.getNumSamples();

        float delayedSampleLeft = echoBuffer.getSample(0, readPos);
        float delayedSampleRight = echoBuffer.getSample(1, readPos);

        delayedSampleLeft *= modulationFactor;
        delayedSampleRight *= modulationFactor;

        // Apply stereo processing to the individual echo/reflection
        float leftGain = stereoManagers[i].getLeftGain();
        float rightGain = stereoManagers[i].getRightGain();

        delayedSampleLeft *= leftGain;
        delayedSampleRight *= rightGain;

        outputLeft += delayedSampleLeft * gain;
        outputRight += delayedSampleRight * gain;
    }

    outputLeft = lowpassFilterLeft.processSample(outputLeft);
    outputRight = lowpassFilterRight.processSample(outputRight);

    sampleLeft = sampleLeft * (1.0f - smoothedDamp) + outputLeft * smoothedDamp;
    sampleRight = sampleRight * (1.0f - smoothedDamp) + outputRight * smoothedDamp;

    writePos = (writePos + 1) % echoBuffer.getNumSamples();

    if (std::abs(smoothedDamp - lastUpdatedDamp) > 0.01f)
    {
        updateEchoParameters();
        lastUpdatedDamp = smoothedDamp;
    }
}
