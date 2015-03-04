/*
  ==============================================================================

    AudioSourceProcessor.cpp
    Created: 5 Feb 2015 4:29:49pm
    Author:  susnak

  ==============================================================================
*/

#include "AudioSourceProcessor.h"

AudioSourceProcessor::AudioSourceProcessor(AudioSource* const inputSource,
                                           const bool takeOwnership)
: input(inputSource),audioSourceOwned(takeOwnership)
{

}
AudioSourceProcessor::~AudioSourceProcessor()
{
	if (audioSourceOwned)
		delete input;
}
void AudioSourceProcessor::prepareToPlay (double sampleRate,
					                           int estimatedSamplesPerBlock)
{
    this->setPlayConfigDetails (0,
                                2,
                                sampleRate, estimatedSamplesPerBlock);
    input->prepareToPlay (estimatedSamplesPerBlock, sampleRate);
}

void AudioSourceProcessor::releaseResources()
{
	input->releaseResources();

}
void AudioSourceProcessor::processBlock (AudioSampleBuffer& buffer,
				                         MidiBuffer& midiMessages)
{
	AudioSourceChannelInfo info;
	info.buffer=&buffer;
	info.startSample=0;
	info.numSamples=buffer.getNumSamples();

	input->getNextAudioBlock(info);
}


