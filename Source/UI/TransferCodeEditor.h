/*
  ==============================================================================

    TransferCodeEditor.h
    Created: 8 Jan 2023 5:11:40pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class TransferCodeEditor : public juce::CodeEditorComponent
{
public: 
    TransferCodeEditor(juce::CodeDocument& document, juce::CodeTokeniser* tokeniser) : 
        juce::CodeEditorComponent(document, tokeniser), m_tokeniser(tokeniser)
    {
        
        setFont(LF::createFont());
        setLineNumbersShown(false);
        setHasFocusOutline(true);
        setScrollbarThickness(3);

    }

    ~TransferCodeEditor() override
    {

    }
    void focusGained(juce::Component::FocusChangeType cause) override
    {
        if (m_errorRenderer.get() != nullptr) {
            m_errorRenderer->setInterceptsMouseClicks(false, false);
        }
        juce::CodeEditorComponent::focusGained(cause);
    }

    void focusLost(juce::Component::FocusChangeType cause) override
    {
        if (m_errorRenderer.get() != nullptr) {
            m_errorRenderer->setInterceptsMouseClicks(true, true);
        }
        juce::CodeEditorComponent::focusLost(cause);
    }

    
    bool pasteFromClipboard() override
    {
        //newTransaction();
        auto clip = juce::SystemClipboard::getTextFromClipboard();
        if (clip.isNotEmpty()) {
            clip = clip.replace("\n", "");
            clip = clip.replace("\r", "");
            insertTextAtCaret(clip);
        }
        //newTransaction();
        return true;
    }
    
    void handleReturnKey() override
    {
        if (onReturnKey != nullptr) {
            onReturnKey();
        }
        unfocusAllComponents();
    }

    std::function<void(void)> onReturnKey{ nullptr };

    void toggleErrorMode(bool hasError, const juce::String& errorText = "")
    {
        auto font = getFont();
        font.setUnderline(hasError);
        setFont(font);
        auto cs = hasError ? dynamic_cast<ExprtkTokeniser*>(m_tokeniser)->getErrorColourScheme() : m_tokeniser->getDefaultColourScheme();
        setColourScheme(cs);
        if (hasError) {
            m_errorRenderer.reset(new ErrorRenderer(errorText));
            addAndMakeVisible(m_errorRenderer.get());
            m_errorRenderer->setBounds(getLocalBounds());
        }
        else {
            removeChildComponent(m_errorRenderer.get());
            m_errorRenderer.reset(nullptr);
        }
    }
private: 
    juce::CodeTokeniser* m_tokeniser;
    struct ErrorRenderer : public juce::Component, public juce::TooltipClient
    {
        ErrorRenderer(const juce::String& text) : m_errorText(text) {
            setInterceptsMouseClicks(true, false);
        }
        
        ~ErrorRenderer() override { }
        void paint(juce::Graphics& g) override { }
        void resized() override { }
        juce::String getTooltip() override { return m_errorText; }
    private: 
        juce::String m_errorText;
    }; std::unique_ptr<ErrorRenderer> m_errorRenderer{ nullptr };
};