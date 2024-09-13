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

    void processStereoSample(float& leftSample, float& rightSample);

private:
    FilterType currentType;
    float frequency;
    float slope;
    float q;
    double sampleRate;

    juce::dsp::StateVariableTPTFilter<float> filterLeft;
    juce::dsp::StateVariableTPTFilter<float> filterRight;

    void updateFilters();
};
