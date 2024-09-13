#pragma once
#include <JuceHeader.h>

class PitchShifterManager
{
public:
    PitchShifterManager();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setShiftFactor(float newShiftFactor);
    void setNoiseAmplitude(float amplitude); // Adjust noise amplitude
    void process(float& sample);

private:
    juce::AudioBuffer<float> buffer;
    int writePos;
    float readPos;
    float shiftFactor;
    int bufferSize;
    float crossfadePos;
    float crossfadeDuration;
    float sampleRate;
    float crossfadeIncrement;
    float noiseAmplitude; // Amplitude of the noise

    void calculateCrossfadeIncrement();
    float generateNoise() const; // Generate controlled noise
};
