#include "LFOManager.h"

const std::array<float, LFOManager::NUM_PRESET_FREQUENCIES> LFOManager::presetFrequencies = {
    0.1f, 3.0f, 0.2f, 4.0f, 0.3f, 5.0f, 0.4f, 6.0f, 0.5f, 7.0f,
    0.6f, 8.0f, 0.7f, 9.0f, 0.8f, 10.0f, 0.9f, 11.0f, 1.0f, 12.0f
};

LFOManager::LFOManager()
    : depth(0.0f)
    , sampleRate(44100.0f)
    , phase(0.0f)
    , frequency(1.0f)
    , lastSample(0.0f)
{
}

void LFOManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
}

void LFOManager::reset()
{
    phase = 0.0f;
    lastSample = 0.0f;
}

void LFOManager::setRate(float rateHz)
{
    frequency = juce::jmax(0.01f, rateHz); // Ensure rate is always positive
}

void LFOManager::setDepth(float depthMs)
{
    depth = depthMs / 1000.0f; // Convert ms to seconds
}

float LFOManager::getNextSample()
{
    float sineComponent = std::sin(phase);
    float triangleComponent = 1.0f - std::abs(phase / juce::MathConstants<float>::pi - 1.0f);
    
    // Mix sine and triangle for asymmetry
    float asymmetricalWave = 0.9f * sineComponent + 0.1f * triangleComponent;
    
    // Apply low-pass filter for "lag" (simple one-pole filter)
    asymmetricalWave = 0.99f * lastSample + 0.01f * asymmetricalWave;
    lastSample = asymmetricalWave;

    phase += 2.0f * juce::MathConstants<float>::pi * frequency / sampleRate;
    if (phase >= 2.0f * juce::MathConstants<float>::pi)
        phase -= 2.0f * juce::MathConstants<float>::pi;

    // Scale and offset the wave to the 0-1 range, then apply depth
    return ((asymmetricalWave * 0.5f + 0.5f) * depth);
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
