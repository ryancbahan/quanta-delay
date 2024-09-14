#include "StereoFieldManager.h"
#include <cmath>
#include <random>

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
    static std::mt19937 rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<float> positionJitter(-0.1f, 0.1f); // ±0.1 randomness

    // Distribute positions evenly
    float basePosition = -1.0f + 2.0f * (static_cast<float>(delayIndex) / (totalDelays - 1));

    // Add randomness
    float position = basePosition + positionJitter(rng);

    // Ensure the position is within -1.0 to 1.0
    position = juce::jlimit(-1.0f, 1.0f, position);

    return position;
}



#include <JuceHeader.h> // Ensure you include JUCE header for MathConstants

void StereoFieldManager::process(float& leftSample, float& rightSample)
{
    float position = smoothedPosition.getNextValue();

    // Map position (-1 to 1) to angle (0 to π/2)
    float angle = (position + 1.0f) * (juce::MathConstants<float>::halfPi * 0.5f);

    // Compute gains using sine and cosine for constant power panning
    float leftGain = std::cos(angle);
    float rightGain = std::sin(angle);

    // Apply the gains
    leftSample *= leftGain;
    rightSample *= rightGain;
}
