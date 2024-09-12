#pragma once

#include <JuceHeader.h>

class LFOManager
{
public:
    LFOManager();
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    
    void setRate(float rateHz);
    void setDepth(float depthMs);
    
    float getNextSample();

    void calculateAndSetRate(float normalizedPosition);
    void setSeed(uint32_t newSeed);

private:
    float depth;
    double sampleRate;
    float rate;
    
    // Variables for deterministic LFO
    double x; // State variable for the logistic map
    double r; // Parameter for the logistic map
    int sampleCounter;
    int samplesPerCycle;
    uint32_t seed;
    uint32_t randomState; // For our custom random number generator
    
    void resetToInitialState();
    void updateLogisticMapParameter();
    float generateRandomFloat(); // Custom random number generator
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOManager)
};
