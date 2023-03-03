#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.


    feedbackAmountSlider.setRange (0.0, 0.99999);
    feedbackAmountSlider.setSkewFactorFromMidPoint ( 0.7);
    feedbackAmountSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    feedbackAmountSlider.onValueChange = [&]() 
    {   //awkward to convert 0 -> 0.99999 to 0.0 -> 1.0, but we must
        auto normalized = processorRef.feedbackAmmount->convertTo0to1 (feedbackAmountSlider.getValue());
        processorRef.feedbackAmmount->setValueNotifyingHost (normalized);
    };
    addAndMakeVisible (&feedbackAmountSlider);
    addAndMakeVisible (&feedBackAmountLabel);

    feedbackTimeSlider.setRange (5.0, 1000.0);
    feedbackTimeSlider.setSkewFactorFromMidPoint (125.0);
    feedbackAmountSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    feedbackTimeSlider.onValueChange = [&]()
    {
        auto normalized = processorRef.feedbackTimeMS->convertTo0to1 (feedbackTimeSlider.getValue());
        processorRef.feedbackTimeMS->setValueNotifyingHost (normalized);
    };
    addAndMakeVisible (&feedbackTimeSlider);

    spectralBypass.setToggleState (true, juce::NotificationType::dontSendNotification);
    spectralBypass.onStateChange = [&]()
    {
        // This is not thread-safe and is bad practice but fuck the police. 
        processorRef.doSpectralStuff = spectralBypass.getToggleState();
    };
    addAndMakeVisible (&spectralBypass);
    addAndMakeVisible (&spectralBypassLabel);

    binBandpassSlider.setSliderStyle (juce::Slider::SliderStyle::TwoValueHorizontal);
    binBandpassSlider.setRange ({0, (processorRef.fftSize / 2) - 1}, 1);
    binBandpassSlider.setTextBoxStyle (juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
    binBandpassSlider.onValueChange = [&]()
    {
        processorRef.keeperBins = {static_cast<int> (binBandpassSlider.getMinValue()), 
                                   static_cast<int> (binBandpassSlider.getMaxValue())};
    };
    addAndMakeVisible (binBandpassSlider);
    addAndMakeVisible (&binBandpassLabel);

    randomizePhaseToggle.setToggleState (true, juce::NotificationType::dontSendNotification);
    randomizePhaseToggle.onStateChange = [&]()
    {
        processorRef.randomizePhase = randomizePhaseToggle.getToggleState();
    };
    addAndMakeVisible (&randomizePhaseToggle);
    addAndMakeVisible (&randomizePhaseToggleLabel);


    addAndMakeVisible (&cpuLoadMeter);
    startTimerHz (20);

    setSize (400, 300);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto b = getLocalBounds(); int gap = 4;
    
    auto c = b.removeFromTop (20);
    spectralBypass.setBounds (c.removeFromLeft (20)); 
    spectralBypassLabel.setBounds (c.removeFromLeft (100));
    cpuLoadMeter.setBounds (c.removeFromRight (40));

    b.removeFromTop (gap);
    feedBackAmountLabel.setBounds (b.removeFromTop (20));
    feedbackAmountSlider.setBounds (b.removeFromTop (20));
    // b.removeFromTop (gap);
    // feedbackTimeSlider.setBounds   (b.removeFromTop (40)); 
    b.removeFromTop (gap);
    binBandpassLabel.setBounds (b.removeFromTop (20));
    binBandpassSlider.setBounds (b.removeFromTop (20));
    b.removeFromTop (gap);
    auto d = b.removeFromTop(20);
    randomizePhaseToggle.setBounds (d.removeFromLeft (20)); randomizePhaseToggleLabel.setBounds (d.removeFromLeft (100));

}
