#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, 
                                         private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    juce::Slider feedbackAmountSlider; juce::Label feedBackAmountLabel {"", "FFT Feedback"};
    juce::Slider feedbackTimeSlider;
    juce::ToggleButton spectralBypass; juce::Label spectralBypassLabel {"", "Spectral On?"};

    juce::Slider binBandpassSlider; juce::Label binBandpassLabel {"", "Bin Bandpass"};
    juce::ToggleButton randomizePhaseToggle; juce::Label randomizePhaseToggleLabel {"", "Randomize Phase"};

    juce::Label cpuLoadMeter;
    
    void timerCallback () override { cpuLoadMeter.setText (juce::String(processorRef.cpuMeter.getCPULoad(), 2), juce::NotificationType::dontSendNotification); repaint(); }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)




};
