#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>
#include <random>
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

    // Constants
    static constexpr int MAX_ECHOES = 10;        // Maximum number of echoes
    static constexpr int MAX_REFLECTIONS = 5;    // Maximum number of reflections
    static constexpr float MAX_ECHO_TIME = 1.0f; // Maximum echo time in seconds
    static constexpr float PRE_DELAY_MS = 20.0f; // Pre-delay in milliseconds
    static constexpr int MODULATION_TABLE_SIZE = 1024;

    // Member variables
    float sampleRate;
    float damp;
    float smoothedDamp;
    float lastUpdatedDamp;
    juce::SmoothedValue<float> smoothedDamping;

    float roomSize;
    float reflectionGain;
    float decayTime;
    float modulationRate;
    int modulationRateInt;
    float modulationDepth;
    float modulationPhase;

    float initialCutoff;
    float cutoffDecayRate;

    int writePos;

    std::array<float, MODULATION_TABLE_SIZE> modulationTable;

    // Echo parameters
    std::array<int, MAX_ECHOES> echoDelays;
    std::array<float, MAX_ECHOES> echoGains;
    std::array<float, MAX_ECHOES> decayGainsLeft;
    std::array<float, MAX_ECHOES> decayGainsRight;

    // Reflection parameters
    std::vector<int> reflectionDelays;
    std::vector<float> reflectionGains;
    std::vector<float> reflectionDecayGainsLeft;
    std::vector<float> reflectionDecayGainsRight;

    std::array<StereoFieldManager, MAX_ECHOES + MAX_REFLECTIONS> stereoManagers;

    juce::AudioBuffer<float> echoBuffer;

    juce::dsp::IIR::Coefficients<float>::Ptr lowpassCoeffsLeft;
    juce::dsp::IIR::Filter<float> lowpassFilterLeft;
    juce::dsp::IIR::Coefficients<float>::Ptr lowpassCoeffsRight;
    juce::dsp::IIR::Filter<float> lowpassFilterRight;

    int numActiveEchoes;
    int numActiveReflections;

    std::mt19937 rng; // Random number generator
};
