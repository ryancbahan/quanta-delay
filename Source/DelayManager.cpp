#include "DelayManager.h"

DelayManager::DelayManager()
    : delayTimeInSamples(0.0f)
    , feedback(0.5f)
    , wetLevel(0.5f)
    , sampleRate(44100.0)
    , maxDelayTimeInSeconds(2.0f)
{
}

void DelayManager::prepare(const juce::dsp::ProcessSpec& spec, float initialDelayTime) {
    sampleRate = spec.sampleRate;
    reset();
    
    int maxDelaySamples = static_cast<int>(maxDelayTimeInSeconds * sampleRate);
    delayLine.setMaximumDelayInSamples(maxDelaySamples);
    delayLine.prepare(spec);
    
    smoothedDelayTime.reset(sampleRate, 0.05); // 50ms smoothing time
    smoothedDelayTime.setCurrentAndTargetValue(initialDelayTime);
}

void DelayManager::reset()
{
    delayLine.reset();
}

void DelayManager::setDelayTime(float delayTimeInSeconds)
{
    smoothedDelayTime.setTargetValue(delayTimeInSeconds);
}

void DelayManager::setFeedback(float newFeedback)
{
    feedback = juce::jlimit(0.0f, 0.95f, newFeedback);
}

void DelayManager::setWetLevel(float newWetLevel)
{
    wetLevel = juce::jlimit(0.0f, 1.0f, newWetLevel);
}

void DelayManager::process(const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    auto numSamples = inputBlock.getNumSamples();
    auto numChannels = inputBlock.getNumChannels();

    for (size_t channel = 0; channel < numChannels; ++channel)
    {
        auto* inputSamples = inputBlock.getChannelPointer(channel);
        auto* outputSamples = outputBlock.getChannelPointer(channel);

        for (size_t i = 0; i < numSamples; ++i)
        {
            float currentDelayTime = smoothedDelayTime.getNextValue();
            delayLine.setDelay(currentDelayTime * sampleRate);

            float delaySample = delayLine.popSample(channel);
            float inputSample = inputSamples[i];

            float delayedSample = inputSample + feedback * delaySample;
            delayLine.pushSample(channel, delayedSample);

            outputSamples[i] = inputSample + wetLevel * delaySample;
        }
    }
}
