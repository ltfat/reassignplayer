/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 4.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright (c) 2015 - ROLI Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_92E596E9AF51CD06__
#define __JUCE_HEADER_92E596E9AF51CD06__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "AudioHandler.h"
#include "Spectrogram.h"
#include "ReassignedBLFilterbank.h"
#include "MainGuiWidgets.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainContentComponent  : public Component,
    private ButtonListener,
    private ChangeListener,
    private LabelListener,
    private MenuBarModel
{
public:
    //==============================================================================
    MainContentComponent ();
    ~MainContentComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //==============================================================================
    // MenuBarModel related

    enum MENUITEMS { FILE = 0, OPTIONS, INFO };
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName) override;
    StringArray getMenuBarNames() override;

    void changeListenerCallback(ChangeBroadcaster* source) override;
    void buttonClicked (Button* b) override;
    void labelTextChanged (Label* l) override;

    void replaceFilterbank(ReassignedBLFilterbank* fb);
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ScopedPointer<AudioHandler> aHandler;
    ScopedPointer<Spectrogram> spectrogram;
    ScopedPointer<ReassignedBLFilterbank> filterbank;
    ScopedPointer<OpenGLContext> ogl;

    ScopedPointer<FilterbankDataHolder> dataHolder;


    // GUI
    ScopedPointer<MenuBarModel> menuBarModel;
    ScopedPointer<MenuBarComponent> menuBarComponent;
    Toolbar toolbar;
    ScopedPointer<ToolbarItemFactory> tbfac;
    Label fileLabel;

    ScopedPointer<PlaylistWindow> plWindow;

    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_92E596E9AF51CD06__
