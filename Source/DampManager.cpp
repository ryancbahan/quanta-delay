#include "DampManager.h"

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0), smoothedDamp(0.0f), lastUpdatedDamp(0.0f)
{
    echoBuffer.setSize(1, static_cast<int>(MAX_ECHO_TIME * sampleRate) + 1);
}

void DampManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
    updateEchoParameters();
}

void DampManager::reset()
{
    echoBuffer.clear();
    writePos = 0;
    smoothedDamp = damp;
    lastUpdatedDamp = damp;
}

void DampManager::setDamp(float newDamp)
{
    damp = juce::jlimit(0.0f, 20.0f, newDamp);
}

void DampManager::process(float& sample)
{
    // Smooth damp parameter
    smoothedDamp += (damp - smoothedDamp) * 0.001f;

    if (smoothedDamp < 0.01f) {  // If damping is very low, don't process
        return;
    }
    
    // Write the input sample to the buffer
    echoBuffer.setSample(0, writePos, sample);

    float echoes = 0.0f;

    // Process echoes
    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (echoDelays[i] == 0) break;  // Exit loop if we reach inactive echoes

        int readPos = writePos - echoDelays[i];
        if (readPos < 0) readPos += echoBuffer.getNumSamples();
        
        float echoSample = echoBuffer.getSample(0, readPos);
        // Apply a simple low-pass filter to each echo
        echoSample = echoSample * 0.7f + lastSample * 0.3f;
        lastSample = echoSample;

        echoes += echoSample * echoGains[i];
    }

    // Add echoes to the original sample with increased intensity
    float dampIntensity = smoothedDamp / 10.0f;  // Increased from 20.0f to 10.0f
    sample = sample * (1.0f - dampIntensity * 0.5f) + echoes * dampIntensity;

    writePos = (writePos + 1) % echoBuffer.getNumSamples();

    // Update echo parameters if damp has changed significantly
    if (std::abs(smoothedDamp - lastUpdatedDamp) > 0.01f)
    {
        updateEchoParameters();
        lastUpdatedDamp = smoothedDamp;
    }
}

void DampManager::updateEchoParameters()
{
    float normalizedDamp = smoothedDamp / 20.0f; // Normalize to 0-1 range
    int numActiveEchoes = static_cast<int>(normalizedDamp * MAX_ECHOES) + 1;

    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (i < numActiveEchoes)
        {
            float t = static_cast<float>(i) / static_cast<float>(numActiveEchoes - 1);
            echoGains[i] = std::pow(1.0f - t, 1.5f) * 0.7f;  // Increased from 0.5f to 0.7f
            float delayFactor = 0.1f + 0.9f * std::pow(t, 0.8f);  // Non-linear delay distribution
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
