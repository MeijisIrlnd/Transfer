/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Expression.h"
#include <Audio/Distortion.h>
#include <Utils/ErrorReporter.h>
//==============================================================================
/**
*/
using namespace Transfer;
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
    void setOversamplingFactor(const size_t newFactor);
    void setSize(const int sizeIndex);
private:
    ErrorReporter m_errorReporter;
    size_t m_oversamplingFactor{ 4 }; // 16x
    std::atomic_bool m_needsPrepare{ false };
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() noexcept;
    juce::AudioProcessorValueTreeState parameterTree;
    std::unique_ptr<juce::dsp::Oversampling<float> > m_oversampler{ nullptr };
    std::unique_ptr<Transfer::Expression<float>> context{nullptr};
    Transfer::Audio::Distortion m_distortion;
    double currentCoefficient = 1;
    double currentY{ 0 };
    double currentZ = 0;
    struct BufferSettings {
        int samplesPerBlockExpected;
        double sampleRate;
    } m_currentBufferSettings;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransferAudioProcessor)
};
