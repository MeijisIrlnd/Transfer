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
    ), 
#else 
    : 
#endif
    parameterTree(*this, nullptr, juce::Identifier("Transfer"), createParameterLayout()), m_distortion(parameterTree)
{
    //context.reset(new Expression<float>("x", 0, 0));
    context.reset(new Expression<float>("x", 0, 0, m_errorReporter));
    parameterTree.addParameterListener("D", this);
    parameterTree.addParameterListener("Y", this);
    parameterTree.addParameterListener("Z", this);
}

TransferAudioProcessor::~TransferAudioProcessor()
{
}

void TransferAudioProcessor::parameterChanged(const juce::String& id, float newValue)
{
    if (id == "D") setDistortionCoefficient(newValue);
    if (id == "Y") setY(newValue);
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
    if(m_oversamplingFactor != 0){
        m_oversampler.reset(new juce::dsp::Oversampling<float>(getTotalNumOutputChannels(), m_oversamplingFactor, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple));
        m_oversampler->setUsingIntegerLatency(true);
        m_oversampler->initProcessing(samplesPerBlock);
        setLatencySamples(m_oversampler->getLatencyInSamples());
    }
    m_distortion.prepareToPlay(samplesPerBlock, m_oversamplingFactor == 0 ? sampleRate : sampleRate * m_oversamplingFactor);
    m_currentBufferSettings = {samplesPerBlock, sampleRate};
    m_needsPrepare.store(false);
}

void TransferAudioProcessor::releaseResources()
{
    m_distortion.releaseResources();
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
    if(m_needsPrepare.load()) {
        prepareToPlay(m_currentBufferSettings.sampleRate, m_currentBufferSettings.samplesPerBlockExpected);
    }

    juce::dsp::AudioBlock<float> block{ buffer };
    if(m_oversamplingFactor != 0){
        auto oversampled = m_oversampler->processSamplesUp(block);
        m_distortion.getNextAudioBlock(oversampled);
        m_oversampler->processSamplesDown(block);
    }
    else {
        m_distortion.getNextAudioBlock(block);
    }
    
}

//==============================================================================
bool TransferAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TransferAudioProcessor::createEditor()
{
    auto* editor = new TransferAudioProcessorEditor(*this, parameterTree, m_errorReporter);
    m_errorReporter.setEditor(editor);
    return editor;
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
        tree.setProperty("Size", juce::var(1), nullptr);
        tree.setProperty("OversamplingFactor", juce::var(4), nullptr);
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
                tree.setProperty("OversamplingFactor", juce::var(4), nullptr);
                tree.setProperty("Size", juce::var(1), nullptr);
                currentTree.addChild(tree, -1, nullptr);
            }
            parameterTree.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
    setOversamplingFactor(static_cast<size_t>(static_cast<int>(parameterTree.state.getChildWithName("Internal").getProperty("OversamplingFactor"))));
    std::string transferStr = parameterTree.state.getChildWithName("Internal").getProperty("Function").toString().toStdString();
    transferStr = transferStr == "" ? "x" : transferStr;
    setContext(transferStr);
    // if the UI exists, update its context too...
    if (getActiveEditor() != nullptr) {
        TransferAudioProcessorEditor* e = dynamic_cast<TransferAudioProcessorEditor*>(getActiveEditor());
        e->contextChangedInternal(transferStr);
        // And its size...
        // ...
        // ...
    }
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
    m_distortion.setShapingFunction(context->getTransferFunction());
}

void TransferAudioProcessor::setDistortionCoefficient(double newCoefficient)
{
    currentCoefficient = newCoefficient;
    if(context != nullptr) context->setDistortionCoefficient(newCoefficient);
}

void TransferAudioProcessor::setY(double newY)
{
    currentY = newY;
    if (context != nullptr) context->setY(newY);
}


void TransferAudioProcessor::setZ(double newZ)
{
    currentZ = newZ;
    if (context != nullptr) context->setZ(newZ);
}

void TransferAudioProcessor::setOversamplingFactor(const size_t newFactor) {
    m_oversamplingFactor = newFactor;
    m_needsPrepare.store(true);
    // Write this to the tree also..
    auto state = parameterTree.state;
    if(!state.getChildWithName("Internal").isValid()) {
        juce::Identifier ident("Internal");
        juce::ValueTree currentTree(ident);
        parameterTree.state.addChild(currentTree, -1, nullptr);
    }
    state.getChildWithName("Internal").setProperty("OversamplingFactor", juce::var(static_cast<int>(newFactor)), nullptr);
}

void TransferAudioProcessor::setSize(const int sizeIndex) {
    auto state = parameterTree.state;
    if(!state.getChildWithName("Internal").isValid()) {
        juce::Identifier ident("Internal");
        juce::ValueTree currentTree(ident);
        parameterTree.state.addChild(currentTree, -1, nullptr);
    }
    state.getChildWithName("Internal").setProperty("Size", juce::var(sizeIndex), nullptr);
}

juce::AudioProcessorValueTreeState::ParameterLayout TransferAudioProcessor::createParameterLayout() noexcept
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    using FloatParam = juce::AudioParameterFloat;
    using ChoiceParam = juce::AudioParameterChoice;
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"D",1}, "D", juce::NormalisableRange<float>(0, 10, 0.001), 0));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{ "Y", 1 }, "Y", juce::NormalisableRange<float>(0, 1, 0.01), 0));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Z",1}, "Z", juce::NormalisableRange<float>(0, 1, 0.001), 0));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Threshold", 1}, "Threshold", -100, 0, -10));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Ratio", 1}, "Ratio", 1.5f, 20, 2));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Attack", 1}, "Attack", juce::NormalisableRange<float>(1e-5f, 0.5f, 1e-5f), 1e-5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"Release", 1}, "Release", juce::NormalisableRange<float>(1e-5f, 0.5f, 1e-5f), 1e-5f));
    layout.add(std::make_unique<ChoiceParam>(juce::ParameterID{"EmphasisFilterType", 1}, "EmphasisFilterType", juce::StringArray("Low Shelf", "High Shelf", "Peak"), 0));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"EmphasisFilterCutoff", 1}, "EmphasisFilterCutoff", juce::NormalisableRange<float>(20, 20000, 0.01f, 0.5f), 1000));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"EmphasisFilterSlope", 1}, "EmphasisFilterSlope", juce::NormalisableRange<float>(0.01f, 1, 0.01f), 0.5f));
    layout.add(std::make_unique<FloatParam>(juce::ParameterID{"EmphasisFilterGain", 1}, "EmphasisFilterGain", juce::NormalisableRange<float>(-12, 12, 0.01f), 0.0f));
    
    return layout;
}
