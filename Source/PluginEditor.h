#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class QuantadelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor&);
    ~QuantadelayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    QuantadelayAudioProcessor& audioProcessor;
    
    juce::Slider mixParamSlider;
    juce::Slider feedbackParamSlider;
    juce::Slider delayTimeParamSlider;
    juce::Slider delayLinesParamSlider;
    juce::Slider depthParamSlider;
    juce::Slider spreadParamSlider;
    juce::Slider octavesParamSlider;

    void setupKnob(juce::Slider& slider, juce::RangedAudioParameter* parameter,
                   int x, int y, int width, int height);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantadelayAudioProcessorEditor)
};
