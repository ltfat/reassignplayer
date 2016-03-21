/*
  ==============================================================================

    MainGuiWidgets.cpp
    Created: 9 Mar 2016 11:47:28am
    Author:  susnak

  ==============================================================================
*/

#include "MainGuiWidgets.h"
//==============================================================================
// Toolbar items factory

MainToolbarItemFactory
::MainToolbarItemFactory(ButtonListener* listener_)
    : listener(listener_)
{

}

MainToolbarItemFactory
::~MainToolbarItemFactory()
{

}

ToolbarItemComponent* MainToolbarItemFactory
::createItem(int itemId)
{
    int NumBytes;
    const char* iconData;
    String buttonText, binDataOff, binDataOn, binDataOnOne;

    if ( itemId != loopToggle )
    {
        Drawable* iconOff;
        Drawable* iconOn;
        switch (itemId)
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

        iconData = BinaryData::getNamedResource(binDataOff.toRawUTF8(), NumBytes);
        iconOff = Drawable::createFromImageData(iconData, NumBytes);
        iconData = BinaryData::getNamedResource(binDataOn.toRawUTF8(), NumBytes);
        iconOn = Drawable::createFromImageData(iconData, NumBytes);
        ToolbarButton* b = new ToolbarButton(itemId, buttonText, iconOff, iconOn);
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

        iconData = BinaryData::getNamedResource(binDataOnOne.toRawUTF8(), NumBytes);
        tempPtr = Drawable::createFromImageData(iconData, NumBytes);
        loopIcons.add(tempPtr);
        iconData = BinaryData::getNamedResource(binDataOff.toRawUTF8(), NumBytes);
        tempPtr = Drawable::createFromImageData(iconData, NumBytes);
        loopIcons.add(tempPtr);
        iconData = BinaryData::getNamedResource(binDataOn.toRawUTF8(), NumBytes);
        tempPtr = Drawable::createFromImageData(iconData, NumBytes);
        loopIcons.add(tempPtr);
        CustomToolbarButton* b = new CustomToolbarButton(itemId, buttonText, loopIcons);
        b->addListener(listener);
        return b;
    }
}

void MainToolbarItemFactory
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
    //ids.add(saveImg);
    ids.add(playlist);
    ids.add (flexibleSpacerId);
}

void MainToolbarItemFactory
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

// Playlist Window

PlaylistWindow::PlaylistWindow(ButtonListener* blistener_, AudioHandler* ah)
    : DocumentWindow("Playlist", Colours::lightgrey, 0, true),
      aHandler(ah), listener(blistener_)
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
    playListBox = new ListBox("stuff", this);
    playListBox->setBounds(0, 20, getWidth(), getHeight() - 60);
    playListBox->setMultipleSelectionEnabled(true);
    playListBox->updateContent();
    playListBox->repaint();
    Component::addAndMakeVisible(*playListBox);
    playListBox->selectRow (0);
    playListBox->setColour (ListBox::backgroundColourId, Colour::greyLevel (0.9f));

    DBG("Before toolbar");
    // Init fileControl
    plfac = new PlaylistWindowToolbarItemFactory(listener);
    fileControl.setBounds(0, getHeight() - 40, getWidth(), 40);
    fileControl.setComponentID("playlistToolbar");
    fileControl.addDefaultItems(*plfac);
    Component::addAndMakeVisible(fileControl);
    DBG("After toolbar");

}

void PlaylistWindow::resized()
{
    DocumentWindow::resized();
    // This line somehow breaks the program
    //fileControl.setBounds(0,getHeight()-40,getWidth(),40);
    //playListBox->setBounds(0,20,getWidth(),getHeight()-60);
}

PlaylistWindow::~PlaylistWindow()
{

}

int PlaylistWindow::getNumRows ()
{
    return playList.size();
}

void PlaylistWindow::deleteKeyPressed (int lastRowSelected)
{
    removeItemsFromList(playListBox->getSelectedRows());
}

void PlaylistWindow::substitutePlaylist (Array<File> newList)
{
    playList.clear();
    // pluginHolder->clearFileList();
    addItemsToList(newList);
    playListBox->updateContent();
}

void PlaylistWindow::clearPlaylist ()
{
    playList.clear();
    // pluginHolder->clearFileList();
    playListBox->updateContent();
}

void PlaylistWindow::addItemsToList (Array<File> newList)
{
    for (int kk = 0; kk < newList.size(); ++kk)
    {
        playList.add(newList[kk].getFileNameWithoutExtension());
        aHandler->addFile(newList.getReference(kk));
    }
    playListBox->updateContent();
}

/*void PlaylistWindow::addItemsToList (File& newList)
{

}*/

void PlaylistWindow::removeItemsFromList (SparseSet<int> selectedRows)
{
    for (int kk = getNumRows() - 1; kk >= 0 ; --kk)
    {
        if ( selectedRows.contains(kk) )
        {
            playList.remove(kk);
            aHandler->removeFile(kk);
        }
    }
    playListBox->deselectAllRows();
    playListBox->updateContent();
}


void PlaylistWindow::closeButtonPressed()
{
    this->setVisible(false);
}

void PlaylistWindow::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (TextEditor::highlightColourId));
    if ( rowNumber == aHandler->getCurrentFileIdx() )
        g.setColour (Colours::darkblue);
    else
        g.setColour (Colours::black);
    g.setFont (12.0f);
    g.drawFittedText (playList[rowNumber].toRawUTF8(), 8, 0, this->getWidth() - 16, 20, Justification::centredLeft, 1);
    // 8, 0, width - 16, height, Justification::centredLeft, 2);

}

void PlaylistWindow::listBoxItemDoubleClicked (int row, const MouseEvent &)
{
    //aHandler->stopPlaying();
    aHandler->setCurrentFileIdx(row, false);
    sendChangeMessage();
    //this->repaint();
}

//========================================================================================================================================
// PlaylistWindowToolbarItemFactory

PlaylistWindow::PlaylistWindowToolbarItemFactory::PlaylistWindowToolbarItemFactory(ButtonListener* listener_)
    : listener(listener_)
{

}

PlaylistWindow::PlaylistWindowToolbarItemFactory::~PlaylistWindowToolbarItemFactory()
{

}

void PlaylistWindow::PlaylistWindowToolbarItemFactory::getAllToolbarItemIds(Array<int> &ids)
{
    ids.add(addFiles);
    ids.add(removeSelected);
    ids.add(clearList);

    // Spacers and Separators
    ids.add (separatorBarId);
    ids.add (spacerId);
    ids.add (flexibleSpacerId);
}

void PlaylistWindow::PlaylistWindowToolbarItemFactory::getDefaultItemSet(Array<int> &ids)
{
    ids.add(addFiles);
    ids.add(removeSelected);
    ids.add(clearList);
    ids.add (flexibleSpacerId);
}

ToolbarItemComponent* PlaylistWindow::PlaylistWindowToolbarItemFactory::createItem(int itemId)
{
    int NumBytes;
    const char* iconData;
    String buttonText, binData;

    Drawable* iconOff;
    switch (itemId)
    {
    case addFiles:
        binData = "addFiles_svg";
        buttonText = "addFiles";
        break;
    case removeSelected:
        binData = "removeSelected_svg";
        buttonText = "removeSelected";
        break;
    case clearList:
        binData = "clearList_svg";
        buttonText = "clearPlaylist";
        break;
    default:
        break;
    }

    iconData = BinaryData::getNamedResource(binData.toRawUTF8(), NumBytes);
    iconOff = Drawable::createFromImageData(iconData, NumBytes);
    ToolbarButton* b = new ToolbarButton(itemId, buttonText, iconOff, nullptr);
    b->addListener(listener);
    return b;
}


