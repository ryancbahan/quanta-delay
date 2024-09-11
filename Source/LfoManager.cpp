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
    lfo.setFrequency(rateHz);
}

void LFOManager::setDepth(float depthMs)
{
    depth = depthMs / 1000.0f; // Convert ms to seconds
}

float LFOManager::getNextSample()
{
    return lfo.processSample(0.0f) * depth;
}
