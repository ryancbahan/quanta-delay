#pragma once

#include <JuceHeader.h>
#include <random>

class StereoFieldManager
{
public:
    StereoFieldManager();
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setPosition(float newPosition);
    void calculateAndSetPosition(int delayIndex, int totalDelays);
    void process(float& leftSample, float& rightSample);

private:
    float calculateStereoPosition(int delayIndex, int totalDelays);

    float sampleRate;
    float currentPosition = 0.0f;
    juce::SmoothedValue<float> smoothedPosition;
    std::mt19937 rng; // Random number generator
};
