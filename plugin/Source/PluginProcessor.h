/*
  ==============================================================================

    PluginProcessor.h
    Created: 5 Feb 2015 10:55:12am
    Author:  susnak

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED


#ifdef STANDALONE
#define JUCE_DONT_DECLARE_PROJECTINFO 1
#endif

#include "../../plugin/JuceLibraryCode/JuceHeader.h"
#include "ltfat.h"
#include "RingTransformBuffer.h"
//Because of the memcpy
#include <cstring>

#ifdef STANDALONE
#undef JUCE_DONT_DECLARE_PROJECTINFO
#endif



//==============================================================================
/**
*/
class PluginAudioProcessor  : public AudioProcessor
{
public:
   // We want to limit range of supported buffers as we have filterbanks only for
   // distinct lengths
   enum Params
   {
      kActChannel = 0,
      kReassignedSwitch,
      kNumParams
   };

   enum class suppBufLens
   {
      b2048 = 2048, empty
   };
   //==============================================================================
   PluginAudioProcessor(Array<File> fbData, suppBufLens bufferLen_ = suppBufLens::b2048);
   ~PluginAudioProcessor();

   //==============================================================================
   void prepareToPlay (double sampleRate, int samplesPerBlock) override;
   void releaseResources() override;

   void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

   //==============================================================================
   AudioProcessorEditor* createEditor() override;
   bool hasEditor() const override;

   //==============================================================================
   const String getName() const override;

   int getNumParameters() override;
   float getParameter (int index) override;
   void setParameter (int index, float newValue) override;

   const String getParameterName (int index) override;
   const String getParameterText (int index) override;


   const String getInputChannelName (int channelIndex) const override;
   const String getOutputChannelName (int channelIndex) const override;
   bool isInputChannelStereoPair (int index) const override;
   bool isOutputChannelStereoPair (int index) const override;

   bool acceptsMidi() const override;
   bool producesMidi() const override;
   bool silenceInProducesSilenceOut() const override;
   double getTailLengthSeconds() const override;

   //==============================================================================
   int getNumPrograms() override;
   int getCurrentProgram() override;
   void setCurrentProgram (int index) override;
   const String getProgramName (int index) override;
   void changeProgramName (int index, const String& newName) override;

   //==============================================================================
   void getStateInformation (MemoryBlock& destData) override;
   void setStateInformation (const void* data, int sizeInBytes) override;

   //void setBufferLen(PluginAudioProcessor::supportedBufferLengths bLen);
   RingTransformBuffer* getRingBuffer();

   AudioProcessor* JUCE_CALLTYPE createCustomPluginFilter();
private:
   int bufLen;
   Array<File> filterbankData;
   ScopedPointer<RingReassignedBLFilterbankBuffer> fftBuf;
   // Parameters
   int paramActChannel, paramReassignedSwitch;

   //==============================================================================
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED

