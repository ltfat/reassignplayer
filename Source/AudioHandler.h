/*
  ==============================================================================

    AudioHandler.h
    Created: 19 Oct 2015 11:23:50pm
    Author:  susnak

  ==============================================================================
*/

#ifndef AUDIOHANDLER_H_INCLUDED
#define AUDIOHANDLER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include "fftw3.h"
#include <atomic>
#include "ReassignedBLFilterbank.h"


class AudioHandler : public AudioIODeviceCallback,
                     public ChangeListener,
                     public ChangeBroadcaster
{
public:
    AudioHandler();
    ~AudioHandler();

    void audioDeviceIOCallback (const float **inputChannelData,
                                int numInputChannels,
                                float **outputChannelData,
                                int numOutputChannels, int numSamples) override;

    void audioDeviceAboutToStart (AudioIODevice *device) override;

    void audioDeviceStopped() override;

    void audioDeviceError (const String &errorMessage) override;

    void loadFileIntoTransportAndStart(File& f, int64 startingPosition = 0);

    void setInputIsMic();
    void setInputIsFile();
    void setInputIsZero();

    bool startPlaying();
    bool pausePlaying();
    bool stopPlaying();
    bool addFile(File& file);
    int getCurrentFileIdx();
    bool setCurrentFileIdx(int newFileIdx, bool forceStart = false);
    void toggleLooping();
    bool playNext();
    bool playPrevious();
    bool removeFile(int fileIndex);
    void clearFileList();

    int getCurrentSource();

    void showAudioDeviceManagerDialog();
    void showFileChooserDialog();
    AudioIODevice* getCurrentAudioDevice();

    static AudioDeviceManager& getAudioDeviceManager();

    void setFilterbank(ReassignedBLFilterbank* filterbank_){filterbank.set(filterbank_);}

    // Change Listener callback
    void changeListenerCallback(ChangeBroadcaster* source);
private:
    Atomic<ReassignedBLFilterbank*> filterbank;
    std::atomic_int inputIndex; // 0 - no input, 1 - mic, 2 - file

    ScopedPointer<AudioSource> inputFileSource;
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

    TimeSliceThread filePreloadThread;

    bool currentFromPlaylist;
    int currentFileIdx;
    void setNextFile();
    int loopState;
    OwnedArray<File> listOfFiles;
    ScopedPointer<File> currentFile;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioHandler)
};








#endif  // AUDIOHANDLER_H_INCLUDED
