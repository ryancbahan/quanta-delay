#include "LFOManager.h"

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

void LFOManager::calculateAndSetRate(float normalizedPosition)
{
    float rate = mapToFrequencyRange(normalizedPosition);
    setRate(rate);
}

float LFOManager::mapToFrequencyRange(float input) const
{
    // Map input from 0-1 to 0.2-10 Hz (1/5 Hz to 10 Hz)
    return juce::jmax(0.2f, 0.2f + input * 9.8f);
}
