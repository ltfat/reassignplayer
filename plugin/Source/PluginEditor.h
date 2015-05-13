/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_F1B68972952BE4__
#define __JUCE_HEADER_F1B68972952BE4__

//[Headers]     -- You can add your own extra header files here --
#ifdef STANDALONE
#define JUCE_DONT_DECLARE_PROJECTINFO 1
#endif

#include "../../plugin/JuceLibraryCode/JuceHeader.h"

#ifdef STANDALONE
#undef JUCE_DONT_DECLARE_PROJECTINFO
#endif

#include "PluginProcessor.h"
#include "Spectrogram.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class PluginEditor  : public AudioProcessorEditor,
                      public ButtonListener,
                      public ComboBoxListener,
                      public ChangeListener
{
public:
    //==============================================================================
    PluginEditor (PluginAudioProcessor& p);
    ~PluginEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel);
    void changeListenerCallback(ChangeBroadcaster *source);
    void comboBoxChanged (ComboBox* comboBox);
    void buttonClicked (Button* button);
    Spectrogram* getSpectrogram() const {return spectrogram; }
    FileFilter& getFileFilter() const {return *fileFilter; }

    void setSettings(PropertySet* settingsToUse){ settings = settingsToUse;};
    void loadFilters();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ScopedPointer<OpenGLContext> ogl;
    PluginAudioProcessor& processor;
    OwnedArray<DirectoryContentsList> dirContents;
    ScopedPointer<TimeSliceThread> tsThread;
    ScopedPointer<FileFilter> fileFilter;
    OwnedArray<File> filterbankFiles;

    PropertySet* settings;
    OwnedArray<Component> trash;


    // GUI components
    ScopedPointer<Button> reassignToggle;
    ScopedPointer<Button> showSelector;
    ScopedPointer<ComboBox> channelChooser;

    //[/UserVariables]

    //==============================================================================
    ScopedPointer<Spectrogram> spectrogram;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_F1B68972952BE4__
