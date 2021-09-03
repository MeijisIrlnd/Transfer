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
#include "KalexpanderProcessor.h"

    template <typename T> 
    class KrunchProcessor
    {
    public: 
        KrunchProcessor<T>(juce::AudioProcessorValueTreeState& t) : tree(t), inputGate(t)
        {
            waveshapers.add(new DSPCommon::Shaper<T>());
            waveshapers.add(new DSPCommon::Shaper<T>());

        }

        ~KrunchProcessor()
        {

        }


        void toggleGate(bool newState)
        {
            gateState = newState;
        }

        void setGateThreshold(double newThreshold)
        {
            inputGate.setThreshold(newThreshold);
        }

        void setGateRatio(double newRatio)
        {
            inputGate.setRatio(newRatio);
        }

        void setGateAttack(double newAttack)
        {
            inputGate.setAttack(newAttack);
        }

        void setGateRelease(double newRelease)
        {
            inputGate.setRelease(newRelease);
        }

        void setGain(double newGain) {
            for (auto& shaper : waveshapers) {
                shaper->setGain(newGain);
            }
        }

        void setDistortionCoefficient(double newDistortionCoefficient)
        {
            storedCoefficient = newDistortionCoefficient;
            if (hasBeenPrepared) {
                for (auto& shaper : waveshapers) {
                    shaper->setDistortionCoefficient(newDistortionCoefficient);
                }
            }
        }

        void setShapingFunction(DSPCommon::DSPShaping::FUNCTION f)
        {
            for (auto& shaper : waveshapers) {
                shaper->setShaperFunction(f);
            }
        }

        void setShapingFunction(std::function<T(T)>& f)
        {
            storedTransfer = f;
            if (hasBeenPrepared) {
                for (auto& shaper : waveshapers) {
                    shaper->setShaperFunction(f);
                }
            }
        }

        void prepare(int spb, double sampleRate)
        {
            for (auto& shaper : waveshapers) {
                shaper->prepare(spb, sampleRate);
                shaper->setGain(1);
                shaper->setShaperFunction(storedTransfer);
                shaper->setDistortionCoefficient(storedCoefficient);
                
            }
            inputGate.prepare(spb, sampleRate);
            hasBeenPrepared = true;
        }

        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
        {
            auto* read = bufferToFill.buffer->getArrayOfReadPointers();
            auto* write = bufferToFill.buffer->getArrayOfWritePointers();
            for (auto sample = 0; sample < bufferToFill.numSamples; sample++)
            {
                for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++) {
                    auto s = gateState ? inputGate.processSample(channel, read[channel][sample]) : read[channel][sample];
                    s = waveshapers[channel]->getNextSample(s);
                    if (std::isnan(s) || std::isinf(s)) s = 0;
                    write[channel][sample] = s;
                }
            }
        }

        void release()
        {
            for (auto& shaper : waveshapers) shaper->release();
            inputGate.release();
        }

    private:
        juce::OwnedArray<DSPCommon::Shaper<T> > waveshapers;
        DSPCommon::Kalexpander<T> inputGate;
        bool gateState = true;
        juce::AudioProcessorValueTreeState& tree; 
        std::function<float(float)> storedTransfer;
        double storedCoefficient = 1;
        bool hasBeenPrepared = false;
    };
