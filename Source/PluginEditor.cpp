/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CommandLineDistortionAudioProcessorEditor::CommandLineDistortionAudioProcessorEditor (CommandLineDistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    setLookAndFeel(&lookAndFeel);
    expressionInput.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    expressionLabel.setText("Transfer Function:", juce::dontSendNotification);
    expressionInput.setFont(lookAndFeel.getFont());
    expressionInput.setText("tanh(x*d)", juce::dontSendNotification);
    addAndMakeVisible(&expressionLabel);

    yLabel.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    yLabel.setReadOnly(true);
    yLabel.setText("Y = ", juce::dontSendNotification);
    yLabel.setFont(lookAndFeel.getFont());

    addAndMakeVisible(&yLabel);


    helpBlock.setFont(lookAndFeel.getFont());
    helpBlock.setColour(juce::TextEditor::textColourId, juce::Colour(100, 100, 100));
    expressionInput.onReturnKey = [this] {
        audioProcessor.setContext(expressionInput.getText().toStdString());

    };
    addAndMakeVisible(&expressionInput);
    helpBlock.setMultiLine(true);
    //atan(x*d) * 1/z
    helpBlock.setText(
        "Variables:\nx = Input Sample\nd = Distortion Coefficient\nz = User Defined, 0 to 1\nE = 2.7182818284590452354\nPI = 3.14159265358979323846\n\nFunctions:\nsin(x), cos(x), tan(x)\nasin(x), acos(x), atan(x), sinh(x), cosh(x), tanh(x)\npow(x, y), sqrt(x)\nlog(x), log2(x), log10(x)\nfrac(x), recip(x)\nmin(x,y), max(x,y)\navg(x,y), ceil(x), floor(x)\nround(x), roundeven(x), trunc(x)\nsignbit(x), copysign(x)\n\nOperators:\n+, -, *, /, %\n!(x), -(x)\n", false
    );
    helpBlock.setReadOnly(true);
    addAndMakeVisible(&helpBlock);
    distortionCoefficientLabel.setJustificationType(juce::Justification::centredTop);
    distortionCoefficientLabel.setText("Distortion Coefficient", juce::dontSendNotification);
    addAndMakeVisible(&distortionCoefficientLabel);
    distortionCoefficientSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    distortionCoefficientSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    distortionCoefficientSlider.setRange(1, 10, 0.001);
    distortionCoefficientSlider.setValue(1);
    addAndMakeVisible(&distortionCoefficientSlider);
    distortionCoefficientSlider.onValueChange = [this] {
        audioProcessor.setDistortionCoefficient(distortionCoefficientSlider.getValue());
    };
    zLabel.setJustificationType(juce::Justification::centredTop);
    zLabel.setText("Z", juce::dontSendNotification);
    addAndMakeVisible(&zLabel);
    zSlider.setRange(0, 1, 0.001);
    zSlider.setValue(0);
    zSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    zSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&zSlider);
    zSlider.onValueChange = [this] {
        audioProcessor.setZ(zSlider.getValue());
    };
}

CommandLineDistortionAudioProcessorEditor::~CommandLineDistortionAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void CommandLineDistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void CommandLineDistortionAudioProcessorEditor::resized()
{
    auto h = getHeight() / 10;
    expressionLabel.setBounds(0, 0, getWidth(), h);
    yLabel.setBounds(0, h, getWidth() / 24, h);
    expressionInput.setBounds(getWidth() / 24, h, getWidth(), h);
    helpBlock.setBounds(0, h*2, getWidth(), getHeight() - h*3);
    distortionCoefficientLabel.setBounds(0, getHeight() - (h * 2), getWidth() / 4, h );
    distortionCoefficientSlider.setBounds(getWidth() / 4, getHeight() - (h * 2), getWidth() - getWidth() / 4, h / 4);
    zLabel.setBounds(0, getHeight() - h, getWidth() / 4, h);
    zSlider.setBounds(getWidth() / 4, getHeight() - h, getWidth() - getWidth() / 4, h / 4);
}
