#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

class QuantadelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor&);
    ~QuantadelayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    QuantadelayAudioProcessor& audioProcessor;
    std::unique_ptr<CustomLookAndFeel> customLookAndFeel;
    
    juce::Slider mixParamSlider;
    juce::Slider feedbackParamSlider;
    juce::Slider delayTimeParamSlider;
    juce::Slider delayLinesParamSlider;
    juce::Slider depthParamSlider;
    juce::Slider spreadParamSlider;
    juce::Slider octavesParamSlider;
    
    juce::Slider lowPassFreqSlider;
    juce::Slider highPassFreqSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassFreqAttachment;

    void setupKnob(juce::Slider& slider, juce::RangedAudioParameter* parameter,
                   int x, int y, int width, int height, const juce::String& labelText);
    
    void setupSlider(juce::Slider& slider, juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& parameterID, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantadelayAudioProcessorEditor)
};
