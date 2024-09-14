#include "StereoFieldManager.h"
#include <cmath>

StereoFieldManager::StereoFieldManager()
    : sampleRate(44100.0f)
{
    // Seed RNG with a unique value
    std::random_device rd;
    rng.seed(rd());
}

void StereoFieldManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    reset();
}

void StereoFieldManager::reset()
{
    smoothedPosition.reset(sampleRate, 0.05f); // 50ms smoothing time
    smoothedPosition.setCurrentAndTargetValue(currentPosition);
}

void StereoFieldManager::setPosition(float newPosition)
{
    newPosition = juce::jlimit(-1.0f, 1.0f, newPosition);
    currentPosition = newPosition;
    smoothedPosition.setTargetValue(newPosition);
}

void StereoFieldManager::calculateAndSetPosition(int delayIndex, int totalDelays)
{
    float position = calculateStereoPosition(delayIndex, totalDelays);
    setPosition(position);
}

float StereoFieldManager::calculateStereoPosition(int delayIndex, int totalDelays)
{
    std::uniform_real_distribution<float> positionJitter(-0.1f, 0.1f); // ±0.1 randomness

    // Avoid division by zero
    float basePosition = (totalDelays > 1) ? (-1.0f + 2.0f * (static_cast<float>(delayIndex) / (totalDelays - 1))) : 0.0f;

    // Add randomness
    float position = basePosition + positionJitter(rng);

    // Ensure the position is within -1.0 to 1.0
    position = juce::jlimit(-1.0f, 1.0f, position);

    return position;
}

void StereoFieldManager::process(float& leftSample, float& rightSample)
{
    float position = smoothedPosition.getNextValue();

    // Map position (-1 to 1) to angle (-π/4 to π/4)
    float angle = position * (juce::MathConstants<float>::pi / 4.0f);

    // Compute gains using sine and cosine for equal power panning
    float leftGain = std::cos(angle);
    float rightGain = std::sin(angle);

    // Apply the gains
    leftSample *= leftGain;
    rightSample *= rightGain;
}
