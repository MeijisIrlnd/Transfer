/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CommandLineDistortionAudioProcessor::CommandLineDistortionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
     ), parameterTree(*this, nullptr, juce::Identifier("CMDistortion"), {
         std::make_unique<juce::AudioParameterFloat>("coeff", "Distortion Coefficient", 1, 10, 1),
         std::make_unique<juce::AudioParameterFloat>("z", "Z", 0, 1, 0)
})
#endif
{
    ctx.addBuiltIns();
    juce::ValueTree subTree("StringParams");
    subTree.setProperty("function", "", nullptr);
    parameterTree.state.addChild(subTree, 1, nullptr);
}

CommandLineDistortionAudioProcessor::~CommandLineDistortionAudioProcessor()
{
}

//==============================================================================
const juce::String CommandLineDistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CommandLineDistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CommandLineDistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CommandLineDistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CommandLineDistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CommandLineDistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CommandLineDistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CommandLineDistortionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CommandLineDistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void CommandLineDistortionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CommandLineDistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    krunch.prepare(samplesPerBlock, sampleRate);
}

void CommandLineDistortionAudioProcessor::releaseResources()
{
    krunch.release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CommandLineDistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void CommandLineDistortionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    juce::AudioSourceChannelInfo bufferToFill(buffer);
    krunch.getNextAudioBlock(bufferToFill);
}

//==============================================================================
bool CommandLineDistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CommandLineDistortionAudioProcessor::createEditor()
{
    return new CommandLineDistortionAudioProcessorEditor (*this);
}

//==============================================================================
void CommandLineDistortionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameterTree.copyState();
    std::unique_ptr<juce::XmlElement> xmlState(state.createXml());
    copyXmlToBinary(*xmlState, destData);
}

void CommandLineDistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CommandLineDistortionAudioProcessor();
}

void CommandLineDistortionAudioProcessor::setContext(const std::string& expr)
{
    parameterTree.state.getChild(1).setProperty("function", juce::String(expr), nullptr);
    context.reset(new Expression(ctx, expr, currentCoefficient, currentZ));
    krunch.setShapingFunction(context->getTransferFunction());
}

mathpresso::Expression* CommandLineDistortionAudioProcessor::getMathExpr()
{
    if (context != nullptr) {
        return context->getExpr();
    }
    else {
        return nullptr;
    }
}

void CommandLineDistortionAudioProcessor::setDistortionCoefficient(double newCoefficient)
{
    currentCoefficient = newCoefficient;

    if(context != nullptr) context->setDistortionCoefficient(newCoefficient);
}

void CommandLineDistortionAudioProcessor::setZ(double newZ)
{
    currentZ = newZ;
    if (context != nullptr) context->setZ(newZ);
}