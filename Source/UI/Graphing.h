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
class Graphing : public juce::Component
{
public:
    Graphing()
    {
        setLookAndFeel(&lookAndFeel);
        dataset.reserve(100);
    }

    ~Graphing() override
    {
        setLookAndFeel(nullptr);
    }

    void paint(juce::Graphics& g) override
    {
        auto frame = getLocalBounds();
        const int numPixels = frame.getWidth();

        float posIncrement = frame.getWidth() / (double)numPixels;
        juce::Path p;
        auto pos = frame.getX();
        if (hasTransfer) {
            p.startNewSubPath(frame.getX(), frame.getBottom() - frame.getHeight() * dataset[0]);
            for (auto i = 1; i < numPixels; i++)
            {
                pos += posIncrement;
                p.lineTo(pos, frame.getBottom() - frame.getHeight() * dataset[i]);
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

    void updateExpr(mathpresso::Expression* newExpr) {
        if (newExpr != nullptr) {
            expr = newExpr;
            hasTransfer = true;
            recalculate();
        }
        else {
            hasTransfer = false;
            recalculate();
        }
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
        if (expr != nullptr) {
            double data[] = { 0, coeff, z };
            auto valueIncrement = 1 / (double)numPixels;
            for (double i = 0; i < 1; i += valueIncrement)
            {
                data[0] = i;
                auto res = expr->evaluate(data);
                dataset.push_back(res);
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
    mathpresso::Expression* expr = nullptr;
    std::vector<float> dataset;

};