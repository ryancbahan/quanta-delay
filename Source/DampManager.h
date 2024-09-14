#pragma once

#include <JuceHeader.h>
#include <vector>

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
    void generateReflectionPattern();
    float cubicInterpolation(float y0, float y1, float y2, float y3, float t);

    juce::AudioBuffer<float> echoBuffer;
    juce::SmoothedValue<float> smoothedDamping;

    float sampleRate;
    float damp;
    float smoothedDamp;
    float lastUpdatedDamp;
    float lastSample;

    // Reflection parameters
    float roomSize;
    float reflectionGain;
    
    int writePos;
    std::vector<int> reflectionDelays;
    std::vector<float> reflectionGains;

    static constexpr float PRE_DELAY_MS = 10.0f;  // 10ms pre-delay
    static constexpr int MAX_ECHOES = 10;
    static constexpr int MAX_REFLECTIONS = 20;
    static constexpr float MAX_ECHO_TIME = 2.0f;  // Increased to 2 seconds

    int echoDelays[MAX_ECHOES] = { 0 };
    float echoGains[MAX_ECHOES] = { 0.0f };
};
