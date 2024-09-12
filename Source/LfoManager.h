#pragma once

#include <JuceHeader.h>
#include <array>

class LFOManager
{
public:
    LFOManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setRate(float rateHz);
    void setDepth(float depthMs);
    
    float getNextSample();

    void calculateAndSetRate(int index);

    static constexpr int NUM_PRESET_FREQUENCIES = 20;

private:
    static const std::array<float, NUM_PRESET_FREQUENCIES> presetFrequencies;

    juce::dsp::Oscillator<float> lfo;
    float depth;
    double sampleRate;
    float phase;
    float frequency;
    float lastSample;
    
    float mapToFrequencyRange(float input) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOManager)
};
