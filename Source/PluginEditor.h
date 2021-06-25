/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ExpressionParser.h"
#include "UI/LF.h"
//==============================================================================
/**
*/
class CommandLineDistortionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CommandLineDistortionAudioProcessorEditor (CommandLineDistortionAudioProcessor&);
    ~CommandLineDistortionAudioProcessorEditor() override;
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::Label expressionLabel;
    juce::TextEditor expressionInput, yLabel;
    juce::TextEditor helpBlock;
    juce::Label distortionCoefficientLabel, zLabel;
    juce::Slider distortionCoefficientSlider, zSlider;
    CommandLineDistortionAudioProcessor& audioProcessor;
    LF lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CommandLineDistortionAudioProcessorEditor)
};
