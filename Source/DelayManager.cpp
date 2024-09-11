#include "DelayManager.h"

DelayManager::DelayManager()
    : feedback(0.5f)
    , wetLevel(0.5f)
    , sampleRate(44100.0)
{
}

void DelayManager::prepare(const juce::dsp::ProcessSpec& spec, float initialDelayTime)
{
    sampleRate = spec.sampleRate;
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(sampleRate * 2.0); // Maximum 2 seconds delay
    
    smoothedDelayTime.reset(sampleRate, 0.05);
    smoothedDelayTime.setCurrentAndTargetValue(initialDelayTime * sampleRate);
    
    smoothedFeedback.reset(sampleRate, 0.05);
    smoothedFeedback.setCurrentAndTargetValue(feedback);
}

void DelayManager::reset()
{
    delayLine.reset();
}

void DelayManager::setDelayTime(float delayTimeInSeconds)
{
    smoothedDelayTime.setTargetValue(delayTimeInSeconds * sampleRate);
}

void DelayManager::setFeedback(float newFeedback)
{
    smoothedFeedback.setTargetValue(newFeedback);
}

float DelayManager::processSample(float inputSample)
{
    float currentDelayTime = smoothedDelayTime.getNextValue();
    float currentFeedback = smoothedFeedback.getNextValue();
    
    delayLine.setDelay(currentDelayTime);
    
    float delayedSample = delayLine.popSample(0);
    float feedbackSample = delayedSample * currentFeedback;
    delayLine.pushSample(0, inputSample + feedbackSample);
    
    return delayedSample;
}
