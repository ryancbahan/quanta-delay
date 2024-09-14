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
    // No per-sample processing needed
}

void StereoFieldManager::setPosition(float newPosition)
{
    newPosition = juce::jlimit(-1.0f, 1.0f, newPosition);
    currentPosition = newPosition;
    calculateGains();
}

void StereoFieldManager::calculateAndSetPosition(int delayIndex, int totalDelays)
{
    float position = calculateStereoPosition(delayIndex, totalDelays);
    setPosition(position);
}

float StereoFieldManager::calculateStereoPosition(int delayIndex, int totalDelays)
{
    // Reduced randomness
    std::uniform_real_distribution<float> positionJitter(-0.05f, 0.05f);

    if (totalDelays < 1)
        totalDelays = 1;

    // Symmetrical distribution
    float basePosition = -1.0f + 2.0f * (static_cast<float>(delayIndex) + 0.5f) / totalDelays;

    // Add randomness
    float position = basePosition + positionJitter(rng);

    // Ensure position is within bounds
    position = juce::jlimit(-1.0f, 1.0f, position);

    return position;
}

void StereoFieldManager::calculateGains()
{
    // Approximate equal-power panning
    float angle = (currentPosition + 1.0f) * 0.25f * juce::MathConstants<float>::pi; // Map to 0 to π/2
    leftGain = std::cos(angle);
    rightGain = std::sin(angle);
}
