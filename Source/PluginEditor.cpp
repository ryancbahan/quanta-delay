#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"  // Include the custom look and feel

//==============================================================================
// Constructor
QuantadelayAudioProcessorEditor::QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the editor's size to whatever you need it to be.
    setSize (400, 400);
    
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
    
    setupSlider(lowPassFreqSlider, audioProcessor.parameters, "lowPassFreq", "Low Pass");
    setupSlider(highPassFreqSlider, audioProcessor.parameters, "highPassFreq", "High Pass");
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
    // ... existing code for other UI elements ...

    int sliderLeft = 120;
    int sliderWidth = getWidth() - sliderLeft - 10;
    int sliderHeight = 20;
    
    lowPassFreqSlider.setBounds(sliderLeft, getHeight() - 60, sliderWidth, sliderHeight);
    highPassFreqSlider.setBounds(sliderLeft, getHeight() - 30, sliderWidth, sliderHeight);
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

void QuantadelayAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::AudioProcessorValueTreeState& apvts,
                                                  const juce::String& parameterID, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(slider);

    auto* label = new juce::Label();
    label->setText(labelText, juce::NotificationType::dontSendNotification);
    label->attachToComponent(&slider, true);
    addAndMakeVisible(label);

    auto* attachment = new juce::AudioProcessorValueTreeState::SliderAttachment(apvts, parameterID, slider);
    if (parameterID == "lowPassFreq")
        lowPassFreqAttachment.reset(attachment);
    else if (parameterID == "highPassFreq")
        highPassFreqAttachment.reset(attachment);
}
