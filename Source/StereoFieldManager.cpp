#include "StereoFieldManager.h"

StereoFieldManager::StereoFieldManager()
    : sampleRate(44100.0f)
{
}

void StereoFieldManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
}

void StereoFieldManager::reset()
{
    smoothedPosition.reset(sampleRate, 0.05f); // 50ms smoothing time
    smoothedPosition.setCurrentAndTargetValue(0.0f); // Center position
}

void StereoFieldManager::setPosition(float newPosition)
{
    // Ensure the position is between -1 (full left) and 1 (full right)
    newPosition = juce::jlimit(-1.0f, 1.0f, newPosition);
    smoothedPosition.setTargetValue(newPosition);
}

void StereoFieldManager::process(float& leftSample, float& rightSample)
{
    float position = smoothedPosition.getNextValue();
    
    // Calculate gains for left and right channels
    float leftGain = 0.5f * (1.0f - position);
    float rightGain = 0.5f * (1.0f + position);

    // Apply the gains
    leftSample *= leftGain;
    rightSample *= rightGain;
}
