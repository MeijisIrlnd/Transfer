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
    m_graphing(m_tree), m_codeEditor(m_document, &m_tokeniser),
    m_oversamplingButtons(p)
{
    setSize (600, 600 * 1.25);
    m_document.addListener(this);
    m_expressionLabel.setLookAndFeel(&m_titleLF);
    m_expressionLabel.setFont(m_titleLF.getLabelFont(m_expressionLabel));
    m_expressionLabel.setJustificationType(juce::Justification::centredTop);
    m_expressionLabel.setText("transfer", juce::dontSendNotification);
    addAndMakeVisible(&m_expressionLabel);
    setLookAndFeel(&m_lookAndFeel);
    addAndMakeVisible(&m_graphing);


    std::string ipText = m_tree.state.getChildWithName("Internal").getProperty("Function").toString().toStdString();
    if (ipText == "") { ipText = "x"; }
    m_document.insertText(0, ipText);
    m_graphing.updateExpr(ipText);
    addAndMakeVisible(&m_codeEditor);
    initialiseGraphingParams();
    
    m_hxLabel.setColour(juce::TextEditor::textColourId, juce::Colour(0xFF7F7F7F));
    m_hxLabel.setText("H(s) = ", juce::dontSendNotification);
    m_hxLabel.setFont(m_lookAndFeel.getFont());

    addAndMakeVisible(&m_hxLabel);
    addAndMakeVisible(&m_filterPanel);
    addAndMakeVisible(&m_gatePanel);
    initialiseShownPanel();
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
    m_codeEditor.onReturnKey = [this] {
        try {
            m_audioProcessor.setContext(m_document.getAllContent().toStdString());
            m_graphing.updateExpr(m_document.getAllContent().toStdString());
        }
        catch (std::exception& e) {

        }
    };
    m_helpBlock.setMultiLine(true);

    m_displayText << "Variables:\nx: Input Sample\nd: distortion coefficient (0 to 10)\ny: user defined (0 to 1)\nz: user defined (0 to 1)\ngpr[0...4]: general purpose registers\n\n";
    m_displayText << "Logical:\nand, nand, mand, &, nor, xor, |, xnor, not, \n\n";
    m_displayText << "General:\nabs(x), avg(x,y..), ceil(x), clamp(mn,x,mx), equal(x,y), erf(x), erfc(x), exp(x), expm1(x), floor(x), frac(x), hypot(x,y), iclamp(mn,x,mx),";
    m_displayText << "inrange(mn,x,mx), log(x), log10(x), log1p(x), log2(x), logn(x,n), max(x,y..), min(x,y..), mul(x,y..), ncdf(x), not_equal(x,y), pow(x,y), root(x,n),";
    m_displayText << "round(x), roundn(x,n), sgn(x), sqrt(x), sum(x,y..), swap(x,y), trunc(x)\n\n";
    m_displayText << "Trig:\nsin(x), cos(x), tan(x), asin(x), acos(x), atan(x), sinh(x), cosh(x), tanh(x), asinh(x), acosh(x), atanh(x), atan2(x,y), sinc(x), sec(x), cot(x), csc(x), deg2rad(x),";
    m_displayText << "deg2grad(x), rad2deg(x), grad2deg(x)\n\n";
    m_displayText << "Equalities & Operators:\n +, -, *, /, %, ^\n==, !=, <, <=, >, >=\n\n";
    m_displayText << "Control Flow:\nif(x) y;, if(x) y; else z;, \nif(x) y; else if(z) a;\n\n";
    m_displayText << "Assignment:\n:=, +=, -=, *=, /=, %=\n\n";
    m_displayText << "~(x, y, ... n): evaluate exprs left to right, return value of last expr";

    m_helpBlock.setText(m_displayText.str(), false);
    m_helpBlock.setReadOnly(true);
    m_helpBlock.setScrollbarsShown(false);
    addAndMakeVisible(&m_helpBlock);
    LF::instantiateHorizontalSlider(this, &m_distortionCoefficientSlider, &m_distortionCoefficientLabel, "Distortion Coefficient");
    m_coeffAttachment.reset(new juce::SliderParameterAttachment(*m_tree.getParameter("D"), m_distortionCoefficientSlider));
    LF::instantiateHorizontalSlider(this, &m_ySlider, &m_yLabel, "Y");
    m_yAttachment.reset(new juce::SliderParameterAttachment(*m_tree.getParameter("Y"), m_ySlider));
    LF::instantiateHorizontalSlider(this, &m_zSlider, &m_zLabel, "Z");
    m_zAttachment.reset(new juce::SliderParameterAttachment(*m_tree.getParameter("Z"), m_zSlider));
    addAndMakeVisible(&m_sizeButtons);
    addAndMakeVisible(&m_oversamplingButtons);
    //setWantsKeyboardFocus(true);
    resized();
    // If we have errors from initialising the processor, handle them.. 
    if (ErrorReporter::getInstance()->hasError()) {
        showError(ErrorReporter::getInstance()->getErrors());
    }
}

TransferAudioProcessorEditor::~TransferAudioProcessorEditor()
{
    ErrorReporter::getInstance()->removeEditor();
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
        m_document.replaceAllContent("x");
        m_codeEditor.onReturnKey();
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

void TransferAudioProcessorEditor::codeDocumentTextInserted(const juce::String& newText, int insertIndex)
{
    ErrorReporter::getInstance()->clearErrors();
}

void TransferAudioProcessorEditor::codeDocumentTextDeleted(int startIndex, int endIndex)
{
    ErrorReporter::getInstance()->clearErrors();
}

void TransferAudioProcessorEditor::showError(const std::vector<error_t>& errors)
{
    std::stringstream errorStream;
    for (auto& err : errors) {
        errorStream << err.diagnostic << " (pos: " << err.token.position << ", token: \"" << err.token.value << "\")" << "\n";
        //errorStream << "position: " << err.token.position << " token: \"" << err.token.value << "\"\n" << exprtk::parser_error::to_str(err.mode) << "\n";
    }
    m_codeEditor.toggleErrorMode(true, juce::String(errorStream.str()));
}

void TransferAudioProcessorEditor::clearErrors()
{
    m_codeEditor.toggleErrorMode(false);
}

void TransferAudioProcessorEditor::initialiseGraphingParams()
{
    auto dParam = m_tree.getParameter("D");
    auto dv = dParam->getNormalisableRange().convertFrom0to1(dParam->getValue());
    m_graphing.setDistortionCoeff(dv);
    auto yParam = m_tree.getParameter("Y");
    auto rangedY = yParam->getNormalisableRange().convertFrom0to1(yParam->getValue());
    m_graphing.setY(rangedY);
    auto zParam = m_tree.getParameter("Z");
    auto dz = zParam->getNormalisableRange().convertFrom0to1(zParam->getValue());
    m_graphing.setZ(dz);
}

void TransferAudioProcessorEditor::initialiseShownPanel()
{
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
}


//==============================================================================
void TransferAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colour(m_codeEditor.hasKeyboardFocus(true) ? 0xFF000000 : 0x7F000000));
    g.drawRect(m_codeEditor.getBounds().expanded(2, 2), 1.0f);
}

void TransferAudioProcessorEditor::resized()
{
    auto h = getHeight() / 10;
    m_expressionLabel.setBounds(0, h / 5, getWidth(), h / 2);
    m_sizeButtons.setBounds(getWidth() - getWidth() / 5, 0, getWidth() / 5, h / 4);
    m_oversamplingButtons.setBounds(m_sizeButtons.getX(), m_sizeButtons.getY() + m_sizeButtons.getHeight(), getWidth() / 5, h/4);
    m_hxLabel.setBounds(getWidth() / 32, h, getWidth() / 12, h / 4);
    m_codeEditor.setBounds(getWidth() / 32 + getWidth() / 12, h, getWidth() - getWidth() / 16 - getWidth() / 12, h / 4);
    m_helpBlock.setBounds(0, h * 1.5, getWidth() / 2 - getWidth() / 10 - 1 , getHeight() - h*3);
    m_helpBlock.applyFontToAllText(LF::createFont().withHeight(m_helpBlock.getHeight() / 50.0f));
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

void TransferAudioProcessorEditor::contextChangedInternal(const std::string& newText)
{
    m_document.replaceAllContent(newText);
    // manually call graphing's update 
    m_graphing.updateExpr(newText);
}

TransferAudioProcessorEditor::SizeController::SizeController() : m_smallButton("S", juce::Colour(0x7F232323), true),
    m_mediumButton("M", juce::Colour(0x7F232323), true),
    m_largeButton("L", juce::Colour(0x7F232323), true),
    m_buttons({&m_smallButton, &m_mediumButton, &m_largeButton}),
    m_sizeOpts({std::tuple<int, int>{300, 300 * 1.25}, std::tuple<int, int>{700, 700 * 1.25}, std::tuple<int, int>{800, 800 * 1.25}})
{
    for (auto& b : m_buttons) {
        addAndMakeVisible(b);
        b->addListener(this);
    }
}

TransferAudioProcessorEditor::SizeController::~SizeController()
{
}

void TransferAudioProcessorEditor::SizeController::paint(juce::Graphics& g)
{
    //g.fillAll(juce::Colour(0x7F232323));
}

void TransferAudioProcessorEditor::SizeController::resized()
{
    auto w = getWidth() / 4;
    m_sizeLabel.setBounds(0, 0, getWidth() / 4, getHeight());
    for (auto i = 0; i < m_buttons.size(); i++) {
        m_buttons[i]->setBounds(m_sizeLabel.getX() + (w * (i + 1)), m_sizeLabel.getY(), w, getHeight());
    }
}

void TransferAudioProcessorEditor::SizeController::onLabelButtonClicked(LabelButton* l)
{
    auto it = std::find(m_buttons.begin(), m_buttons.end(), l);
    if (it == m_buttons.end()) return;
    auto [width, height] = m_sizeOpts[std::distance(m_buttons.begin(), it)];
    getParentComponent()->setSize(width, height);
    
}


TransferAudioProcessorEditor::OversamplingButtons::OversamplingButtons(TransferAudioProcessor& p) :
    m_off("OFF", juce::Colour(0x7F232323), true),
    m_4x("4X", juce::Colour(0x7F232323), true),
    m_16x("16X", juce::Colour(0x7F232323), true),
    m_buttons({&m_off, &m_4x, &m_16x}),
    m_oversamplingOpts({0, 2, 4}),
    m_processor(p)
{
    for(auto& b : m_buttons) {
        addAndMakeVisible(b);
        b->addListener(this);
    }
}

TransferAudioProcessorEditor::OversamplingButtons::~OversamplingButtons() {

}

void TransferAudioProcessorEditor::OversamplingButtons::paint(juce::Graphics& g)
{
    
}

void TransferAudioProcessorEditor::OversamplingButtons::resized()
{
    auto w = getWidth() / 4;
    m_oversamplingLabel.setBounds(0, 0, w, getHeight());
    for(auto i = 0; i < m_buttons.size(); i++) {
        m_buttons[i]->setBounds(m_oversamplingLabel.getWidth() + (i * w), m_oversamplingLabel.getY(), w, getHeight());
    }
}

void TransferAudioProcessorEditor::OversamplingButtons::onLabelButtonClicked(LabelButton *l)
{
    auto it = std::find(m_buttons.begin(), m_buttons.end(), l);
    if(it == m_buttons.end()) return;
    size_t idx = std::distance(m_buttons.begin(), it);
    m_processor.setOversamplingFactor(m_oversamplingOpts[idx]);
}
