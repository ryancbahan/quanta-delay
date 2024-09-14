#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include "StereoFieldManager.h"

class DampManager
{
public:
    DampManager();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void setDamp(float newDamp);
    void process(float& sampleLeft, float& sampleRight);

private:
    void precalculateValues();
    void generateReflectionPattern();
    void updateEchoParameters();

    float sampleRate;
    float damp;
    float smoothedDamp;
    float lastUpdatedDamp;
    juce::SmoothedValue<float> smoothedDamping;

    float roomSize;
    float reflectionGain;
    float decayTime;
    float modulationRate;
    float modulationDepth;
    float modulationPhase;

    float initialCutoff;
    float cutoffDecayRate;

    int writePos;

    static constexpr int MAX_ECHOES = 10;
    static constexpr float MAX_ECHO_TIME = 2.0f; // Max echo time in seconds

    static constexpr int MAX_REFLECTIONS = 10;
    static constexpr float PRE_DELAY_MS = 20.0f;

    static constexpr int MODULATION_TABLE_SIZE = 1024;
    std::array<float, MODULATION_TABLE_SIZE> modulationTable;
    
    std::array<StereoFieldManager, MAX_ECHOES + MAX_REFLECTIONS> stereoManagers;


    juce::AudioBuffer<float> echoBuffer;

    std::array<int, MAX_ECHOES> echoDelays;
    std::array<float, MAX_ECHOES> echoGains;
    std::array<float, MAX_ECHOES> decayGains;

    std::vector<int> reflectionDelays;
    std::vector<float> reflectionGains;
    std::vector<float> reflectionDecayGains;

    juce::dsp::IIR::Coefficients<float>::Ptr lowpassCoeffsLeft;
    juce::dsp::IIR::Filter<float> lowpassFilterLeft;
    juce::dsp::IIR::Coefficients<float>::Ptr lowpassCoeffsRight;
    juce::dsp::IIR::Filter<float> lowpassFilterRight;
};
