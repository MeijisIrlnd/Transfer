/*
  ==============================================================================

    Distortion.h
    Created: 10 Dec 2022 11:39:05pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <SDSP/SDSP.h>
#include <Audio/PreEmphasisFilter.h>
#include <Audio/Gate.h>
namespace Transfer::Audio
{
    class Distortion : public APVTS::Listener
    {
    public:
        Distortion(juce::AudioProcessorValueTreeState& tree);
        ~Distortion() override;
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
        void releaseResources();

        void parameterChanged(const juce::String& id, float newValue) override;
        SDSP_INLINE void setShapingFunction(const std::function<float(float)>& newFunc) {
            m_storedTransfer = newFunc;
            if (m_prepared) {
                m_waveshaper.functionToUse = m_storedTransfer;
            }
        }

        SDSP_INLINE void setFilterType(FILTER_TYPE t) {
            for (auto i = 0; i < 2; i++) {
                m_forwardEmphasisFilters[i].setFilterType(t);
                m_backwardEmphasisFilters[i].setFilterType(t);
            }
        }

        SDSP_INLINE void setFilterFrequency(double newFreq) {
            for (auto i = 0; i < 2; i++) {
                m_forwardEmphasisFilters[i].setCutoff(newFreq);
                m_backwardEmphasisFilters[i].setCutoff(newFreq);
            }
        }

        SDSP_INLINE void setFilterGain(double newGainDB) {
            for (auto i = 0; i < 2; i++) {
                m_forwardEmphasisFilters[i].setGain(newGainDB);
                m_backwardEmphasisFilters[i].setGain(-newGainDB);
            }
        }

        SDSP_INLINE void setFilterSlope(double newSlope) {
            for (auto i = 0; i < 2; i++) {
                m_forwardEmphasisFilters[i].setSlope(newSlope);
                m_backwardEmphasisFilters[i].setSlope(newSlope);
            }
        }
        SDSP_INLINE void setGateState(bool newState) {
            m_gateState = newState;
        }
        SDSP_INLINE void setGateThreshold(float newThreshold) {
            for (auto& g : m_gates) {
                g.setThreshold(newThreshold);
            }
        }

        SDSP_INLINE void setGateRatio(float newRatio) {
            for (auto& g : m_gates) {
                g.setRatio(newRatio);
            }
        }

        SDSP_INLINE void setGateAttack(float newAttack) {
            for (auto& g : m_gates) {
                g.setAttack(newAttack);
            }
        }

        SDSP_INLINE void setGateRelease(float newRelease) {
            for (auto& g : m_gates) {
                g.setRelease(newRelease);
            }
        }

    private: 
        juce::AudioProcessorValueTreeState& m_tree;
        const int m_updateRate = 100;
        int m_samplesUntilUpdate = 0;
        juce::dsp::WaveShaper<float, std::function<float(float)> > m_waveshaper;
        std::array<PreEmphasisFilter, 2> m_forwardEmphasisFilters, m_backwardEmphasisFilters; 
        std::array<Gate, 2> m_gates;
        std::function<float(float)> m_storedTransfer = [this](float x) { return x; };
        bool m_gateState{ true };
        bool m_prepared{ false };
    };
}