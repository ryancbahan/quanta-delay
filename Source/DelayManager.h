#pragma once

#include <JuceHeader.h>

class DelayManager
{
public:
    DelayManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec, float initialDelayTime);
    void reset();
    
    void setDelayTime(float delayTimeInSeconds);
    void setFeedback(float newFeedback);
    void setWetLevel(float newWetLevel);
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context);
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    juce::SmoothedValue<float> smoothedDelayTime;
    
    float delayTimeInSamples;
    float feedback;
    float wetLevel;
    double sampleRate;
    float maxDelayTimeInSeconds;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayManager)
};
