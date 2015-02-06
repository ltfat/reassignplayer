/*
  ==============================================================================

    AudioSourceProcessor.h
    Created: 5 Feb 2015 4:29:49pm
    Author:  susnak

  =============================================================================
  
  Based on:
  http://www.juce.com/forum/topic/audiosamplebuffer-speed-ups
*/

#ifndef AUDIOSOURCEPROCESSOR_H_INCLUDED
#define AUDIOSOURCEPROCESSOR_H_INCLUDED
#include "JuceHeader.h"

class AudioSourceProcessor : public AudioProcessor
{
public:
    AudioSourceProcessor(AudioSource* const s, bool takeOwnership);
    virtual ~AudioSourceProcessor();
    
    void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock) override;
    
    void releaseResources() override;
    void processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessage) override;
    
    bool silenceInProducesSilenceOut() const { return true; }
    double getTailLengthSeconds() const { return 0.0; }
    bool hasEditor() const { return false; }
    
    const String getName() const { return L"AudioSource wrapper"; }
    
    const String getInputChannelName(const int channelIndex) const { return String(channelIndex + 1); }
    const String getOutputChannelName(const int channelIndex) const { return String(channelIndex + 1); }
    
    bool isInputChannelStereoPair (int index)	const	{ return false;	}
	bool isOutputChannelStereoPair (int index)	const	{ return false; }
	bool acceptsMidi() const							{ return false; }
	bool producesMidi() const							{ return false; }
	
	AudioProcessorEditor* createEditor()						{ return nullptr; }
	int getNumParameters()										{ return 0; }
	const String getParameterName (int parameterIndex)			{ return String::empty; }
	float getParameter (int parameterIndex)						{ return 0.0; }
	const String getParameterText (int parameterIndex)			{ return String::empty; }
	void setParameter (int parameterIndex, float newValue)		{ }
	int getNumPrograms()										{ return 0; }
	int getCurrentProgram()										{ return 0;	}
	void setCurrentProgram (int index)                          { }
	const String getProgramName (int index)                     { return String::empty; }
	void changeProgramName (int index, const String& newName)   { }
	void getStateInformation (MemoryBlock& destData)			{ }
	void setStateInformation (const void* data,int sizeInBytes)	{ }
private:
	AudioSource* const input;
	const bool audioSourceOwned;

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSourceProcessor)
};



#endif  // AUDIOSOURCEPROCESSOR_H_INCLUDED
