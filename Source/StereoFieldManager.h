#pragma once

#include <JuceHeader.h>

class StereoFieldManager
{
public:
    StereoFieldManager();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setPosition(float newPosition);
    void process(float& leftSample, float& rightSample);

private:
    juce::SmoothedValue<float> smoothedPosition;
    float sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoFieldManager)
};
