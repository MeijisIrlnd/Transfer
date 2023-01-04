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
    void contextChangedInternal(const std::string& newText);
    void onLabelButtonClicked(LabelButton* l) override;

private:
    class SizeController : public juce::Component, public LabelButton::Listener
    {
    public: 
        SizeController();
        ~SizeController() override;
        void paint(juce::Graphics& g) override;
        void resized() override;
        void onLabelButtonClicked(LabelButton* l) override;

    private:
        juce::Label m_sizeLabel;
        LabelButton m_smallButton, m_mediumButton, m_largeButton;
        std::array<LabelButton*, 3> m_buttons;
        std::array<std::tuple<int, int>, 3> m_sizeOpts;
    } m_sizeButtons;

    juce::AudioProcessorValueTreeState& m_tree;
    juce::ComponentBoundsConstrainer m_constrainer;
    TitleLF m_titleLF;
    InputLF m_inputLF;
    juce::Label m_expressionLabel, m_hxLabel;
    juce::TextEditor m_expressionInput;
    juce::TextEditor m_helpBlock;
    
    juce::Label m_distortionCoefficientLabel, m_yLabel, m_zLabel;
    juce::Slider m_distortionCoefficientSlider, m_ySlider, m_zSlider;
    std::unique_ptr<juce::SliderParameterAttachment> m_coeffAttachment, m_yAttachment, m_zAttachment;
    TransferAudioProcessor& m_audioProcessor;
    LF m_lookAndFeel;
    Graphing<double> m_graphing;
    GatePanel m_gatePanel;
    Transfer::UI::FilterPanel m_filterPanel;
    LabelButton m_filterButton, m_graphButton, m_gateButton, m_registerClearButton, m_panicButton;
    std::stringstream m_displayText;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransferAudioProcessorEditor)
};
