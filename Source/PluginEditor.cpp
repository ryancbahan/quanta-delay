#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"  // Include the custom look and feel

//==============================================================================
// Constructor
QuantadelayAudioProcessorEditor::QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the editor's size to whatever you need it to be.
    setSize (400, 300);
    
    // Initialize the custom look-and-feel object
    customLookAndFeel = std::make_unique<CustomLookAndFeel>();

    auto& params = processor.getParameters();

    // Setup the knobs and apply the custom look-and-feel
    setupKnob(mixParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(0)), 0, 0, 100, 100);
    mixParamSlider.setLookAndFeel(customLookAndFeel.get());  // Apply custom look-and-feel

    setupKnob(delayTimeParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(1)), 100, 0, 100, 100);
    delayTimeParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(feedbackParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(2)), 200, 0, 100, 100);
    feedbackParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(delayLinesParamSlider, static_cast<juce::AudioParameterInt*>(params.getUnchecked(3)), 300, 100, 100, 100);
    delayLinesParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(depthParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(4)), 300, 0, 100, 100);
    depthParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(spreadParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(5)), 200, 100, 100, 100);
    spreadParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(octavesParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(6)), 200, 200, 100, 100);
    octavesParamSlider.setLookAndFeel(customLookAndFeel.get());
}

// Destructor
QuantadelayAudioProcessorEditor::~QuantadelayAudioProcessorEditor()
{
    // It's important to reset the look-and-feel of the sliders back to nullptr before destruction
    mixParamSlider.setLookAndFeel(nullptr);
    delayTimeParamSlider.setLookAndFeel(nullptr);
    feedbackParamSlider.setLookAndFeel(nullptr);
    delayLinesParamSlider.setLookAndFeel(nullptr);
    depthParamSlider.setLookAndFeel(nullptr);
    spreadParamSlider.setLookAndFeel(nullptr);
    octavesParamSlider.setLookAndFeel(nullptr);
}

//==============================================================================
// Paint method
void QuantadelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

// Resized method
void QuantadelayAudioProcessorEditor::resized()
{
    // This is where you'll layout the positions of your subcomponents
}

//==============================================================================
// Setup knob helper function
void QuantadelayAudioProcessorEditor::setupKnob(juce::Slider& slider, juce::RangedAudioParameter* parameter,
               int x, int y, int width, int height)
{
    slider.setBounds(x, y, width, height);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    slider.setRange(parameter->getNormalisableRange().start, parameter->getNormalisableRange().end);
    slider.setValue(parameter->getValue());
    addAndMakeVisible(slider);

    slider.onValueChange = [&slider, parameter] {
        parameter->setValueNotifyingHost(parameter->convertTo0to1(slider.getValue()));
    };

    slider.onDragStart = [parameter] {
        parameter->beginChangeGesture();
    };

    slider.onDragEnd = [parameter] {
        parameter->endChangeGesture();
    };
}
