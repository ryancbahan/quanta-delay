#pragma once

#include <JuceHeader.h>

class DelayBuffer
{
public:
    DelayBuffer(int numChannels, int maxDelayInSamples);

    void pushSample(int channel, float sample);
    float getSample(int channel, float delaySamples) const;
    void clear();

    int getNumChannels() const { return buffer.getNumChannels(); }
    int getBufferSize() const { return bufferSize; }

private:
    juce::AudioBuffer<float> buffer;
    std::atomic<int> writePosition;
    int bufferSize;

    int wrapPosition(int position) const;
    float interpolateSample(int channel, float readPosition) const;
};
