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

class StandalonePluginHolder    : public ChangeListener,
                                  public ChangeBroadcaster
{
public:
    /** Creates an instance of the default plugin.

        The settings object can be a PropertySet that the class should use to
        store its settings - the object that is passed-in will be owned by this
        class and deleted automatically when no longer needed. (It can also be null)
    */
    StandalonePluginHolder ( PropertySet* settingsToUse, bool takeOwnershipOfSettings);

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
    //
    void startPlaying();
    void stopPlaying();
    void reloadPluginState();
    void savePluginState();
    //
    void saveAudioDeviceState();
    String reloadAudioDeviceState();
    void showAudioSettingsDialog();

    int getCurrentFileIdx();
    bool setCurrentFileIdx(int newFileIdx, bool forceStart = false);
    bool loadFile(File& file);
    bool addFile(File& file);
    bool clearFileList();
    bool removeFile(int fileIndex);

    bool playNext();
    bool playPrevious();

    // Playback control
    bool changePlaybackState(int state);
    void toggleLooping();

    // Graph manipulation routines
    void inputIsMicOnly();
    void inputIsFileOnly();
    int getCurrentSource();

    // Change Listener callback
    void changeListenerCallback(ChangeBroadcaster* source);

    // For holding properties
    OptionalScopedPointer<PropertySet> settings;

    // These has to be accesible from the main application
    AudioProcessor* const getPluginProcessor() const { return pluginProcessor; }
    AudioProcessorEditor* const getPluginEditor() const { return pluginProcessor->getActiveEditor(); }
    AudioDeviceManager& getDeviceManager() { return deviceManager; }
private:
    double sampleRate;
    int samplesPerBlock;
    int currentSource;
    bool wasPlaying;
    int loopState;
    bool currentFromPlaylist;
    int64 oldStreamPosition;
    Array<File> filterbankData;
    // Produces AudioFormatReader
    AudioFormatManager formatManager;
    // this is killed together with formatReaderSource
    // reader has the following public attributes:
    // .sampleRate
    // .lengthInSamples
    // .numChannels
    AudioFormatReader* reader;
    // This is wrapped in AudioProcessorSource
    // Preloads samples
    AudioTransportSource transportSource;
    // This is wrapped in AudioTransportSource
    ScopedPointer<AudioFormatReaderSource> formatReaderSource;

    //
    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;

    // Processor of the wrapped plugin
    // It is added to a graph which take controll over it
    AudioProcessor* pluginProcessor;
    // Processor wrapper for a AudioSource
    // Added to graph which takes controll
    AudioSourceProcessor* sourceProcessor;

    // Graph of processors, acts as AudioProcessor and it is
    // also used in AudioProcessorPlayer
    ScopedPointer<AudioProcessorGraph> processorGraph;


    //----------------------
    // Pointers to some nodes in the graph
    AudioProcessorGraph::Node* inNode;
    AudioProcessorGraph::Node* pluginNode;
    // background thread
    TimeSliceThread thread;

    int currentFileIdx;
    ScopedPointer<File> currentFile;
    OwnedArray<File> listOfFiles;

    bool setFile(File& file, int64 startingPosition = 0);
    bool setNextFile();
    bool loadFileIntoTransport();

    String setupAudioDevices();

    void shutDownAudioDevices();

    void connectNodes(uint32 node1,uint32 node2);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandalonePluginHolder)
};




#endif  // STANDALONEPLUGINHOLDER_H_INCLUDED
