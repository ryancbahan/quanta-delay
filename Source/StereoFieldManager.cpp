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

    // Generate panning tables
    for (int i = 0; i < PANNING_TABLE_SIZE; ++i)
    {
        float position = -1.0f + 2.0f * i / (PANNING_TABLE_SIZE - 1);
        float angle = (position + 1.0f) * 0.25f * juce::MathConstants<float>::pi; // Map position to angle between 0 and pi/2

        // Equal-power panning gains
        leftPanningTable[i] = std::cos(angle);
        rightPanningTable[i] = std::sin(angle);

        // Normalize gains so that leftGain^2 + rightGain^2 = 1
        float normalizationFactor = 1.0f / std::sqrt(leftPanningTable[i] * leftPanningTable[i] + rightPanningTable[i] * rightPanningTable[i]);
        leftPanningTable[i] *= normalizationFactor;
        rightPanningTable[i] *= normalizationFactor;
    }
}

void StereoFieldManager::reset()
{
    // No per-sample processing needed
}

void StereoFieldManager::setPosition(float newPosition)
{
    currentPosition = juce::jlimit(-1.0f, 1.0f, newPosition);
    calculateGains();
}

void StereoFieldManager::calculateAndSetPosition(int delayIndex, int totalDelays)
{
    float position = calculateStereoPosition(delayIndex, totalDelays);
    setPosition(position);
}

float StereoFieldManager::calculateStereoPosition(int delayIndex, int totalDelays)
{
    if (totalDelays < 1)
        totalDelays = 1;

    float centerIndex = (totalDelays - 1) * 0.5f;
    float position = (delayIndex - centerIndex) / centerIndex; // Normalize to -1.0 to 1.0

    // Reduce randomness
    std::uniform_real_distribution<float> positionJitter(-0.02f, 0.02f);
    float randomValue = positionJitter(rng);

    position += randomValue;

    return juce::jlimit(-1.0f, 1.0f, position);
}


void StereoFieldManager::calculateGains()
{
    // Map currentPosition to table index
    int index = static_cast<int>(((currentPosition + 1.0f) * 0.5f) * (PANNING_TABLE_SIZE - 1));
    index = juce::jlimit(0, PANNING_TABLE_SIZE - 1, index);

    leftGain = leftPanningTable[index];
    rightGain = rightPanningTable[index];
}
