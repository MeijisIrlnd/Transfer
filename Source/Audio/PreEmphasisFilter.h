/*
  ==============================================================================

    PreEmphasisFilter.h
    Created: 10 Dec 2022 11:57:42pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <Types/FilterType.h>
#include <SDSP/SDSP.h>
using APVTS = juce::AudioProcessorValueTreeState;
namespace Transfer::Audio
{
    class PreEmphasisFilter : public APVTS::Listener
    {
    public: 
        PreEmphasisFilter(APVTS& tree, bool forward);
        PreEmphasisFilter(const PreEmphasisFilter& other);
        PreEmphasisFilter(PreEmphasisFilter&& other);
        ~PreEmphasisFilter() override;
        void prepareToPlay(SDSP_UNUSED int samplesPerBlockExpected, double sampleRate);
        float processSample(float in);
        void releaseResources();
        void parameterChanged(const juce::String& id, float value) override;
        SDSP_INLINE void setFilterType(Transfer::FILTER_TYPE t) { m_type = t; }
        SDSP_INLINE void setCutoff(double newCutoff) { m_cutoff = newCutoff; }
        SDSP_INLINE void setQ(double newQ) { m_q = newQ; }
        SDSP_INLINE void setBandwidth(double newBandWidth) { m_bandwidth = newBandWidth; }
        SDSP_INLINE void setSlope(double newSlope) { m_slope = newSlope; }
        SDSP_INLINE void setGain(double newGain) { m_gainDB = m_forward ? newGain : -newGain; }

    private: 
        void bindListeners();
        void interpolateCoefficients();
        void getCoeffs(Transfer::FILTER_TYPE t);
        APVTS& m_tree;
        const int m_updateRate = 100;
        int m_samplesUntilUpdate = 0;
        Transfer::FILTER_TYPE m_type { FILTER_TYPE::LOWSHELF };
        double m_sampleRate;
        double m_cutoff{ 1000 }, m_q{ 0.5 }, m_bandwidth{ 2 }, m_slope{ 0.7 }, m_gainDB{ 0 };
        SDSP::BiquadCascade<1> m_filter;
        SDSP::SmoothedFilterCoefficients<1> m_coeffs;
        bool m_forward{ true };
        bool m_prepared{ false };
    };
}