/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "DelayManager.h"

#define MAX_DELAY_TIME 2
#define MAX_DELAY_LINES 8

//==============================================================================
/**
*/
class QuantadelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    QuantadelayAudioProcessor();
    ~QuantadelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;

private:
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* delayTimeParameter = nullptr;
    std::atomic<float>* feedbackParameter = nullptr;
    std::atomic<float>* delayLinesParameter = nullptr;

    std::array<DelayManager, MAX_DELAY_LINES> delayManagersLeft;
    std::array<DelayManager, MAX_DELAY_LINES> delayManagersRight;
    
    int previousDelayLinesValue = 1;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantadelayAudioProcessor)
};
