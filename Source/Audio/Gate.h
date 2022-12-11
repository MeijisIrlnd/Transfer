/*
  ==============================================================================

    Gate.h
    Created: 10 Dec 2022 11:39:10pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <SDSP/SDSP.h>
namespace Transfer::Audio
{
    class Gate : public APVTS::Listener
    {
    public: 
        Gate(APVTS& tree);
        Gate(const Gate& other);
        Gate(Gate&& other) noexcept;
        ~Gate() override;
        Gate& operator=(const Gate& other) = delete;
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        float processSample(float in);
        void releaseResources();

        void parameterChanged(const juce::String& id, float newValue) override;
        SDSP_INLINE void setThreshold(float newThresholdDB) {
            auto db = juce::Decibels::decibelsToGain<float>(newThresholdDB, -200.0f);
            m_threshold = db;
            if (m_prepared) {
                m_smoothedThreshold.setTargetValue(db);
            }
        }
        SDSP_INLINE void setRatio(float newRatio) {
            m_ratio = newRatio;
            if (m_prepared) {
                m_smoothedRatio = newRatio;
            }
        }

        SDSP_INLINE void setAttack(float newAttack) {
            m_attack = newAttack;
            if (m_prepared) {
                m_smoothedAttack = newAttack;
            }
        }

        SDSP_INLINE void setRelease(float newRelease) {
            m_release = newRelease;
            if (m_prepared) {
                m_smoothedRelease = newRelease;
            }
        }

    private: 
        void bindListeners();
        APVTS& m_tree;
        float m_threshold{0}, m_ratio{1}, m_attack{1e-5f}, m_release{1e-5f};
        juce::SmoothedValue<float> m_smoothedThreshold, m_smoothedRatio, m_smoothedAttack, m_smoothedRelease;
        SDSP::BiquadCascade<2> m_highEmphasisFilter;
        SDSP::SmoothedFilterCoefficients<2> m_coeffs;
        juce::dsp::BallisticsFilter<float> m_envelopeBallistics, m_rmsBallistics;
        std::array<double, 2> m_qs;
        bool m_prepared{ false };
    };
}