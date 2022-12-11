/*
  ==============================================================================

    GatePanel.h
    Created: 2 Sep 2021 3:28:01am
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class GatePanel : public juce::Component
{
public: 
    GatePanel(juce::AudioProcessorValueTreeState& t) : tree(t)
    {
        instantiateSlider(&thresholdSlider, &thresholdLabel, "Threshold", "dB");
        thresholdAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("Threshold"), thresholdSlider));
        instantiateSlider(&ratioSlider, &ratioLabel, "Ratio", ":1");
        ratioAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("Ratio"), ratioSlider));
        instantiateSlider(&attackSlider, &attackLabel, "Attack", "s");
        attackAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("Attack"), attackSlider));
        instantiateSlider(&releaseSlider, &releaseLabel, "Release", "s");
        releaseAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("Release"), releaseSlider));
    }

    ~GatePanel() override
    {

    }

    void instantiateSlider(juce::Slider* s, juce::Label* l, std::string labelText, std::string units = "")
    {
        l->setText(labelText, juce::dontSendNotification);
        
        addAndMakeVisible(l);
        s->setSliderStyle(juce::Slider::LinearBar);
        s->setTextBoxStyle(juce::Slider::TextBoxRight, true, 50, 30);
        s->setTextValueSuffix(units);
        addAndMakeVisible(s);
    }

    void paint(juce::Graphics& g) override
    {
        juce::Rectangle<int> bounds(0, 0, getWidth(), getHeight());
        g.drawFittedText(juce::String("Gate"), juce::Rectangle<int>(0, 0, getWidth(), getHeight() / 8), juce::Justification::centred, 1);
        g.drawRect(bounds);
    }

    void resized() override
    {

        thresholdLabel.setBounds(0, getHeight() / 8 + 1, getWidth() / 4, getHeight() / 10);
        thresholdSlider.setBounds(getWidth() / 4, getHeight() / 8 + 1, getWidth() - getWidth() / 4 - 1, getHeight() / 10);
        ratioLabel.setBounds(0, getHeight() / 4 + 1 + getHeight() / 8, getWidth() / 4 , getHeight() / 10);
        ratioSlider.setBounds(getWidth() / 4, ratioLabel.getY(), getWidth() - getWidth() / 4 - 1, getHeight() / 10);
        attackLabel.setBounds(0, getHeight() / 2 + 1 + getHeight() / 8, getWidth() / 4, getHeight() / 10);
        attackSlider.setBounds(getWidth() / 4, attackLabel.getY(), getWidth() - getWidth() / 4 - 1, getHeight() / 10);
        releaseLabel.setBounds(0, (getHeight() / 4) * 3 + 1 + getHeight() / 8, getWidth() / 4, getHeight() / 10);
        releaseSlider.setBounds(getWidth() / 4, releaseLabel.getY(), getWidth() - getWidth() / 4 - 1, getHeight() / 10);
    }

private: 
    juce::AudioProcessorValueTreeState& tree;
    juce::Label thresholdLabel, ratioLabel, attackLabel, releaseLabel;
    juce::Slider thresholdSlider, ratioSlider, attackSlider, releaseSlider;
    std::unique_ptr<juce::SliderParameterAttachment> thresholdAttachment, ratioAttachment, attackAttachment, releaseAttachment;
};