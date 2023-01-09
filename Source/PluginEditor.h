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

#include <Utils/ExprtkTokeniser.h>
#include <UI/TransferCodeEditor.h>
#include <Utils/ErrorReporter.h>

//==============================================================================
/**
*/
class TransferAudioProcessorEditor  : public juce::AudioProcessorEditor, public LabelButton::Listener, public juce::CodeDocument::Listener
{
public:
    TransferAudioProcessorEditor (TransferAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~TransferAudioProcessorEditor() override;
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void contextChangedInternal(const std::string& newText);
    void onLabelButtonClicked(LabelButton* l) override;
    void codeDocumentTextInserted(const juce::String& newText, int insertIndex) override;
    void codeDocumentTextDeleted(int startIndex, int endIndex) override;
    void showError(const std::vector<error_t>& errors);
    void clearErrors();
private:
    void initialiseGraphingParams();
    void initialiseShownPanel();
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
    juce::TooltipWindow m_tooltipWindow;
    juce::AudioProcessorValueTreeState& m_tree;
    juce::ComponentBoundsConstrainer m_constrainer;
    TitleLF m_titleLF;
    InputLF m_inputLF;
    juce::Label m_expressionLabel, m_hxLabel;
    ExprtkTokeniser m_tokeniser;
    juce::CodeDocument m_document;
    TransferCodeEditor m_codeEditor;
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
