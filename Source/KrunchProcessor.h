/*
  ==============================================================================

    KrunchProcessor.h
    Created: 17 Jun 2021 4:16:26pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <DSPCommon.h>

namespace DSPCommon
{
    template <typename T> 
    class KrunchProcessor
    {
    public: 
        KrunchProcessor<T>()
        {
            waveshapers.add(new Shaper<T>());
            waveshapers.add(new Shaper<T>());
        }

        ~KrunchProcessor()
        {

        }

        void setGain(double newGain) {
            for (auto& shaper : waveshapers) {
                shaper->setGain(newGain);
            }
        }

        void setDistortionCoefficient(double newDistortionCoefficient)
        {
            for (auto& shaper : waveshapers) {
                shaper->setDistortionCoefficient(newDistortionCoefficient);
            }
        }

        void setShapingFunction(DSPShaping::FUNCTION f)
        {
            for (auto& shaper : waveshapers) {
                shaper->setShaperFunction(f);
            }
        }

        void setShapingFunction(std::function<T(T)>& f)
        {
            for (auto& shaper : waveshapers) {
                shaper->setShaperFunction(f);
            }
        }

        void prepare(int spb, double sampleRate)
        {
            for (auto& shaper : waveshapers) {
                shaper->prepare(spb, sampleRate);
                shaper->setGain(1);
            }
        }

        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
        {
            auto* read = bufferToFill.buffer->getArrayOfReadPointers();
            auto* write = bufferToFill.buffer->getArrayOfWritePointers();
            for (auto sample = 0; sample < bufferToFill.numSamples; sample++)
            {
                for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++) {
                    write[channel][sample] = waveshapers[channel]->getNextSample(read[channel][sample]);
                }
            }
        }

        void release()
        {
            for (auto& shaper : waveshapers) shaper->release();
        }

    private:
        juce::OwnedArray<DSPCommon::Shaper<T> > waveshapers;
    };
}