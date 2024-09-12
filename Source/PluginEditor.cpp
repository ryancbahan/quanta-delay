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
    setLookAndFeel(customLookAndFeel.get());

    auto& params = processor.getParameters();

    // Setup the knobs with labels and apply the custom look-and-feel
    setupKnob(mixParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(0)), 0, 12, 100, 100, "Mix");
    mixParamSlider.setLookAndFeel(customLookAndFeel.get());  // Apply custom look-and-feel

    setupKnob(delayTimeParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(1)), 100, 12, 100, 100, "Delay Time");
    delayTimeParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(feedbackParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(2)), 200, 12, 100, 100, "Feedback");
    feedbackParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(delayLinesParamSlider, static_cast<juce::AudioParameterInt*>(params.getUnchecked(3)), 300, 150, 100, 100, "Delay Lines");
    delayLinesParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(depthParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(4)), 300, 12, 100, 100, "Depth");
    depthParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(spreadParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(5)), 200, 150, 100, 100, "Spread");
    spreadParamSlider.setLookAndFeel(customLookAndFeel.get());

    setupKnob(octavesParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(6)), 0, 150, 100, 100, "Octaves");
    octavesParamSlider.setLookAndFeel(customLookAndFeel.get());
    
    createTempoKnob(static_cast<juce::AudioParameterFloat*>(params.getUnchecked(7)));
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
    setLookAndFeel(nullptr);
}

//==============================================================================
// Paint method
void QuantadelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background
    customLookAndFeel->drawBackground(g, getWidth(), getHeight());

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

// Resized method
void QuantadelayAudioProcessorEditor::resized()
{
    // This is where you'll layout the positions of your subcomponents
}

void QuantadelayAudioProcessorEditor::createTempoKnob(juce::AudioParameterFloat* tempoParam)
{
    tempoParamSlider.setBounds(100, 150, 100, 100);  // Position and size of the knob
    tempoParamSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    tempoParamSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    tempoParamSlider.setRange(0.0, 6.0, 1.0);  // Range values correspond to the 1/1 to 1/64 note durations
    tempoParamSlider.setValue(tempoParam->get());
    addAndMakeVisible(tempoParamSlider);

    tempoParamSlider.onValueChange = [this, tempoParam] {
        float value = tempoParamSlider.getValue();
        tempoParam->setValueNotifyingHost(value);
    };

    tempoParamSlider.onDragStart = [tempoParam] {
        tempoParam->beginChangeGesture();
    };

    tempoParamSlider.onDragEnd = [tempoParam] {
        tempoParam->endChangeGesture();
    };
    
    auto* label = new juce::Label();
    label->setText("Tempo", juce::NotificationType::dontSendNotification);
    label->attachToComponent(&tempoParamSlider, false);
    label->setJustificationType(juce::Justification::centred);
    label->setBounds(100, 250, 100, 20); // Position label below the slider

    addAndMakeVisible(*label);
}

//==============================================================================
// Setup knob helper function
void QuantadelayAudioProcessorEditor::setupKnob(juce::Slider& slider, juce::RangedAudioParameter* parameter,
               int x, int y, int width, int height, const juce::String& labelText)
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
    
    auto* label = new juce::Label();
    label->setText(labelText, juce::NotificationType::dontSendNotification);
    label->attachToComponent(&slider, false);
    label->setJustificationType(juce::Justification::centred);
    label->setBounds(x, y + height + 5, width, 20); // Position label below the slider

    addAndMakeVisible(*label);
}
