#pragma once

#include <JuceHeader.h>

class PitchShifterManager
{
public:
    PitchShifterManager();
    ~PitchShifterManager() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setShiftFactor(float newShiftFactor);
    float getShiftFactor() const { return shiftFactor; }
    
    void process(float& leftSample, float& rightSample);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    juce::SmoothedValue<float> smoothedShiftFactor;

    float shiftFactor = 1.0f;
    float readPos = 0.0f;
    double sampleRate;
    static constexpr int MAX_DELAY_SAMPLES = 88200;
    
    float processSample(float inputSample);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterManager)
};
