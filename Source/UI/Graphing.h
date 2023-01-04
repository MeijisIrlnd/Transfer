/*
  ==============================================================================

    Graphing.h
    Created: 25 Jun 2021 11:48:26pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <sstream>
#include "LF.h"
#include <atmsp.h>
#include <exprtk.hpp>
#include <SDSP/SDSP.h>

#if defined USE_EXPRTK
template <typename T>
#endif
class Graphing : public juce::Component, juce::AudioProcessorValueTreeState::Listener, public juce::Timer 
{
public:
#if defined USE_EXPRTK
    Graphing<T>(juce::AudioProcessorValueTreeState& t) :
#else 
    Graphing(juce::AudioProcessorValueTreeState& t) :
#endif
    m_tree(t)
    {
        setLookAndFeel(&lookAndFeel);
        dataset.reserve(100);
        t.addParameterListener("D", this);
        t.addParameterListener("Y", this);
        t.addParameterListener("Z", this);
        symbolTable.add_variable("x", currentIp);
        symbolTable.add_variable("d", coeff);
        symbolTable.add_variable("y", y);
        symbolTable.add_variable("z", z);
        symbolTable.add_variable("prev", previous);
        symbolTable.add_vector("gpr", gpr.data(), gpr.size());
        
        symbolTable.add_constants();
        expression.register_symbol_table(symbolTable);
        parser.settings().disable_all_control_structures();
        parser.settings().enable_all_control_structures();
        startTimer(16.7);
    }

    ~Graphing() override
    {
        setLookAndFeel(nullptr);
        m_tree.removeParameterListener("D", this);
        m_tree.removeParameterListener("Y", this);
        m_tree.removeParameterListener("Z", this);
    }

    void parameterChanged(const juce::String& id, float newValue) override
    {
        if (id == "D") setDistortionCoeff(newValue);
        else if (id == "Y") setY(newValue);
        else if (id == "Z") setZ(newValue);
    }

    void timerCallback() override
    {
        if (repaintRequested.load()) {
            recalculate();
        }
    }

    void paint(juce::Graphics& g) override
    {
        try {
            auto frame = getLocalBounds();
            const int numPixels = frame.getWidth();

            float posIncrement = frame.getWidth() / (double)numPixels;
            juce::Path p;
            auto pos = frame.getX();
            if (hasTransfer && dataset.size() != 0) {
                if (!std::isnan(dataset[0]))
                {
                    auto mappedStartY = juce::jmap<double>(dataset[0], -1, 1, 0, 1);
                    p.startNewSubPath(frame.getX(), frame.getBottom() - frame.getHeight() * mappedStartY);
                    for (auto i = 1; i < numPixels; i++)
                    {
                        pos += posIncrement;
                        if (std::isnan(dataset[i]) || std::isinf(dataset[i])) {
                            dataset[i] = 0;
                        }
                        auto mappedY = juce::jmap<double>(dataset[i], -1, 1, 0, 1);
                        mappedY = std::isnan(mappedY) || std::isinf(mappedY) ? 0 : mappedY;
                        p.lineTo(pos, frame.getBottom() - frame.getHeight() * mappedY);
                    }
                }
                else {
                    p.startNewSubPath(frame.getX(), frame.getBottom() - frame.getHeight() * 0);
                    for (auto i = 1; i < numPixels; i++) {
                        pos += posIncrement;
                        p.lineTo(pos, frame.getBottom() - frame.getHeight() * i / (double)numPixels);
                    }
                }

            }
            else {
                p.startNewSubPath(frame.getX(), frame.getBottom() - frame.getHeight() * 0);
                for (auto i = 1; i < numPixels; i++) {
                    pos += posIncrement;
                    p.lineTo(pos, frame.getBottom() - frame.getHeight() * i / (double)numPixels);
                }
            }
            try {
                g.setColour(juce::Colours::black);
                p.scaleToFit(0, 0, getWidth(), getHeight(), true);
                g.strokePath(p, juce::PathStrokeType(1.0));
                g.setColour(juce::Colour(100, 100, 100));
                g.drawRect(frame, 1.0);
            }
            catch (std::exception& e) {

            }
        }
        catch (std::exception& e) {}
    }

    void updateExpr(std::string newExpr)
    {
        clearRegisters();
        if (parser.compile(newExpr, expression)) { hasTransfer = true; }
        else { hasTransfer = false; }

        repaintRequested.store(true);
    }

    SDSP_INLINE void clearRegisters() {
        std::fill(gpr.begin(), gpr.end(), 0);
    }
    SDSP_INLINE void setDistortionCoeff(double newCoeff)
    {
        coeff = newCoeff;
        repaintRequested.store(true);
    }

    SDSP_INLINE void setY(double newY) {
        y = newY;
        repaintRequested.store(true);
    }

    SDSP_INLINE void setZ(double newZ)
    {
        z = newZ;
        repaintRequested.store(true);
    }

    void recalculate()
    {
        dataset.clear();
        auto valIncrement = 2 / (double)numPixels;
        for (double i = -1; i <= 1; i += valIncrement) {
            currentIp = i;
            try {
                auto res = expression.value();
                previous = res == std::isnan(res) ? 0 : res;
                if (std::isnan(res) || std::isinf(res)) {
                    throw std::exception();
                }
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (std::isnan(res) || std::isinf(res)) dataset.push_back(0);
                    else dataset.push_back(res);
                }

            }
            catch (std::exception& e) {
                dataset.push_back(0);
            }
        }

        repaintRequested.store(false);
        repaint();
    }

    void resized() override
    {
        numPixels = getWidth();
        recalculate();
    }

private:

    class GLF : public LF
    {
    public:
        GLF() : LF()
        {
            setColour(juce::Label::textColourId, juce::Colour(100, 100, 100));
        }
    };
    APVTS& m_tree;
    GLF lookAndFeel;
    int numPixels;
    bool hasTransfer = false;

    exprtk::symbol_table<T> symbolTable;
    exprtk::expression<T> expression;
    exprtk::parser<T> parser;
    T y{ 0 }, z{ 0 }, coeff{ 1 };
    T currentIp = 0;
    T previous = 0;
    std::array<T, 4> gpr = { 0, 0, 0, 0 };
    std::mutex m_mutex;
    std::vector<float> dataset;
    std::atomic_bool repaintRequested{ true };

};
