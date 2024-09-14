#pragma once

#include <JuceHeader.h>

class DampManager
{
public:
    DampManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setDamp(float newDamp);
    void process(float& sample);

private:
    void updateEchoParameters();

    // Cubic interpolation helper function
    float cubicInterpolation(float y0, float y1, float y2, float y3, float t);

    // JUCE-specific
    juce::AudioBuffer<float> echoBuffer;   // Buffer for storing echoes
    juce::SmoothedValue<float> smoothedDamping; // Smoothly changing damp parameter

    float sampleRate;
    float damp;
    float smoothedDamp;
    float lastUpdatedDamp;
    float lastSample = 0.0f;

    int writePos;
    
    static constexpr int MAX_ECHOES = 10;
    static constexpr float MAX_ECHO_TIME = 1.0f;  // Maximum echo time in seconds

    int echoDelays[MAX_ECHOES] = { 0 };   // Array to store delay times for echoes
    float echoGains[MAX_ECHOES] = { 0.0f };   // Array to store gains for each echo
};
