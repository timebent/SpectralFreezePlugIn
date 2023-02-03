#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Gamma/DFT.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "Gamma/Delay.h"
#include "Gamma/Gamma.h"

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioParameterFloat* feedbackAmmount;
    juce::AudioParameterFloat* feedbackTimeMS;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
    float * phases;
    double sampleRate; // keeping a copy for convenience (wow! such luxury!)
    gam::Comb<> delay;
    gam::Sine<> osc;
    gam::SamplePlayer<> player;
    gam::Accum<> timer{1./2.};
    int captureCount = 0;
    gam::STFT stft{
		65536,		// Window size
		65536 / 4,		// Hop size; number of samples between transforms
		0,			// Pad size; number of zero-valued samples appended to window
		gam::HANN,		// Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
					//		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
		gam::MAG_PHASE		// Format of frequency samples:
					//		COMPLEX, MAG_PHASE, or MAG_FREQ
	};

     gam::STFT prevstft{
		65536,		// Window size
		65536 / 4,		// Hop size; number of samples between transforms
		0,			// Pad size; number of zero-valued samples appended to window
		gam::HANN,		// Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
					//		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
		gam::MAG_PHASE		// Format of frequency samples:
					//		COMPLEX, MAG_PHASE, or MAG_FREQ
	};
};
