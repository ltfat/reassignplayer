/*
  ==============================================================================

    StandalonePluginHolder.cpp
    Created: 5 Feb 2015 8:35:01pm
    Author:  susnak

  ==============================================================================
*/

#include "StandalonePluginHolder.h"
//#include "../../plugin/Source/PluginProcessor.h"


//extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();
extern AudioProcessor* JUCE_CALLTYPE createCustomPluginFilter(Array<File> fbData_);

//==============================================================================
/**
    An object that creates and plays a standalone instance of an AudioProcessor.

    The object will create your processor using the same createPluginFilter()
    function that the other plugin wrappers use, and will run it through the
    computer's audio/MIDI devices using AudioDeviceManager and AudioProcessorPlayer.

    The class employs AudioProcessorGraph containing 4 nodes:

      in               Graph input node        AudioProcessorGraph::AudioGraphIOProcessor
      out              Graph output node       AudioProcessorGraph::AudioGraphIOProcessor
      sourceProcessor  AudioSourceProcessor    AudioSource as AudioProcessor
      pluginProcessor  AudioPrcessor           containing the wrapped plugin

   The Nodes are connected as follows:

         [in]     [sourceProcessor]
            \    /
             \  /
        [pluginProcessor]
              |
              |
            [out]
*/


StandalonePluginHolder::StandalonePluginHolder (Array<File> fbData, PropertySet* settingsToUse,
      bool takeOwnershipOfSettings)
   : settings (settingsToUse, takeOwnershipOfSettings),
     thread("File preload"),
     currentFile(nullptr),
     filterbankData(fbData),
     sampleRate(44100),
     samplesPerBlock(512),
     currentSource(0),
     currentFileIdx(-1),
     loopState(0),
     oldStreamPosition(-1),
     currentFromPlaylist(false)
{
   DBG("StandalonePluginHolder constructor begin");
   String errMsg = setupAudioDevices();
   DBG(errMsg);
   if(errMsg.isNotEmpty())
   {
       DBG("Initialization failed!");
       throw errMsg;
   }

   // Initialization
   createPlugin();

   reloadPluginState();
   thread.startThread();
   formatManager.registerBasicFormats();

   transportSource.addChangeListener(this);

   //
   wasPlaying = false;
   //currentSource = 0;   // No input

   //
   sourceProcessor = new AudioSourceProcessor(&transportSource, false);
   sourceProcessor->prepareToPlay(sampleRate, samplesPerBlock);

   // Setup the graph
   processorGraph = new AudioProcessorGraph();
   processorGraph->setPlayConfigDetails(2, 2, sampleRate, samplesPerBlock);

   // Input and output node of the graph
   AudioProcessorGraph::AudioGraphIOProcessor* in =
      new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
   AudioProcessorGraph::AudioGraphIOProcessor* out =
      new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

   // Add nodes to graph, keep inNode and pluginNode as atributes
   inNode = processorGraph->addNode(in);
   AudioProcessorGraph::Node* sourceNode = processorGraph->addNode(sourceProcessor);
   pluginNode = processorGraph->addNode(pluginProcessor);
   AudioProcessorGraph::Node* outNode = processorGraph->addNode(out);

   // Connect everything to everything
   for (int ii = 0; ii < 2; ++ii)
   {
      processorGraph->addConnection(sourceNode->nodeId, ii, pluginNode->nodeId, ii);
      //processorGraph->addConnection(inNode->nodeId, ii, pluginNode->nodeId, ii);
      processorGraph->addConnection(pluginNode->nodeId, ii, outNode->nodeId, ii);
   }

   // Cut the connection between inNode and pluginNode
   //inputIsFileOnly();

   processorGraph->prepareToPlay(sampleRate, samplesPerBlock);
   startPlaying();
   DBG("StandalonePluginHolder constructor end");
}

StandalonePluginHolder::~StandalonePluginHolder()
{
   transportSource.setSource(nullptr);
   deletePlugin();
   shutDownAudioDevices();
}

//==============================================================================
void StandalonePluginHolder::createPlugin()
{
   AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Standalone);
   pluginProcessor = createCustomPluginFilter(filterbankData);
   jassert (pluginProcessor != nullptr); // Your createPluginFilter() function must return a valid object!
   AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);

   pluginProcessor->setPlayConfigDetails (JucePlugin_MaxNumInputChannels,
                                          JucePlugin_MaxNumOutputChannels,
                                          sampleRate, samplesPerBlock);
}

void StandalonePluginHolder::deletePlugin()
{
   stopPlaying();
   AudioProcessorEditor* e = pluginProcessor->getActiveEditor();
   if(nullptr!=e) delete e;
   pluginProcessor = nullptr;
}

bool StandalonePluginHolder::loadFile(File& file)
{
    if ( currentFromPlaylist )
        oldStreamPosition = transportSource.getNextReadPosition();
    setFile(file);
    currentFromPlaylist = false;
    return true;
}

bool StandalonePluginHolder::addFile(File& file)
{
    File* newFile = new File(file);
    listOfFiles.add(newFile);
    if ( listOfFiles.size() == 1 )
    {
        currentFileIdx = 0;
        setFile(file);
        currentFromPlaylist = true;
    }
    return true;
}

bool StandalonePluginHolder::removeFile(int fileIndex)
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

bool StandalonePluginHolder::clearFileList()
{
    listOfFiles.clear(true);
}

bool StandalonePluginHolder::setFile(File& file, int64 startingPosition)
{
   if ( file.existsAsFile() )
   {
        currentFile = new File(file);
        if (! loadFileIntoTransport())
            return false;

        inputIsFileOnly();
        if ( startingPosition < transportSource.getTotalLength() )
                transportSource.setNextReadPosition(startingPosition);
        transportSource.start();
        return true;
   }
   else
        return setNextFile();
}


//==============================================================================
File StandalonePluginHolder::getLastFile() const
{
   File f;

   if (settings != nullptr)
      f = File (settings->getValue ("lastStateFile"));

   if (f == File::nonexistent)
      f = File::getSpecialLocation (File::userDocumentsDirectory);

   return f;
}

void StandalonePluginHolder::setLastFile (const FileChooser& fc)
{
   if (settings != nullptr)
      settings->setValue ("lastStateFile", fc.getResult().getFullPathName());
}

/** Pops up a dialog letting the user save the processor's state to a file. */
void StandalonePluginHolder::askUserToSaveState (const String& fileSuffix)
{
   FileChooser fc (TRANS("Save current state"), getLastFile(), getFilePatterns (fileSuffix), false);

   if (fc.browseForFileToSave (true))
   {
      setLastFile (fc);

      MemoryBlock data;
      pluginProcessor->getStateInformation (data);

      if (! fc.getResult().replaceWithData (data.getData(), data.getSize()))
         AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                      TRANS("Error whilst saving"),
                                      TRANS("Couldn't write to the specified file!"));
   }
}

/** Pops up a dialog letting the user re-load the processor's state from a file. */
void StandalonePluginHolder::askUserToLoadState (const String& fileSuffix)
{
   FileChooser fc (TRANS("Load a saved state"), getLastFile(), getFilePatterns (fileSuffix), false);

   if (fc.browseForFileToOpen())
   {
      setLastFile (fc);

      MemoryBlock data;

      if (fc.getResult().loadFileAsData (data))
         pluginProcessor->setStateInformation (data.getData(), (int) data.getSize());
      else
         AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                      TRANS("Error whilst loading"),
                                      TRANS("Couldn't read from the specified file!"));
   }
}

//==============================================================================
void StandalonePluginHolder::startPlaying()
{
   player.setProcessor (processorGraph);
   //player.setProcessor (pluginProcessor);
}

void StandalonePluginHolder::stopPlaying()
{
   player.setProcessor (nullptr);
}

//==============================================================================
/** Shows an audio properties dialog box modally. */
void StandalonePluginHolder::showAudioSettingsDialog()
{
   DialogWindow::LaunchOptions o;
   if(nullptr == pluginProcessor) DBG("pluginProcessor is NULL");

   o.content.setOwned (new AudioDeviceSelectorComponent (deviceManager,
                       pluginProcessor->getNumInputChannels(),
                       pluginProcessor->getNumInputChannels(),
                       pluginProcessor->getNumOutputChannels(),
                       pluginProcessor->getNumOutputChannels(),
                       false, false,
                       true, false));
   o.content->setSize (500, 450);

   o.dialogTitle                   = TRANS("Audio Settings");
   o.dialogBackgroundColour        = Colour (0xfff0f0f0);
   o.escapeKeyTriggersCloseButton  = true;
   o.useNativeTitleBar             = true;
   o.resizable                     = false;

   o.launchAsync();
}

void StandalonePluginHolder::saveAudioDeviceState()
{
   if (settings != nullptr)
   {
      ScopedPointer<XmlElement> xml (deviceManager.createStateXml());
      settings->setValue ("audioSetup", xml);
   }
}

String StandalonePluginHolder::reloadAudioDeviceState()
{
   ScopedPointer<XmlElement> savedState;

   if (settings != nullptr)
      savedState = settings->getXmlValue ("audioSetup");

   DBG("Before inilialization");
   String errMsg = deviceManager.initialise (JucePlugin_MaxNumInputChannels,
                                              JucePlugin_MaxNumOutputChannels,
                                              savedState,
                                              true);
   DBG("After initialization\n" << errMsg );
   return errMsg;
}

//==============================================================================
void StandalonePluginHolder::savePluginState()
{
   std::cout << "savePluginState called " << std::endl;
   if (settings != nullptr && pluginProcessor != nullptr)
   {
      MemoryBlock data;
      pluginProcessor->getStateInformation (data);

      settings->setValue ("filterState", data.toBase64Encoding());
   }
}

void StandalonePluginHolder::reloadPluginState()
{
   if (settings != nullptr)
   {
      MemoryBlock data;

      if (data.fromBase64Encoding (settings->getValue ("filterState")) && data.getSize() > 0)
         pluginProcessor->setStateInformation (data.getData(), (int) data.getSize());
   }
}

//==============================================================================

bool StandalonePluginHolder::loadFileIntoTransport()
{
   transportSource.stop();
   transportSource.setSource(nullptr);
   formatReaderSource = nullptr;

   if (currentFile != nullptr)
   {
      // The old reader is killed together with formatReaderSource on the previous line
      reader = formatManager.createReaderFor(*currentFile);

      if (reader != nullptr)
      {
         formatReaderSource = new AudioFormatReaderSource(reader, true);
         formatReaderSource->setLooping(false);
         // We want the file to be read at the common sample-rate
         // Read up to samplesToPreload in advance
         int samplesToPreload = 10*reader->bitsPerSample/8*samplesPerBlock;
         transportSource.setSource(formatReaderSource, samplesToPreload, &thread, sampleRate);
         return true;
      }
      else
      {
         currentFile = nullptr;
         return false;
      }
   }
   return false;

}


String StandalonePluginHolder::setupAudioDevices()
{
    DBG("Before device manager");
   deviceManager.addAudioCallback (&player);
   deviceManager.addMidiInputCallback (String::empty, &player);

   return reloadAudioDeviceState();
}

void StandalonePluginHolder::shutDownAudioDevices()
{
   saveAudioDeviceState();

   deviceManager.removeMidiInputCallback (String::empty, &player);
   deviceManager.removeAudioCallback (&player);
}


void StandalonePluginHolder::inputIsMicOnly()
{
   if (! processorGraph->isConnected(inNode->nodeId, pluginNode->nodeId))
   {
      connectNodes(inNode->nodeId, pluginNode->nodeId);
   }

   if (transportSource.isPlaying())
   {
      wasPlaying = true;
      transportSource.stop();
   }
   else
   {
      wasPlaying = false;
   }

   currentSource = 2; // MIC/AUX input
}

void StandalonePluginHolder::inputIsFileOnly()
{
   // Just disconnect the node
   if ( processorGraph->isConnected(inNode->nodeId, pluginNode->nodeId))
   {
      processorGraph->disconnectNode(inNode->nodeId);
   }

   if (! transportSource.isPlaying() && wasPlaying)
   {
      transportSource.start();
   }

   currentSource = 1; // File input
}

int StandalonePluginHolder::getCurrentSource()
{
    return currentSource;
}

void StandalonePluginHolder::connectNodes(uint32 node1, uint32 node2)
{
   processorGraph->addConnection(node1, 0, node2, 0);
   processorGraph->addConnection(node1, 1, node2, 1);
}

// Control Playback

bool StandalonePluginHolder::changePlaybackState(int state)
{
    enum allPlaybackStates {PLAY = 1, PAUSE, STOP};
    bool wasP = transportSource.isPlaying();

    switch(state)
    {
    case PLAY:
        if (! wasP)
        {
            transportSource.start();
        }
        break;
    case PAUSE:
        if (wasP)
        {
            transportSource.stop();
        }
        break;
    case STOP:
        if (wasP)
        {
            transportSource.stop();
        }
        transportSource.setPosition(0.0);
        break;
    default:
        break;
    }

    return wasP;
}

// Loop button interface

void StandalonePluginHolder::toggleLooping()
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

void StandalonePluginHolder::changeListenerCallback(ChangeBroadcaster*)
{
    if ( transportSource.getNextReadPosition() > transportSource.getTotalLength() + 1)
    {
        DBG("stream end");
        if ( currentFileIdx < listOfFiles.size()-1 || loopState != 0 )
            setNextFile();
    }
}

bool StandalonePluginHolder::setNextFile()
{
    int64 startPosition = oldStreamPosition;
    if ( currentFile == nullptr )
    {
        DBG("Stopping playback");
        return false;
    }
    else if ( loopState == 2 || ( loopState == 1 && listOfFiles.size() == 0 ))
    {
        DBG("Replaying current file");
        DBG(loopState);
        currentFromPlaylist = false;
        return setFile(*currentFile);
    }
    else if ( listOfFiles.size() != 0 )
    {
        if ( currentFileIdx < listOfFiles.size()-1 )
        {
            DBG("Next file in playlist played");
            DBG(listOfFiles.size());
            currentFromPlaylist = true;
            sendChangeMessage();
            return setFile(*listOfFiles[++currentFileIdx]);
        }
        else if ( loopState == 1 )
        {
            DBG("Playlist exhausted, starting again");
            DBG(listOfFiles.size());
            currentFileIdx = 0;
            currentFromPlaylist = true;
            oldStreamPosition = -1;
            sendChangeMessage();
            return setFile(*listOfFiles[currentFileIdx]);
        }
        else if ( loopState == 2 && currentFile != nullptr )
        {
            DBG("Replay current playlist file");
            return setFile(*currentFile);
        }
    }
    else
    {
        DBG("Stopping playback unexpectedly");
        return false;
    }
}

int StandalonePluginHolder::getCurrentFileIdx()
{
    return currentFileIdx;
}

bool StandalonePluginHolder::setCurrentFileIdx(int newFileIdx)
{
    if ( newFileIdx > -1 && newFileIdx < listOfFiles.size())
    {
        if ( currentFileIdx != newFileIdx )
        {
            currentFileIdx = newFileIdx;
            setFile(*listOfFiles[currentFileIdx]);
            currentFromPlaylist = true;
        }
        return true;
    }
    return false;
}

bool StandalonePluginHolder::playNext()
{
    if ( currentFileIdx == listOfFiles.size()-1 )
        return setCurrentFileIdx(0);
    else
        return setCurrentFileIdx(currentFileIdx+1);
}

bool StandalonePluginHolder::playPrevious()
{
    if ( currentFileIdx == 0 )
        return setCurrentFileIdx(listOfFiles.size()-1);
    else
        return setCurrentFileIdx(currentFileIdx-1);
}
