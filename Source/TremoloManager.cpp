#include "TremoloManager.h"

TremoloManager::TremoloManager()
    : rate(2.0f)
    , depth(0.5f)
    , sampleRate(44100.0)
    , phaseIncrement(0.0)
{
    auto initChannel = [](ChannelData& channel) {
        channel.lfoLow.initialise([](float x) { return std::sin(x); });
        channel.lfoHigh.initialise([](float x) { return std::sin(x); });
    };

    initChannel(leftChannel);
    initChannel(rightChannel);
}

void TremoloManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    
    auto prepareChannel = [&](ChannelData& channel) {
        channel.lfoLow.prepare(spec);
        channel.lfoHigh.prepare(spec);
        
        auto lowpassCoefs = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1200.0f);
        channel.lowpassFilter.coefficients = lowpassCoefs;
        channel.lowpassFilter.prepare(spec);
        
        auto highpassCoefs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1200.0f);
        channel.highpassFilter.coefficients = highpassCoefs;
        channel.highpassFilter.prepare(spec);
    };

    prepareChannel(leftChannel);
    prepareChannel(rightChannel);

    setRate(rate); // This will set the correct phaseIncrement
}

void TremoloManager::reset()
{
    auto resetChannel = [](ChannelData& channel) {
        channel.lfoLow.reset();
        channel.lfoHigh.reset();
        channel.lowpassFilter.reset();
        channel.highpassFilter.reset();
    };

    resetChannel(leftChannel);
    resetChannel(rightChannel);
}

void TremoloManager::setRate(float rateHz)
{
    rate = rateHz;
    phaseIncrement = 2.0 * juce::MathConstants<double>::pi * rate / sampleRate;
    leftChannel.lfoLow.setFrequency(rate);
    leftChannel.lfoHigh.setFrequency(rate);
    rightChannel.lfoLow.setFrequency(rate);
    rightChannel.lfoHigh.setFrequency(rate);
}

void TremoloManager::setDepth(float newDepth)
{
    depth = juce::jlimit(0.0f, 1.0f, newDepth);
}

void TremoloManager::process(float& leftSample, float& rightSample)
{
    auto processSample = [this](ChannelData& channel, float& sample) {
        // Split into low and high bands
        float lowSample = channel.lowpassFilter.processSample(sample);
        float highSample = channel.highpassFilter.processSample(sample);
        
        // Apply tremolo to each band
        float lfoLowValue = channel.lfoLow.processSample(0.0f) * 0.5f + 0.5f;
        float lfoHighValue = channel.lfoHigh.processSample(0.0f) * 0.5f + 0.5f;
        
        lowSample *= 1.0f - (depth * (1.0f - lfoLowValue));
        highSample *= 1.0f - (depth * (1.0f - lfoHighValue));
        
        // Sum the processed bands
        sample = lowSample + highSample;
    };

    processSample(leftChannel, leftSample);
    processSample(rightChannel, rightSample);
}
