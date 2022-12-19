/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TransferAudioProcessorEditor::TransferAudioProcessorEditor(TransferAudioProcessor& p, juce::AudioProcessorValueTreeState& t)
    : AudioProcessorEditor(&p), m_audioProcessor(p), m_tree(t), m_gatePanel(m_tree), m_filterPanel(m_tree), m_filterButton("Filter", juce::Colour(200, 200, 200), true),
    m_graphButton("Graph", juce::Colour(200, 200, 200), true),
    m_gateButton("Gate", juce::Colour(200, 200, 200), true),
    m_registerClearButton("CLR GPR", juce::Colour(200, 200, 200), true),
    m_panicButton("PANIC", juce::Colour(200, 200, 200), true),
    m_graphing(m_tree)
{
    setSize (800, 1000);
    m_expressionLabel.setLookAndFeel(&m_titleLF);
    m_expressionLabel.setFont(m_titleLF.getLabelFont(m_expressionLabel));
    m_expressionLabel.setJustificationType(juce::Justification::centred);
    m_expressionLabel.setText("transfer", juce::dontSendNotification);
    setLookAndFeel(&m_lookAndFeel);
    addAndMakeVisible(&m_graphing);
    m_expressionInput.setLookAndFeel(&m_inputLF);
    m_expressionInput.setColour(juce::TextEditor::textColourId, juce::Colours::black);

    m_expressionInput.setFont(m_lookAndFeel.getFont());

    std::string ipText = m_tree.state.getChildWithName("Internal").getProperty("Function").toString().toStdString();
    if (ipText == "") { ipText = "x"; }
    m_expressionInput.setText(ipText, juce::dontSendNotification);
    m_graphing.updateExpr(m_expressionInput.getText().toStdString());
    auto dParam = m_tree.getParameter("D");
    auto dv = dParam->getNormalisableRange().convertFrom0to1(dParam->getValue());
    m_graphing.setDistortionCoeff(dv);
    auto yParam = m_tree.getParameter("Y");
    auto rangedY = yParam->getNormalisableRange().convertFrom0to1(yParam->getValue());
    m_graphing.setY(rangedY);
    auto zParam = m_tree.getParameter("Z");
    auto dz = zParam->getNormalisableRange().convertFrom0to1(zParam->getValue());
    m_graphing.setZ(dz);

    addAndMakeVisible(&m_expressionLabel);

    m_hxLabel.setColour(juce::TextEditor::textColourId, juce::Colours::black);
    m_hxLabel.setReadOnly(true);
    m_hxLabel.setText("H(s)=", juce::dontSendNotification);
    m_hxLabel.setFont(m_lookAndFeel.getFont());

    addAndMakeVisible(&m_hxLabel);
    addAndMakeVisible(&m_filterPanel);
    addAndMakeVisible(&m_gatePanel);
    auto selectedPanel = m_tree.state.getChildWithName("Internal").getProperty("Page");
    auto intPanel = static_cast<int>(selectedPanel);
    switch (intPanel)
    {
        case 0: {
            // Graph
            m_graphing.setVisible(true);
            m_gatePanel.setVisible(false);
            m_filterPanel.setVisible(false);
            break;
        }
        case 1: {
            // Gate
            m_graphing.setVisible(false);
            m_gatePanel.setVisible(true);
            m_filterPanel.setVisible(false);
            break;
        }
        case 2: {
            m_graphing.setVisible(false);
            m_gatePanel.setVisible(false);
            m_filterPanel.setVisible(true);
            break;
        }
    }
    addAndMakeVisible(&m_registerClearButton);
    addAndMakeVisible(&m_panicButton);
    addAndMakeVisible(&m_filterButton);
    addAndMakeVisible(&m_graphButton);
    addAndMakeVisible(&m_gateButton);

    m_registerClearButton.addListener(this);
    m_panicButton.addListener(this);
    m_filterButton.addListener(this);
    m_graphButton.addListener(this);
    m_gateButton.addListener(this);
    

    m_helpBlock.setFont(m_lookAndFeel.getFont());
    m_helpBlock.setColour(juce::TextEditor::textColourId, juce::Colour(100, 100, 100));
    m_expressionInput.onReturnKey = [this] {
        try {

            m_audioProcessor.setContext(m_expressionInput.getText().toStdString());
            m_graphing.updateExpr(m_expressionInput.getText().toStdString());
        }
        catch(std::exception& e){
            //Don't
        }
        m_expressionInput.unfocusAllComponents();
    };
    addAndMakeVisible(&m_expressionInput);
    m_helpBlock.setMultiLine(true);
    std::stringstream displayText;
    displayText << "Variables:\nx: Input Sample\nd: distortion coefficient (0 to 10)\ny: user defined (0 to 1)\nz: user defined (0 to 1)\ngpr[0...4]: general purpose registers\n\n";
    displayText << "Logical:\nand, nand, mand, &, nor, xor, |, nor, xnor, not, \n\n";
    displayText << "General:\nabs(x), avg(x,y..), ceil(x), clamp(mn,x,mx), equal(x,y), erf(x), erfc(x), exp(x), expm1(x), floor(x), frac(x), hypot(x,y), iclamp(mn,x,mx),";
    displayText << "inrange(mn,x,mx), log(x), log10(x), log1p(x), log2(x), logn(x,n), max(x,y..), min(x,y..), mul(x,y..), ncdf(x), not_equal(x,y), pow(x,y), root(x,n),";
    displayText << "round(x), roundn(x,n), sgn(x), sqrt(x), sum(x,y..), swap(x,y), trunc(x)\n\n";
    displayText << "Trig:\nsin(x), cos(x), tan(x), asin(x), acos(x), atan(x), sinh(x), cosh(x), tanh(x), asinh(x), acosh(x), atanh(x), atan2(x,y), sinc(x), sec(x), cot(x), csc(x), deg2rad(x),";
    displayText << "deg2grad(x), rad2deg(x), grad2deg(x)\n\n";
    displayText << "Equalities & Operators:\n +, -, *, /, %, ^\n==, !=, <, <=, >, >=\n\n";
    displayText << "Control Flow:\nif(x) y;, if(x) y; else z;, \nif(x) y; else if(z) a;\n\n";
    displayText << "Assignment:\n:=, +=, -=, *=, /=, %=\n\n";
    displayText << "~(x, y, ... n): evaluate exprs left to right, return value of last expr";

    m_helpBlock.setText(displayText.str(), false);

    m_helpBlock.setReadOnly(true);
    addAndMakeVisible(&m_helpBlock);
    LF::instantiateHorizontalSlider(this, &m_distortionCoefficientSlider, &m_distortionCoefficientLabel, "Distortion Coefficient");
    m_coeffAttachment.reset(new juce::SliderParameterAttachment(*m_tree.getParameter("D"), m_distortionCoefficientSlider));
    LF::instantiateHorizontalSlider(this, &m_ySlider, &m_yLabel, "Y");
    m_yAttachment.reset(new juce::SliderParameterAttachment(*m_tree.getParameter("Y"), m_ySlider));
    LF::instantiateHorizontalSlider(this, &m_zSlider, &m_zLabel, "Z");
    m_zAttachment.reset(new juce::SliderParameterAttachment(*m_tree.getParameter("Z"), m_zSlider));

    //setWantsKeyboardFocus(true);
}

TransferAudioProcessorEditor::~TransferAudioProcessorEditor()
{
    m_expressionInput.setLookAndFeel(nullptr);
    m_expressionInput.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void TransferAudioProcessorEditor::onLabelButtonClicked(LabelButton* l)
{
    if (l == &m_registerClearButton) {
        m_audioProcessor.clearRegisters();
        m_graphing.clearRegisters();
    }

    else if (l == &m_panicButton) {
        // Set transfer to 0, and invoke all callbacks..
        m_expressionInput.setText("x", juce::sendNotification);
        m_expressionInput.onReturnKey();
    }

    else if (l == &m_graphButton)
    {
        m_graphing.setVisible(true);
        m_gatePanel.setVisible(false);
        m_filterPanel.setVisible(false);
        if (!m_tree.state.getChildWithName("Internal").isValid()) {
            juce::Identifier ident("Internal");
            juce::ValueTree internalTree(ident);
            m_tree.state.addChild(internalTree, -1, nullptr);
            m_tree.state.getChildWithName("Internal").setProperty("Page", juce::var(0), nullptr);
        }
        else {
            m_tree.state.getChildWithName("Internal").setProperty("Page", juce::var(0), nullptr);
        }
    }
    else if(l == &m_gateButton){
        m_graphing.setVisible(false);
        m_gatePanel.setVisible(true);
        m_filterPanel.setVisible(false);
        if (!m_tree.state.getChildWithName("Internal").isValid()) {
            juce::Identifier ident("Internal");
            juce::ValueTree internalTree(ident);
            m_tree.state.addChild(internalTree, -1, nullptr);
            m_tree.state.getChildWithName("Internal").setProperty("Page", juce::var(1), nullptr);
        }
        else {
            m_tree.state.getChildWithName("Internal").setProperty("Page", juce::var(1), nullptr);
        }
    }
    else if (l == &m_filterButton) {
        m_graphing.setVisible(false);
        m_gatePanel.setVisible(false);
        m_filterPanel.setVisible(true);
        if (!m_tree.state.getChildWithName("Internal").isValid()) {
            juce::Identifier ident("Internal");
            juce::ValueTree internalTree(ident);
            m_tree.state.addChild(internalTree, -1, nullptr);
            m_tree.state.getChildWithName("Internal").setProperty("Page", juce::var(2), nullptr);
        }
        else {
            m_tree.state.getChildWithName("Internal").setProperty("Page", juce::var(2), nullptr);
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
    m_expressionLabel.setBounds(0, h / 5, getWidth(), h / 2);
    m_hxLabel.setBounds(0, h, getWidth() / 16, h / 2);
    m_expressionInput.setBounds(getWidth() / 16, h, (getWidth() - m_hxLabel.getWidth()) - 10, h / 4);
    m_helpBlock.setBounds(0, h*1.5, getWidth()/2 - getWidth() / 10 - 1 , getHeight() - h*3);
    m_graphing.setBounds(getWidth() / 2 , (h*1.5) + 10, getWidth() / 2, (getHeight() - h*4) - 20);
    m_gatePanel.setBounds(getWidth() / 2, (h * 1.5) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    m_filterPanel.setBounds(getWidth() / 2, (h * 1.5) + 10, getWidth() / 2, (getHeight() - h * 4) - 20);
    m_registerClearButton.setBounds(m_graphing.getX(), m_graphing.getY() + m_graphing.getHeight(), m_graphing.getWidth() / 4, h / 4);
    m_panicButton.setBounds(m_registerClearButton.getX() + m_registerClearButton.getWidth(), m_registerClearButton.getY(), m_registerClearButton.getWidth(), m_registerClearButton.getHeight());
    m_graphButton.setBounds(m_graphing.getX() - getWidth() / 10, m_graphing.getY(), getWidth() / 10 - 1, getHeight() / 15);
    m_gateButton.setBounds(m_graphButton.getX(), m_graphButton.getY() + m_graphButton.getHeight() + 2, m_graphButton.getWidth(), m_graphButton.getHeight());
    m_filterButton.setBounds(m_gateButton.getX(), m_gateButton.getY() + m_gateButton.getHeight() + 2, m_gateButton.getWidth(), m_gateButton.getHeight());

    // three sliders at the bottom, so remaining height is height - h / 3.. 
    int remainingHeight = getHeight() - (h / 3);
    m_distortionCoefficientLabel.setBounds(0, getHeight() - (h * 1.5), getWidth() / 4, h );
    m_distortionCoefficientSlider.setBounds(getWidth() / 4, getHeight() - (h * 1.5), (getWidth() - getWidth() / 4) - 10, h / 4);
    m_yLabel.setBounds(0, getHeight() - h, getWidth() / 4, h);
    m_ySlider.setBounds(getWidth() / 4, getHeight() - h, (getWidth() - getWidth() / 4) - 10, h / 4);
    m_zLabel.setBounds(0, getHeight() - (h / 2), getWidth() / 4, h);
    m_zSlider.setBounds(getWidth() / 4, getHeight() - (h / 2), (getWidth() - getWidth() / 4) - 10, h / 4);
}
