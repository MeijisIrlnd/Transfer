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
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(200, 200, 200));
        setColour(juce::ComboBox::textColourId, juce::Colour(juce::Colours::black));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(200, 200, 200));
        setColour(juce::PopupMenu::ColourIds::highlightedTextColourId, juce::Colour(200, 200, 200));
        setColour(juce::PopupMenu::ColourIds::textColourId, juce::Colours::black);
        font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::DroidSansMono_ttf, BinaryData::DroidSansMono_ttfSize));
    }

    ~LF() override { }
    virtual juce::Font getLabelFont(juce::Label& label) override { return font; }
    juce::Font getComboBoxFont(juce::ComboBox& source) override { return font.withHeight(source.getHeight()); }
    juce::Font getPopupMenuFont() override { return font; }
    juce::Font& getFont() { return font; }
protected: 
    juce::Font font;
};

class TitleLF : public LF
{
public:
    TitleLF()
    {
        setColour(juce::Label::ColourIds::textColourId, juce::Colour(100, 100, 100));
    }

    ~TitleLF() override { }
    juce::Font getLabelFont(juce::Label& label) override { return font.withHeight(label.getHeight() / 2.0f).withExtraKerningFactor(-.05); }
};

class InputLF : public LF
{
public: 
    InputLF()
    {
        setColour(juce::TextEditor::outlineColourId, juce::Colour(100, 100, 100));
    }

    ~InputLF() override {}
};