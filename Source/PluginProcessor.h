/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Expression.h"
#include <Audio/Distortion.h>
//==============================================================================
/**
*/
class TransferAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    TransferAudioProcessor();
    ~TransferAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void parameterChanged(const juce::String& id, float newValue) override;
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

    void setContext(const std::string expression);
    void setDistortionCoefficient(double newCoefficient);
    void setY(double newY);
    void setZ(double newZ);
    SDSP_INLINE void clearRegisters() { if (context != nullptr) context->zeroRegisters(); }

private:
    const size_t m_oversamplingFactor{ 4 }; // 16x
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() noexcept;
    juce::AudioProcessorValueTreeState parameterTree;
    juce::dsp::Oversampling<float> m_oversampler;
    std::unique_ptr<Expression<float>> context;
    Transfer::Audio::Distortion m_distortion;
    double currentCoefficient = 1;
    double currentY{ 0 };
    double currentZ = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransferAudioProcessor)
};
