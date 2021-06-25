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
        setColour(juce::Label::textColourId, juce::Colours::black);
        setColour(juce::TextEditor::outlineColourId, juce::Colour(200, 200, 200));
        font = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::DroidSansMono_ttf, BinaryData::DroidSansMono_ttfSize));
    }

    juce::Font getLabelFont(juce::Label& label) override { return font; }
    
    juce::Font& getFont() { return font; }
private: 
    juce::Font font;
};