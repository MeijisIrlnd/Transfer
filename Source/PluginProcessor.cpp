/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
// TODO: 
// Test on macOS
// Test in DAW
//==============================================================================
TransferAudioProcessor::TransferAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), parameterTree(*this, nullptr, juce::Identifier("CMDistortion"), {
        std::make_unique<juce::AudioParameterFloat>("D", "D", juce::NormalisableRange<float>(0, 10, 0.001), 1),
        std::make_unique<juce::AudioParameterFloat>("Z", "Z", 0, 1, 0),
         std::make_unique<juce::AudioParameterFloat>("Threshold", "Threshold", -100, 0, -10),
         std::make_unique<juce::AudioParameterFloat>("Ratio", "Ratio", 1.5, 20, 2),
         std::make_unique<juce::AudioParameterFloat>("Attack", "Attack", juce::NormalisableRange<float>(1e-5f, 0.5f, 1e-5f), 1e-5f),
         std::make_unique<juce::AudioParameterFloat>("Release", "Release", juce::NormalisableRange<float>(1e-5f, 0.5f, 1e-5f), 1e-5f)
        }), krunch(std::ref(parameterTree))
#else 
    : parameterTree(*this, nullptr, juce::Identifier("CMDistortion"), {
         std::make_unique<juce::AudioParameterFloat>("D", "D", juce::NormalisableRange<float>(0, 10, 0.001), 1),
         std::make_unique<juce::AudioParameterFloat>("Z", "Z", 0, 1, 0) ,
         std::make_unique<juce::AudioParameterFloat>("Threshold", "Threshold", -100, 0, -10),
         std::make_unique<juce::AudioParameterFloat>("Ratio", "Ratio", 1.5f, 20, 2),
         std::make_unique<juce::AudioParameterFloat>("Attack", "Attack", juce::NormalisableRange<float>(1e-5f, 0.5f, 1e-5f), 1e-5f),
         std::make_unique<juce::AudioParameterFloat>("Release", "Release", juce::NormalisableRange<float>(1e-5f, 0.5f, 1e-5f), 1e-5f)
         }), krunch(std::ref(parameterTree))
#endif
{
    context.reset(new Expression<float>("y", 0, 0));
    parameterTree.addParameterListener("D", this);
    parameterTree.addParameterListener("Z", this);
    krunch.setShapingFunction(DSPCommon::DSPShaping::FUNCTION::TANH);
}

TransferAudioProcessor::~TransferAudioProcessor()
{
}

void TransferAudioProcessor::parameterChanged(const juce::String& id, float newValue)
{
    if (id == "D") setDistortionCoefficient(newValue);
    else if (id == "Z") setZ(newValue);
}

//==============================================================================
const juce::String TransferAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TransferAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TransferAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TransferAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TransferAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TransferAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TransferAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TransferAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TransferAudioProcessor::getProgramName (int index)
{
    return {};
}

void TransferAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TransferAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    krunch.prepare(samplesPerBlock, sampleRate);
}

void TransferAudioProcessor::releaseResources()
{
    krunch.release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TransferAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TransferAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
bool TransferAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TransferAudioProcessor::createEditor()
{
    return new TransferAudioProcessorEditor (*this, parameterTree);
}

//==============================================================================
void TransferAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameterTree.copyState();
    if (!state.getChildWithName("Internal").isValid()) {
        juce::Identifier ident("Internal");
        juce::ValueTree tree(ident);
        tree.setProperty("Function", juce::var("x"), nullptr);
        tree.setProperty("Page", juce::var(0), nullptr);
        state.addChild(tree, -1, nullptr);
    }
    std::unique_ptr<juce::XmlElement> xmlState(state.createXml());
    copyXmlToBinary(*xmlState, destData);
}

void TransferAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameterTree.state.getType())) {
            auto currentTree = juce::ValueTree::fromXml(*xmlState);
            if (!currentTree.getChildWithName("Internal").isValid()) {
                juce::Identifier ident("Function");
                juce::ValueTree tree(ident);
                tree.setProperty("Function", juce::var("x"), nullptr);
                tree.setProperty("Page", juce::var(0), nullptr);
                currentTree.addChild(tree, -1, nullptr);
            }
            parameterTree.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
    std::string transferStr = parameterTree.state.getChildWithName("Internal").getProperty("Function").toString().toStdString();
    double d = parameterTree.state.getProperty("D");
    double z = parameterTree.state.getProperty("Z");
    setContext(transferStr);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TransferAudioProcessor();
}

void TransferAudioProcessor::setContext(const std::string expr)
{
    if (parameterTree.state.getChildWithName("Internal").isValid()) {
        parameterTree.state.getChildWithName("Internal").setProperty("Function", juce::var(expr), nullptr);
    } 
    else {
        juce::Identifier ident("Internal");
        juce::ValueTree tree(ident);
        parameterTree.state.addChild(tree, -1, nullptr);
        parameterTree.state.getChildWithName("Internal").setProperty("Function", juce::var(expr), nullptr);

    }
    context->setExpr(expr);
    krunch.setShapingFunction(context->getTransferFunction());
}

void TransferAudioProcessor::setDistortionCoefficient(double newCoefficient)
{
    currentCoefficient = newCoefficient;

    if(context != nullptr) context->setDistortionCoefficient(newCoefficient);
}

void TransferAudioProcessor::setZ(double newZ)
{
    currentZ = newZ;
    if (context != nullptr) context->setZ(newZ);
}