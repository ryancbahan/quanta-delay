/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QuantadelayAudioProcessor::QuantadelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                    parameters(*this, nullptr, "Parameters", createParameterLayout())

#endif
{
    mixParameter = parameters.getRawParameterValue("mix");
    delayTimeParameter = parameters.getRawParameterValue("delayTime");
    feedbackParameter = parameters.getRawParameterValue("feedback");
    
    delayManagerLeft.reset();
    delayManagerRight.reset();
}

QuantadelayAudioProcessor::~QuantadelayAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout QuantadelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1), "mix",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("delayTime", 2), "Delay time",
        juce::NormalisableRange<float>(0.0f, 2.0f), 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("feedback", 3), "Feedback",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f));
    
    return { params.begin(), params.end() };
    
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String QuantadelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool QuantadelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool QuantadelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool QuantadelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double QuantadelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int QuantadelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int QuantadelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void QuantadelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String QuantadelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void QuantadelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void QuantadelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = getTotalNumOutputChannels();

    float initialDelayTime = *delayTimeParameter;

    delayManagerLeft.prepare(spec, initialDelayTime);
    delayManagerRight.prepare(spec, initialDelayTime);
}

void QuantadelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool QuantadelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void QuantadelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    
    // Split the context into left and right channels
    auto leftContext = context.getOutputBlock().getSingleChannelBlock(0);
    auto rightContext = context.getOutputBlock().getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContextReplacing(leftContext);
    juce::dsp::ProcessContextReplacing<float> rightContextReplacing(rightContext);
    
    float mixValue = mixParameter->load();
    float delayTimeValue = delayTimeParameter->load();
    float feedbackValue = feedbackParameter->load();
    
    delayManagerLeft.setDelayTime(delayTimeValue);
    delayManagerRight.setDelayTime(delayTimeValue);
    delayManagerLeft.setFeedback(feedbackValue);
    delayManagerRight.setFeedback(feedbackValue);
    delayManagerLeft.setWetLevel(mixValue);
    delayManagerRight.setWetLevel(mixValue);
    
    delayManagerLeft.process(leftContextReplacing);
    delayManagerRight.process(rightContextReplacing);
}

//==============================================================================
bool QuantadelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* QuantadelayAudioProcessor::createEditor()
{
    return new QuantadelayAudioProcessorEditor (*this);
}

//==============================================================================
void QuantadelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void QuantadelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QuantadelayAudioProcessor();
}
