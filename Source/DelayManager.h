
#include <JuceHeader.h>

class DelayManager
{
public:
    DelayManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec, float initialDelayTime);
    void reset();
    
    void setDelayTime(float delayTimeInSeconds);
    void setFeedback(float newFeedback);
    
    float processSample(float inputSample);
    
private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine;
    juce::SmoothedValue<float> smoothedDelayTime;
    juce::SmoothedValue<float> smoothedFeedback;
    juce::SmoothedValue<float> smoothedWetLevel;
    float feedback;
    float wetLevel;
    double sampleRate;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayManager)
};
