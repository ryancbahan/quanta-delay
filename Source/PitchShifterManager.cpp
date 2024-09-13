#include "PitchShifterManager.h"

PitchShifterManager::PitchShifterManager()
    : writePos(0)
    , readPos(0.0f)
    , shiftFactor(1.0f)
    , bufferSize(44100)
    , crossfadePos(0.0f)
    , crossfadeDuration(0.01f) // Default to 10ms
    , sampleRate(44100.0f) // Default sample rate
{
    buffer.setSize(1, bufferSize);
    buffer.clear();
}

void PitchShifterManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    reset();
    calculateCrossfadeIncrement();
}

void PitchShifterManager::reset()
{
    writePos = 0;
    readPos = 0.0f;
    crossfadePos = 0.0f;
    buffer.clear();
}

void PitchShifterManager::setShiftFactor(float newShiftFactor)
{
    // Ensure shift factor is valid and bounded
    shiftFactor = juce::jlimit(0.5f, 2.0f, newShiftFactor);
}

void PitchShifterManager::process(float& sample)
{
    // Write the input sample to the buffer
    buffer.setSample(0, writePos, sample);
    writePos = (writePos + 1) % bufferSize;

    // Get the buffer data
    float* channelData = buffer.getWritePointer(0);

    // Calculate read positions based on shift factor
    float tempReadPos = readPos;
    int readPosIndex1 = static_cast<int>(tempReadPos) % bufferSize;
    int readPosIndex2 = (readPosIndex1 + 1) % bufferSize;
    float frac = tempReadPos - std::floor(tempReadPos);

    // Linear interpolation for the read position
    float out = channelData[readPosIndex1] * (1.0f - frac) + channelData[readPosIndex2] * frac;

    // Update read position
    tempReadPos += shiftFactor;
    if (tempReadPos >= bufferSize)
        tempReadPos -= bufferSize;

    // Set the updated read position
    readPos = tempReadPos;

    // Output the pitch-shifted sample
    sample = out;
}

// Helper to calculate crossfade increment based on the sample rate and crossfade duration
void PitchShifterManager::calculateCrossfadeIncrement()
{
    crossfadeIncrement = 1.0f / (crossfadeDuration * sampleRate);
}
