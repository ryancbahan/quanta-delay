#include "PitchShifterManager.h"

PitchShifterManager::PitchShifterManager()
    : writePos(0)
    , readPos(0.0f)
    , shiftFactor(1.0f)
    , bufferSize(44100)
{
    buffer.setSize(1, bufferSize);
    buffer.clear();
}

void PitchShifterManager::prepare(const juce::dsp::ProcessSpec& spec)
{
    reset();
}

void PitchShifterManager::reset()
{
    writePos = 0;
    readPos = 0.0f;
    buffer.clear();
}

void PitchShifterManager::setShiftFactor(float newShiftFactor)
{
    shiftFactor = newShiftFactor;
}

void PitchShifterManager::process(float& sample)
{
    // Write the input sample to the buffer
    buffer.setSample(0, writePos, sample);
    writePos = (writePos + 1) % bufferSize;

    // Read from the buffer with the pitch shift
    float* channelData = buffer.getWritePointer(0);
    int readPos1 = static_cast<int>(readPos) % bufferSize;
    int readPos2 = (readPos1 + 1) % bufferSize;
    float frac = readPos - std::floor(readPos);

    // Linear interpolation
    float out = channelData[readPos1] * (1.0f - frac) + channelData[readPos2] * frac;

    // Update read position
    readPos += shiftFactor;
    if (readPos >= bufferSize)
        readPos -= bufferSize;

    // Output the pitch-shifted sample
    sample = out;
}
