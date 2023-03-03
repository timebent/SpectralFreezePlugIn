#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Gamma/DFT.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "Gamma/Delay.h"
#include "Gamma/Gamma.h"

class CPUMeter
{
public:
    void prepare (double sr, int size)
    {
        sampleRate = sr;
        bufferSize = size;
        loadMeasurer.reset (sampleRate, bufferSize);
    }
    void reset () { loadMeasurer.reset (sampleRate, bufferSize); }
    // only to be used in the process call-back
    juce::AudioProcessLoadMeasurer& getLoadMeasurer () { return loadMeasurer; }
    int getXRunCount() { return loadMeasurer.getXRunCount(); }
    float getCPULoad () { return cpuRead.getNext (loadMeasurer.getLoadAsPercentage()); }
private:
    juce::AudioProcessLoadMeasurer loadMeasurer;
    double sampleRate; int bufferSize;

    struct SmoothFall
    {
        float getNext (float newValue)
        {
            if (newValue > fallingValue)
                fallingValue = newValue;
            else
                fallingValue *= scalar;

            return fallingValue;
        }
    private:
        float fallingValue = 0.0f;
        float scalar = 0.99f;
    };
    SmoothFall cpuRead;
};


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

    CPUMeter cpuMeter;
    bool doSpectralStuff = true;
    bool randomizePhase  = true;
    juce::Range<int> keeperBins;
    static const int fftSize = 2048;

private:
    float * phases;
    double sampleRate; // keeping a copy for convenience (wow! such luxury!)
    gam::Comb<> delay;
    // gam::Sine<> osc;
    gam::Square<> osc;
    gam::SamplePlayer<> player;
    gam::Accum<> timer{1./2.};
    int captureCount = 0;
    gam::STFT stft{
		2048,		// Window size
		2048 / 4,		// Hop size; number of samples between transforms
		0,			// Pad size; number of zero-valued samples appended to window
		gam::HANN,		// Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
					//		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
		gam::COMPLEX		// Format of frequency samples:
					//		COMPLEX, MAG_PHASE, or MAG_FREQ
	};
    juce::Array<gam::Complex<float>> history;

    //  gam::STFT prevstft{
	// 	2048,		// Window size
	// 	2048 / 4,		// Hop size; number of samples between transforms
	// 	0,			// Pad size; number of zero-valued samples appended to window
	// 	gam::HANN,		// Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
	// 				//		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
	// 	gam::COMPLEX		// Format of frequency samples:
	// 				//		COMPLEX, MAG_PHASE, or MAG_FREQ
	// };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
