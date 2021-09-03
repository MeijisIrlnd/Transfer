/*
  ==============================================================================

    LabelButton.h
    Created: 2 Sep 2021 3:47:50am
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LabelButton : public juce::Component
{
public: 
    struct Listener
    {
        virtual ~Listener() {} 
        virtual void onLabelButtonClicked(LabelButton* l) = 0;
    };

    LabelButton(std::string txt, const juce::Colour& c, bool d = false) : colour(c), drawOutline(d)
    {
        l.setText(txt, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(l);
    }

    void addListener(Listener* newListener) { pListener = newListener; }

    void mouseUp(const juce::MouseEvent& ev) override
    {
        if (pListener != nullptr) pListener->onLabelButtonClicked(this);
    }

    void paint(juce::Graphics& g) override
    {
        if (drawOutline) g.drawRect(juce::Rectangle<float>(0, 0, getWidth(), getHeight()));
    }

    void resized() override
    {
        l.setBounds(0, 0, getWidth(), getHeight());
    }

private: 
    juce::Label l;
    juce::Colour colour;
    bool drawOutline = false;
    Listener* pListener = nullptr;
};