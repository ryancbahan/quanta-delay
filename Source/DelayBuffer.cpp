#include "DelayBuffer.h"

DelayBuffer::DelayBuffer(int numChannels, int maxDelayInSamples)
    : buffer(numChannels, maxDelayInSamples),
      writePosition(0),
      bufferSize(maxDelayInSamples)
{
    jassert(numChannels > 0);
    jassert(maxDelayInSamples > 0);
    buffer.clear();
}

void DelayBuffer::pushSample(int channel, float sample)
{
    jassert(juce::isPositiveAndBelow(channel, buffer.getNumChannels()));
    
    int currentWritePosition = writePosition.load(std::memory_order_relaxed);
    buffer.setSample(channel, currentWritePosition, sample);
    
    currentWritePosition = wrapPosition(currentWritePosition + 1);
    writePosition.store(currentWritePosition, std::memory_order_release);
}

float DelayBuffer::getSample(int channel, float delaySamples) const
{
    jassert(juce::isPositiveAndBelow(channel, buffer.getNumChannels()));
    jassert(delaySamples >= 0 && delaySamples < bufferSize);

    int currentWritePosition = writePosition.load(std::memory_order_acquire);
    float readPosition = currentWritePosition - delaySamples;
    
    if (readPosition < 0)
        readPosition += bufferSize;

    return interpolateSample(channel, readPosition);
}

void DelayBuffer::clear()
{
    buffer.clear();
    writePosition.store(0, std::memory_order_relaxed);
}

int DelayBuffer::wrapPosition(int position) const
{
    if (position >= bufferSize)
        return position - bufferSize;
    if (position < 0)
        return position + bufferSize;
    return position;
}

float DelayBuffer::interpolateSample(int channel, float readPosition) const
{
    int index1 = static_cast<int>(readPosition);
    int index2 = wrapPosition(index1 + 1);
    
    float fraction = readPosition - index1;
    
    float sample1 = buffer.getSample(channel, index1);
    float sample2 = buffer.getSample(channel, index2);
    
    return sample1 + fraction * (sample2 - sample1);
}
