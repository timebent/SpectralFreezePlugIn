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
    feedbackAmountSlider.setSkewFactorFromMidPoint ( 0.95);
    feedbackAmountSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    feedbackAmountSlider.onValueChange = [&]() 
    {   //awkward to convert 0 -> 0.99999 to 0.0 -> 1.0, but we must
        auto normalized = processorRef.feedbackAmmount->convertTo0to1 (feedbackAmountSlider.getValue());
        processorRef.feedbackAmmount->setValueNotifyingHost (normalized);
    };
    addAndMakeVisible (&feedbackAmountSlider);

    feedbackTimeSlider.setRange (5.0, 1000.0);
    feedbackTimeSlider.setSkewFactorFromMidPoint (125.0);
    feedbackAmountSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    feedbackTimeSlider.onValueChange = [&]()
    {
        auto normalized = processorRef.feedbackTimeMS->convertTo0to1 (feedbackTimeSlider.getValue());
        processorRef.feedbackTimeMS->setValueNotifyingHost (normalized);
    };
    addAndMakeVisible (&feedbackTimeSlider);

    setSize (400, 300);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto b = getLocalBounds(); int gap = 4;
    feedbackAmountSlider.setBounds (b.removeFromTop (40));
    b.removeFromTop (gap);
    feedbackTimeSlider.setBounds   (b.removeFromTop (40)); 
    b.removeFromTop (gap);
}
