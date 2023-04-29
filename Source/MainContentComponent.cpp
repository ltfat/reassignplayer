#include "MainContentComponent.h"

//==============================================================================
MainContentComponent::MainContentComponent()
{
    // Manubar
    menuBarComponent = new juce::MenuBarComponent (this);
    addAndMakeVisible (menuBarComponent);
    // Toolbar
    tbfac = new MainToolbarItemFactory (this);
    toolbar.addDefaultItems (*tbfac);
    toolbar.setComponentID ("mainToolbar");
    addAndMakeVisible (toolbar);

    aHandler = new AudioHandler();
    spectrogram = new Spectrogram();
    filterbank = ReassignedBLFilterbank::makeDefault();
    aHandler->setFilterbank (filterbank);
    spectrogram->setSpectrogramSource (filterbank);
    ogl = new juce::OpenGLContext();
    ogl->setContinuousRepainting (true);
    ogl->attachTo (*spectrogram);

    addAndMakeVisible (spectrogram);

    // Label
    fileLabel.setText (juce::String ("No file loaded"), juce::dontSendNotification);
    fileLabel.setColour (juce::Label::textColourId, juce::Colours::red);
    // fileLabel.setColour(Label::backgroundColourId, backgroundColour);
    addAndMakeVisible (fileLabel);
    plWindow = new PlaylistWindow (this, aHandler);
    plWindow->addChangeListener (this);
    aHandler->addChangeListener (this);
    setSize (800, 600);
}

MainContentComponent::~MainContentComponent()
{
    ogl->detach();
    ogl = nullptr;

    aHandler = nullptr;
    spectrogram = nullptr;
    filterbank = nullptr;
    menuBarComponent = nullptr;
    menuBarModel = nullptr;
}

//==============================================================================
void MainContentComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::white);
}

void MainContentComponent::resized()
{
    menuBarComponent->setBounds (0, 0, getWidth(), 20);
    spectrogram->setBounds (0, 20, getWidth(), getHeight() - 80);
    toolbar.setBounds (0, getHeight() - 60, getWidth(), 40);
    fileLabel.setBounds (0, getHeight() - 20, getWidth(), 20);
}

void MainContentComponent::replaceFilterbank (ReassignedBLFilterbank* fb)
{
    fb->setActivePlotReassigned (filterbank->getActivePlotReassigned());
    aHandler->setFilterbank (fb);
    spectrogram->stopPlotting();
    spectrogram->setStripWidth (5 * std::floor (fb->getBufLen() / 2048.0));
    spectrogram->setSpectrogramSource (fb);
    filterbank = fb;
    spectrogram->startPlotting();
}

void MainContentComponent::labelTextChanged (juce::Label* l)
{
    if (l == &fileLabel)
    {
        std::cout << l->getText (false) << std::endl;
    }
}

void MainContentComponent::buttonClicked (juce::Button* b)
{
    // Check where the button comes from
    if (toolbar.getComponentID() == b->getParentComponent()->getComponentID())
    {
        for (int ii = 0; ii < toolbar.getNumItems(); ii++)
        {
            if (b == toolbar.getItemComponent (ii))
            {
                juce::ToolbarItemComponent* c;
                juce::ToolbarButton* cc;
                bool isP;
                if (toolbar.getItemId (ii) == 6)
                {
                    b->setToggleState (!(b->getToggleState()), juce::dontSendNotification);

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
                else if (toolbar.getItemId (ii) == 9)
                {
                    DBG ("SAVE button pressed");
                    break;
                }
                else if (toolbar.getItemId (ii) == 10)
                {
                    DBG ("PLAYLIST button pressed");
                    if (plWindow != nullptr)
                        plWindow->setVisible (true);
                    break;
                }
                else if (toolbar.getItemId (ii) == 11)
                {
                    ReassignedBLFilterbank* replacementFb = nullptr;
                    try
                    {
                        replacementFb = ReassignedBLFilterbank::makeFromChooser();
                    } catch (juce::String e)
                    {
                        std::cout << e << std::endl;
                    }
                    if (replacementFb)
                        replaceFilterbank (replacementFb);
                    break;
                }
                else if (toolbar.getItemId (ii) == 12)
                {
                    filterbank->toggleActivePlotReassigned();
                    c = toolbar.getItemComponent (ii);
                    cc = static_cast<juce::ToolbarButton*> (c);
                    cc->setToggleState (!cc->getToggleState(), juce::dontSendNotification);
                    break;
                }
                else if (aHandler->getCurrentSource() == 2)
                {
                    switch (toolbar.getItemId (ii))
                    {
                        case 1:
                            aHandler->playPrevious();
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId (jj))
                                {
                                    case 2:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (1, juce::dontSendNotification);
                                        break;
                                    case 3:
                                    case 4:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (0, juce::dontSendNotification);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            plWindow->repaint();
                            DBG ("BACK button pressed");
                            break;
                        case 2:
                            isP = aHandler->startPlaying();
                            plWindow->repaint();
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId (jj))
                                {
                                    case 2:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (1, juce::dontSendNotification);
                                        break;
                                    case 3:
                                    case 4:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (0, juce::dontSendNotification);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            DBG ("PLAY button pressed");
                            break;
                        case 3:
                            isP = aHandler->pausePlaying();
                            if (isP)
                            {
                                for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                                {
                                    switch (toolbar.getItemId (jj))
                                    {
                                        case 2:
                                            c = toolbar.getItemComponent (jj);
                                            cc = static_cast<juce::ToolbarButton*> (c);
                                            cc->setToggleState (0, juce::dontSendNotification);
                                            break;
                                        case 3:
                                            c = toolbar.getItemComponent (jj);
                                            cc = static_cast<juce::ToolbarButton*> (c);
                                            cc->setToggleState (1, juce::dontSendNotification);
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                            DBG ("PAUSE button pressed");
                            break;
                        case 4:
                            isP = aHandler->stopPlaying();
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId (jj))
                                {
                                    case 2:
                                    case 3:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (0, juce::dontSendNotification);
                                        break;
                                    case 4:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (1, juce::dontSendNotification);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            DBG ("STOP button pressed");
                            break;
                        case 5:
                            aHandler->playNext();
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId (jj))
                                {
                                    case 2:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (1, juce::dontSendNotification);
                                        break;
                                    case 3:
                                    case 4:
                                        c = toolbar.getItemComponent (jj);
                                        cc = static_cast<juce::ToolbarButton*> (c);
                                        cc->setToggleState (0, juce::dontSendNotification);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            plWindow->repaint();
                            DBG ("FORWARD button pressed");
                            break;
                        /*case 7:
                            DBG("FILE button pressed");
                            break;*/
                        case 8:
                            DBG ("LOOP button pressed");
                            aHandler->toggleLooping();
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId (jj))
                                {
                                    case 8:
                                        c = toolbar.getItemComponent (jj);
                                        DBG ("Hey, do it!");
                                        static_cast<CustomToolbarButton*> (c)->advanceState();
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
    else if (plWindow->fileControl.getComponentID() == b->getParentComponent()->getComponentID())
    {
        for (int ii = 0; ii < plWindow->fileControl.getNumItems(); ii++)
        {
            if (b == plWindow->fileControl.getItemComponent (ii))
            {
                switch (plWindow->fileControl.getItemId (ii))
                {
                    case 1:
                        menuItemSelected (2, FILE);
                        break;
                    case 2:
                        plWindow->deleteKeyPressed (0);
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

void MainContentComponent::changeListenerCallback (
    juce::ChangeBroadcaster* source)
{
    if (plWindow == source)
    {
        toolbar.getItemComponent (2)->triggerClick();
    }
    if (aHandler == source)
    {
        plWindow->repaint();
    }
}

//============================================================================
// MenuBarModel related

juce::StringArray MainContentComponent ::getMenuBarNames()
{
    const char* const nameBarNames[] = { "Open File", "Options", "Info", nullptr };
    return juce::StringArray (nameBarNames);
}

juce::PopupMenu
    MainContentComponent ::getMenuForIndex (int topLevelMenuIndex,
        const juce::String& /*menuName */)
{
    juce::PopupMenu pm;
    switch (topLevelMenuIndex)
    {
        case FILE:
            pm.addItem (1, TRANS ("Load Audio file..."));
            pm.addItem (2, TRANS ("Add Audio file..."));
            pm.addSeparator();
            pm.addItem (3, TRANS ("Save Image..."));
            pm.addSeparator();
            pm.addItem (4, TRANS ("Exit"));
            break;
        case OPTIONS:
            pm.addItem (1, TRANS ("Audio Settings..."));
            pm.addSeparator();
            pm.addItem (2, TRANS ("Load filter bank file..."));
            pm.addSeparator();
            pm.addItem (3, TRANS ("Reassignment ON/OFF"));
            break;
        case INFO:
            pm.addItem (1, TRANS ("Information"));
            break;
        default:
            break;
    }
    return pm;
}

void MainContentComponent ::menuItemSelected (int menuItemID,
    int topLevelMenuIndex)
{
    switch (topLevelMenuIndex)
    {
        case FILE:
            switch (menuItemID)
            {
                case 1:
                {
                    juce::FileChooser chooser (juce::String ("Select an audio file to play..."),
                        juce::File(),
                        juce::String ("*.wav;*.mp3;*.flac"));

                    if (chooser.browseForFileToOpen())
                    {
                        juce::File file (chooser.getResult());
                        aHandler->loadFileIntoTransportAndStart (file);
                        aHandler->setInputIsFile();
                        juce::String s = juce::String (L"Loaded: ") + file.getFullPathName();
                        fileLabel.setText (s, juce::sendNotification);
                        setName (file.getFileName());
                        // Set toolbar button activation pattern
                        juce::ToolbarItemComponent* c;
                        juce::ToolbarButton* cc;
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId (jj))
                            {
                                case 2:
                                    c = toolbar.getItemComponent (jj);
                                    cc = static_cast<juce::ToolbarButton*> (c);
                                    cc->setToggleState (1, juce::dontSendNotification);
                                    break;
                                case 3:
                                case 4:
                                case 6:
                                    c = toolbar.getItemComponent (jj);
                                    cc = static_cast<juce::ToolbarButton*> (c);
                                    cc->setToggleState (0, juce::dontSendNotification);
                                    break;
                                case 8:
                                    c = toolbar.getItemComponent (jj);
                                    cc = static_cast<juce::ToolbarButton*> (c);
                                    cc->setToggleState (1, juce::dontSendNotification);
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
                    juce::FileChooser chooser ("Select audio files to add...", juce::File(), "*.wav;*.mp3;*.flac", false);

                    if (chooser.browseForMultipleFilesToOpen())
                    {
                        plWindow->addItemsToList (chooser.getResults());
                        juce::String s = juce::String (L"Added: ") + chooser.getResults().getLast().getFullPathName();
                        fileLabel.setText (s, juce::sendNotification);
                        setName (chooser.getResults().getLast().getFileName());
                        // Set toolbar button activation pattern
                        juce::ToolbarItemComponent* c;
                        juce::ToolbarButton* cc;
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId (jj))
                            {
                                case 2:
                                    c = toolbar.getItemComponent (jj);
                                    cc = static_cast<juce::ToolbarButton*> (c);
                                    cc->setToggleState (1, juce::dontSendNotification);
                                    break;
                                case 3:
                                case 4:
                                case 6:
                                    c = toolbar.getItemComponent (jj);
                                    cc = static_cast<juce::ToolbarButton*> (c);
                                    cc->setToggleState (0, juce::dontSendNotification);
                                    break;
                                case 8:
                                    c = toolbar.getItemComponent (jj);
                                    cc = static_cast<juce::ToolbarButton*> (c);
                                    cc->setToggleState (1, juce::dontSendNotification);
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
                    juce::JUCEApplicationBase::quit();
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
                    } catch (juce::String e)
                    {
                        std::cout << e << std::endl;
                    }
                    if (replacementFb)
                        replaceFilterbank (replacementFb);
                }
                break;
                case 3:
                    // Toolbar Item Component 10 should be the reassignment switch
                    toolbar.getItemComponent (14)->triggerClick();
                    break;
                default:
                    break;
            }
            break;
        case INFO:
        {
            juce::DialogWindow::LaunchOptions o;
            o.content.setOwned (new juce::Label (
                juce::String ("Information"),
                juce::String ("This project is based on the JUCE C++ Library and uses "
                              "the following sources:\n\n"
                              "Microphone icon made by SimpleIcon from www.flaticon.com "
                              "is licensed under CC BY 3.0\n\n"
                              "Button icons partially based on Minicons Free Vector "
                              "Icons Pack, www.webalys.com/minicons")));

            o.content->setSize (400, 150);

            o.dialogTitle = TRANS ("Info");
            o.dialogBackgroundColour = juce::Colour (0xfff0f0f0);
            o.escapeKeyTriggersCloseButton = true;
            o.useNativeTitleBar = true;
            o.resizable = false;

            o.launchAsync();
            break;
        }
        default:
            break;
    }
}
