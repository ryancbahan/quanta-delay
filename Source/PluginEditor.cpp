#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
QuantadelayAudioProcessorEditor::QuantadelayAudioProcessorEditor (QuantadelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 400);
    
    customLookAndFeel = std::make_unique<CustomLookAndFeel>();
    setLookAndFeel(customLookAndFeel.get());

    auto& params = processor.getParameters();

    setupKnob(mixParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(0)), 0, 12, 100, 100, "Mix");
    setupKnob(delayTimeParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(1)), 100, 12, 100, 100, "Delay Time");
    setupKnob(feedbackParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(2)), 200, 12, 100, 100, "Feedback");
    setupKnob(delayLinesParamSlider, static_cast<juce::AudioParameterInt*>(params.getUnchecked(4)), 300, 150, 100, 100, "Depth");
    setupKnob(depthParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(3)), 300, 12, 100, 100, "Delay lines");
    setupKnob(spreadParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(5)), 200, 150, 100, 100, "Spread");
    setupKnob(octavesParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(6)), 0, 150, 100, 100, "Octaves");
    setupKnob(dampParamSlider, static_cast<juce::AudioParameterFloat*>(params.getUnchecked(9)), 100, 150, 100, 100, "Damp");

    setupSlider(lowPassFreqSlider, audioProcessor.parameters, "lowPassFreq", "Low Pass");
    setupSlider(highPassFreqSlider, audioProcessor.parameters, "highPassFreq", "High Pass");
}

QuantadelayAudioProcessorEditor::~QuantadelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void QuantadelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    customLookAndFeel->drawBackground(g, getWidth(), getHeight());
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
}

void QuantadelayAudioProcessorEditor::resized()
{
    int sliderLeft = 20;
    int sliderWidth = getWidth() - 40;
    int sliderHeight = 30;
    
    lowPassFreqSlider.setBounds(sliderLeft, getHeight() - 70, sliderWidth, sliderHeight);
    highPassFreqSlider.setBounds(sliderLeft, getHeight() - 30, sliderWidth, sliderHeight);
}

//==============================================================================
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
    
    auto label = std::make_unique<juce::Label>();
    label->setText(labelText, juce::NotificationType::dontSendNotification);
    label->attachToComponent(&slider, false);
    label->setJustificationType(juce::Justification::centred);
    label->setBounds(x, y + height + 5, width, 20);

    addAndMakeVisible(*label);
    sliderLabels.push_back(std::move(label));
}

void QuantadelayAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::AudioProcessorValueTreeState& apvts,
                                                  const juce::String& parameterID, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider.setLookAndFeel(customLookAndFeel.get());
    addAndMakeVisible(slider);

    auto attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, parameterID, slider);
    if (parameterID == "lowPassFreq")
        lowPassFreqAttachment = std::move(attachment);
    else if (parameterID == "highPassFreq")
        highPassFreqAttachment = std::move(attachment);

    auto label = std::make_unique<juce::Label>();
    label->setText(labelText, juce::NotificationType::dontSendNotification);
    label->attachToComponent(&slider, false);
    label->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(*label);
    sliderLabels.push_back(std::move(label));
}
