/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EZDistortionAudioProcessor::EZDistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
apvts(*this, nullptr, "Parameters",
{
std::make_unique<AudioParameterFloat>(ParameterID("MIX",1), "Mix", NormalisableRange<float> { 0.0f, 1.0f, .001f }, 0.5f),
std::make_unique<AudioParameterFloat>(ParameterID("GAIN",1), "Gain", NormalisableRange<float> { -40.0f, 6.0f, .01f }, -17.f),
std::make_unique<AudioParameterFloat>(ParameterID("THRESHOLD",1), "Threshold", NormalisableRange<float> { -40.0f, 6.0f, .01f }, 0.f),
std::make_unique<AudioParameterInt>(ParameterID("TYPE",1), "Type", 1, 4, 1)


}
    )
#endif
{
}

EZDistortionAudioProcessor::~EZDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String EZDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EZDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EZDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EZDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EZDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EZDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EZDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EZDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EZDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void EZDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EZDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void EZDistortionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EZDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void EZDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto& mix = *apvts.getRawParameterValue("MIX");
    auto& gain = *apvts.getRawParameterValue("GAIN");
    auto& type = *apvts.getRawParameterValue("TYPE");
    auto& thresh = *apvts.getRawParameterValue("THRESHOLD");
    int typeInt = (int) type;
    auto gainDigital = juce::Decibels::decibelsToGain((float) gain);
    auto threshDigital = juce::Decibels::decibelsToGain((float) thresh);
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            float newSample = 0;
            float sampleValue = buffer.getSample(channel, sample);
            float gainSample = sampleValue * (1+gainDigital);
            float newGain = 0;
            float gainSample1 = 0;
            float foldThresh = 0;
            float foldRatio = 0;
        
            switch (typeInt) {
                case 1:
                    if (gainSample <= -threshDigital)
                        newSample = -2/3;
                    else if (gainSample > -threshDigital && gainSample < threshDigital)
                        newSample = gainSample - (gainSample * gainSample * gainSample)/3;
                    else if (gainSample <= threshDigital)
                        newSample = 2/3;
                    break;
                case 2:
                    if (gainSample <= -threshDigital)
                        newSample = -1;
                    else if (gainSample > -threshDigital && gainSample < threshDigital)
                        newSample = gainSample;
                    else if (gainSample >= threshDigital)
                        newSample = 1;
                    break;
                case 3:
                    newSample = foldback(gainSample, threshDigital);
                    break;
                case 4:
                    foldThresh = threshDigital * 10;
                    newGain = gain * foldThresh;
                    gainSample1 = sampleValue * newGain;
                    foldRatio = 1 / foldThresh;
                    newSample = foldRatio * (abs(foldRatio * gainSample1 + foldRatio - round(foldRatio * gainSample1 + foldRatio) - .25));
                    break;
                default:
                    break;
            }
            
            buffer.setSample(channel, sample, sampleValue * (1-mix) + newSample * mix);
        }
    }
}

//==============================================================================
bool EZDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EZDistortionAudioProcessor::createEditor()
{
    return new EZDistortionAudioProcessorEditor (*this);
}

//==============================================================================
void EZDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EZDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EZDistortionAudioProcessor();
}
