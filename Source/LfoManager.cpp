#include "LFOManager.h"

const std::array<float, LFOManager::NUM_PRESET_FREQUENCIES> LFOManager::presetFrequencies = {
    0.1f, 3.0f, 0.2f, 4.0f, 0.3f, 5.0f, 0.4f, 6.0f, 0.5f, 7.0f,
    0.6f, 8.0f, 0.7f, 9.0f, 0.8f, 10.0f, 0.9f, 11.0f, 1.0f, 12.0f
};

LFOManager::LFOManager()
    : depth(0.0f)
    , sampleRate(44100.0f)
{
    lfo.initialise([](float x) { return std::sin(x); });
}

void LFOManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    lfo.prepare(spec);
}

void LFOManager::reset()
{
    lfo.reset();
}

void LFOManager::setRate(float rateHz)
{
    lfo.setFrequency(juce::jmax(0.01f, rateHz)); // Ensure rate is always positive
}

void LFOManager::setDepth(float depthMs)
{
    depth = depthMs / 1000.0f; // Convert ms to seconds
}

float LFOManager::getNextSample()
{
    return (lfo.processSample(0.0f) * 0.5f + 0.5f) * depth;
}

void LFOManager::calculateAndSetRate(int index)
{
    if (index >= 0 && index < NUM_PRESET_FREQUENCIES)
    {
        setRate(presetFrequencies[index]);
    }
    else
    {
        // Fallback to a default rate if the index is out of bounds
        setRate(1.0f);
    }
}

float LFOManager::mapToFrequencyRange(float input) const
{
    // Map input from 0-1 to 0.1-12 Hz (matching our preset range)
    return juce::jmap(input, 0.1f, 12.0f);
}
