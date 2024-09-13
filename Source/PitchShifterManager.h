#pragma once
#include <JuceHeader.h>

class PitchShifterManager
{
public:
    PitchShifterManager();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setShiftFactor(float newShiftFactor);
    void process(float& sample);

private:
    juce::AudioBuffer<float> buffer;
    int writePos;
    float readPos;
    float shiftFactor;
    int bufferSize;
    
    // Interpolation properties
    float crossfadePos;
    float crossfadeIncrement;
    float crossfadeDuration;
    float sampleRate;
    
    void calculateCrossfadeIncrement(); // Helper to calculate crossfade increment based on sample rate
};
