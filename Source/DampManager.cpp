#include "DampManager.h"

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0), smoothedDamp(0.0f), lastUpdatedDamp(0.0f),
      smoothedDamping(0.001f), roomSize(1.0f), reflectionGain(0.7f),
      decayTime(1.5f), modulationRate(0.5f), modulationDepth(0.1f), modulationPhase(0.0f),
      initialCutoff(20000.0f), cutoffDecayRate(0.5f)
{
    echoBuffer.setSize(2, static_cast<int>(MAX_ECHO_TIME * sampleRate) + 1);
    echoDelays.fill(0);
    echoGains.fill(0.0f);
    decayGains.fill(1.0f);

    // reflectionDecayGains is a vector; use resize instead of fill
    reflectionDecayGains.clear();
}

void DampManager::prepare(const juce::dsp::ProcessSpec& spec)
{

    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
    updateEchoParameters();
    generateReflectionPattern();
    precalculateValues();
    
    for (int i = 0; i < MAX_ECHOES + MAX_REFLECTIONS; ++i) {
        stereoManagers[i].prepare(spec);
    }

    // Set the initial cutoff frequency higher if needed
    initialCutoff = 15000.0f; // Increase cutoff to 15kHz
    lowpassCoeffsLeft = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, initialCutoff);
    *lowpassFilterLeft.coefficients = *lowpassCoeffsLeft;

    lowpassCoeffsRight = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, initialCutoff);
    *lowpassFilterRight.coefficients = *lowpassCoeffsRight;
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
    for (int i = 0; i < MODULATION_TABLE_SIZE; ++i)
    {
        float phase = 2.0f * juce::MathConstants<float>::pi * i / MODULATION_TABLE_SIZE;
        modulationTable[i] = 1.0f + modulationDepth * std::sin(phase);
    }

    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (echoDelays[i] > 0)
        {
            float time = echoDelays[i] / sampleRate;
            decayGains[i] = echoGains[i] * std::exp(-2.0f * time / decayTime); // Reduced decay rate
        }
        else
        {
            decayGains[i] = 0.0f;
        }
    }

    // Ensure reflectionDecayGains has the correct size
    reflectionDecayGains.resize(reflectionDelays.size());

    for (size_t i = 0; i < reflectionDelays.size(); ++i)
    {
        float time = reflectionDelays[i] / sampleRate;
        reflectionDecayGains[i] = reflectionGains[i] * std::exp(-2.0f * time / decayTime); // Reduced decay rate
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

        // Ensure delay does not exceed buffer size
        delay = std::min(delay, echoBuffer.getNumSamples() - 1);

        float gain = std::pow(1.5f, i); // Decreasing gain for each reflection
        totalReflectionGain += gain;

        reflectionDelays.push_back(delay);
        reflectionGains.push_back(gain);
    }

    // Normalize reflection gains
    if (totalReflectionGain > 0.0f)
    {
        for (size_t i = 0; i < reflectionGains.size(); ++i)
        {
            reflectionGains[i] *= (1.0f / totalReflectionGain); // Adjust the total gain to 1.0f
        }
    }

    // Resize reflectionDecayGains to match the new size
    reflectionDecayGains.resize(reflectionDelays.size(), 1.0f);
}




void DampManager::process(float& sampleLeft, float& sampleRight)
{
    smoothedDamp = smoothedDamping.getNextValue();

    if (smoothedDamp < 0.01f) {
        return;
    }

    echoBuffer.setSample(0, writePos, sampleLeft);
    echoBuffer.setSample(1, writePos, sampleRight);

    float outputLeft = 0.0f;
    float outputRight = 0.0f;
    float currentTime = static_cast<float>(writePos) / sampleRate;
    int modulationIndex = static_cast<int>(currentTime * modulationRate * MODULATION_TABLE_SIZE) % MODULATION_TABLE_SIZE;

    // Process echoes and reflections
    for (int i = 0; i < MAX_ECHOES + MAX_REFLECTIONS; ++i)
    {
        int delay, readPos;
        float gain;

        if (i < MAX_ECHOES) {
            if (echoDelays[i] == 0) continue;
            delay = echoDelays[i];
            gain = decayGains[i];
        } else {
            int j = i - MAX_ECHOES;
            if (j >= reflectionDelays.size()) break;
            delay = reflectionDelays[j];
            gain = reflectionDecayGains[j];
        }

        readPos = (writePos - delay + echoBuffer.getNumSamples()) % echoBuffer.getNumSamples();

        // Ensure readPos is within bounds
        if (readPos < 0 || readPos >= echoBuffer.getNumSamples())
            continue;

        float delayedSampleLeft = echoBuffer.getSample(0, readPos);
        float delayedSampleRight = echoBuffer.getSample(1, readPos);

        int sampleModIndex = (modulationIndex + delay) % MODULATION_TABLE_SIZE;
        delayedSampleLeft *= modulationTable[sampleModIndex];
        delayedSampleRight *= modulationTable[sampleModIndex];
        
        stereoManagers[i].calculateAndSetPosition(i, MAX_ECHOES + MAX_REFLECTIONS);
        stereoManagers[i].process(delayedSampleLeft, delayedSampleRight);

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

void DampManager::updateEchoParameters()
{
    int numActiveEchoes = static_cast<int>(smoothedDamp * MAX_ECHOES) + 1;
    float totalEchoGain = 0.0f;

    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (i < numActiveEchoes)
        {
            float t = static_cast<float>(i) / static_cast<float>(numActiveEchoes - 1);
            echoGains[i] = std::pow(1.0f - t, 1.5f);
            totalEchoGain += echoGains[i];

            float delayFactor = 0.1f + 0.9f * std::pow(t, 0.8f);
            echoDelays[i] = static_cast<int>(delayFactor * MAX_ECHO_TIME * sampleRate);
            echoDelays[i] = std::min(echoDelays[i], echoBuffer.getNumSamples() - 1);
        }
        else
        {
            echoGains[i] = 0.0f;
            echoDelays[i] = 0;
        }
    }

    // Normalize echo gains
    if (totalEchoGain > 0.0f)
    {
        for (int i = 0; i < numActiveEchoes; ++i)
        {
            echoGains[i] *= (0.7f / totalEchoGain); // Adjust the total gain to 0.7f
        }
    }

    // Recalculate decay gains with the new echo gains
    precalculateValues();
}

