#pragma once

#include <JuceHeader.h>
#include <array>
#include <random>

class StereoFieldManager
{
public:
    StereoFieldManager();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setPosition(float newPosition);
    void calculateAndSetPosition(int delayIndex, int totalDelays);

    float getLeftGain() const { return leftGain; }
    float getRightGain() const { return rightGain; }
    float getCurrentPosition() const { return currentPosition; }

private:
    float calculateStereoPosition(int delayIndex, int totalDelays);
    void calculateGains();

    static constexpr int PANNING_TABLE_SIZE = 256;
    std::array<float, PANNING_TABLE_SIZE> leftPanningTable;
    std::array<float, PANNING_TABLE_SIZE> rightPanningTable;

    float sampleRate;
    float currentPosition = 0.0f;
    float leftGain = 0.7071f;  // Default to center position
    float rightGain = 0.7071f;

    std::mt19937 rng; // Random number generator
};
