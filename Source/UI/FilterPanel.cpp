/*
  ==============================================================================

    FilterPanel.cpp
    Created: 11 Dec 2022 3:58:14am
    Author:  Syl

  ==============================================================================
*/

#include "FilterPanel.h"

namespace Transfer::UI
{
    FilterPanel::FilterPanel(APVTS& tree) : m_tree(tree)
    {
        instantiateComboBox(&m_filterTypeSelector, &m_filterTypeLabel, { "Low Shelf", "High Shelf" }, "Filter Type");
        m_filterTypeAttachment.reset(new juce::ComboBoxParameterAttachment(*tree.getParameter("EmphasisFilterType"), m_filterTypeSelector));
        instantiateSlider(&m_filterCutoffSlider, &m_filterCutoffLabel, "Cutoff", "Hz");
        m_cutoffAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("EmphasisFilterCutoff"), m_filterCutoffSlider));
        instantiateSlider(&m_filterSlopeSlider, &m_filterSlopeLabel, "Slope", "dB/8ve");
        m_slopeAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("EmphasisFilterSlope"), m_filterSlopeSlider));
        instantiateSlider(&m_filterGainSlider, &m_filterGainLabel, "Gain", "dB");
        m_gainAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("EmphasisFilterGain"), m_filterGainSlider));
    }

    FilterPanel::~FilterPanel()
    {
    }

    void FilterPanel::paint(juce::Graphics& g)
    {
        juce::Rectangle<int> bounds(0, 0, getWidth(), getHeight());
        g.drawFittedText(juce::String("Emphasis Filter"), juce::Rectangle<int>(0, 0, getWidth(), getHeight() / 8), juce::Justification::centred, 1);
        g.drawRect(bounds);
    }

    void FilterPanel::resized()
    {
        m_filterTypeLabel.setBounds(0, getHeight() / 8 + 1, getWidth() / 4, getHeight() / 10);
        m_filterTypeSelector.setBounds(getWidth() / 4, getHeight() / 8 + 1, getWidth() - getWidth() / 4 - 1, getHeight() / 10);
        m_filterCutoffLabel.setBounds(0, getHeight() / 4 + 1 + getHeight() / 8, getWidth() / 4, getHeight() / 10);
        m_filterCutoffSlider.setBounds(getWidth() / 4, m_filterCutoffLabel.getY(), getWidth() - getWidth() / 4 - 1, getHeight() / 10);
        m_filterSlopeLabel.setBounds(0, getHeight() / 2 + 1 + getHeight() / 8, getWidth() / 4, getHeight() / 10);
        m_filterSlopeSlider.setBounds(getWidth() / 4, m_filterSlopeLabel.getY(), getWidth() - getWidth() / 4 - 1, getHeight() / 10);
        m_filterGainLabel.setBounds(0, (getHeight() / 4) * 3 + 1 + getHeight() / 8, getWidth() / 4, getHeight() / 10);
        m_filterGainSlider.setBounds(getWidth() / 4, m_filterGainLabel.getY(), getWidth() - getWidth() / 4 - 1, getHeight() / 10);
    }

    void FilterPanel::instantiateSlider(juce::Slider* s, juce::Label* l, std::string labelText, std::string units)
    {
        l->setText(labelText, juce::dontSendNotification);
        addAndMakeVisible(l);
        s->setSliderStyle(juce::Slider::LinearBar);
        s->setTextBoxStyle(juce::Slider::TextBoxRight, true, 50, 30);
        s->setTextValueSuffix(units);
        addAndMakeVisible(s);
    }

    void FilterPanel::instantiateComboBox(juce::ComboBox* c, juce::Label* l, const juce::StringArray& opts, std::string labelText)
    {
        l->setText(labelText, juce::dontSendNotification);
        addAndMakeVisible(l);
        c->addItemList(opts, 1);
        c->setSelectedItemIndex(0, juce::dontSendNotification);
        addAndMakeVisible(c);
    }
}