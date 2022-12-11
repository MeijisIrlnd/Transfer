/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TransferAudioProcessorEditor::TransferAudioProcessorEditor (TransferAudioProcessor& p, juce::AudioProcessorValueTreeState& t)
    : AudioProcessorEditor (&p), audioProcessor (p), tree(t), gatePanel(tree), filterPanel(tree), filterButton("Filter", juce::Colour(200, 200, 200), true), graphButton("Graph", juce::Colour(200, 200, 200), true),
    gateButton("Gate", juce::Colour(200, 200, 200), true), graphing(tree)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 900);
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&graphing);
    expressionInput.setLookAndFeel(&inputLF);
    expressionInput.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    expressionLabel.setLookAndFeel(&titleLF);
    expressionLabel.setJustificationType(juce::Justification::centred);
    expressionLabel.setText("are you sure you want me to die?", juce::dontSendNotification);
    expressionInput.setFont(lookAndFeel.getFont());
    std::string ipText = tree.state.getChildWithName("Internal").getProperty("Function").toString().toStdString();
    if (ipText == "") { ipText = "x"; }
    expressionInput.setText(ipText, juce::dontSendNotification);
    graphing.updateExpr(expressionInput.getText().toStdString());
    auto dParam = tree.getParameter("D");
    auto dv = dParam->getNormalisableRange().convertFrom0to1(dParam->getValue());
    graphing.setDistortionCoeff(dv);
    auto zParam = tree.getParameter("Z");
    auto dz = zParam->getNormalisableRange().convertFrom0to1(zParam->getValue());
    graphing.setZ(dz);

    addAndMakeVisible(&expressionLabel);

    yLabel.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    yLabel.setReadOnly(true);
    yLabel.setText("H(s)=", juce::dontSendNotification);
    yLabel.setFont(lookAndFeel.getFont());

    addAndMakeVisible(&yLabel);
    addAndMakeVisible(&filterPanel);
    addAndMakeVisible(&gatePanel);
    auto selectedPanel = tree.state.getChildWithName("Internal").getProperty("Page");
    auto intPanel = static_cast<int>(selectedPanel);
    switch (intPanel)
    {
        case 0: {
            // Graph
            graphing.setVisible(true);
            gatePanel.setVisible(false);
            filterPanel.setVisible(false);
            break;
        }
        case 1: {
            // Gate
            graphing.setVisible(false);
            gatePanel.setVisible(true);
            filterPanel.setVisible(false);
            break;
        }
        case 2: {
            graphing.setVisible(false);
            gatePanel.setVisible(false);
            filterPanel.setVisible(true);
            break;
        }
    }

    addAndMakeVisible(&filterButton);
    addAndMakeVisible(&graphButton);
    addAndMakeVisible(&gateButton);

    filterButton.addListener(this);
    graphButton.addListener(this);
    gateButton.addListener(this);
    


    helpBlock.setFont(lookAndFeel.getFont());
    helpBlock.setColour(juce::TextEditor::textColourId, juce::Colour(100, 100, 100));
    expressionInput.onReturnKey = [this] {
        try {
#ifndef USE_EXPRTK
            auto text = expressionInput.getText();
            if (!text.contains("atan2")) {
                audioProcessor.setContext(expressionInput.getText().toStdString());
                graphing.updateExpr(expressionInput.getText().toStdString());
            }
            else {
                audioProcessor.setContext("x");
                graphing.updateExpr("x");
            }
#else 
            audioProcessor.setContext(expressionInput.getText().toStdString());
            graphing.updateExpr(expressionInput.getText().toStdString());
#endif
        }
        catch(std::exception& e){
            //Don't
        }
    };
    addAndMakeVisible(&expressionInput);
    helpBlock.setMultiLine(true);
#if defined USE_EXPRTK
    std::stringstream displayText;
    displayText << "Variables:\nx = Input Sample\nd = Distortion coefficient\nz = User defined (0 to 1)\n\n";
    displayText << "Logical:\nand, nand, mand, &, nor, xor, |, nor, xnor, not, \n\n";
    displayText << "General:\nabs(x), avg(x,y..), ceil(x), clamp(mn,x,mx), equal(x,y), erf(x), erfc(x), exp(x), expm1(x), floor(x), frac(x), hypot(x,y), iclamp(mn,x,mx),";
    displayText << "inrange(mn,x,mx), log(x), log10(x), log1p(x), log2(x), logn(x,n), max(x,y..), min(x,y..), mul(x,y..), ncdf(x), not_equal(x,y), pow(x,y), root(x,n),";
    displayText << "round(x), roundn(x,n), sgn(x), sqrt(x), sum(x,y..), swap(x,y), trunc(x)\n\n";
    displayText << "Trig:\nsin(x), cos(x), tan(x), asin(x), acos(x), atan(x), sinh(x), cosh(x), tanh(x), asinh(x), acosh(x), atanh(x), atan2(x,y), sinc(x), sec(x), cot(x), csc(x), deg2rad(x),";
    displayText << "deg2grad(x), rad2deg(x), grad2deg(x)\n\n";
    displayText << "Equalities & Operators:\n +, -, *, /, %, ^\n==, !=, <, <=, >, >=";

    helpBlock.setText(displayText.str(), false);
#else 

    helpBlock.setText(
        juce::String("Variables:\n") +
        juce::String("x = Input Sample\n") +
        juce::String("d = Distortion Coefficient (1 to 10)\n") +
        juce::String("z = User defined (0 to 1)\n") +
        juce::String("$e = Euler's Number (2.7182818284590452354)\n") +
        juce::String("$pi = PI(3.14159265358979323846)\n\n") +
        juce::String("Functions: \n") +
        juce::String("abs(x),\n") +
        juce::String("sin(x), cos(x), tan(x),\n") +
        juce::String("sinh(x), cosh(x), tanh(x),\n") +
        juce::String("asin(x), acos(x), atan(x),\n") +
        juce::String("exp(x),\n") +
        juce::String("log(x), log2(x), log10(x),\n") +
        juce::String("sqrt(x),\n") +
        juce::String("floor(x, y), round(x), min(x, y), max(x, y),\n") +
        juce::String("sig(x)\n\n") +
        juce::String("Operators: \n") +
        juce::String("+, -, *, /, ^"), false);
#endif

    helpBlock.setReadOnly(true);
    addAndMakeVisible(&helpBlock);
    distortionCoefficientLabel.setJustificationType(juce::Justification::centredTop);
    distortionCoefficientLabel.setText("Distortion Coefficient", juce::dontSendNotification);
    addAndMakeVisible(&distortionCoefficientLabel);
    distortionCoefficientSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    distortionCoefficientSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    coeffAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("D"), distortionCoefficientSlider));
    addAndMakeVisible(&distortionCoefficientSlider);

    zLabel.setJustificationType(juce::Justification::centredTop);
    zLabel.setText("Z", juce::dontSendNotification);
    addAndMakeVisible(&zLabel);
    zSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    zSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&zSlider);
    zAttachment.reset(new juce::SliderParameterAttachment(*tree.getParameter("Z"), zSlider));
    setWantsKeyboardFocus(true);
}

TransferAudioProcessorEditor::~TransferAudioProcessorEditor()
{
    expressionInput.setLookAndFeel(nullptr);
    expressionInput.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void TransferAudioProcessorEditor::onLabelButtonClicked(LabelButton* l)
{
    if (l == &graphButton)
    {
        graphing.setVisible(true);
        gatePanel.setVisible(false);
        filterPanel.setVisible(false);
        if (!tree.state.getChildWithName("Internal").isValid()) {
            juce::Identifier ident("Internal");
            juce::ValueTree internalTree(ident);
            tree.state.addChild(internalTree, -1, nullptr);
            tree.state.getChildWithName("Internal").setProperty("Page", juce::var(0), nullptr);
        }
        else {
            tree.state.getChildWithName("Internal").setProperty("Page", juce::var(0), nullptr);
        }
    }
    else if(l == &gateButton){
        graphing.setVisible(false);
        gatePanel.setVisible(true);
        filterPanel.setVisible(false);
        if (!tree.state.getChildWithName("Internal").isValid()) {
            juce::Identifier ident("Internal");
            juce::ValueTree internalTree(ident);
            tree.state.addChild(internalTree, -1, nullptr);
            tree.state.getChildWithName("Internal").setProperty("Page", juce::var(1), nullptr);
        }
        else {
            tree.state.getChildWithName("Internal").setProperty("Page", juce::var(1), nullptr);
        }
    }
    else if (l == &filterButton) {
        graphing.setVisible(false);
        gatePanel.setVisible(false);
        filterPanel.setVisible(true);
        if (!tree.state.getChildWithName("Internal").isValid()) {
            juce::Identifier ident("Internal");
            juce::ValueTree internalTree(ident);
            tree.state.addChild(internalTree, -1, nullptr);
            tree.state.getChildWithName("Internal").setProperty("Page", juce::var(2), nullptr);
        }
        else {
            tree.state.getChildWithName("Internal").setProperty("Page", juce::var(2), nullptr);
        }
    }
}

//==============================================================================
void TransferAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void TransferAudioProcessorEditor::resized()
{
    auto h = getHeight() / 10;
    //expressionLabel.setBounds(0, h, getWidth() / 8, h);
    //yLabel.setBounds(getWidth() / 8, h, getWidth() / 24, h);
    //expressionInput.setBounds(getWidth() / 8 + getWidth() / 24, h, getWidth() - (getWidth() / 8 + getWidth() / 24), h);
    expressionLabel.setBounds(0, h / 5, getWidth(), h / 2);
    yLabel.setBounds(0, h, getWidth() / 16, h / 2);
    expressionInput.setBounds(getWidth() / 16, h, (getWidth() - yLabel.getWidth()) - 10, h / 4);
    //helpBlock.setBounds(0, h*2, getWidth()/2 - getWidth() / 10 - 1 , getHeight() - h*3);
    //graphing.setBounds(getWidth() / 2 , (h*2) + 10, getWidth() / 2, (getHeight() - h*4) - 20);
    //gatePanel.setBounds(getWidth() / 2, (h * 2) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    //filterPanel.setBounds(getWidth() / 2, (h * 2) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    //graphButton.setBounds(graphing.getX() - getWidth() / 10, graphing.getY(), getWidth() / 10 - 1, getHeight() / 15);
    //gateButton.setBounds(graphButton.getX(), graphButton.getY() + graphButton.getHeight() + 2, graphButton.getWidth(), graphButton.getHeight());
    //filterButton.setBounds(gateButton.getX(), gateButton.getY() + gateButton.getHeight() + 2, gateButton.getWidth(), gateButton.getHeight());
    //distortionCoefficientLabel.setBounds(0, getHeight() - (h * 2), getWidth() / 4, h );
    //distortionCoefficientSlider.setBounds(getWidth() / 4, getHeight() - (h * 2), getWidth() - getWidth() / 4, h / 4);
    //zLabel.setBounds(0, getHeight() - h, getWidth() / 4, h);
    //zSlider.setBounds(getWidth() / 4, getHeight() - h, getWidth() - getWidth() / 4, h / 4);
    helpBlock.setBounds(0, h*1.5, getWidth()/2 - getWidth() / 10 - 1 , getHeight() - h*3);
    graphing.setBounds(getWidth() / 2 , (h*1.5) + 10, getWidth() / 2, (getHeight() - h*4) - 20);
    gatePanel.setBounds(getWidth() / 2, (h * 1.5) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    filterPanel.setBounds(getWidth() / 2, (h * 1.5) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    graphButton.setBounds(graphing.getX() - getWidth() / 10, graphing.getY(), getWidth() / 10 - 1, getHeight() / 15);
    gateButton.setBounds(graphButton.getX(), graphButton.getY() + graphButton.getHeight() + 2, graphButton.getWidth(), graphButton.getHeight());
    filterButton.setBounds(gateButton.getX(), gateButton.getY() + gateButton.getHeight() + 2, gateButton.getWidth(), gateButton.getHeight());
    distortionCoefficientLabel.setBounds(0, getHeight() - (h * 1.5), getWidth() / 4, h );
    distortionCoefficientSlider.setBounds(getWidth() / 4, getHeight() - (h * 1.5), (getWidth() - getWidth() / 4) - 10, h / 4);
    zLabel.setBounds(0, getHeight() - h, getWidth() / 4, h);
    zSlider.setBounds(getWidth() / 4, getHeight() - h, (getWidth() - getWidth() / 4) - 10, h / 4);
}
