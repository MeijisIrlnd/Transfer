/*
  ==============================================================================

    PreEmphasisFilter.cpp
    Created: 10 Dec 2022 11:57:42pm
    Author:  Syl

  ==============================================================================
*/

#include "PreEmphasisFilter.h"

namespace Transfer::Audio
{
    PreEmphasisFilter::PreEmphasisFilter(APVTS& tree, bool forward) : m_tree(tree), m_forward(forward)
    {
        bindListeners();
    }

    PreEmphasisFilter::PreEmphasisFilter(const PreEmphasisFilter& other) : m_tree(other.m_tree)
    {
        bindListeners();
    }

    PreEmphasisFilter::PreEmphasisFilter(PreEmphasisFilter&& other) : m_tree(other.m_tree)
    {
        bindListeners();
    }

    PreEmphasisFilter::~PreEmphasisFilter()
    {
        m_tree.removeParameterListener("EmphasisFilterType", this);
        m_tree.removeParameterListener("EmphasisFilterCutoff", this);
        m_tree.removeParameterListener("EmphasisFilterSlope", this);
        m_tree.removeParameterListener("EmphasisFilterGain", this);
    }

    void PreEmphasisFilter::prepareToPlay(SDSP_UNUSED int samplesPerBlockExpected, double sampleRate)
    {
        m_sampleRate = sampleRate;
        getCoeffs(m_type);
        std::memcpy(m_coeffs.current(0), m_coeffs.target(0), sizeof(double) * 6);
        m_filter.setCoefficients(m_coeffs.target(0));
        m_prepared = true;
    }

    float PreEmphasisFilter::processSample(float in)
    {
        if (m_samplesUntilUpdate == 0) {
            getCoeffs(m_type);
            m_samplesUntilUpdate = m_updateRate;
        }
        interpolateCoefficients();
        --m_samplesUntilUpdate;
        return m_filter.processSample(in);
    }

    void PreEmphasisFilter::releaseResources()
    {

    }

    void PreEmphasisFilter::parameterChanged(const juce::String& id, float value)
    {
        if (id == "EmphasisFilterType") {
            setFilterType(static_cast<FILTER_TYPE>(static_cast<int>(value)));
        }
        else if (id == "EmphasisFilterCutoff") { setCutoff(static_cast<double>(value)); }
        else if (id == "EmphasisFilterSlope") { setSlope(static_cast<double>(value)); }
        else if (id == "EmphasisFilterGain") { setGain(static_cast<double>(value)); }
    }

    void PreEmphasisFilter::bindListeners()
    {
        m_tree.addParameterListener("EmphasisFilterType", this);
        m_tree.addParameterListener("EmphasisFilterCutoff", this);
        m_tree.addParameterListener("EmphasisFilterSlope", this);
        m_tree.addParameterListener("EmphasisFilterGain", this);
    }

    void PreEmphasisFilter::interpolateCoefficients()
    {
        m_coeffs.interpolate();
        m_filter.setCoefficients(m_coeffs.current(0));
    }

    void PreEmphasisFilter::getCoeffs(Transfer::FILTER_TYPE t)
    {
        switch (t)
        {
            case FILTER_TYPE::LOWSHELF: {
                SDSP::RBJ::lowShelf(m_coeffs.target(0), m_sampleRate, m_cutoff, m_gainDB, m_slope);
                break;
            }
            case FILTER_TYPE::HIGHSHELF: {
                SDSP::RBJ::highShelf(m_coeffs.target(0), m_sampleRate, m_cutoff, m_gainDB, m_slope);
                break;
            }
            default: return;
        }
        
    }
}