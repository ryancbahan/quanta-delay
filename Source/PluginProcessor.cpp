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
    delayLinesParameter = parameters.getRawParameterValue("delayLines");
    depthParameter = parameters.getRawParameterValue("depth");
    spreadParameter = parameters.getRawParameterValue("spread");
    octavesParameter = parameters.getRawParameterValue("octaves");
    lowPassFreqParameter = parameters.getRawParameterValue("lowPassFreq");
    highPassFreqParameter = parameters.getRawParameterValue("highPassFreq");
    dampParameter = parameters.getRawParameterValue("damp");

    
    highPassFilter.reset();
    lowPassFilter.reset();
    StereoFieldManager().reset();
    PitchShifterManager().reset();

    for (int i = 0; i < MAX_DELAY_LINES; ++i)
    {
        delayManagersLeft[i].reset();
        delayManagersRight[i].reset();
        lfoManagersLeft[i].reset();
        lfoManagersRight[i].reset();
    }

    highPassFilter.setType(FilterManager::FilterType::HighPass);
    highPassFilter.setFrequency(500.0f);  // 500 Hz
    highPassFilter.setQ(0.707f);  // Butterworth response
    highPassFilter.setSlope(1.0f);  // 12 dB/octave

    lowPassFilter.setType(FilterManager::FilterType::LowPass);
    lowPassFilter.setFrequency(2000.0f);  // 2000 Hz
    lowPassFilter.setQ(0.707f);  // Butterworth response
    lowPassFilter.setSlope(1.0f);  // 12 dB/octave
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
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID("delayLines", 4), "Delay Lines", 1, 20, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("depth", 5), "Depth",
        juce::NormalisableRange<float>(0.0f, 10.0f), 0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("spread", 6), "Spread",
        juce::NormalisableRange<float>(0.5f, 0.99f), 0.875f));
    
    params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID("octaves", 7), "Octaves", 1, 11, 1));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("lowPassFreq", 8), "Low Pass Freq",
        juce::NormalisableRange<float>(250.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("highPassFreq", 9), "High Pass Freq",
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.3f), 20.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("damp", 6), "damp",
        juce::NormalisableRange<float>(0.0f, 20.0f), 0.0f));
    
    
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
    
    highPassFilter.prepare(spec);
    lowPassFilter.prepare(spec);
    
    dampManager.prepare(spec);

    float initialDelayTime = *delayTimeParameter;

    for (int i = 0; i < MAX_DELAY_LINES; ++i)
    {
        lfoManagersLeft[i].reset();
        lfoManagersRight[i].reset();
        
        float currentDelayTime = initialDelayTime * std::pow(0.66f, i);
        delayManagersLeft[i].prepare(spec, currentDelayTime);
        delayManagersRight[i].prepare(spec, currentDelayTime);
        stereoManagers[i].prepare(spec);

        stereoManagers[i].calculateAndSetPosition(i, MAX_DELAY_LINES);
        
        lfoManagersLeft[i].prepare(spec);
        lfoManagersRight[i].prepare(spec);
        lfoManagersLeft[i].setDepth(1.0f);
        lfoManagersRight[i].setDepth(1.0f);
        
        lfoManagersLeft[i].calculateAndSetRate(i);
        lfoManagersRight[i].calculateAndSetRate(i);
    }
    
    for (auto& pitchShifter : pitchShifterManagers)
    {
        pitchShifter.prepare(spec);
    }

    smoothedDelayLines.reset(sampleRate, 0.05);
    smoothedDelayLines.setCurrentAndTargetValue(1.0f);
}

void QuantadelayAudioProcessor::releaseResources()
{
    StereoFieldManager().reset();
    PitchShifterManager().reset();
    highPassFilter.reset();
    lowPassFilter.reset();

    for (int i = 0; i < MAX_DELAY_LINES; ++i)
    {
        delayManagersLeft[i].reset();
        delayManagersRight[i].reset();
        lfoManagersLeft[i].reset();
        lfoManagersRight[i].reset();
    }
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
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    float mixValue = mixParameter->load();
    float delayTimeValue = delayTimeParameter->load();
    float feedbackValue = feedbackParameter->load();
    float depthValue = depthParameter->load();
    float spreadValue = spreadParameter->load();
    float octavesValue = octavesParameter->load();
    float lowPassFreq = lowPassFreqParameter->load();
    float highPassFreq = highPassFreqParameter->load();
    float dampValue = dampParameter->load();

    dampManager.setDamp(dampValue);

    lowPassFilter.setFrequency(lowPassFreq);
    highPassFilter.setFrequency(highPassFreq);


    int targetDelayLines = static_cast<int>(std::round(delayLinesParameter->load()));
    targetDelayLines = juce::jlimit(1, MAX_DELAY_LINES, targetDelayLines);

    smoothedDelayLines.setTargetValue(static_cast<float>(targetDelayLines));
    
    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update parameters for all delay lines
    for (int i = 0; i < MAX_DELAY_LINES; ++i)
    {
        float normalizedPosition = static_cast<float>(i) / static_cast<float>(MAX_DELAY_LINES - 1);
        lfoManagersLeft[i].calculateAndSetRate(i);
        lfoManagersRight[i].calculateAndSetRate(i);
        
        lfoManagersLeft[i].setDepth(depthValue);
        lfoManagersRight[i].setDepth(depthValue);
        
        float lfoValueLeft = lfoManagersLeft[i].getNextSample();
        float lfoValueRight = lfoManagersRight[i].getNextSample();
        float currentDelayTimeLeft = delayTimeValue * std::pow(spreadValue, i) + lfoValueLeft;
        float currentDelayTimeRight = delayTimeValue * std::pow(spreadValue, i) + lfoValueRight;

        delayManagersLeft[i].setDelayTime(currentDelayTimeLeft);
        delayManagersRight[i].setDelayTime(currentDelayTimeRight);
        delayManagersLeft[i].setFeedback(feedbackValue);
        delayManagersRight[i].setFeedback(feedbackValue);

        if (i == 0) {
            // First delay line remains unshifted
            pitchShifterManagers[i].setShiftFactor(1.0f);
        } else if (i % 4 == 1) {
            // Every 4th line (1, 5, 9, ...) is shifted up an octave
            pitchShifterManagers[i].setShiftFactor(2.0f);
        } else if (i % 2 == 1) {
            // Other odd lines (3, 7, 11, ...) are shifted down an octave
            pitchShifterManagers[i].setShiftFactor(0.5f);
        } else {
            // Even lines (2, 4, 6, 8, ...) are unshifted
            pitchShifterManagers[i].setShiftFactor(1.0f);
        }
    }


    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        float inputSampleLeft = leftChannel[sample];
        float inputSampleRight = rightChannel[sample];
        float wetSignalLeft = 0.0f;
        float wetSignalRight = 0.0f;

        float currentDelayLines = smoothedDelayLines.getNextValue();
        int fullDelayLines = static_cast<int>(std::floor(currentDelayLines));

        for (int i = 0; i < fullDelayLines; ++i)
        {
            float delayedSampleLeft = delayManagersLeft[i].processSample(inputSampleLeft);
            float delayedSampleRight = delayManagersRight[i].processSample(inputSampleRight);
            
            float leftOutput = delayedSampleLeft;
            float rightOutput = delayedSampleRight;
            
            if (i < octavesValue && i < fullDelayLines + 1) {
                pitchShifterManagers[i].process(leftOutput);
                pitchShifterManagers[i].process(rightOutput);
            }

            stereoManagers[i].process(leftOutput, rightOutput);
            
            wetSignalLeft += leftOutput;
            wetSignalRight += rightOutput;
        }

//         Scale the wet signal by the current (smoothed) number of delay lines
        wetSignalLeft /= currentDelayLines;
        wetSignalRight /= currentDelayLines;
        
        dampManager.process(wetSignalLeft, wetSignalRight);
        
        highPassFilter.processStereoSample(wetSignalLeft, wetSignalRight);
        lowPassFilter.processStereoSample(wetSignalLeft, wetSignalRight);

        leftChannel[sample] = inputSampleLeft + mixValue * wetSignalLeft;
        rightChannel[sample] = inputSampleRight + mixValue * wetSignalRight;
    }
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
