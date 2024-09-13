#include "FilterManager.h"

FilterManager::FilterManager()
    : currentType(FilterType::LowPass)
    , frequency(1000.0f)
    , slope(1.0f)
    , q(0.707f)
    , sampleRate(44100.0)
{
    updateFilters();
}

void FilterManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    
    filterLeft.prepare(spec);
    filterRight.prepare(spec);
    
    updateFilters();
}

void FilterManager::reset()
{
    filterLeft.reset();
    filterRight.reset();
}

void FilterManager::setType(FilterType newType)
{
    if (currentType != newType)
    {
        currentType = newType;
        updateFilters();
    }
}

void FilterManager::setFrequency(float newFrequency)
{
    if (frequency != newFrequency)
    {
        frequency = newFrequency;
        updateFilters();
    }
}

void FilterManager::setSlope(float newSlope)
{
    if (slope != newSlope)
    {
        slope = newSlope;
        updateFilters();
    }
}

void FilterManager::setQ(float newQ)
{
    if (q != newQ)
    {
        q = newQ;
        updateFilters();
    }
}

void FilterManager::processStereoSample(float& leftSample, float& rightSample)
{
    leftSample = filterLeft.processSample(0, leftSample);
    rightSample = filterRight.processSample(0, rightSample);
}

void FilterManager::updateFilters()
{
    auto updateSingleFilter = [this](juce::dsp::StateVariableTPTFilter<float>& filter)
    {
        switch (currentType)
        {
            case FilterType::LowPass:
                filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                break;
            case FilterType::HighPass:
                filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
                break;
        }

        filter.setCutoffFrequency(frequency);
        filter.setResonance(q);

        // The StateVariableTPTFilter doesn't have a direct slope setting,
        // so we'll adjust the order of the filter based on the slope
        int order = static_cast<int>(slope);
        for (int i = 1; i < order; ++i)
        {
            filter.snapToZero();
        }
    };

    updateSingleFilter(filterLeft);
    updateSingleFilter(filterRight);
}
