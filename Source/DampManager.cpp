#include "DampManager.h"

DampManager::DampManager()
    : sampleRate(44100.0f), damp(0.0f), writePos(0)
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
}

void DampManager::setDamp(float newDamp)
{
    damp = juce::jlimit(0.0f, 20.0f, newDamp);
    updateEchoParameters();
}

void DampManager::process(float& sample)
{
    if (damp < 0.01f) {  // If damping is very low, don't process
        return;
    }
    
    // Write the input sample to the buffer
    echoBuffer.setSample(0, writePos, sample);

    float echoes = 0.0f;

    // Process echoes
    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (echoDelays[i] == 0) continue;  // Skip inactive echoes

        int readPos = writePos - echoDelays[i];
        if (readPos < 0) readPos += echoBuffer.getNumSamples();
        
        echoes += echoBuffer.getSample(0, readPos) * echoGains[i];
    }
    // Add echoes to the original sample
    sample += echoes;

    writePos = (writePos + 1) % echoBuffer.getNumSamples();
}

void DampManager::updateEchoParameters()
{
    float normalizedDamp = damp / 20.0f; // Normalize to 0-1 range
    int numActiveEchoes = static_cast<int>(normalizedDamp * MAX_ECHOES) + 1;

    for (int i = 0; i < MAX_ECHOES; ++i)
    {
        if (i < numActiveEchoes)
        {
            float t = static_cast<float>(i) / static_cast<float>(numActiveEchoes - 1);
            echoGains[i] = std::pow(1.0f - t, 2) * 0.5f * normalizedDamp;
            echoDelays[i] = static_cast<int>(t * MAX_ECHO_TIME * sampleRate);
            echoDelays[i] = std::min(echoDelays[i], echoBuffer.getNumSamples() - 1);
        }
        else
        {
            echoGains[i] = 0.0f;
            echoDelays[i] = 0;
        }
    }
}
