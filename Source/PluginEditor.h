/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/LF.h"
#include "UI/Graphing.h"
#include "UI/GatePanel.h"
#include "UI/LabelButton.h"
#include <UI/FilterPanel.h>
//==============================================================================
/**
*/
class TransferAudioProcessorEditor  : public juce::AudioProcessorEditor, public LabelButton::Listener
{
public:
    TransferAudioProcessorEditor (TransferAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~TransferAudioProcessorEditor() override;
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void onLabelButtonClicked(LabelButton* l) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::AudioProcessorValueTreeState& tree;
    TitleLF titleLF;
    InputLF inputLF;
    juce::Label expressionLabel;
    juce::TextEditor expressionInput, yLabel;
    juce::TextEditor helpBlock;
    
    juce::Label distortionCoefficientLabel, zLabel;
    juce::Slider distortionCoefficientSlider, zSlider;
    std::unique_ptr<juce::SliderParameterAttachment> coeffAttachment, zAttachment;
    TransferAudioProcessor& audioProcessor;
    LF lookAndFeel;
#if defined USE_EXPRTK
    Graphing<double> graphing;
#else
    Graphing graphing;
#endif
    GatePanel gatePanel;
    Transfer::UI::FilterPanel filterPanel;
    LabelButton filterButton, graphButton, gateButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransferAudioProcessorEditor)
};
