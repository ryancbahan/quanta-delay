/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class QuantadelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor&);
    ~QuantadelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    QuantadelayAudioProcessor& audioProcessor;
    
    juce::Slider mixParamSlider;
    juce::Slider feedbackParamSlider;
    juce::Slider delayTimeParamSlider;
    juce::Slider delayLinesParamSlider;
    juce::Slider depthParamSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantadelayAudioProcessorEditor)
};
