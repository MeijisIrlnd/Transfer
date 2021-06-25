/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Expression.h"
#include "KrunchProcessor.h"
//==============================================================================
/**
*/
class CommandLineDistortionAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CommandLineDistortionAudioProcessor();
    ~CommandLineDistortionAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void setContext(const std::string& expression);
    void setDistortionCoefficient(double newCoefficient);
    void setZ(double newZ);
private:
    mathpresso::Context ctx;
    std::unique_ptr<Expression> context;
    DSPCommon::KrunchProcessor<float> krunch;
    double currentCoefficient = 1;
    double currentZ = 0;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CommandLineDistortionAudioProcessor)
};
