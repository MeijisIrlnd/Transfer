/*
  ==============================================================================

    FilterPanel.h
    Created: 11 Dec 2022 3:58:14am
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <SDSP/SDSP.h>

namespace Transfer::UI
{
    class FilterPanel : public juce::Component
    {
    public: 
        FilterPanel(APVTS& tree);
        ~FilterPanel() override;
        void paint(juce::Graphics& g) override;
        void resized() override;
        void instantiateSlider(juce::Slider* s, juce::Label* l, std::string labelText, std::string units = "");
        void instantiateComboBox(juce::ComboBox* c, juce::Label* l, const juce::StringArray& opts, std::string labelText);
    private: 
        APVTS& m_tree;
        juce::Label m_filterTypeLabel, m_filterCutoffLabel, m_filterSlopeLabel, m_filterGainLabel;
        juce::ComboBox m_filterTypeSelector;
        juce::Slider m_filterCutoffSlider, m_filterSlopeSlider, m_filterGainSlider;
        std::unique_ptr<juce::ComboBoxParameterAttachment> m_filterTypeAttachment;
        std::unique_ptr<juce::SliderParameterAttachment> m_cutoffAttachment, m_slopeAttachment, m_gainAttachment;
    };
}