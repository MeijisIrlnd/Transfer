/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TransferAudioProcessorEditor::TransferAudioProcessorEditor (TransferAudioProcessor& p, juce::AudioProcessorValueTreeState& t)
    : AudioProcessorEditor (&p), audioProcessor (p), tree(t), gatePanel(tree), graphButton("Graph", juce::Colour(200, 200, 200), true), 
    gateButton("Gate", juce::Colour(200, 200, 200), true), graphing(tree)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(&graphing);
    expressionInput.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    expressionLabel.setText("Transfer Function:", juce::dontSendNotification);
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
    yLabel.setText("Y = ", juce::dontSendNotification);
    yLabel.setFont(lookAndFeel.getFont());

    addAndMakeVisible(&yLabel);
    addAndMakeVisible(&gatePanel);
    auto selectedPanel = tree.state.getChildWithName("Internal").getProperty("Page");
    auto intPanel = (int)selectedPanel;
    if (intPanel == 0) {
        graphing.setVisible(true);
        gatePanel.setVisible(false);
    }
    else {
        graphing.setVisible(false);
        gatePanel.setVisible(true);
    }
    addAndMakeVisible(&graphButton);
    addAndMakeVisible(&gateButton);

    graphButton.addListener(this);
    gateButton.addListener(this);
    


    helpBlock.setFont(lookAndFeel.getFont());
    helpBlock.setColour(juce::TextEditor::textColourId, juce::Colour(100, 100, 100));
    expressionInput.onReturnKey = [this] {
        try {
            audioProcessor.setContext(expressionInput.getText().toStdString());
            graphing.updateExpr(expressionInput.getText().toStdString());
            
        }
        catch(std::exception& e){
            //Don't
        }
    };
    addAndMakeVisible(&expressionInput);
    helpBlock.setMultiLine(true);
    //atan(x*d) * 1/z
    helpBlock.setText(
        "Variables:\nx = Input Sample\nd = Distortion Coefficient\nz = User Defined, 0 to 1\nE = 2.7182818284590452354\nPI = 3.14159265358979323846\n\nFunctions:\nsin(x), cos(x), tan(x)\nasin(x), acos(x), atan(x), sinh(x), cosh(x), tanh(x)\npow(x, y), sqrt(x)\nlog(x), log2(x), log10(x)\nfrac(x), recip(x)\nmin(x,y), max(x,y)\navg(x,y), ceil(x), floor(x)\nround(x), roundeven(x), trunc(x)\nsignbit(x), copysign(x,y)\n\nOperators:\n+, -, *, /, %\n!(x), -(x)\n", false
    );
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
}

TransferAudioProcessorEditor::~TransferAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void TransferAudioProcessorEditor::onLabelButtonClicked(LabelButton* l)
{
    if (l == &graphButton)
    {
        graphing.setVisible(true);
        gatePanel.setVisible(false);
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
    expressionLabel.setBounds(0, 0, getWidth(), h);
    yLabel.setBounds(0, h, getWidth() / 24, h);
    expressionInput.setBounds(getWidth() / 24, h, getWidth(), h);
    helpBlock.setBounds(0, h*2, getWidth()/2 - getWidth() / 10 - 1 , getHeight() - h*3);
    graphing.setBounds(getWidth() / 2 , (h*2) + 10, getWidth() / 2, (getHeight() - h*4) - 20);
    gatePanel.setBounds(getWidth() / 2, (h * 2) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    graphButton.setBounds(graphing.getX() - getWidth() / 10, graphing.getY(), getWidth() / 10 - 1, getHeight() / 15);
    gateButton.setBounds(graphButton.getX(), graphButton.getY() + graphButton.getHeight() + 2, graphButton.getWidth(), graphButton.getHeight());
    distortionCoefficientLabel.setBounds(0, getHeight() - (h * 2), getWidth() / 4, h );
    distortionCoefficientSlider.setBounds(getWidth() / 4, getHeight() - (h * 2), getWidth() - getWidth() / 4, h / 4);
    zLabel.setBounds(0, getHeight() - h, getWidth() / 4, h);
    zSlider.setBounds(getWidth() / 4, getHeight() - h, getWidth() - getWidth() / 4, h / 4);
}
