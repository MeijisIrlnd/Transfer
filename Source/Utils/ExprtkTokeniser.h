/*
  ==============================================================================

    ExprtkTokeniser.h
    Created: 8 Jan 2023 5:12:27pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class ExprtkTokeniser : public juce::CodeTokeniser
{
public: 
    ~ExprtkTokeniser() override;
    int readNextToken(juce::CodeDocument::Iterator& source) override;
    juce::CodeEditorComponent::ColourScheme getDefaultColourScheme() override;
    juce::CodeEditorComponent::ColourScheme getErrorColourScheme();
private: 
    static int parseIdentifier(juce::CodeDocument::Iterator& source) noexcept;
    static bool isIdentifierStart(const juce::juce_wchar c) noexcept
    {
        return juce::CharacterFunctions::isLetter(c)
            || c == '_' || c == '@';
    }
    static bool isIdentifierBody(const juce::juce_wchar c) noexcept
    {
        return juce::CharacterFunctions::isLetterOrDigit(c)
            || c == '_' || c == '@';
    }
    static bool isNumeral(juce::juce_wchar c) noexcept;
    static bool isNamedOperator(juce::String::CharPointerType token, const int tokenLength) noexcept;
    static bool isVariable(juce::String::CharPointerType token, const int tokenLength) noexcept;
    static bool isControlFlow(juce::String::CharPointerType token, const int tokenLength) noexcept;
    static bool isReservedKeyword(juce::String::CharPointerType token, const int tokenLength) noexcept;

    enum class TOKEN_TYPE
    {
        ERROR_TYPE, 
        VARIABLE,
        BRACKET,
        FUNCTION,
        IDENTIFIER,
        OPERATOR,
        CONTROL_FLOW
    };


};