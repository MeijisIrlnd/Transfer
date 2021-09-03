/*
  ==============================================================================

    KalexpanderProcessor.h
    Created: 22 Jun 2021 7:10:39pm
    Author:  Syl

  ==============================================================================
*/

#pragma once
#include <DSPCommon.h>
#include <JuceHeader.h>
namespace DSPCommon
{
    /// <summary>
    /// Expander implementation, with high emphasis filters
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template <typename T> 
    class Kalexpander : public juce::AudioProcessorValueTreeState::Listener
    {
    public: 
        Kalexpander<T>(juce::AudioProcessorValueTreeState& t)
        {
            rmsBallistics.setLevelCalculationType(juce::dsp::BallisticsFilterLevelCalculationType::RMS);
            ensureFiltersCreated();
            t.addParameterListener("Threshold", this);
            t.addParameterListener("Ratio", this);
            t.addParameterListener("Attack", this);
            t.addParameterListener("Release", this);
        }

        ~Kalexpander()
        {

        }

        void parameterChanged(const juce::String& id, float newValue) override
        {
            if (id == "Threshold") {
                storedThreshold = newValue;
                if (hasBeenPrepared) setThreshold(newValue);
            }
            else if (id == "Ratio") {
                storedRatio = newValue;
                if (hasBeenPrepared) setRatio(newValue);
            }
            else if (id == "Attack") {
                storedAttack = newValue;
                if (hasBeenPrepared) setAttack(newValue);
            }
            else if (id == "Release") {
                storedRelease = newValue;
                if (hasBeenPrepared) setRelease(newValue);
            }
        }


        void ensureFiltersCreated()
        {
            if (highEmphasisFilters.size() == 0)
            {
                highEmphasisFilters.add(new VariableFilter<T, 4>(DSPCommon::LADDER_HPF));                
                highEmphasisFilters[0]->setCutoff(200);
                highEmphasisFilters.add(new VariableFilter<T, 4>(DSPCommon::LADDER_HPF));
                highEmphasisFilters[1]->setCutoff(200);
            }
        }
        void setThreshold(double newThreshold)
        {
            auto db = juce::Decibels::decibelsToGain<T>(newThreshold, static_cast<T>(-200.0));
            smoothedThreshold.setTargetValue(db);
            smoothedThresholdInverse.setTargetValue(1 / static_cast<T>(db));
        }

        void setAttack(double newAttack)
        {
            envelopeBallistics.setAttackTime(newAttack * sr);
        }

        void setRelease(double newRelease)
        {
            envelopeBallistics.setReleaseTime(newRelease * sr);
        }

        void setRatio(double newRatio)
        {
            smoothedRatio.setTargetValue(newRatio);
        }

        void setDryWet(double newDryWet)
        {
            smoothedDryWet.setTargetValue(newDryWet);
        }

        void setHighEmphasisCutoff(double newCutoff)
        {
            for (auto& f : highEmphasisFilters) {
                f->setCutoff(newCutoff);
            }
        }

        void prepare(int spb, double sampleRate)
        {
            sr = sampleRate;
            ensureFiltersCreated();
            juce::dsp::ProcessSpec spec;
            spec.maximumBlockSize = spb;
            spec.sampleRate = sampleRate;
            spec.numChannels = 2;
            envelopeBallistics.prepare(spec);
            rmsBallistics.prepare(spec);
            for (auto& f : highEmphasisFilters) {
                f->prepare(spb, sampleRate);
            }
            smoothedThreshold.reset(sampleRate, 0.001);
            smoothedThresholdInverse.reset(sampleRate, 0.001);
            smoothedRatio.reset(sampleRate, 0.001);
            smoothedDryWet.reset(sampleRate, 0.001);
            setDryWet(1);
            setHighEmphasisCutoff(200);

            smoothedThreshold.setCurrentAndTargetValue(0.5);
            smoothedThresholdInverse.setCurrentAndTargetValue(1 / 0.5);
            smoothedRatio.setCurrentAndTargetValue(4);
            smoothedDryWet.setCurrentAndTargetValue(1);

            setThreshold(storedThreshold);
            setRatio(storedRatio);
            setAttack(storedAttack);
            setRelease(storedRelease);
            hasBeenPrepared = true;
        }

        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
        {
            auto* read = bufferToFill.buffer->getArrayOfReadPointers();
            auto* write = bufferToFill.buffer->getArrayOfWritePointers();
            for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
                threshold = smoothedThreshold.getNextValue();
                thresholdInverse = smoothedThresholdInverse.getNextValue();
                dryWet = smoothedDryWet.getNextValue();
                ratio = smoothedRatio.getNextValue();
                for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++) {
                    write[channel][sample] = expandSample(channel, read[channel][sample]) * dryWet + read[channel][sample] * (1 - dryWet);
                }
            }
        }

        void getNextAudioBlock(const juce::AudioSourceChannelInfo& carrier, const juce::AudioSourceChannelInfo& modulator)
        {
            auto* cRead = carrier.buffer->getArrayOfReadPointers();
            auto* mRead = modulator.buffer->getArrayOfReadPointers();
            auto* cWrite = carrier.buffer->getArrayOfWritePointers();
            auto* mWrite = modulator.buffer->getArrayOfWritePointers();
            for (auto sample = 0; sample < carrier.numSamples; sample++) {
                threshold = smoothedThreshold.getNextValue();
                thresholdInverse = smoothedThresholdInverse.getNextValue();
                dryWet = smoothedDryWet.getNextValue();
                ratio = smoothedRatio.getNextValue();
                for (auto channel = 0; channel < carrier.buffer->getNumChannels(); channel++) {
                    cWrite[channel][sample] = expandSample(channel, cRead[channel][sample], mRead[channel][sample]) * dryWet + cRead[channel][sample] * (1 - dryWet);
                }
            }
        }

        T processSample(int channel, T sample)
        {
            if (channel == 0) {
                threshold = smoothedThreshold.getNextValue();
                thresholdInverse = smoothedThresholdInverse.getNextValue();
                dryWet = smoothedDryWet.getNextValue();
                ratio = smoothedRatio.getNextValue();
            }
            
            return expandSample(channel, sample);
        }

        void release()
        {
            envelopeBallistics.reset();
            rmsBallistics.reset();
            for (auto& f : highEmphasisFilters) {
                f->release();
            }
        }

    private: 

        T expandSample(int channel, T sample)
        {
            T filteredSample = highEmphasisFilters[channel]->getNextSample(sample);
            auto env = rmsBallistics.processSample(channel, filteredSample);
            env = envelopeBallistics.processSample(channel, env);
            auto gain = env > threshold ? static_cast<T>(1.0) : std::pow(env * thresholdInverse, ratio - 1.0);
            return gain * sample;
        }

        T expandSample(int channel, T carrier, T modulator)
        {
            T filteredSample = highEmphasisFilters[channel]->getNextSample(modulator);
            auto env = rmsBallistics.processSample(channel, filteredSample);
            env = envelopeBallistics.processSample(channel, env);
            auto gain = env > threshold ? static_cast<T>(1.0) : std::pow(env * thresholdInverse, ratio - 1.0);
            return gain * carrier;
        }
        juce::dsp::BallisticsFilter<T> rmsBallistics;
        juce::dsp::BallisticsFilter<T> envelopeBallistics;
        juce::OwnedArray<VariableFilter<T, 4> > highEmphasisFilters;
        juce::SmoothedValue<double> smoothedThreshold, smoothedThresholdInverse;
        juce::SmoothedValue<double> smoothedRatio;
        juce::SmoothedValue<double> smoothedDryWet;
        double threshold, ratio, rms;
        double thresholdInverse;
        float tav = 0.01;
        float gain = 1;
        double sr;
        double dryWet = 1;
        double storedThreshold = 0, storedRatio = 2, storedAttack = 1e-5, storedRelease = 1e-5;
        bool hasBeenPrepared = false;
    };
}