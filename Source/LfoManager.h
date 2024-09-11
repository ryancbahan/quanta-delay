#pragma once

#include <JuceHeader.h>

class LFOManager
{
public:
    LFOManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setRate(float rateHz);
    void setDepth(float depthMs);
    
    float getNextSample();

private:
    juce::dsp::Oscillator<float> lfo;
    float depth;
    float sampleRate;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOManager)
};
