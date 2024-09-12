#pragma once

#include <JuceHeader.h>

class FilterManager
{
public:
    enum class FilterType
    {
        LowPass,
        HighPass
    };

    FilterManager();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();

    void setType(FilterType newType);
    void setFrequency(float newFrequency);
    void setSlope(float newSlope);
    void setQ(float newQ);

    float processSample(float sample);

private:
    void updateFilter();

    juce::dsp::StateVariableTPTFilter<float> filter;
    FilterType currentType;
    float frequency;
    float slope;
    float q;
    double sampleRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterManager)
};
