#include "PitchShifterManager.h"
#include <cmath>

// Cubic interpolation function
float cubicInterpolate(float p0, float p1, float p2, float p3, float t)
{
    float a = (-p0 / 2.0f) + (3.0f * p1 / 2.0f) - (3.0f * p2 / 2.0f) + (p3 / 2.0f);
    float b = p0 - (5.0f * p1 / 2.0f) + (2.0f * p2) - (p3 / 2.0f);
    float c = (-p0 / 2.0f) + (p2 / 2.0f);
    float d = p1;

    return a * t * t * t + b * t * t + c * t + d;
}

PitchShifterManager::PitchShifterManager()
    : writePos(0)
    , readPos(0.0f)
    , shiftFactor(1.0f)
    , bufferSize(88200)
    , crossfadePos(0.0f)
    , crossfadeDuration(0.01f)
    , sampleRate(44100.0f)
    , noiseAmplitude(0.0005f) // Ensure a very low amplitude
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
    shiftFactor = juce::jlimit(0.5f, 2.0f, newShiftFactor);
}

void PitchShifterManager::setNoiseAmplitude(float amplitude)
{
    noiseAmplitude = juce::jlimit(0.0f, 0.001f, amplitude); // Ensure amplitude is within a reasonable range
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
    int readPosIndex = static_cast<int>(tempReadPos) % bufferSize;

    // Indices for cubic interpolation
    int i0 = (readPosIndex - 1 + bufferSize) % bufferSize;
    int i1 = readPosIndex;
    int i2 = (readPosIndex + 1) % bufferSize;
    int i3 = (readPosIndex + 2) % bufferSize;

    // Fractional part for interpolation
    float frac = tempReadPos - std::floor(tempReadPos);

    // Perform cubic interpolation
    float out = cubicInterpolate(
        channelData[i0], channelData[i1], channelData[i2], channelData[i3], frac);

    // Update read position
    tempReadPos += shiftFactor;
    if (tempReadPos >= bufferSize)
        tempReadPos -= bufferSize;

    // Set the updated read position
    readPos = tempReadPos;

    // Add controlled noise to the output sample
    out += generateNoise() * noiseAmplitude; // Apply amplitude directly here

    // Output the pitch-shifted sample
    sample = out;
}

// Helper to calculate crossfade increment based on the sample rate and crossfade duration
void PitchShifterManager::calculateCrossfadeIncrement()
{
    crossfadeIncrement = 1.0f / (crossfadeDuration * sampleRate);
}

// Generate controlled noise
float PitchShifterManager::generateNoise() const
{
    return juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f; // [-1, 1] range
}
