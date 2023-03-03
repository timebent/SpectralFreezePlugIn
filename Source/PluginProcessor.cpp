#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    // always be suspicious of the use of 'new' in modern C++. Here it is ok because juce::AudioProcessor now owns the pointer, and will be responsible for deleting when done
    addParameter (feedbackAmmount = new juce::AudioParameterFloat ("feedbackAmountParmaseanID", "Feedback Amount", 0.0f, 0.99999f, 0.999f));
    addParameter (feedbackTimeMS  = new juce::AudioParameterFloat ("feedbackTimeParmaseanID", "Feedback Time (MS)", 5.0f, 1000.0f, 100.0f));

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // storing sample rate to set delay by MS in process block
    this->sampleRate = sampleRate; // using "this->" to avoid coming up with a different name for these
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
   //  gam::Domain domain = gam::Domain::master();
    gam::Domain::master().spu(sampleRate);
    // osc.domain(domain);
    //player.load("/Users/jthompson/Desktop/sf/Mix_option_3.aif");
    player.load("C:/Users/Anderson/Downloads/Steely Dan - Do It Again.wav");
    delay.maxDelay(2.0f);
    
    history.resize (fftSize);
    history.clear();

    cpuMeter.prepare (sampleRate, samplesPerBlock);
    //these are now set at the beginning of the process block, controlled by either default param values or slider changes
//    delay.delaySamples(sampleRate * 0.1);
//    delay.fbk(0.99);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::AudioProcessLoadMeasurer::ScopedTimer m(cpuMeter.getLoadMeasurer()); 
    osc.freq(400);

    // setting here is once-per-block  ((Setting them like this is technically illegal b/c it's not thread-safe! I've seen this in production code though, just not good production code))
    delay.delaySamples ((*feedbackTimeMS) * 0.001f * sampleRate);
    delay.fbk (*feedbackAmmount);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (buffer.getNumChannels() < 1)
        return;

    auto* b = buffer.getWritePointer (0);
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        float s = osc() * 0.1f;
        player.rate(1);
        float sample = player();
        //sample = delay (sample);
        //sample = osc() * 0.1f;
      

        if (!doSpectralStuff) // if bypassing spectral
        {
            b[i] = sample; 
            continue; // skip the rest and go to next i;
        }

        // if(stft(sample)) 
        // {
        //     for(int k=0; k<stft.numBins(); ++k) 
        //     {
        //         stft.bin(k) = (stft.bin(k)*0.5) + (prevstft.bin(k) * 0.5);
        //         stft.bin(k)[1] = gam::rnd::uni(M_2PI);
        //         prevstft.bin(k) = stft.bin(k);
        //     }
        // }

        if(stft(sample)) 
        {
            for(int k=0; k<stft.numBins(); ++k) 
            {

                if (!keeperBins.contains (k))
                    stft.bin (k).mag (0.0f);
                
                if (randomizePhase)
                    stft.bin(k).arg (gam::rnd::uni(M_2PI));


                // so far this is just an expensive way to do nothing. 
                // The idea is to have a recursive feedback loop on fft frames
                // Things aren't working quite like I expect. 
                // in time domain audio, you just 
                // currentSample = historySample + currentSample;
                // historySample += currentSample;
                // 
                auto tempCurrentBin = stft.bin (k);
                auto tempHistoryBin = history[k];
                stft.bin(k) = (tempHistoryBin.mag ((1.0f - (*feedbackAmmount)) * tempHistoryBin.mag())) + 
                              (tempCurrentBin.mag (        (*feedbackAmmount)  * tempCurrentBin.mag()));
                history[k] += stft.bin(k);

                //history[k] += tempCurrentBin.mag (*feedbackAmmount * tempCurrentBin (k).mag() * 10.0f);
                //stft.bin (k).mag (*feedbackAmmount * stft.bin (k).mag() * 10.0f);
                //prevstft.bin(k) = stft.bin(k);
            }
        }
        // if (stft (sample))
        //     for (auto bin : stft)
        //         if (bin.mag() > 0.5f)
        //             bin.mag (0.0f);
    
        
           
        sample = stft();
        b[i] = sample;
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
