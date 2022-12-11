/*
  ==============================================================================

    Gate.cpp
    Created: 10 Dec 2022 11:39:10pm
    Author:  Syl

  ==============================================================================
*/

#include "Gate.h"
namespace Transfer::Audio
{
    Gate::Gate(APVTS& tree) : m_tree(tree)
    {
        m_rmsBallistics.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::RMS);
        bindListeners();
    }

    Gate::Gate(const Gate& other) : m_tree(other.m_tree)
    {
        m_rmsBallistics.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::RMS);
        bindListeners();
    }

    Gate::Gate(Gate&& other) noexcept : m_tree(other.m_tree)
    {
        m_rmsBallistics.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::RMS);
        bindListeners();
    }

    Gate::~Gate()
    {
        m_tree.removeParameterListener("Threshold", this);
        m_tree.removeParameterListener("Ratio", this);
        m_tree.removeParameterListener("Attack", this);
        m_tree.removeParameterListener("Release", this);
    }


    void Gate::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 1 };
        m_envelopeBallistics.prepare(spec);
        m_rmsBallistics.prepare(spec);
        m_qs = SDSP::getButterworthQs<4>();
        SDSP::RBJ::highpass(m_coeffs.target(0), sampleRate, 1000, m_qs[0]);
        std::memcpy(m_coeffs.current(0), m_coeffs.target(0), sizeof(double) * 6);
        SDSP::RBJ::highpass(m_coeffs.target(1), sampleRate, 1000, m_qs[1]);
        std::memcpy(m_coeffs.current(1), m_coeffs.target(1), sizeof(double) * 6);
        for (auto stage = 0; stage < 2; stage++) {
            m_highEmphasisFilter.setCoefficients(m_coeffs.target(stage), stage);
        }
        m_smoothedThreshold.reset(sampleRate, 0.1f);
        m_smoothedThreshold.setCurrentAndTargetValue(m_threshold);
        m_smoothedRatio.reset(sampleRate, 0.1f);
        m_smoothedRatio.setCurrentAndTargetValue(m_ratio);
        m_smoothedAttack.reset(sampleRate, 0.1f);
        m_smoothedAttack.setCurrentAndTargetValue(m_attack);
        m_smoothedRelease.reset(sampleRate, 0.1f);
        m_smoothedRelease.setCurrentAndTargetValue(m_release);
        m_prepared = true;
    }

    float Gate::processSample(float in)
    {
        m_envelopeBallistics.setAttackTime(m_smoothedAttack.getNextValue());
        m_envelopeBallistics.setReleaseTime(m_smoothedRelease.getNextValue());

        float filtered = m_highEmphasisFilter.processSample(in);
        auto env = m_rmsBallistics.processSample(0, filtered);
        env = m_envelopeBallistics.processSample(0, env);
        float currentThreshold = m_smoothedThreshold.getNextValue();
        float currentThresholdInverse = 1 / currentThreshold;
        float currentRatio = m_smoothedRatio.getNextValue();
        auto gain = env > currentThreshold ? 1.0f : std::pow(env * currentThresholdInverse, currentRatio - 1.0);
        return gain * in;
    }

    void Gate::releaseResources()
    {
    }

    void Gate::parameterChanged(const juce::String& id, float newValue)
    {
        if (id == "Threshold") { setThreshold(newValue); }
        else if (id == "Ratio") { setRatio(newValue); }
        else if (id == "Attack") { setAttack(newValue); }
        else if (id == "Release") { setRelease(newValue); }
    }

    void Gate::bindListeners()
    {
        m_tree.addParameterListener("Threshold", this);
        m_tree.addParameterListener("Ratio", this);
        m_tree.addParameterListener("Attack", this);
        m_tree.addParameterListener("Release", this);
    }
}