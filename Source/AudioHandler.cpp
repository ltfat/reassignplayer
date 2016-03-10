/*
  ==============================================================================

    AudioHandler.cpp
    Created: 19 Oct 2015 11:23:50pm
    Author:  susnak

  ==============================================================================
*/

#include "AudioHandler.h"

static ScopedPointer<AudioDeviceManager> oneAudioDeviceManager;

int AudioHandler::getCurrentSource()
{
    return inputIndex.load();
}

void AudioHandler::setInputIsMic()
{
    inputIndex.store(1);
}
void AudioHandler::setInputIsFile()
{
    inputIndex.store(2);
}
void AudioHandler::setInputIsZero()
{
    inputIndex.store(0);
}

AudioHandler::AudioHandler():
    filePreloadThread("filePreThread"),
    filterbank(nullptr)
{
    formatManager.registerBasicFormats();
    formatManager.registerFormat(new MP3AudioFormat(), true);
    formatManager.registerFormat(new OggVorbisAudioFormat(), true);
    formatManager.registerFormat(new FlacAudioFormat(), true);

    filePreloadThread.startThread();
    setInputIsMic();

    AudioDeviceManager& adm = AudioHandler::getAudioDeviceManager();
    adm.addAudioCallback(this);
}

AudioHandler::~AudioHandler()
{
    filePreloadThread.stopThread(1000);
    transportSource.stop();
    transportSource.setSource(nullptr);

    oneAudioDeviceManager->removeAudioCallback(this);
    oneAudioDeviceManager->closeAudioDevice();
    oneAudioDeviceManager = nullptr;
}

void AudioHandler::audioDeviceIOCallback (
    const float **inputChannelData, int numInputChannels,
    float **outputChannelData, int numOutputChannels, int numSamples)
{
    switch (inputIndex.load())
    {
    case 0: // Zero-out all optput channels
    {
        for (int ii = 0; ii < numOutputChannels; ii++)
        {
            memset(outputChannelData[ii], 0, numSamples * sizeof(float));
        }
    }
    break;
    case 1: // Copy input channels to output channels one to one, zero-out execessive channels
    {
        int minNumChannels = std::min(numInputChannels, numOutputChannels);
        for (int ii = 0; ii < minNumChannels; ii++)
        {
            memcpy(outputChannelData[ii], inputChannelData[0], numSamples * sizeof(float));
        }
        for (int ii = minNumChannels; ii < numOutputChannels; ii++)
        {
            memset(outputChannelData[ii], 0, numSamples * sizeof(float));
        }

        if (numOutputChannels > 0 && nullptr != filterbank.get())
        {
            filterbank.get()->appendSamples(outputChannelData[0], numSamples);
        }
    }
    break;
    case 2: // Read data from a file
    {
        AudioSampleBuffer buf(outputChannelData, numOutputChannels, numSamples);
        AudioSourceChannelInfo info;
        info.buffer = &buf;
        info.startSample = 0;
        info.numSamples = numSamples;

        transportSource.getNextAudioBlock(info);

        if (numOutputChannels > 0 && nullptr != filterbank.get())
        {
            filterbank.get()->appendSamples(outputChannelData[0], numSamples);
        }
    }
    break;
    }

}

void AudioHandler::audioDeviceAboutToStart (AudioIODevice *device)
{
    transportSource.prepareToPlay(device->getCurrentBufferSizeSamples(), device->getCurrentSampleRate());
    // asChanInfo = new AudioSourceChannelInfo();
}

void AudioHandler::audioDeviceStopped()
{
}

void AudioHandler::audioDeviceError (const String &errorMessage)
{

}

void AudioHandler::loadFileIntoTransportAndStart(File f)
{
    AudioDeviceManager& adm = AudioHandler::getAudioDeviceManager();
    AudioIODevice* aiod = adm.getCurrentAudioDevice();

    transportSource.stop();
    transportSource.setSource(nullptr);
    formatReaderSource = nullptr;

    // The old reader is killed together with formatReaderSource on the previous line
    reader = formatManager.createReaderFor(f);

    if (reader != nullptr)
    {
        formatReaderSource = new AudioFormatReaderSource(reader, true);
        formatReaderSource->setLooping(false);
        // We want the file to be read at the common sample-rate
        // Read up to samplesToPreload in advance
        int samplesToPreload = 10 * reader->bitsPerSample / 8 * aiod->getCurrentBufferSizeSamples();
        transportSource.setSource(formatReaderSource, samplesToPreload,
                                  &filePreloadThread, aiod->getCurrentSampleRate());
        transportSource.start();
        setInputIsFile();
    }
}


AudioDeviceManager& AudioHandler::getAudioDeviceManager()
{
    if (oneAudioDeviceManager == nullptr)
    {
        oneAudioDeviceManager = new AudioDeviceManager();
        oneAudioDeviceManager->initialiseWithDefaultDevices (0, 2);
        // oneAudioDeviceManager->initialise (2, 2, 0, true, String::empty, 0);
    }

    return *oneAudioDeviceManager;
}

AudioIODevice* AudioHandler::getCurrentAudioDevice()
{
   return AudioHandler::getAudioDeviceManager().getCurrentAudioDevice();
}

void AudioHandler::showAudioDeviceManagerDialog()
{

    DialogWindow::LaunchOptions o;

    o.content.setOwned (new AudioDeviceSelectorComponent (AudioHandler::getAudioDeviceManager(),
                        0, 2, 0, 2, false, false, true, false));

    o.content->setSize (500, 450);

    o.dialogTitle                   = TRANS("Audio Settings");
    o.dialogBackgroundColour        = Colour (0xfff0f0f0);
    o.escapeKeyTriggersCloseButton  = true;
    o.useNativeTitleBar             = true;
    o.resizable                     = false;

    o.launchAsync();
}

void AudioHandler::showFileChooserDialog()
{
    FileChooser myChooser ("Please select the moose you want to load...",
                           File::getSpecialLocation (File::userHomeDirectory),
                           "*.mp3,*.*");
    if (myChooser.browseForFileToOpen())
    {
        loadFileIntoTransportAndStart(File(myChooser.getResult()));
    }
}

bool AudioHandler::startPlaying()
{
    bool wasP = transportSource.isPlaying();
    if (!wasP)
        transportSource.start();

    return wasP;
}

bool AudioHandler::pausePlaying()
{
    bool wasP = transportSource.isPlaying();
    if (wasP)
        transportSource.stop();

    return wasP;
}

bool AudioHandler::stopPlaying()
{

    bool wasP = transportSource.isPlaying();
    if (wasP)
    {
        transportSource.stop();
        transportSource.setPosition(0);
    }
    return wasP;
}

bool AudioHandler::addFile(File& file)
{
    File* newFile = new File(file);
    listOfFiles.add(newFile);
    if ( listOfFiles.size() == 1 )
    {
        currentFileIdx = 0;
        loadFileIntoTransportAndStart(*newFile);
        currentFromPlaylist = true;
    }
    return true;
}

int AudioHandler::getCurrentFileIdx()
{
    return currentFileIdx;
}

bool AudioHandler::setCurrentFileIdx(int newFileIdx, bool forceStart)
{
    if ( newFileIdx > -1 && newFileIdx < listOfFiles.size())
    {
        if ( currentFileIdx != newFileIdx || forceStart )
        {
            currentFileIdx = newFileIdx;
            loadFileIntoTransportAndStart(*listOfFiles[currentFileIdx]);
            currentFromPlaylist = true;
        }
        return true;
    }
    return false;
}

void AudioHandler::toggleLooping()
{
    loopState = (loopState+1) % 3;
    //if (transportSource != nullptr)
    //{
    /*    if (formatReaderSource->isLooping())
            formatReaderSource->setLooping(false);
        else
            formatReaderSource->setLooping(true);*/
    //}
}

bool AudioHandler::removeFile(int fileIndex)
{
    if ( fileIndex < listOfFiles.size())
    {
        listOfFiles.remove(fileIndex,true);
        if ( currentFileIdx >= fileIndex )
            currentFileIdx--;
        //else if ( currentFileIdx = fileIndex )
        //    currentFileIdx = -1;
        return true;
    }
    else
        return false;
}
