/*
  ==============================================================================

    Distortion.cpp
    Created: 10 Dec 2022 11:39:05pm
    Author:  Syl

  ==============================================================================
*/

#include "Distortion.h"

namespace Transfer::Audio
{
    Distortion::Distortion(juce::AudioProcessorValueTreeState& tree) : 
        m_tree(tree), 
        m_forwardEmphasisFilters({ PreEmphasisFilter(tree, true), PreEmphasisFilter(tree, true) }),
        m_backwardEmphasisFilters({ PreEmphasisFilter(tree, false), PreEmphasisFilter(tree, false) }),
        m_gates({tree, tree})
    {
    }

    Distortion::~Distortion()
    {
    }

    void Distortion::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
    {
        juce::dsp::ProcessSpec spec{ sampleRate, static_cast<juce::uint32>(samplesPerBlockExpected), 2 };
        m_waveshaper.prepare(spec);
        m_waveshaper.functionToUse = m_storedTransfer;
        for (auto i = 0; i < 2; i++) {
            m_forwardEmphasisFilters[i].prepareToPlay(samplesPerBlockExpected, sampleRate);
            m_backwardEmphasisFilters[i].prepareToPlay(samplesPerBlockExpected, sampleRate);
            m_gates[i].prepareToPlay(samplesPerBlockExpected, sampleRate);
        }
        m_prepared = true;
    }

    void Distortion::getNextAudioBlock(const juce::dsp::AudioBlock<float>& block)
    {

        for (auto sample = 0; sample < block.getNumSamples(); sample++) {
            
            for (auto channel = 0; channel < block.getNumChannels(); channel++) {
                auto* buffer = block.getChannelPointer(channel);
                float current = m_gateState ? m_gates[channel].processSample(buffer[sample]) : buffer[sample];
                //current = m_forwardEmphasisFilters[channel].processSample(current);
                current = m_waveshaper.processSample(current);
                current = isnan(current) ? 0.0f : (isinf(current) ? 0.0f : current);
                //current = m_backwardEmphasisFilters[channel].processSample(current);
                buffer[sample] = current;
            }
        }
    }

    void Distortion::releaseResources()
    {
    }
    void Distortion::parameterChanged(const juce::String& id, float newValue)
    {
    }
}
