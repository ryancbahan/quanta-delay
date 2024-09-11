/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QuantadelayAudioProcessorEditor::QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    auto& params = processor.getParameters();
    
    juce::AudioParameterFloat* mixParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    juce::AudioParameterFloat* delayTimeParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);
    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);
    juce::AudioParameterInt* delayLinesParameter = (juce::AudioParameterInt*)params.getUnchecked(3);
    juce::AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);
    juce::AudioParameterFloat* spreadParameter = (juce::AudioParameterFloat*)params.getUnchecked(5);

    
    // Mix parameter slider
    mixParamSlider.setBounds(0, 0, 100, 100);
    mixParamSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mixParamSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mixParamSlider.setRange(mixParameter->range.start, mixParameter->range.end);
    mixParamSlider.setValue(*mixParameter);
    addAndMakeVisible(mixParamSlider);
    
    mixParamSlider.onValueChange = [this, mixParameter] {
        *mixParameter = mixParamSlider.getValue();
    };
    
    mixParamSlider.onDragStart = [mixParameter] {
        mixParameter->beginChangeGesture();
    };
    
    mixParamSlider.onDragEnd = [mixParameter] {
        mixParameter->endChangeGesture();
    };
    
    // Delay time parameter slider
    delayTimeParamSlider.setBounds(100, 0, 100, 100);
    delayTimeParamSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    delayTimeParamSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    delayTimeParamSlider.setRange(delayTimeParameter->range.start, delayTimeParameter->range.end);
    delayTimeParamSlider.setValue(*delayTimeParameter);
    addAndMakeVisible(delayTimeParamSlider);
    
    delayTimeParamSlider.onValueChange = [this, delayTimeParameter] {
        *delayTimeParameter = delayTimeParamSlider.getValue();
    };
    
    delayTimeParamSlider.onDragStart = [delayTimeParameter] {
        delayTimeParameter->beginChangeGesture();
    };
    
    delayTimeParamSlider.onDragEnd = [delayTimeParameter] {
        delayTimeParameter->endChangeGesture();
    };
    
    // Feedback parameter slider
    feedbackParamSlider.setBounds(200, 0, 100, 100);
    feedbackParamSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    feedbackParamSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    feedbackParamSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    feedbackParamSlider.setValue(*feedbackParameter);
    addAndMakeVisible(feedbackParamSlider);
    
    feedbackParamSlider.onValueChange = [this, feedbackParameter] {
        *feedbackParameter = feedbackParamSlider.getValue();
    };
    
    feedbackParamSlider.onDragStart = [feedbackParameter] {
        feedbackParameter->beginChangeGesture();
    };
    
    feedbackParamSlider.onDragEnd = [feedbackParameter] {
        feedbackParameter->endChangeGesture();
    };
    
    // Delay lines parameter slider
    delayLinesParamSlider.setBounds(300, 100, 100, 100);
    delayLinesParamSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    delayLinesParamSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    delayLinesParamSlider.setRange(delayLinesParameter->getRange().getStart(), delayLinesParameter->getRange().getEnd());
    delayLinesParamSlider.setValue(*delayLinesParameter);
    addAndMakeVisible(delayLinesParamSlider);
    
    delayLinesParamSlider.onValueChange = [this, delayLinesParameter] {
        *delayLinesParameter = delayLinesParamSlider.getValue();
    };
    
    delayLinesParamSlider.onDragStart = [delayLinesParameter] {
        delayLinesParameter->beginChangeGesture();
    };
    
    delayLinesParamSlider.onDragEnd = [delayLinesParameter] {
        delayLinesParameter->endChangeGesture();
    };
    
    
    // depth parameter slider
    depthParamSlider.setBounds(300, 0, 100, 100);
    depthParamSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    depthParamSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    depthParamSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    depthParamSlider.setValue(*depthParameter);
    addAndMakeVisible(depthParamSlider);
    
    depthParamSlider.onValueChange = [this, depthParameter] {
        *depthParameter = depthParamSlider.getValue();
    };
    
    depthParamSlider.onDragStart = [depthParameter] {
        depthParameter->beginChangeGesture();
    };
    
    depthParamSlider.onDragEnd = [depthParameter] {
        depthParameter->endChangeGesture();
    };
    
    // depth parameter slider
    spreadParamSlider.setBounds(200, 100, 100, 100);
    spreadParamSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    spreadParamSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    spreadParamSlider.setRange(spreadParameter->range.start, spreadParameter->range.end);
    spreadParamSlider.setValue(*spreadParameter);
    addAndMakeVisible(spreadParamSlider);
    
    spreadParamSlider.onValueChange = [this, spreadParameter] {
        *spreadParameter = spreadParamSlider.getValue();
    };
    
    depthParamSlider.onDragStart = [spreadParameter] {
        spreadParameter->beginChangeGesture();
    };
    
    depthParamSlider.onDragEnd = [spreadParameter] {
        spreadParameter->endChangeGesture();
    };
}

QuantadelayAudioProcessorEditor::~QuantadelayAudioProcessorEditor()
{
}

//==============================================================================
void QuantadelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void QuantadelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
