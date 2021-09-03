/*
  ==============================================================================

    Graphing.h
    Created: 25 Jun 2021 11:48:26pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <mathpresso.h>
#include <sstream>
#include "LF.h"
class Graphing : public juce::Component, juce::AudioProcessorValueTreeState::Listener
{
public:
    Graphing(juce::AudioProcessorValueTreeState& t)
    {
        context.addBuiltIns();
        context.addVariable("x", 0 * sizeof(double));
        context.addVariable("d", 1 * sizeof(double));
        context.addVariable("z", 2 * sizeof(double));

        setLookAndFeel(&lookAndFeel);
        dataset.reserve(100);
        t.addParameterListener("D", this);
        t.addParameterListener("Z", this);
    }

    ~Graphing() override
    {
        setLookAndFeel(nullptr);
    }

    void parameterChanged(const juce::String& id, float newValue) override
    {
        if (id == "D") setDistortionCoeff(newValue);
        else if (id == "Z") setZ(newValue);
    }

    void paint(juce::Graphics& g) override
    {
        try {
            auto frame = getLocalBounds();
            const int numPixels = frame.getWidth();

            float posIncrement = frame.getWidth() / (double)numPixels;
            juce::Path p;
            auto pos = frame.getX();
            if (hasTransfer) {
                if (!std::isnan(dataset[0]))
                {
                    auto mappedStartY = juce::jmap<double>(dataset[0], -1, 1, 0, 1);
                    p.startNewSubPath(frame.getX(), frame.getBottom() - frame.getHeight() * mappedStartY);
                    for (auto i = 1; i < numPixels; i++)
                    {
                        pos += posIncrement;
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
            g.setColour(juce::Colours::black);
            g.strokePath(p, juce::PathStrokeType(1.0));
            g.setColour(juce::Colour(100, 100, 100));
            g.drawRect(frame, 1.0);
        }
        catch (std::exception& e) {}
    }

    void updateExpr(std::string newExpr)
    {
        auto err = expr.compile(context, newExpr.c_str(), mathpresso::kNoOptions);
        if (err == mathpresso::kErrorOk) {
            hasTransfer = true;
        }
        else {
            hasTransfer = false;
        }
        recalculate();
    }

    void setDistortionCoeff(double newCoeff)
    {
        coeff = newCoeff;
        recalculate();
    }

    void setZ(double newZ)
    {
        z = newZ;
        recalculate();
    }

    void recalculate()
    {
        dataset.clear();
        if (expr.isCompiled()) {
            double data[] = { 0, coeff, z };
            //auto valueIncrement = 1 / (double)numPixels;
            auto valueIncrement = 2 / (double)numPixels;
            for (double i = -1; i <= 1; i += valueIncrement)
            {
                data[0] = i;
                auto res = expr.evaluate(data);
                if (std::isnan(res) || std::isinf(res)) dataset.push_back(0);
                else dataset.push_back((res));
            }
        }
        repaint();
    }

    void resized() override
    {
        numPixels = getWidth();
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
    GLF lookAndFeel;
    int numPixels;
    bool hasTransfer = false;
    double z = 0;
    double coeff = 1;
    mathpresso::Context context;
    mathpresso::Expression expr;
    std::vector<float> dataset;

};