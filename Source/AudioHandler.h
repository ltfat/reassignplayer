/*
  ==============================================================================

    AudioHandler.h
    Created: 19 Oct 2015 11:23:50pm
    Author:  susnak

  ==============================================================================
*/

#pragma once

#define _USE_MATH_DEFINES
#include "ReassignedBLFilterbank.h"
#include "fftw3.h"
#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_audio_utils/juce_audio_utils.h"

class AudioHandler : public juce::AudioIODeviceCallback,
                     public juce::ChangeListener,
                     public juce::ChangeBroadcaster
{
public:
    AudioHandler();
    ~AudioHandler();

    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                                   int numInputChannels,
                                                   float* const* outputChannelData,
                                                   int numOutputChannels,
                                                   int numSamples,
                                                   const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;

    void audioDeviceStopped() override;

    void audioDeviceError (const juce::String& errorMessage) override;

    void loadFileIntoTransportAndStart (juce::File& f,
        juce::int64 startingPosition = 0);

    void setInputIsMic();
    void setInputIsFile();
    void setInputIsZero();

    bool startPlaying();
    bool pausePlaying();
    bool stopPlaying();
    bool addFile (juce::File& file);
    int getCurrentFileIdx();
    bool setCurrentFileIdx (int newFileIdx, bool forceStart = false);
    void toggleLooping();
    bool playNext();
    bool playPrevious();
    bool removeFile (int fileIndex);
    void clearFileList();

    int getCurrentSource();

    void showAudioDeviceManagerDialog();
    void showFileChooserDialog();
    juce::AudioIODevice* getCurrentAudioDevice();

    static juce::AudioDeviceManager& getAudioDeviceManager();

    void setFilterbank (ReassignedBLFilterbank* filterbank_)
    {
        filterbank.set (filterbank_);
    }

    // Change Listener callback
    void changeListenerCallback (ChangeBroadcaster* source);

private:
    juce::Atomic<ReassignedBLFilterbank*> filterbank;
    std::atomic_int inputIndex; // 0 - no input, 1 - mic, 2 - file

    juce::ScopedPointer<juce::AudioSource> inputFileSource;
    // Produces AudioFormatReader
    juce::AudioFormatManager formatManager;
    // this is killed together with formatReaderSource
    // reader has the following public attributes:
    // .sampleRate
    // .lengthInSamples
    // .numChannels
    juce::AudioFormatReader* reader;
    // This is wrapped in AudioProcessorSource
    // Preloads samples
    juce::AudioTransportSource transportSource;
    // This is wrapped in AudioTransportSource
    juce::ScopedPointer<juce::AudioFormatReaderSource> formatReaderSource;

    juce::TimeSliceThread filePreloadThread;

    bool currentFromPlaylist;
    int currentFileIdx;
    void setNextFile();
    int loopState;
    juce::OwnedArray<juce::File> listOfFiles;
    juce::ScopedPointer<juce::File> currentFile;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioHandler)
};
