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
PluginAudioProcessor::PluginAudioProcessor(Array<File> fbData)
    : fftBuf(nullptr),
      fftBufReplacing(nullptr),
      paramActChannel(0),
      paramReassignedSwitch(1)
{
    DBG("PLuginAudioProcessor constructor");
    dataHolder = new FilterbankDataHolder(fbData);
    // dataHolder->addChangeListenerToWindow(this);
}

PluginAudioProcessor::PluginAudioProcessor()
    : fftBuf(nullptr),
      fftBufReplacing(nullptr),
      paramActChannel(0),
      paramReassignedSwitch(1)
{
    DBG("Noarg PluginAudioProcessor constructor");

    /* Read the filterbank data from the resource file */
    int resLen = 0;
    const char* fb = PluginBinaryData::getNamedResource("default_lfb", resLen);
    MemoryBlock fbMemBlock(fb,resLen);
    const char* fbFgrad = PluginBinaryData::getNamedResource("default_fgrad_lfb", resLen);
    MemoryBlock fbFgradMemBlock(fbFgrad,resLen);
    const char* fbTgrad = PluginBinaryData::getNamedResource("default_tgrad_lfb", resLen);
    MemoryBlock fbTgradMemBlock(fbTgrad,resLen);

    Array<FilterbankDataHolder::BLFilterbankDef*> filterbankDefs;

    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(fbMemBlock,0));
    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(fbFgradMemBlock,0));
    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(fbTgradMemBlock,0));

    fftBuf = new RingReassignedBLFilterbankBuffer(filterbankDefs.getRawDataPointer(),
            PluginBinaryData::bufferLen, RingFFTBuffer::winType::hann, 1, 3);

    // dataHolder->addChangeListenerToWindow(this);
}

PluginAudioProcessor::~PluginAudioProcessor()
{
//   if(nullptr != fftBufReplacing.get()) delete fftBufReplacing.get();
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
    switch (index)
    {
    case kActChannel:
        return static_cast<float>(paramActChannel);
    case kReassignedSwitch:
        return static_cast<float>(paramReassignedSwitch);
    default:
        return 0.0f;
    }
}

void PluginAudioProcessor::setParameter (int index, float newValue)
{

    switch (index)
    {
    case kActChannel:
        paramActChannel = std::min(static_cast<int>(newValue), getNumInputChannels() - 1);
        break;
    case kReassignedSwitch:
        paramReassignedSwitch = std::fabs(newValue) < 1e-6 ? 0 : 1;
        break;
    default:
        break;
    }
}

const String PluginAudioProcessor::getParameterName (int index)
{
    switch (index)
    {
    case kActChannel:
        return "act. channel";
    case kReassignedSwitch:
        return "reassignment switch";
    default:
        return String::empty;
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

//==========================================
void PluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Choose next bigger supported BufferLength or throw an error

    // jassert(samplesPerBlock > bufLen / 2 );

    DBG("prepareToPlay in PluginAudioProcessor");
    //Array<File> files;
    //fftBuf = createRingBufferFromData();
    {
    // const MessageManagerLock mmlock;
    // This must be done elsewhere as the Editor is created by the host by at demand
    // We should also not try to store pointer to spectrogram as it
    // might become invalid when Editor is recreated 
    //PluginEditor* pe = dynamic_cast<PluginEditor*>(createEditorIfNeeded());
    //spectrogram = pe->getSpectrogram();
    //spectrogram->setSpectrogramSource(fftBuf);
    }
    DBG("prepareToPlay in PluginAudioProcessor end");
}

void PluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.

    // Lock is necessary here as we do not want to end up with
    // the editor being deleted in the middle of detaching the spectrogram
    //
    const MessageManagerLock mmlock;
    // We must first detach spectrogram to be able to safely remove fftBuf
    PluginEditor* pe = dynamic_cast<PluginEditor*>(getActiveEditor());

    if(nullptr != pe)
    {
        pe->getSpectrogram()->aboutToChangeSpectrogramSource();
        while (pe->getSpectrogram()->trySetSpectrogramSource(nullptr)) {}
    }
    // Delete the buffer
    fftBuf = nullptr;
    // Delete the replacing buffer if it exists
    if (nullptr != fftBufReplacing.get()) delete fftBufReplacing.get();
}

void PluginAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // Get the initial time
    double startTime = Time::getMillisecondCounterHiRes();

    // Clear excesive buffers
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // length of the loaded buffer
    int actBufLen = buffer.getNumSamples();
    // pointer to the buffer
    const float* srcPtr = buffer.getReadPointer(paramActChannel);
    // Get the replacing buffer
    RingFFTBuffer* repl = fftBufReplacing.get();

    if (nullptr == repl)
    {
        // Working with the same buffer as in the last iteration
        if (nullptr != fftBuf)
            fftBuf->appendSamples(&srcPtr, actBufLen);
    }
    else
    {
        // There is a new ring buffer
        // Changing fftBufReplacing is blocked until it is set to nullptr again;
        if (nullptr == fftBuf)
        {
            // This is easy...., the old fftBuf is freed automatically as it is a
            // ScopedPointer
            // We assume here that fftBuf was not set in spectrogram either
            fftBuf = repl;
            fftBuf->appendSamples(&srcPtr, actBufLen);

           // spectrogram->setSpectrogramSource(fftBuf);
            fftBufReplacing = nullptr;
        }
        else
        {
            /*
            // This is hard(er)
            // 1) Start appending samples to the next buffer
            // 2) Wait until the old buffer is empty and do the switch
            // 3) In the meantime, the new buffer might overflow..
            // Spectrogram must not try to read coefficients as we do the switch

            // Alert spectrogram about the intention to change the ring buffer
            spectrogram->aboutToChangeSpectrogramSource();

            fftBufReplacing.get()->appendSamples(&srcPtr, actBufLen);

            // Try changing the source in spectrogram...if it fails just
            // wait until the next iteration..
            // We might also allow to spend some time checking the availability
            if (spectrogram->trySetSpectrogramSource(repl))
            {
                fftBuf = repl;
                fftBufReplacing = nullptr;
            }
            */
        }
    }

    // Get the final time
    double endTime = Time::getMillisecondCounterHiRes();

    //spectrogram->setAudioLoopMs(endTime - startTime);

    if (nullptr != fftBuf && typeid(*fftBuf) == typeid(RingReassignedBLFilterbankBuffer) )
        (dynamic_cast<RingReassignedBLFilterbankBuffer*>(fftBuf.get()))->setActivePlotReassigned(paramReassignedSwitch);
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
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    DBG("noarg createPluginFilter called");
    return new PluginAudioProcessor();
}

AudioProcessor* JUCE_CALLTYPE createCustomPluginFilter(Array<File> filterbankData)
{
    DBG("createPluginFilter called");
    return new PluginAudioProcessor(filterbankData);
}


RingTransformBuffer* PluginAudioProcessor::getRingBuffer()
{
    return fftBuf;
}

bool PluginAudioProcessor::trySetRingBuffer(RingFFTBuffer* rtb)
{
// Number of buffers must be the same as in the old one?

    // We do not want this to change again until the switch is complete
    // returns false if fftBufReplacing was not nullptr
    return fftBufReplacing.compareAndSetBool(rtb, nullptr);
}

FilterbankDataHolder* PluginAudioProcessor::getFilterbankDataHolder()
{
    return dataHolder;
}

void PluginAudioProcessor::setFilterbankDataHolder(FilterbankDataHolder* dataHolder_)
{
    this->dataHolder = dataHolder_;
}



void PluginAudioProcessor::changeListenerCallback(ChangeBroadcaster* source)
{
    std::cout << "active Filterbank changed" << std::endl;
    RingFFTBuffer* tmpBuf = createRingBufferFromData();
    while ( !trySetRingBuffer(tmpBuf)) {}
}

RingFFTBuffer* PluginAudioProcessor::createRingBufferFromData()
{
    jassert(dataHolder);

    RingFFTBuffer* newBuf;
    try
    {

        Array<MemoryBlock> loadedFilterbankData;
        bool successful = dataHolder->getFilterbankData(loadedFilterbankData);
        if ( !successful )
            throw String("PluginProcessor failed to interpret filterbank data");

        Array<FilterbankDataHolder::BLFilterbankDef*> filterbankDefs;

        switch ( loadedFilterbankData.size() )
        {
        case 1:
            filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                               ::createDefFromData(loadedFilterbankData.getReference(0), dataHolder->getStartingByte(dataHolder->getActiveFilterbank())));
            newBuf = new RingBLFilterbankBuffer(filterbankDefs,
                                                dataHolder->getBlockLength(dataHolder->getActiveFilterbank()),
                                                RingFFTBuffer::winType::hann, 1, 3);
            break;
        case 3:
            for (int kk = 0; kk < 3; ++kk )
            {
                filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                                   ::createDefFromData(loadedFilterbankData.getReference(kk), dataHolder->getStartingByte(dataHolder->getActiveFilterbank())));
            }
            newBuf = new RingReassignedBLFilterbankBuffer(filterbankDefs.getRawDataPointer(),
                    dataHolder->getBlockLength(dataHolder->getActiveFilterbank()),
                    RingFFTBuffer::winType::hann, 1, 3);
            break;
        default:
            throw String("PluginProcessor: failed to interpret filterbank data");
        }
    }
    catch (String& thisException)
    {
        std::cout << thisException << std::endl;
    }
    return newBuf;
}

