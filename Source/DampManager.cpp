#include "DampManager.h"

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0), smoothedDamp(0.0f), lastUpdatedDamp(0.0f),
      smoothedDamping(0.001f), roomSize(1.0f), reflectionGain(0.7f),
      decayTime(1.5f), modulationRate(0.5f), modulationDepth(0.1f), modulationPhase(0.0f),
      initialCutoff(20000.0f), cutoffDecayRate(0.5f)
{
    echoBuffer.setSize(1, static_cast<int>(MAX_ECHO_TIME * sampleRate) + 1);
    echoDelays.fill(0);
    echoGains.fill(0.0f);
    decayGains.fill(1.0f);
    reflectionDecayGains.fill(1.0f);
}

void DampManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
    updateEchoParameters();
    generateReflectionPattern();
    precalculateValues();

    lowpassCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, initialCutoff);
    *lowpassFilter.coefficients = *lowpassCoeffs;
}

void DampManager::reset()
{
    echoBuffer.clear();
    writePos = 0;
    smoothedDamp = damp;
    lastUpdatedDamp = damp;
    smoothedDamping.reset(sampleRate, 0.1);
    modulationPhase = 0.0f;
    lowpassFilter.reset();
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
            decayGains[i] = echoGains[i] * std::exp(-3.0f * time / decayTime);
        }
        else
        {
            decayGains[i] = 0.0f;
        }
    }

    for (size_t i = 0; i < reflectionDelays.size(); ++i)
    {
        float time = reflectionDelays[i] / sampleRate;
        reflectionDecayGains[i] = reflectionGains[i] * std::exp(-3.0f * time / decayTime);
    }
}

void DampManager::generateReflectionPattern()
{
    reflectionDelays.clear();
    reflectionGains.clear();

    int maxDelay = static_cast<int>(roomSize * sampleRate);
    int preDelaySamples = static_cast<int>((PRE_DELAY_MS / 1000.0f) * sampleRate);

    for (int i = 0; i < MAX_REFLECTIONS; ++i)
    {
        int delay = preDelaySamples + static_cast<int>(maxDelay * (i + 1) / MAX_REFLECTIONS);
        float gain = reflectionGain;
        reflectionDelays.push_back(delay);
        reflectionGains.push_back(gain);
    }
}

void DampManager::process(float& sample)
{
    smoothedDamp = smoothedDamping.getNextValue();
    
    if (smoothedDamp < 0.01f) {
        return;
    }
    
    echoBuffer.setSample(0, writePos, sample);

    float output = 0.0f;
    float currentTime = static_cast<float>(writePos) / sampleRate;
    int modulationIndex = static_cast<int>(currentTime * modulationRate * MODULATION_TABLE_SIZE) % MODULATION_TABLE_SIZE;

    // Process echoes and reflections
    for (int i = 0; i < MAX_ECHOES + MAX_REFLECTIONS; ++i)
    {
        int delay, readPos;
        float gain;

        if (i < MAX_ECHOES) {
            if (echoDelays[i] == 0) break;
            delay = echoDelays[i];
            gain = decayGains[i];
        } else {
            int j = i - MAX_ECHOES;
            if (j >= reflectionDelays.size()) break;
            delay = reflectionDelays[j];
            gain = reflectionDecayGains[j];
        }

        readPos = (writePos - delay + echoBuffer.getNumSamples()) % echoBuffer.getNumSamples();
        float delayedSample = echoBuffer.getSample(0, readPos);
        
        int sampleModIndex = (modulationIndex + delay) % MODULATION_TABLE_SIZE;
        delayedSample *= modulationTable[sampleModIndex];
        
        output += delayedSample * gain;
    }

    output = lowpassFilter.processSample(output);
    sample = sample * (1.0f - smoothedDamp) + output * smoothedDamp;

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

    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (i < numActiveEchoes)
        {
            float t = static_cast<float>(i) / static_cast<float>(numActiveEchoes - 1);
            echoGains[i] = std::pow(1.0f - t, 1.5f) * 0.7f;
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

    precalculateValues();
}
