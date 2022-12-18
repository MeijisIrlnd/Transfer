/*
  ==============================================================================

    LF.h
    Created: 24 Jun 2021 9:16:51pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LF : public juce::LookAndFeel_V4
{
public: 
    LF()
    {
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(200, 200, 200));
        setColour(juce::TextEditor::backgroundColourId, juce::Colour(200, 200, 200));
        setColour(juce::TextEditor::textColourId, juce::Colours::black);
        setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentWhite);
        setColour(juce::Label::textColourId, juce::Colours::black);
        setColour(juce::TextEditor::outlineColourId, juce::Colour(200, 200, 200));
        setColour(juce::TextEditor::highlightColourId, juce::Colour(100, 100, 100));
        
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(200, 200, 200));
        setColour(juce::ComboBox::textColourId, juce::Colour(juce::Colours::black));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(200, 200, 200));
        setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, juce::Colour(200, 200, 200));
        setColour(juce::PopupMenu::ColourIds::textColourId, juce::Colours::black);
        
        setColour(juce::CaretComponent::ColourIds::caretColourId, juce::Colour(0xFF7F7F7F));

        font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::DroidSansMono_ttf, BinaryData::DroidSansMono_ttfSize));
        //font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::agreepersonaluse_regular_otf, BinaryData::agreepersonaluse_regular_otfSize));
    }

    ~LF() override { }
    virtual juce::Font getLabelFont(juce::Label& label) override { return font; }
    juce::Font getComboBoxFont(juce::ComboBox& source) override { return font.withHeight(source.getHeight()); }
    juce::Font getPopupMenuFont() override { return font; }
    juce::Font& getFont() { return font; }

    static inline void instantiateHorizontalSlider(juce::Component* parent, juce::Slider* s, juce::Label* l, const juce::String& labelText)
    {
        l->setJustificationType(juce::Justification::centredTop);
        l->setText(labelText, juce::dontSendNotification);
        parent->addAndMakeVisible(l);
        s->setSliderStyle(juce::Slider::SliderStyle::LinearBar);
        s->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        parent->addAndMakeVisible(s);
    }

protected: 
    juce::Font font;
};

class TitleLF : public juce::LookAndFeel_V4
{
public:
    TitleLF()
    {
        setColour(juce::Label::ColourIds::textColourId, juce::Colour(0x237F7F7F));
    }

    ~TitleLF() override { }
    juce::Font getLabelFont(juce::Label& label) override { 
        return juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::WetdreamzmediumLYxZ_otf, BinaryData::WetdreamzmediumLYxZ_otfSize)).withHeight(label.getHeight());
    }
};

class InputLF : public LF
{
public: 
    InputLF()
    {
        setColour(juce::TextEditor::outlineColourId, juce::Colour(100, 100, 100));
        setColour(juce::TextEditor::highlightColourId, juce::Colour(100, 100, 100));
        setColour(juce::TextEditor::ColourIds::focusedOutlineColourId, juce::Colours::black);
    }

    ~InputLF() override {}
    
};