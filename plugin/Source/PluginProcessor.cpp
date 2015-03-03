/*
  ==============================================================================

    PluginProcessor.cpp
    Created: 5 Feb 2015 10:55:12am
    Author:  susnak

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
PluginAudioProcessor::PluginAudioProcessor(Array<File> fbData, PluginAudioProcessor::suppBufLens bufferLen_)
   : filterbankData(fbData),
     bufLen(static_cast<int>(bufferLen_)),
     paramActChannel(0),
     paramReassignedSwitch(1.0f)
{
   DBG("PLuginAudioProcessor constructor");
}

PluginAudioProcessor::~PluginAudioProcessor()
{
}

//==============================================================================
const String PluginAudioProcessor::getName() const
{
   return JucePlugin_Name;
}

int PluginAudioProcessor::getNumParameters()
{
   return kNumParams;
}

float PluginAudioProcessor::getParameter (int index)
{
   switch(index)
   {
      case kActChannel:         return static_cast<float>(paramActChannel);
      case kReassignedSwitch:   return static_cast<float>(paramReassignedSwitch);
      default:                  return 0.0f;
   }
}

void PluginAudioProcessor::setParameter (int index, float newValue)
{

   switch(index)
   {
      case kActChannel:         paramActChannel = std::min(static_cast<int>(newValue),getNumInputChannels()-1); break;
      case kReassignedSwitch:   paramReassignedSwitch = std::fabs(newValue)<1e-6?0:1; break;
      default:                  break;
   }
}

const String PluginAudioProcessor::getParameterName (int index)
{
   switch(index)
   {
      case kActChannel:         return "act. channel";
      case kReassignedSwitch:   return "reassignment switch";
      default:                  return String::empty;
   }
}

const String PluginAudioProcessor::getParameterText (int index)
{
   return String();
}

const String PluginAudioProcessor::getInputChannelName (int channelIndex) const
{

   return String (channelIndex + 1);
}

const String PluginAudioProcessor::getOutputChannelName (int channelIndex) const
{

   return String (channelIndex);
}

bool PluginAudioProcessor::isInputChannelStereoPair (int index) const
{
   return true;
}

bool PluginAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool PluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
   return true;
#else
   return false;
#endif
}

bool PluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
   return true;
#else
   return false;
#endif
}

bool PluginAudioProcessor::silenceInProducesSilenceOut() const
{
   return false;
}

double PluginAudioProcessor::getTailLengthSeconds() const
{
   return 0.0;
}

int PluginAudioProcessor::getNumPrograms()
{
   return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
   // so this should be at least 1, even if you're not really implementing programs.
}

int PluginAudioProcessor::getCurrentProgram()
{
   return 0;
}

void PluginAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String PluginAudioProcessor::getProgramName (int index)
{
   return String();

}

void PluginAudioProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

//==============================================================================
void PluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Choose next bigger supported BufferLength or throw an error

   jassert(samplesPerBlock > bufLen / 2 );

   //Array<File> files;
   try
   {
        fftBuf = new RingBLFilterbankBuffer(filterbankData,bufLen,RingFFTBuffer::winType::hann,1,3);
   }
   catch(String& thisException)
   {
        std::cout << thisException << std::endl;
   }
   //fftBuf = new RingFFTBuffer(bufLen,RingFFTBuffer::winType::hann,1,3);
   PluginEditor* pe = dynamic_cast<PluginEditor*>(createEditorIfNeeded());
   pe->getSpectrogram()->setSpectrogramSource(fftBuf);
   // The buffer will notify listeners when a new FFT buffer if available
   //fftBuf->addChangeListener(dynamic_cast<PluginEditor*>(createEditorIfNeeded()));
}

void PluginAudioProcessor::releaseResources()
{
   // When playback stops, you can use this as an opportunity to free up any
   // spare memory, etc.
   //
   //
   fftBuf = nullptr;
}

void PluginAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
   double startTime = Time::getMillisecondCounterHiRes();
   // Clear excesive buffers
   for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
      buffer.clear (i, 0, buffer.getNumSamples());

   // Start rebuffering
   int actBufLen = buffer.getNumSamples();
   const float* srcPtr = buffer.getReadPointer(paramActChannel);
   // Add samples to the buffer
   fftBuf->appendSamples(&srcPtr, actBufLen);
   (dynamic_cast<PluginEditor*>(getActiveEditor()))->getSpectrogram()->setAudioLoopMs(Time::getMillisecondCounterHiRes()-startTime);
}

//==============================================================================
bool PluginAudioProcessor::hasEditor() const
{
   return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PluginAudioProcessor::createEditor()
{

   PluginEditor* p = new PluginEditor (*this);
   return p;
}

//==============================================================================
void PluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
   // You should use this method to restore your parameters from this memory block,
   // whose contents will have been created by the getStateInformation() call.
}



//==============================================================================
// This creates new instances of the plugin..
/*AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
   Array<File> fbData;
   return new PluginAudioProcessor(fbData);
}*/

AudioProcessor* JUCE_CALLTYPE createCustomPluginFilter(Array<File> filterbankData)
{
   return new PluginAudioProcessor(filterbankData);
}


RingTransformBuffer* PluginAudioProcessor::getRingBuffer()
{
   return fftBuf;
}
