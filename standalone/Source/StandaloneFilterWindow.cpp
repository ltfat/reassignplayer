/*
  ==============================================================================

    StandalonePluginWindow.cpp
    Created: 5 Feb 2015 8:38:47pm
    Author:  susnak

  ==============================================================================
*/

#include "StandaloneFilterWindow.h"
#include "CustomToolbarButton.h"


StandaloneFilterWindow::StandaloneFilterWindow (const String& title,
      Colour backgroundColour,
      PropertySet* settingsToUse,
      bool takeOwnershipOfSettings)
   : DocumentWindow (title, backgroundColour, DocumentWindow::minimiseButton |
                     DocumentWindow::maximiseButton | DocumentWindow::closeButton)
     //optionsButton ("options"),
     //micfileButton ("MIC"),
     //fileChooserButton ("Open file...")
{
   //ogl->attachTo(*this);
   DBG("StandaloneFilterWindow constructor");
   menuBarComponent = new MenuBarComponent(this);

   // Load filter bank data
   Array<File> fbData = FilterbankDataHolder::FilterbankFileLoader();

   // Create the wrapped AudioProcessorEditor
   try
   {
   pluginHolder = new StandalonePluginHolder (fbData, settingsToUse, takeOwnershipOfSettings);
   }
   catch(String& err)
   {
        std::cout<< "Audio device initialization failed with the following message:"
                 << std::endl <<  err << std::endl;
        throw err;
   }
   pluginHolder->addChangeListener(this);

   createEditorComp();
   AudioProcessorEditor* e = pluginHolder->getPluginEditor();
   if (nullptr == e )
   {
      std::cout << "This is a GUI-less plugin" << std::endl;
   }
   int oldWidth = e->getWidth();
   int oldHeight = e->getHeight();
   setSize(e->getWidth(), e->getHeight());

   // Window dimensions
   setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow:: maximiseButton |DocumentWindow::closeButton, false);
   setTitleBarHeight(20);
   setSize(oldWidth+12, oldHeight + 5 * getTitleBarHeight());
   setResizable(true, true);

   // Label
   fileLabel.setText(String("No file loaded"), dontSendNotification);
   fileLabel.setColour(Label::textColourId, Colours::red);
   fileLabel.setColour(Label::backgroundColourId, backgroundColour);

   DBG("Before toolbar");
   // Init toolbar
   tbfac = new FilterWindowToolbarItemFactory(this);
   toolbar.addDefaultItems(*tbfac);
   toolbar.setComponentID("mainToolbar");
   Component::addAndMakeVisible(toolbar);
   DBG("After toolbar");

   Component::addAndMakeVisible(menuBarComponent);
   Component::addAndMakeVisible(e, true);
   Component::addAndMakeVisible (fileLabel);

   fileLabel.addListener(this);

   //Playlist Window
   DBG("Before playlist");
   plWindow = new PlaylistWindow(this,pluginHolder);
   plWindow->setVisible(true);
   DBG("After playlist");

   if (PropertySet* props = pluginHolder->settings)
   {
      const int x = props->getIntValue ("windowX", -100);
      const int y = props->getIntValue ("windowY", -100);

      if (x != -100 && y != -100)
         setBoundsConstrained (juce::Rectangle<int> (x, y, getWidth(), getHeight()));
      else
         centreWithSize (getWidth(), getHeight());
   }
   else
   {
      centreWithSize (getWidth(), getHeight());
   }

   DBG("StandaloneFilterWindow constructor end");
}

StandaloneFilterWindow::~StandaloneFilterWindow()
{
   if (PropertySet* props = pluginHolder->settings)
   {
      props->setValue ("windowX", getX());
      props->setValue ("windowY", getY());
   }

   pluginHolder->stopPlaying();
   deleteEditorComp();
   pluginHolder = nullptr;
}

//==============================================================================

void StandaloneFilterWindow::createEditorComp()
{
   DBG("Before Editor");
   pluginHolder->getPluginProcessor()->createEditorIfNeeded();
   DBG("After Editor");
}

void StandaloneFilterWindow::deleteEditorComp()
{
   AudioProcessorEditor* e = pluginHolder->getPluginEditor();
   if (nullptr != e)
   {
      pluginHolder->getPluginProcessor()->editorBeingDeleted(e);
      delete e;
   }
}


/** Deletes and re-creates the plugin, resetting it to its default state. */
void StandaloneFilterWindow::resetToDefaultState()
{
   pluginHolder->stopPlaying();
   deleteEditorComp();
   pluginHolder->deletePlugin();

   if (PropertySet* props = pluginHolder->settings)
      props->removeValue ("filterState");

   pluginHolder->createPlugin();
   createEditorComp();
   pluginHolder->startPlaying();
}

//==============================================================================
void StandaloneFilterWindow::closeButtonPressed()
{
   JUCEApplicationBase::quit();
}

void StandaloneFilterWindow::buttonClicked (Button* b)
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
                CustomToolbarButton* ccc;
                bool isP;
                if (toolbar.getItemId(ii) == 6)
                {
                    b->setToggleState(!(b->getToggleState()),dontSendNotification);

                    if (b->getToggleState())
                    {
                        pluginHolder->inputIsMicOnly();
                        std::wcout << "Input should be a mic now" << std::endl;
                    }
                    else
                    {
                        pluginHolder->inputIsFileOnly();
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
                else if (pluginHolder->getCurrentSource() == 1)
                {
                    switch (toolbar.getItemId(ii))
                    {
                    case 1:
                        pluginHolder->playPrevious();
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                           switch (toolbar.getItemId(jj))
                           {
                           case 2:
                               c = toolbar.getItemComponent(jj);
                               cc = static_cast<ToolbarButton*>(c);
                               cc->setToggleState(1,dontSendNotification);
                               break;
                           case 3:
                           case 4:
                               c = toolbar.getItemComponent(jj);
                               cc = static_cast<ToolbarButton*>(c);
                               cc->setToggleState(0,dontSendNotification);
                               break;
                           default:
                               break;
                           }
                        }
                        plWindow->repaint();
                        DBG("BACK button pressed");
                        break;
                    case 2:
                        isP = pluginHolder->changePlaybackState(1);
                        if(!isP)
                        {
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId(jj))
                                {
                                case 2:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(1,dontSendNotification);
                                    break;
                                case 3:
                                case 4:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(0,dontSendNotification);
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        DBG("PLAY button pressed");
                        break;
                    case 3:
                        isP = pluginHolder->changePlaybackState(2);
                        if(isP)
                        {
                            for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                            {
                                switch (toolbar.getItemId(jj))
                                {
                                case 2:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(0,dontSendNotification);
                                    break;
                                case 3:
                                    c = toolbar.getItemComponent(jj);
                                    cc = static_cast<ToolbarButton*>(c);
                                    cc->setToggleState(1,dontSendNotification);
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        DBG("PAUSE button pressed");
                        break;
                    case 4:
                        isP = pluginHolder->changePlaybackState(3);
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                            switch (toolbar.getItemId(jj))
                            {
                            case 2:
                            case 3:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(0,dontSendNotification);
                                break;
                            case 4:
                                c = toolbar.getItemComponent(jj);
                                cc = static_cast<ToolbarButton*>(c);
                                cc->setToggleState(1,dontSendNotification);
                                break;
                            default:
                                break;
                            }
                        }
                        DBG("STOP button pressed");
                        break;
                    case 5:
                        pluginHolder->playNext();
                        for (int jj = 0; jj < toolbar.getNumItems(); jj++)
                        {
                           switch (toolbar.getItemId(jj))
                           {
                           case 2:
                               c = toolbar.getItemComponent(jj);
                               cc = static_cast<ToolbarButton*>(c);
                               cc->setToggleState(1,dontSendNotification);
                               break;
                           case 3:
                           case 4:
                               c = toolbar.getItemComponent(jj);
                               cc = static_cast<ToolbarButton*>(c);
                               cc->setToggleState(0,dontSendNotification);
                               break;
                           default:
                               break;
                           }
                        }
                        plWindow->repaint();
                        DBG("FORWARD button pressed");
                        break;
                    case 7:
                        DBG("FILE button pressed");
                        break;
                    case 8:
                        DBG("LOOP button pressed");
                        pluginHolder->toggleLooping();
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
                default:
                    break;
                }
            }
        }
    }
}

void StandaloneFilterWindow::labelTextChanged(Label *l)
{
   if (l == &fileLabel)
   {
      std::cout << l->getText(false) << std::endl;
   }

}

void StandaloneFilterWindow::resized()
{
   DocumentWindow::resized();
   fileLabel.setBounds(0, getHeight() - getTitleBarHeight(), getWidth() - 20, getTitleBarHeight());

   menuBarComponent->setBounds(0, getTitleBarHeight(), getWidth(), getTitleBarHeight());

   AudioProcessorEditor* e = pluginHolder->getPluginEditor();
   if (e != nullptr)
      e->setBounds(4, 2 * getTitleBarHeight(), getWidth()-8,
                   getHeight() - 5 * getTitleBarHeight());

   toolbar.setBounds(0, getHeight() - 3 * getTitleBarHeight(), getWidth(), 2 * getTitleBarHeight());
}


//==============================================================================
// Toolbar items factory

StandaloneFilterWindow::FilterWindowToolbarItemFactory
::FilterWindowToolbarItemFactory(ButtonListener* listener_)
   : listener(listener_)
{

}

StandaloneFilterWindow::FilterWindowToolbarItemFactory
::~FilterWindowToolbarItemFactory()
{

}

ToolbarItemComponent* StandaloneFilterWindow::FilterWindowToolbarItemFactory
::createItem(int itemId)
{  int NumBytes;
   const char* iconData;
   String buttonText, binDataOff, binDataOn, binDataOnOne;

   if ( itemId != loopToggle )
   {
        Drawable* iconOff;
        Drawable* iconOn;
        switch(itemId)
        {
        case back:
            binDataOff = "back_svg";
            binDataOn = "backOn_svg";
            buttonText = "back";
            break;
        case play:
            binDataOff = "play_svg";
            binDataOn = "playOn_svg";
            buttonText = "play";
            break;
        case pause:
            binDataOff = "pause_svg";
            binDataOn = "pauseOn_svg";
            buttonText = "pause";
            break;
        case stop:
            binDataOff = "stop_svg";
            binDataOn = "stopOn_svg";
            buttonText = "stop";
            break;
        case forward:
            binDataOff = "forward_svg";
            binDataOn = "forwardOn_svg";
            buttonText = "forward";
            break;
        case micToggle:
            binDataOff = "mic_svg";
            binDataOn = "micOn_svg";
            buttonText = "toggle MIC";
            break;
        case fileToggle:
            binDataOff = "audfile_svg";
            binDataOn = "audfileOn_svg";
            buttonText = "toggle FILE";
            break;
        case saveImg:
            binDataOff = "save_svg";
            binDataOn = "saveOn_svg";
            buttonText = "save IMG";
            break;
        case playlist:
            binDataOff = "list_svg";
            binDataOn = "list_svg";
            buttonText = "show playlist";
            break;
        default:
            break;
        }

        iconData = BinaryData::getNamedResource(binDataOff.toRawUTF8(),NumBytes);
        iconOff = Drawable::createFromImageData(iconData,NumBytes);
        iconData = BinaryData::getNamedResource(binDataOn.toRawUTF8(),NumBytes);
        iconOn = Drawable::createFromImageData(iconData,NumBytes);
        ToolbarButton* b = new ToolbarButton(itemId,buttonText,iconOff,iconOn);
        b->addListener(listener);
        return b;
   }
   else
   {
        Drawable* tempPtr;
        Array<Drawable*> loopIcons;
        binDataOff = "loop_svg";
        binDataOn = "loopOn_svg";
        binDataOnOne = "loopOnOne_svg";
        buttonText = "toggle LOOP";

        iconData = BinaryData::getNamedResource(binDataOnOne.toRawUTF8(),NumBytes);
        tempPtr = Drawable::createFromImageData(iconData,NumBytes);
        loopIcons.add(tempPtr);
        iconData = BinaryData::getNamedResource(binDataOff.toRawUTF8(),NumBytes);
        tempPtr = Drawable::createFromImageData(iconData,NumBytes);
        loopIcons.add(tempPtr);
        iconData = BinaryData::getNamedResource(binDataOn.toRawUTF8(),NumBytes);
        tempPtr = Drawable::createFromImageData(iconData,NumBytes);
        loopIcons.add(tempPtr);
        CustomToolbarButton* b = new CustomToolbarButton(itemId,buttonText,loopIcons);
        b->addListener(listener);
        return b;
   }
}

void StandaloneFilterWindow::FilterWindowToolbarItemFactory
::getDefaultItemSet(Array<int> &ids)
{
   ids.add (flexibleSpacerId);
   ids.add(back);
   ids.add(play);
   ids.add(pause);
   ids.add(stop);
   ids.add(forward);
   ids.add(loopToggle);
   ids.add (flexibleSpacerId);
   ids.add (separatorBarId);
   ids.add (flexibleSpacerId);
   ids.add(micToggle);
   ids.add (flexibleSpacerId);
   ids.add (separatorBarId);
   ids.add (flexibleSpacerId);
   //ids.add(fileToggle);
   ids.add(saveImg);
   ids.add(playlist);
   ids.add (flexibleSpacerId);
}

void StandaloneFilterWindow::FilterWindowToolbarItemFactory
::getAllToolbarItemIds(Array<int> &ids)
{
   ids.add(back);
   ids.add(play);
   ids.add(pause);
   ids.add(stop);
   ids.add(forward);
   ids.add(micToggle);
   ids.add(fileToggle);
   ids.add(loopToggle);
   ids.add(saveImg);
   ids.add(playlist);

   // Spacers and Separators
   ids.add (separatorBarId);
   ids.add (spacerId);
   ids.add (flexibleSpacerId);
}


//============================================================================
// MenuBarModel related

StringArray StandaloneFilterWindow
::getMenuBarNames()
{
   const char * const nameBarNames[] =  {"Open File","Options", nullptr};
   return StringArray(nameBarNames);
}

PopupMenu StandaloneFilterWindow
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
   default:
      break;
   }
   return pm;
}

void StandaloneFilterWindow
::menuItemSelected (int menuItemID, int topLevelMenuIndex)
{
   switch (topLevelMenuIndex)
   {
   case FILE:
      switch (menuItemID)
      {
      case 1:
         {
         FileChooser chooser ("Select an audio file to play...",
                              File::nonexistent,
                              "*.wav;*.mp3");

         if (chooser.browseForFileToOpen())
            {
               File file (chooser.getResult());
               pluginHolder->loadFile(file);
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
                        cc->setToggleState(1,dontSendNotification);
                        break;
                    case 3:
                    case 4:
                    case 6:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(0,dontSendNotification);
                        break;
                    case 8:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(1,dontSendNotification);
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
                              "*.wav;*.mp3");

         if (chooser.browseForMultipleFilesToOpen())
            {
               /*for ( int kk=0; kk<chooser.getResults().size(); ++kk )
               {
                    File file (chooser.getResults().getUnchecked(kk));
                    pluginHolder->addFile(file);

               }*/
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
                        cc->setToggleState(1,dontSendNotification);
                        break;
                    case 3:
                    case 4:
                    case 6:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(0,dontSendNotification);
                        break;
                    case 8:
                        c = toolbar.getItemComponent(jj);
                        cc = static_cast<ToolbarButton*>(c);
                        cc->setToggleState(1,dontSendNotification);
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
         pluginHolder->showAudioSettingsDialog();
         break;
      case 2:
         pluginHolder->askUserToSaveState();
         break;
      case 3:
         pluginHolder->askUserToLoadState();
         break;
      case 4:
         resetToDefaultState();
         break;
      default:
         break;
      }
      break;
   default:
      break;
   }
}

void StandaloneFilterWindow::changeListenerCallback(ChangeBroadcaster* )
{

}

// Playlist Window

StandaloneFilterWindow::PlaylistWindow::PlaylistWindow(ButtonListener* listener_, StandalonePluginHolder* pluginHolder_)
: DocumentWindow("Playlist",Colours::lightgrey,0,true),
  listener(listener_),
  pluginHolder(pluginHolder_)
{
   // Window dimensions
   setTitleBarButtonsRequired (DocumentWindow::closeButton, false);
   setTitleBarHeight(20);
   setSize(250, 520);
   setResizable(false, false);

   //TestList
   //playList.add("Item 1");
   //playList.add("Item 2");
   //playList.add("Item 3");
   //playList.add("Item 4");

   // Init playListBox
   playListBox = new ListBox("stuff",this);
   playListBox->setBounds(0,20,getWidth(),getHeight()-60);
   playListBox->setMultipleSelectionEnabled(true);
   playListBox->updateContent();
   playListBox->repaint();
   Component::addAndMakeVisible(*playListBox);
   playListBox->selectRow (0);
   playListBox->setColour (ListBox::backgroundColourId, Colour::greyLevel (0.9f));

   DBG("Before toolbar");
   // Init fileControl
   plfac = new PlaylistWindowToolbarItemFactory(listener);
   fileControl.setBounds(0,getHeight()-40,getWidth(),40);
   fileControl.setComponentID("playlistToolbar");
   fileControl.addDefaultItems(*plfac);
   Component::addAndMakeVisible(fileControl);
   DBG("After toolbar");

}

void StandaloneFilterWindow::PlaylistWindow::resized()
{
   DocumentWindow::resized();
   // This line somehow breaks the program
   //fileControl.setBounds(0,getHeight()-40,getWidth(),40);
   //playListBox->setBounds(0,20,getWidth(),getHeight()-60);
}

StandaloneFilterWindow::PlaylistWindow::~PlaylistWindow()
{

}

 int StandaloneFilterWindow::PlaylistWindow::getNumRows ()
 {
    return playList.size();
 }

 void StandaloneFilterWindow::PlaylistWindow::deleteKeyPressed (int lastRowSelected)
 {
    removeItemsFromList(playListBox->getSelectedRows());
 }

 void StandaloneFilterWindow::PlaylistWindow::substitutePlaylist (Array<File> newList)
 {
    playList.clear();
    pluginHolder->clearFileList();
    addItemsToList(newList);
    playListBox->updateContent();
 }

 void StandaloneFilterWindow::PlaylistWindow::clearPlaylist ()
 {
    playList.clear();
    pluginHolder->clearFileList();
    playListBox->updateContent();
 }

 void StandaloneFilterWindow::PlaylistWindow::addItemsToList (Array<File> newList)
 {
    for (int kk = 0; kk < newList.size(); ++kk)
    {
        playList.add(newList[kk].getFileNameWithoutExtension());
        pluginHolder->addFile(newList.getReference(kk));
    }
    playListBox->updateContent();
 }

/*void StandaloneFilterWindow::PlaylistWindow::addItemsToList (File& newList)
{

}*/

void StandaloneFilterWindow::PlaylistWindow::removeItemsFromList (SparseSet<int> selectedRows)
{
    for (int kk = getNumRows()-1; kk >= 0 ; --kk)
    {
        if ( selectedRows.contains(kk) )
        {
            playList.remove(kk);
            pluginHolder->removeFile(kk);
        }
    }
    playListBox->deselectAllRows();
    playListBox->updateContent();
}

/*oid StandaloneFilterWindow::PlaylistWindow::removeItemsFromList (SparseSet<int> selectedRows)
{
    for (int kk = 0; kk < selectedRows.size(); ++kk)
    {
        playList.remove(selectedRows[kk]);
        pluginHolder->removeFile(selectedRows[kk]);
    }
}*/

void StandaloneFilterWindow::PlaylistWindow::closeButtonPressed()
{
    this->setVisible(false);
}

void StandaloneFilterWindow::PlaylistWindow::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
        if (rowIsSelected)
            g.fillAll (findColour (TextEditor::highlightColourId));

        if ( rowNumber == pluginHolder->getCurrentFileIdx() )
            g.setColour (Colours::darkblue);
        else
            g.setColour (Colours::black);

        g.setFont (12.0f);
        g.drawFittedText (playList[rowNumber].toRawUTF8(), 8, 0, this->getWidth()-16, 20, Justification::centredLeft, 1);
        // 8, 0, width - 16, height, Justification::centredLeft, 2);

}

void StandaloneFilterWindow::PlaylistWindow::listBoxItemDoubleClicked (int row, const MouseEvent &)
{
    pluginHolder->setCurrentFileIdx(row,true);
    this->repaint();
}

//========================================================================================================================================
// PlaylistWindowToolbarItemFactory

StandaloneFilterWindow::PlaylistWindow::PlaylistWindowToolbarItemFactory::PlaylistWindowToolbarItemFactory(ButtonListener* listener_)
: listener(listener_)
{

}

StandaloneFilterWindow::PlaylistWindow::PlaylistWindowToolbarItemFactory::~PlaylistWindowToolbarItemFactory()
{

}

void StandaloneFilterWindow::PlaylistWindow::PlaylistWindowToolbarItemFactory::getAllToolbarItemIds(Array<int> &ids)
{
   ids.add(addFiles);
   ids.add(removeSelected);

   // Spacers and Separators
   ids.add (separatorBarId);
   ids.add (spacerId);
   ids.add (flexibleSpacerId);
}

void StandaloneFilterWindow::PlaylistWindow::PlaylistWindowToolbarItemFactory::getDefaultItemSet(Array<int> &ids)
{
   ids.add(addFiles);
   ids.add(removeSelected);
   ids.add (flexibleSpacerId);
}

ToolbarItemComponent* StandaloneFilterWindow::PlaylistWindow::PlaylistWindowToolbarItemFactory::createItem(int itemId)
{
   int NumBytes;
   const char* iconData;
   String buttonText, binData;

   Drawable* iconOff;
   switch(itemId)
   {
   case addFiles:
       binData = "addFiles_svg";
       buttonText = "addFiles";
       break;
   case removeSelected:
       binData = "removeSelected_svg";
       buttonText = "removeSelected";
       break;
   default:
       break;
   }

   iconData = BinaryData::getNamedResource(binData.toRawUTF8(),NumBytes);
   iconOff = Drawable::createFromImageData(iconData,NumBytes);
   ToolbarButton* b = new ToolbarButton(itemId,buttonText,iconOff,nullptr);
   b->addListener(listener);
   return b;
}
