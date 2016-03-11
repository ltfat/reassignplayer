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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MainContentComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainContentComponent::MainContentComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    // Manubar
    menuBarComponent = new MenuBarComponent(this);
    addAndMakeVisible(menuBarComponent);
    // Toolbar
    tbfac = new MainToolbarItemFactory(this);
    toolbar.addDefaultItems(*tbfac);
    toolbar.setComponentID("mainToolbar");
    addAndMakeVisible(toolbar);


    //[/Constructor_pre]


    //[UserPreSize]
    aHandler = new AudioHandler();
    spectrogram = new Spectrogram();
    filterbank = ReassignedBLFilterbank::makeDefault();
    aHandler->setFilterbank(filterbank);
    spectrogram->setSpectrogramSource(filterbank);
    ogl = new OpenGLContext();
    ogl->setContinuousRepainting(true);
    ogl->attachTo(*spectrogram);

    addAndMakeVisible(spectrogram);

    // Label
    fileLabel.setText(String("No file loaded"), dontSendNotification);
    fileLabel.setColour(Label::textColourId, Colours::red);
    // fileLabel.setColour(Label::backgroundColourId, backgroundColour);
    addAndMakeVisible(fileLabel);
    plWindow = new PlaylistWindow(this, aHandler);


    //[/UserPreSize]

    setSize (800, 600);


    //[Constructor] You can add your own custom stuff here..

    //[/Constructor]
}

MainContentComponent::~MainContentComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    ogl->detach();
    ogl = nullptr;
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    aHandler = nullptr;
    spectrogram = nullptr;
    filterbank = nullptr;
    menuBarComponent = nullptr;
    menuBarModel = nullptr;
    //[/Destructor]
}

//==============================================================================
void MainContentComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainContentComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    menuBarComponent->setBounds(0, 0, getWidth(), 20);
    spectrogram->setBounds (0, 20, getWidth(), getHeight() - 80);
    toolbar.setBounds(0, getHeight() - 60, getWidth(), 40);
    fileLabel.setBounds(0, getHeight() - 20, getWidth(), 20);
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
Component* createMainContentComponent()
{
    return new MainContentComponent();
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainContentComponent" componentName=""
                 parentClasses="public Component, private ButtonListener, private LabelListener, private MenuBarModel"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="0" initialWidth="800"
                 initialHeight="600">
  <BACKGROUND backgroundColour="ffffffff"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...

void MainContentComponent::replaceFilterbank(ReassignedBLFilterbank* fb)
{
    aHandler->setFilterbank(fb);
    spectrogram->stopPlotting();
    spectrogram->setSpectrogramSource(fb);
    filterbank = fb;
    spectrogram->startPlotting();
}

void MainContentComponent::labelTextChanged(Label *l)
{
    if (l == &fileLabel)
    {
        std::cout << l->getText(false) << std::endl;
    }

}

void MainContentComponent::buttonClicked (Button* b)
{
// Check where the button comes from
    if ( toolbar.getComponentID() == b->getParentComponent()->getComponentID() )
    {
        for (int ii = 0; ii < toolbar.getNumItems(); ii++)
        {
            if (b == toolbar.getItemComponent(ii))
            {
                ToolbarItemComponent* c;
                ToolbarButton* cc;
                bool isP;
                if (toolbar.getItemId(ii) == 6)
                {
                    b->setToggleState(!(b->getToggleState()), dontSendNotification);

                    if (b->getToggleState())
                    {
                        aHandler->setInputIsMic();
                        std::wcout << "Input should be a mic now" << std::endl;
                    }
                    else
                    {
                        aHandler->setInputIsFile();
                        std::wcout << "Input should be a file now" << std::endl;
                    }
                }
                else if (toolbar.getItemId(ii) == 9)
                {
                    DBG("SAVE button pressed");
                    break;
                }
                else if (toolbar.getItemId(ii) == 10)
                {
                    DBG("PLAYLIST button pressed");
                    if ( plWindow != nullptr )
                        plWindow->setVisible(true);
                    break;
                }
                else if (aHandler->getCurrentSource() == 2)
                {
                    switch (toolbar.getItemId(ii))
                    {
                    case 1:
                        aHandler->playPrevious();
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId(jj))
                            {
                            case 2:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(1, dontSendNotification);
                                break;
                            case 3:
                            case 4:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(0, dontSendNotification);
                                break;
                            default:
                                break;
                            }
                        }
                        plWindow->repaint();
                        DBG("BACK button pressed");
                        break;
                    case 2:
                        isP = aHandler->startPlaying();
                        if (!isP)
                        {
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId(jj))
                                {
                                case 2:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(1, dontSendNotification);
                                    break;
                                case 3:
                                case 4:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(0, dontSendNotification);
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        DBG("PLAY button pressed");
                        break;
                    case 3:
                        isP = aHandler->pausePlaying();
                        if (isP)
                        {
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId(jj))
                                {
                                case 2:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(0, dontSendNotification);
                                    break;
                                case 3:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(1, dontSendNotification);
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        DBG("PAUSE button pressed");
                        break;
                    case 4:
                        isP = aHandler->stopPlaying();
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId(jj))
                            {
                            case 2:
                            case 3:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(0, dontSendNotification);
                                break;
                            case 4:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(1, dontSendNotification);
                                break;
                            default:
                                break;
                            }
                        }
                        DBG("STOP button pressed");
                        break;
                    case 5:
                        aHandler->playNext();
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId(jj))
                            {
                            case 2:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(1, dontSendNotification);
                                break;
                            case 3:
                            case 4:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(0, dontSendNotification);
                                break;
                            default:
                                break;
                            }
                        }
                        plWindow->repaint();
                        DBG("FORWARD button pressed");
                        break;
                    /*case 7:
                        DBG("FILE button pressed");
                        break;*/
                    case 8:
                        DBG("LOOP button pressed");
                        aHandler->toggleLooping();
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId(jj))
                            {
                            case 8:
                                c = toolbar.getItemComponent(jj);
                                DBG("Hey, do it!");
                                static_cast<CustomToolbarButton*>(c)->advanceState();
                                break;
                            default:
                                break;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    else if ( plWindow->fileControl.getComponentID() == b->getParentComponent()->getComponentID() )
    {
        for (int ii = 0; ii < plWindow->fileControl.getNumItems(); ii++)
        {
            if (b == plWindow->fileControl.getItemComponent(ii))
            {
                switch (plWindow->fileControl.getItemId(ii))
                {
                case 1:
                    menuItemSelected (2, FILE);
                    break;
                case 2:
                    plWindow->deleteKeyPressed(0);
                    break;
                case 3:
                    plWindow->clearPlaylist();
                    break;
                default:
                    break;
                }
            }
        }
    }
}

//============================================================================
// MenuBarModel related

StringArray MainContentComponent
::getMenuBarNames()
{
    const char * const nameBarNames[] =  {"Open File", "Options", "Info", nullptr};
    return StringArray(nameBarNames);
}

PopupMenu MainContentComponent
::getMenuForIndex(int topLevelMenuIndex, const String& /*menuName */)
{

    PopupMenu pm;
    switch (topLevelMenuIndex)
    {
    case FILE:
        pm.addItem (1, TRANS("Load Audio file..."));
        pm.addItem (2, TRANS("Add Audio file..."));
        pm.addSeparator();
        pm.addItem (3, TRANS("Save Image..."));
        pm.addSeparator();
        pm.addItem (4, TRANS("Exit"));
        break;
    case OPTIONS:
        pm.addItem (1, TRANS("Audio Settings..."));
        pm.addSeparator();
        pm.addItem (2, TRANS("Save current state..."));
        pm.addItem (3, TRANS("Load a saved state..."));
        pm.addSeparator();
        pm.addItem (4, TRANS("Reset to default state"));
        break;
    case INFO:
        pm.addItem (1, TRANS("Information"));
        break;
    default:
        break;
    }
    return pm;
}

void MainContentComponent
::menuItemSelected (int menuItemID, int topLevelMenuIndex)
{
    switch (topLevelMenuIndex)
    {
    case FILE:
        switch (menuItemID)
        {
        case 1:
        {
            FileChooser chooser (String("Select an audio file to play..."),
                                 File::nonexistent,
                                 String("*.wav;*.mp3;*.flac"));


            if (chooser.browseForFileToOpen())
            {
                File file (chooser.getResult());
                aHandler->loadFileIntoTransportAndStart(file);
                aHandler->setInputIsFile();
                String s = String(L"Loaded: ") + file.getFullPathName();
                fileLabel.setText(s, sendNotification);
                setName(file.getFileName());
                // Set toolbar button activation pattern
                ToolbarItemComponent* c;
                ToolbarButton* cc;
                for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                {
                    switch (toolbar.getItemId(jj))
                    {
                    case 2:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(1, dontSendNotification);
                        break;
                    case 3:
                    case 4:
                    case 6:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(0, dontSendNotification);
                        break;
                    case 8:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(1, dontSendNotification);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        break;
        case 2:
        {
            FileChooser chooser ("Select audio files to add...",
                                 File::nonexistent,
                                 "*.wav;*.mp3;*.flac", false);

            if (chooser.browseForMultipleFilesToOpen())
            {
                plWindow->addItemsToList(chooser.getResults());
                String s = String(L"Added: ") + chooser.getResults().getLast().getFullPathName();
                fileLabel.setText(s, sendNotification);
                setName(chooser.getResults().getLast().getFileName());
                // Set toolbar button activation pattern
                ToolbarItemComponent* c;
                ToolbarButton* cc;
                for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                {
                    switch (toolbar.getItemId(jj))
                    {
                    case 2:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(1, dontSendNotification);
                        break;
                    case 3:
                    case 4:
                    case 6:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(0, dontSendNotification);
                        break;
                    case 8:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(1, dontSendNotification);
                        break;
                    default:
                        break;
                    }
                }
                //------------------------------------------
            }
        }
        break;
        case 3:
            break;
        case 4:
            JUCEApplicationBase::quit();
            break;
        default:
            break;
        }
        break;
    case OPTIONS:
        switch (menuItemID)
        {
        case 1:
            aHandler->showAudioDeviceManagerDialog();
            break;
        case 2:
        {
            ReassignedBLFilterbank* replacementFb = nullptr;
            try
            {
                replacementFb = ReassignedBLFilterbank::makeFromChooser();
            }
            catch (String e)
            {
                std::cout << e << std::endl;
            }
            if (replacementFb)
                replaceFilterbank(replacementFb);
        }
            // pluginHolder->askUserToSaveState();
        break;
        case 3:
            spectrogram->startPlotting();
            // pluginHolder->askUserToLoadState();
            break;
        case 4:
            // resetToDefaultState();
            filterbank->toggleActivePlotReassigned();
            break;
        default:
            break;
        }
        break;
    case INFO:
    {
        DialogWindow::LaunchOptions o;
        o.content.setOwned (new Label(String("Information"),
                                      String("This project is based on the JUCE C++ Library and uses the following sources:\n\n"
                                             "Microphone icon made by SimpleIcon from www.flaticon.com is licensed under CC BY 3.0\n\n"
                                             "Button icons partially based on Minicons Free Vector Icons Pack, www.webalys.com/minicons")));

        o.content->setSize (400, 150);

        o.dialogTitle                   = TRANS("Info");
        o.dialogBackgroundColour        = Colour (0xfff0f0f0);
        o.escapeKeyTriggersCloseButton  = true;
        o.useNativeTitleBar             = true;
        o.resizable                     = false;

        o.launchAsync();
        break;
    }
    default:
        break;
    }
}


//[/EndFile]
