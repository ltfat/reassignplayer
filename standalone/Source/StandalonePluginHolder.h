/*
  ==============================================================================

    StandalonePluginHolder.h
    Created: 5 Feb 2015 8:35:01pm
    Author:  susnak

  ==============================================================================
*/

#ifndef STANDALONEPLUGINHOLDER_H_INCLUDED
#define STANDALONEPLUGINHOLDER_H_INCLUDED

#ifdef STANDALONE
#define JUCE_DONT_DECLARE_PROJECTINFO 1
#endif

#include "../../plugin/JuceLibraryCode/JuceHeader.h"

#ifdef STANDALONE
#undef JUCE_DONT_DECLARE_PROJECTINFO
#endif

#include "AudioSourceProcessor.h"

class StandalonePluginHolder
{
public:
    /** Creates an instance of the default plugin.

        The settings object can be a PropertySet that the class should use to
        store its settings - the object that is passed-in will be owned by this
        class and deleted automatically when no longer needed. (It can also be null)
    */
    StandalonePluginHolder (PropertySet* settingsToUse, bool takeOwnershipOfSettings);

    ~StandalonePluginHolder();

    //==============================================================================
    void createPlugin();

    void deletePlugin();

    static String getFilePatterns (const String& fileSuffix)
    {
        if (fileSuffix.isEmpty())
            return String();

        return (fileSuffix.startsWithChar ('.') ? "*" : "*.") + fileSuffix;        
    }

    //==============================================================================
    File getLastFile() const;

    void setLastFile (const FileChooser& fc);

    /** Pops up a dialog letting the user save the processor's state to a file. */
    void askUserToSaveState (const String& fileSuffix = String());

    /** Pops up a dialog letting the user re-load the processor's state from a file. */
    void askUserToLoadState (const String& fileSuffix = String());

    //==============================================================================
    void startPlaying();
    void stopPlaying();
    void reloadPluginState();
    void saveAudioDeviceState();
    void reloadAudioDeviceState();

    //==============================================================================
    /** Shows an audio properties dialog box modally. */
    void showAudioSettingsDialog();

    //==============================================================================
    void savePluginState();

    //==============================================================================
    //

    void inputIsMicOnly();
    void inputIsFileOnly();

    OptionalScopedPointer<PropertySet> settings;
    ScopedPointer<AudioProcessor> processor;
    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;
    ScopedPointer<AudioProcessorGraph> processorGraph;
    AudioProcessorGraph::Node* inNode;
    AudioProcessorGraph::Node* pluginNode;

private:
    AudioTransportSource transportSource;
    ScopedPointer<AudioSourceProcessor> sourceProcessor;
    ScopedPointer<AudioFormatReaderSource> formatReaderSource;
    AudioFormatManager formatManager;
    TimeSliceThread thread;
    File openedFile;
    
    void loadFileIntoTransport(const File& file);

    void setupAudioDevices();

    void shutDownAudioDevices();

    void connectNodes(uint32 node1,uint32 node2);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandalonePluginHolder)
};




#endif  // STANDALONEPLUGINHOLDER_H_INCLUDED
