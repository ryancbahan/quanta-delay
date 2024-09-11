#include "StereoFieldManager.h"
#include <cmath>

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

void StereoFieldManager::calculateAndSetPosition(int delayIndex, int totalDelays)
{
    float position = calculateStereoPosition(delayIndex, totalDelays);
    setPosition(position);
}

float StereoFieldManager::calculateStereoPosition(int delayIndex, int totalDelays)
{
    // This formula creates a deterministic but varied distribution
    // across the stereo field (-1.0 to 1.0)
    float position = std::sin(delayIndex * 2.39996323f) * 2.0f - 1.0f;
    
    // Ensure the first delay is centered and the last two are hard left/right
    if (delayIndex == 0)
        position = 0.0f;
    else if (delayIndex == totalDelays - 1)
        position = -1.0f;
    else if (delayIndex == totalDelays - 2)
        position = 1.0f;
    
    return position;
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
