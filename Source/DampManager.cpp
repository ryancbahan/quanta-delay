#include "DampManager.h"

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0), smoothedDamp(0.0f), lastUpdatedDamp(0.0f),
      smoothedDamping(0.001f), lastSample(0.0f), roomSize(1.0f), reflectionGain(0.7f)
{
    echoBuffer.setSize(1, static_cast<int>(MAX_ECHO_TIME * sampleRate) + 1);
}

float DampManager::cubicInterpolation(float y0, float y1, float y2, float y3, float t)
{
    float a0 = y3 - y2 - y0 + y1;
    float a1 = y0 - y1 - a0;
    float a2 = y2 - y0;
    float a3 = y1;

    return (a0 * t * t * t) + (a1 * t * t) + (a2 * t) + a3;
}

void DampManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
    updateEchoParameters();
    generateReflectionPattern();
}

void DampManager::reset()
{
    echoBuffer.clear();
    writePos = 0;
    smoothedDamp = damp;
    lastUpdatedDamp = damp;
    smoothedDamping.reset(sampleRate, 0.1);
    lastSample = 0.0f;
}

void DampManager::setDamp(float newDamp)
{
    damp = juce::jlimit(0.0f, 20.0f, newDamp);
    smoothedDamping.setTargetValue(damp);
}

void DampManager::generateReflectionPattern()
{
    reflectionDelays.clear();
    reflectionGains.clear();

    int maxDelay = static_cast<int>(roomSize * sampleRate);

    for (int i = 0; i < MAX_REFLECTIONS; ++i)
    {
        int preDelaySamples = static_cast<int>((PRE_DELAY_MS / 1000.0f) * sampleRate);
        int delay = preDelaySamples + static_cast<int>(maxDelay * (i + 1) / MAX_REFLECTIONS);
        float gain = reflectionGain * std::pow(0.9f, i);

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

    // Process damping effect
    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (echoDelays[i] == 0) break;

        int readPos = writePos - echoDelays[i];
        if (readPos < 0) readPos += echoBuffer.getNumSamples();

        float delayedSample = echoBuffer.getSample(0, readPos);
        output += delayedSample * echoGains[i];
    }

    // Add reflections
    for (size_t i = 0; i < reflectionDelays.size(); ++i)
    {
        int readPos = writePos - reflectionDelays[i];
        if (readPos < 0) readPos += echoBuffer.getNumSamples();

        float delayedSample = echoBuffer.getSample(0, readPos);
        output += delayedSample * reflectionGains[i];
    }

    float dampIntensity = smoothedDamp / 10.0f;
    sample = sample * (1.0f - dampIntensity) + output * dampIntensity;

    writePos = (writePos + 1) % echoBuffer.getNumSamples();

    if (std::abs(smoothedDamp - lastUpdatedDamp) > 0.01f)
    {
        updateEchoParameters();
        lastUpdatedDamp = smoothedDamp;
    }
}

void DampManager::updateEchoParameters()
{
    float normalizedDamp = smoothedDamp / 20.0f;
    int numActiveEchoes = static_cast<int>(normalizedDamp * MAX_ECHOES) + 1;

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
}
