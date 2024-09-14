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
    static constexpr int MAX_ECHOES = 20;
    static constexpr float MAX_ECHO_TIME = 0.2f; // 200ms

    juce::AudioBuffer<float> echoBuffer;
    float sampleRate;
    float damp;
    float smoothedDamp;
    float lastUpdatedDamp;
    float lastSample;
    int writePos;
    std::array<float, MAX_ECHOES> echoGains;
    std::array<int, MAX_ECHOES> echoDelays;

    void updateEchoParameters();
};
