#include "PitchShifterManager.h"

PitchShifterManager::PitchShifterManager()
    : shiftFactor(2.0f)  // Default to octave up
    , readPos(0.0f)
    , sampleRate(44100.0)
{
}

void PitchShifterManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(MAX_DELAY_SAMPLES);
}

void PitchShifterManager::reset()
{
    delayLine.reset();
    readPos = 0.0f;
}

void PitchShifterManager::setShiftFactor(float newShiftFactor)
{
    shiftFactor = std::max(0.01f, std::abs(newShiftFactor));
}

void PitchShifterManager::process(float& leftSample, float& rightSample)
{
    leftSample = processSample(leftSample);
    rightSample = processSample(rightSample);
}

float PitchShifterManager::processSample(float inputSample)
{
    delayLine.pushSample(0, inputSample);
    
    float readIncrement = 1.0f / shiftFactor;
    
    float pitchShiftedSample = 0.0f;
    int numSamplesToRead = static_cast<int>(std::ceil(shiftFactor));
    
    for (int i = 0; i < numSamplesToRead; ++i)
    {
        float fractionalDelay = readPos + i * readIncrement;
        fractionalDelay = std::fmod(fractionalDelay, static_cast<float>(MAX_DELAY_SAMPLES));
        
        float sample = delayLine.popSample(0, fractionalDelay);
        
        float windowWeight = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / numSamplesToRead));
        pitchShiftedSample += sample * windowWeight;
    }
    
    pitchShiftedSample /= numSamplesToRead;  // Normalize
    
    readPos += 1.0f;
    if (readPos >= shiftFactor)
        readPos -= shiftFactor;
    
    return pitchShiftedSample;
}
