#include "LFOManager.h"

LFOManager::LFOManager()
    : depth(0.0f)
    , sampleRate(44100.0f)
    , rate(1.0f)
    , x(0.5)
    , r(3.9)
    , sampleCounter(0)
    , samplesPerCycle(44100)
    , seed(12345)
    , randomState(seed)
{
    resetToInitialState();
}

void LFOManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    resetToInitialState();
}

void LFOManager::reset()
{
    resetToInitialState();
}

void LFOManager::resetToInitialState()
{
    x = 0.5;
    r = 3.9;
    sampleCounter = 0;
    randomState = seed;
    updateLogisticMapParameter();
}

void LFOManager::setRate(float rateHz)
{
    rate = juce::jmax(0.01f, rateHz);
    samplesPerCycle = static_cast<int>(sampleRate / rate);
}

void LFOManager::setDepth(float depthMs)
{
    depth = depthMs / 1000.0f;
}

float LFOManager::getNextSample()
{
    if (sampleCounter >= samplesPerCycle)
    {
        sampleCounter = 0;
        updateLogisticMapParameter();
    }
    
    x = r * x * (1.0f - x);
    sampleCounter++;
    
    // Map x from [0, 1] to [-1, 1] and apply depth
    return (2.0f * x - 1.0f) * depth;
}

void LFOManager::calculateAndSetRate(float normalizedPosition)
{
    float newRate = 0.2f + normalizedPosition * 9.8f;
    setRate(juce::jmax(0.2f, newRate));
}

void LFOManager::updateLogisticMapParameter()
{
    r = 3.7f + (generateRandomFloat() * 0.3f);
}

float LFOManager::generateRandomFloat()
{
    randomState = randomState * 1664525 + 1013904223; // Linear congruential generator
    return (randomState >> 8) / static_cast<float>(0xFFFFFF); // Return a value between 0 and 1
}

void LFOManager::setSeed(uint32_t newSeed)
{
    seed = newSeed;
    resetToInitialState();
}
