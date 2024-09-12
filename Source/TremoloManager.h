#pragma once

#include <JuceHeader.h>

class TremoloManager
{
public:
    TremoloManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setRate(float rateHz);
    void setDepth(float newDepth);
    
    void process(float& leftSample, float& rightSample);

private:
    struct ChannelData
    {
        juce::dsp::Oscillator<float> lfoLow;
        juce::dsp::Oscillator<float> lfoHigh;
        juce::dsp::IIR::Filter<float> lowpassFilter;
        juce::dsp::IIR::Filter<float> highpassFilter;
    };

    ChannelData leftChannel;
    ChannelData rightChannel;
    
    float rate;
    float depth;
    double sampleRate;
    double phaseIncrement;
};
