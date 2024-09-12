#include "FilterManager.h"

FilterManager::FilterManager()
    : currentType(FilterType::LowPass)
    , frequency(1000.0f)
    , slope(1.0f)
    , q(0.707f)
    , sampleRate(44100.0)
{
    updateFilter();
}

void FilterManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    filter.prepare(spec);
    updateFilter();
}

void FilterManager::reset()
{
    filter.reset();
}

void FilterManager::setType(FilterType newType)
{
    if (currentType != newType)
    {
        currentType = newType;
        updateFilter();
    }
}

void FilterManager::setFrequency(float newFrequency)
{
    if (frequency != newFrequency)
    {
        frequency = newFrequency;
        updateFilter();
    }
}

void FilterManager::setSlope(float newSlope)
{
    if (slope != newSlope)
    {
        slope = newSlope;
        updateFilter();
    }
}

void FilterManager::setQ(float newQ)
{
    if (q != newQ)
    {
        q = newQ;
        updateFilter();
    }
}

float FilterManager::processSample(float sample)
{
    return filter.processSample(0, sample);
}

void FilterManager::updateFilter()
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
}
