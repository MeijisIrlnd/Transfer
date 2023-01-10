/*
  ==============================================================================

    ExprtkTokeniser.cpp
    Created: 8 Jan 2023 5:12:27pm
    Author:  Syl

  ==============================================================================
*/

#include "ExprtkTokeniser.h"

ExprtkTokeniser::~ExprtkTokeniser()
{
}

int ExprtkTokeniser::readNextToken(juce::CodeDocument::Iterator& source)
{
    source.skipWhitespace();
    bool wasIdent = false;
    auto firstChar = source.peekNextChar();
    switch (firstChar)
    {
    case ';': 
        source.skip();
        // source dude just trust me
        return static_cast<int>(TOKEN_TYPE::CONTROL_FLOW);
    case '*': case '/': case '+': case '-': 
    case '%': case '^': case '&': case '!':
    case '|': case '=': case ':': case '<':
    case '>':
        source.skip();
        return static_cast<int>(TOKEN_TYPE::OPERATOR);
    case '(': case ')': case '[': case ']':
        source.skip();
        return static_cast<int>(TOKEN_TYPE::BRACKET);
    default: 
        if (isIdentifierStart(firstChar))
            return parseIdentifier(source);
        source.skip();
        if (isNumeral(firstChar)) {
            return static_cast<int>(TOKEN_TYPE::NUMERAL);
        }
        return static_cast<int>(TOKEN_TYPE::ERROR_TYPE);
    }
}

juce::CodeEditorComponent::ColourScheme ExprtkTokeniser::getDefaultColourScheme()
{
    juce::CodeEditorComponent::ColourScheme cs;
    struct Type {
        const char* name;
        std::uint32_t colour;
    };
    const Type mapping[] = {
        {"Error", 0xFF230000},
        {"Variable", 0xFF626262},
        {"Bracket", 0xFF232323}, // LIGHTER
        {"Function", 0xFF003274}, // BLUE ME 
        {"Numeral", 0xFF234124},
        {"Operator", 0xFF000000}, // BLACK ME 
        {"Control Flow", 0xFF5C2F5B}
    };
    for (auto &el : mapping)
    {
        cs.set(el.name, juce::Colour(el.colour));
    }
    return cs;
}

juce::CodeEditorComponent::ColourScheme ExprtkTokeniser::getErrorColourScheme()
{
    juce::CodeEditorComponent::ColourScheme cs;
    struct Type {
        const char* name;
        std::uint32_t colour;
    };
    const Type mapping[] = {
        {"Error", 0xFF230000},
        {"Variable", 0xFFA0A0A0},
        {"Bracket", 0xFF232323}, // LIGHTER
        {"Function", 0xFF003274}, // BLUE ME 
        {"Numeral", 0xFF234124},
        {"Operator", 0xFFFFFFFF}, // BLACK ME 
        {"Control Flow", 0xFFD89ED7}
        // NUMBERS GREEN
    };
    for (auto& el : mapping) {
        cs.set(el.name, juce::Colour(0xFF7F0000));
    }
    return cs;
}

bool ExprtkTokeniser::isNumeral(juce::juce_wchar c) noexcept
{
    return juce::CharacterFunctions::isDigit(c);
}

bool ExprtkTokeniser::isNamedOperator(juce::String::CharPointerType token, const int tokenLength) noexcept
{
    const std::vector<juce::String> operators = {
        "and", "nand", "mand", "nor", "xor", "xnor", "not"
    };
    auto it = std::find(operators.begin(), operators.end(), juce::String(token, tokenLength));
    return it != operators.end();
}

bool ExprtkTokeniser::isVariable(juce::String::CharPointerType token, const int tokenLength) noexcept
{
    const std::vector<juce::String> variables = {
        "x", "y", "z", "d", "gpr"
    };
    auto it = std::find(variables.begin(), variables.end(), juce::String(token, tokenLength));
    return it != variables.end();
}

bool ExprtkTokeniser::isControlFlow(juce::String::CharPointerType token, const int tokenLength) noexcept
{
    const std::vector<juce::String> controlFlowItems = {
        "if", "else", ";", "~"
    };
    auto it = std::find(controlFlowItems.begin(), controlFlowItems.end(), juce::String(token, tokenLength));
    return it != controlFlowItems.end();
}

bool ExprtkTokeniser::isReservedKeyword(juce::String::CharPointerType token, const int tokenLength) noexcept
{
    //and, nand, mand, &, nor, xor, |, nor, xnor, not
    const std::vector<const char*> functions = {
        "abs", "avg", "ceil", "clamp", "equal", "erf", "erfc", "exp", "expm1", "floor", "frac", "hypot", "iclamp",
        "inrange", "log", "log10", "log1p", "log2", "logn", "max", "min", "mul", "ncdf", "not_equal", "pow", "root",
        "round", "roundn", "sgn", "sqrt", "sum", "swap", "trunc",
        "sin", "cos", "tan", "asin", "acos", "atan", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh", "atan2", "sinc", "sec", "cot", "csc", "deg2rad",
        "deg2grad", "rad2deg", "grad2deg"
    };
    juce::String current(token, tokenLength);
    auto it = std::find(functions.begin(), functions.end(), current);
    return it != functions.end();
}

int ExprtkTokeniser::parseIdentifier(juce::CodeDocument::Iterator& source) noexcept
{
    int tokenLength = 0;
    juce::String::CharPointerType::CharType possibleIdentifier[100] = {};
    juce::String::CharPointerType possible(possibleIdentifier);

    while (isIdentifierBody(source.peekNextChar()))
    {
        auto c = source.nextChar();

        if (tokenLength < 20)
            possible.write(c);

        ++tokenLength;
    }

    if (tokenLength > 1 && tokenLength <= 16)
    {
        possible.writeNull();

        if (isReservedKeyword(juce::String::CharPointerType(possibleIdentifier), tokenLength)) {
            return static_cast<int>(TOKEN_TYPE::FUNCTION);
        }
    }
    if (isNamedOperator(juce::String::CharPointerType(possibleIdentifier), tokenLength)) {
        return static_cast<int>(TOKEN_TYPE::OPERATOR);
    }

    if (isControlFlow(juce::String::CharPointerType(possibleIdentifier), tokenLength)) {
        return static_cast<int>(TOKEN_TYPE::CONTROL_FLOW);
    }
    
    if (isVariable(juce::String::CharPointerType(possibleIdentifier), tokenLength))
    {
        return static_cast<int>(TOKEN_TYPE::VARIABLE);
    }

    return static_cast<int>(TOKEN_TYPE::ERROR_TYPE);
    
}
