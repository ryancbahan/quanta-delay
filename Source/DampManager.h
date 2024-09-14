#pragma once

#include <JuceHeader.h>
#include <vector>
#include <array>

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
    void precalculateValues();
    float linearInterpolation(float y1, float y2, float fraction);

    juce::AudioBuffer<float> echoBuffer;
    juce::SmoothedValue<float> smoothedDamping;

    juce::dsp::IIR::Filter<float> lowpassFilter;
    juce::dsp::IIR::Coefficients<float> lowpassCoeffs;

    float sampleRate;
    float damp;
    float smoothedDamp;
    float lastUpdatedDamp;

    // Reflection and decay parameters
    float roomSize;
    float reflectionGain;
    float decayTime;
    
    // Modulation parameters
    float modulationRate;
    float modulationDepth;
    float modulationPhase;

    // Filtering parameters
    float initialCutoff;
    float cutoffDecayRate;

    int writePos;
    std::vector<int> reflectionDelays;
    std::vector<float> reflectionGains;

    static constexpr int MAX_ECHOES = 10;
    static constexpr int MAX_REFLECTIONS = 20;
    static constexpr float MAX_ECHO_TIME = 2.0f;  // 2 seconds
    static constexpr float PRE_DELAY_MS = 10.0f;  // 10ms pre-delay

    std::array<int, MAX_ECHOES> echoDelays;
    std::array<float, MAX_ECHOES> echoGains;

    // Precalculated values
    static constexpr int MODULATION_TABLE_SIZE = 1024;
    std::array<float, MODULATION_TABLE_SIZE> modulationTable;
    std::array<float, MAX_ECHOES> decayGains;
    std::array<float, MAX_REFLECTIONS> reflectionDecayGains;
};
