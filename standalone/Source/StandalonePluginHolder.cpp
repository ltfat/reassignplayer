/*
  ==============================================================================

    StandalonePluginHolder.cpp
    Created: 5 Feb 2015 8:35:01pm
    Author:  susnak

  ==============================================================================
*/

#include "StandalonePluginHolder.h"


extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();

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


StandalonePluginHolder::StandalonePluginHolder (PropertySet* settingsToUse,
                                                bool takeOwnershipOfSettings)
   : thread("File preload"),
     settings (settingsToUse, takeOwnershipOfSettings)
{
   createPlugin();
   setupAudioDevices();
   reloadPluginState();
   thread.startThread();


   formatManager.registerBasicFormats();
   loadFileIntoTransport(File::getCurrentWorkingDirectory().getChildFile("build/serj.wav"));
   sourceProcessor = new AudioSourceProcessor(&transportSource, false);
   sourceProcessor->prepareToPlay(44100, 512);

   processorGraph = new AudioProcessorGraph();
   processorGraph->setPlayConfigDetails(2, 2, 44100, 512);
   // It is a class attribute so it could be suspened
   AudioProcessorGraph::AudioGraphIOProcessor* in =
      new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
   AudioProcessorGraph::AudioGraphIOProcessor* out =
      new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

   inNode = processorGraph->addNode(in);
   AudioProcessorGraph::Node* sourceNode = processorGraph->addNode(sourceProcessor);
   pluginNode = processorGraph->addNode(processor);
   AudioProcessorGraph::Node* outNode = processorGraph->addNode(out);

   // Connect everything to everything
   for (int ii = 0; ii < 2; ++ii)
   {
      processorGraph->addConnection(sourceNode->nodeId, ii, pluginNode->nodeId, ii);
      processorGraph->addConnection(inNode->nodeId, ii, pluginNode->nodeId, ii);
      processorGraph->addConnection(pluginNode->nodeId, ii, outNode->nodeId, ii);
   }
  
   // Cut the connection between inNode and pluginNode 
   inputIsFileOnly();

   processorGraph->prepareToPlay(44100, 512);
   startPlaying();
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
   processor = createPluginFilter();
   jassert (processor != nullptr); // Your createPluginFilter() function must return a valid object!
   AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);

   processor->setPlayConfigDetails (JucePlugin_MaxNumInputChannels,
                                    JucePlugin_MaxNumOutputChannels,
                                    44100, 512);
}

void StandalonePluginHolder::deletePlugin()
{
   stopPlaying();
   processor = nullptr;
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
      processor->getStateInformation (data);

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
         processor->setStateInformation (data.getData(), (int) data.getSize());
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
   o.content.setOwned (new AudioDeviceSelectorComponent (deviceManager,
                       processor->getNumInputChannels(),
                       processor->getNumInputChannels(),
                       processor->getNumOutputChannels(),
                       processor->getNumOutputChannels(),
                       true, false,
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

void StandalonePluginHolder::reloadAudioDeviceState()
{
   ScopedPointer<XmlElement> savedState;

   if (settings != nullptr)
      savedState = settings->getXmlValue ("audioSetup");

   deviceManager.initialise (processor->getNumInputChannels(),
                             processor->getNumOutputChannels(),
                             savedState,
                             true);
}

//==============================================================================
void StandalonePluginHolder::savePluginState()
{
   if (settings != nullptr && processor != nullptr)
   {
      MemoryBlock data;
      processor->getStateInformation (data);

      settings->setValue ("filterState", data.toBase64Encoding());
   }
}

void StandalonePluginHolder::reloadPluginState()
{
   if (settings != nullptr)
   {
      MemoryBlock data;

      if (data.fromBase64Encoding (settings->getValue ("filterState")) && data.getSize() > 0)
         processor->setStateInformation (data.getData(), (int) data.getSize());
   }
}

//==============================================================================


void StandalonePluginHolder::loadFileIntoTransport(const File& file)
{
   transportSource.stop();
   transportSource.setSource(nullptr);
   formatReaderSource = nullptr;

   AudioFormatReader* reader = formatManager.createReaderFor(file);

   if (reader != nullptr)
   {
      formatReaderSource = new AudioFormatReaderSource(reader, true);
      formatReaderSource->setLooping(true);
      transportSource.setSource(formatReaderSource, 32768, &thread);/*
                                      &thread,
                                      reader->sampleRate);*/
      openedFile = file;
   }
   else
   {
      std::wcout << file.getFileName().toWideCharPointer() << " does not exist." << std::endl;
   }

}


void StandalonePluginHolder::setupAudioDevices()
{
   deviceManager.addAudioCallback (&player);
   deviceManager.addMidiInputCallback (String::empty, &player);

   reloadAudioDeviceState();
}

void StandalonePluginHolder::shutDownAudioDevices()
{
   saveAudioDeviceState();

   deviceManager.removeMidiInputCallback (String::empty, &player);
   deviceManager.removeAudioCallback (&player);
}

void StandalonePluginHolder::inputIsMicOnly()
{
   if(! processorGraph->isConnected(inNode->nodeId,pluginNode->nodeId))
   {
      connectNodes(inNode->nodeId,pluginNode->nodeId);
   }

   if(transportSource.isPlaying())
   {
      transportSource.stop();
   }

}

void StandalonePluginHolder::inputIsFileOnly()
{
   // Just disconnect the node
   if( processorGraph->isConnected(inNode->nodeId,pluginNode->nodeId))
   {
      processorGraph->disconnectNode(inNode->nodeId);
   }

   if(! transportSource.isPlaying())
   {
      transportSource.start();
   }
}

void StandalonePluginHolder::connectNodes(uint32 node1,uint32 node2)
{
   processorGraph->addConnection(node1,0, node2,0);
   processorGraph->addConnection(node1,1, node2,1);
}

