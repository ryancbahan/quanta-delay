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
    
    void process(float& sample);

private:
    juce::AudioBuffer<float> buffer;
    int writePos;
    float readPos;
    float shiftFactor;
    int bufferSize;

    juce::SmoothedValue<float> smoothedShiftFactor;
    double sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchShifterManager)
};
