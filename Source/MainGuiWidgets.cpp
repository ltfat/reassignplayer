/*
  ==============================================================================

    MainGuiWidgets.cpp
    Created: 9 Mar 2016 11:47:28am
    Author:  susnak

  ==============================================================================
*/

#include "MainGuiWidgets.h"
#include "BinaryData.h"
//==============================================================================
// Toolbar items factory

MainToolbarItemFactory ::MainToolbarItemFactory (juce::Button::Listener* listener_)
    : listener (listener_)
{
}

MainToolbarItemFactory ::~MainToolbarItemFactory()
{
}

juce::ToolbarItemComponent* MainToolbarItemFactory::createItem (int itemId)
{
    int NumBytes;
    juce::String buttonText, binDataOff, binDataOn, binDataOnOne;

    if (itemId != loopToggle)
    {
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
            case fbfile:
                binDataOff = "loadFB_svg";
                binDataOn = "loadFB_svg";
                buttonText = "load filter bank";
                break;
            case switchreass:
                binDataOff = "reassignOff_svg";
                binDataOn = "reassignOn_svg";
                buttonText = "switch reassignment";
                break;
            default:
                break;
        }

        auto iconDataOff = BinaryData::getNamedResource (binDataOff.toRawUTF8(), NumBytes);
        auto iconDataOn = BinaryData::getNamedResource (binDataOn.toRawUTF8(), NumBytes);
        auto* b = new juce::ToolbarButton (itemId, buttonText, juce::Drawable::createFromImageData (iconDataOff, NumBytes), juce::Drawable::createFromImageData (iconDataOn, NumBytes));
        b->addListener (listener);
        return b;
    }
    else
    {
        std::unique_ptr<juce::Drawable> tempPtr;
        juce::Array<juce::Drawable*> loopIcons;
        binDataOff = "loop_svg";
        binDataOn = "loopOn_svg";
        binDataOnOne = "loopOnOne_svg";
        buttonText = "toggle LOOP";

        auto iconData = BinaryData::getNamedResource (binDataOnOne.toRawUTF8(), NumBytes);
        tempPtr = juce::Drawable::createFromImageData (iconData, NumBytes);
        loopIcons.add (tempPtr.get());
        auto* b = new CustomToolbarButton (itemId, buttonText, loopIcons);
        b->addListener (listener);
        return b;
    }
}

void MainToolbarItemFactory ::getDefaultItemSet (juce::Array<int>& ids)
{
    ids.add (flexibleSpacerId);
    ids.add (back);
    ids.add (play);
    ids.add (pause);
    ids.add (stop);
    ids.add (forward);
    ids.add (loopToggle);
    ids.add (flexibleSpacerId);
    ids.add (separatorBarId);
    ids.add (flexibleSpacerId);
    ids.add (micToggle);
    ids.add (flexibleSpacerId);
    ids.add (separatorBarId);
    ids.add (flexibleSpacerId);
    // ids.add(fileToggle);
    // ids.add(saveImg);
    ids.add (switchreass);
    ids.add (fbfile);
    ids.add (playlist);
    ids.add (flexibleSpacerId);
}

void MainToolbarItemFactory ::getAllToolbarItemIds (juce::Array<int>& ids)
{
    ids.add (back);
    ids.add (play);
    ids.add (pause);
    ids.add (stop);
    ids.add (forward);
    ids.add (micToggle);
    ids.add (fileToggle);
    ids.add (loopToggle);
    ids.add (saveImg);
    ids.add (playlist);
    ids.add (switchreass);
    ids.add (fbfile);

    // Spacers and Separators
    ids.add (separatorBarId);
    ids.add (spacerId);
    ids.add (flexibleSpacerId);
}

// Playlist Window

PlaylistWindow::PlaylistWindow (juce::Button::Listener* blistener_, AudioHandler* ah)
    : DocumentWindow ("Playlist", juce::Colours::lightgrey, 0, true),
      aHandler (ah),
      listener (blistener_)
{
    // Window dimensions
    setTitleBarButtonsRequired (DocumentWindow::closeButton, false);
    setTitleBarHeight (20);
    setSize (250, 520);
    setResizable (false, false);

    // TestList
    // playList.add("Item 1");
    // playList.add("Item 2");
    // playList.add("Item 3");
    // playList.add("Item 4");

    // Init playListBox
    playListBox = new juce::ListBox ("stuff", this);
    playListBox->setBounds (0, 20, getWidth(), getHeight() - 60);
    playListBox->setMultipleSelectionEnabled (true);
    playListBox->updateContent();
    playListBox->repaint();
    Component::addAndMakeVisible (*playListBox);
    playListBox->selectRow (0);
    playListBox->setColour (juce::ListBox::backgroundColourId, juce::Colour::greyLevel (0.9f));

    DBG ("Before toolbar");
    // Init fileControl
    plfac = new PlaylistWindowToolbarItemFactory (listener);
    fileControl.setBounds (0, getHeight() - 40, getWidth(), 40);
    fileControl.setComponentID ("playlistToolbar");
    fileControl.addDefaultItems (*plfac);
    Component::addAndMakeVisible (fileControl);
    DBG ("After toolbar");
}

void PlaylistWindow::resized()
{
    DocumentWindow::resized();
    // This line somehow breaks the program
    // fileControl.setBounds(0,getHeight()-40,getWidth(),40);
    // playListBox->setBounds(0,20,getWidth(),getHeight()-60);
}

PlaylistWindow::~PlaylistWindow()
{
}

int PlaylistWindow::getNumRows()
{
    return playList.size();
}

void PlaylistWindow::deleteKeyPressed (int lastRowSelected)
{
    removeItemsFromList (playListBox->getSelectedRows());
}

void PlaylistWindow::substitutePlaylist (juce::Array<juce::File> newList)
{
    playList.clear();
    // pluginHolder->clearFileList();
    addItemsToList (newList);
    playListBox->updateContent();
}

void PlaylistWindow::clearPlaylist()
{
    playList.clear();
    // pluginHolder->clearFileList();
    playListBox->updateContent();
}

void PlaylistWindow::addItemsToList (juce::Array<juce::File> newList)
{
    for (int kk = 0; kk < newList.size(); ++kk)
    {
        playList.add (newList[kk].getFileNameWithoutExtension());
        aHandler->addFile (newList.getReference (kk));
    }
    playListBox->updateContent();
}

/*void PlaylistWindow::addItemsToList (File& newList)
{

}*/

void PlaylistWindow::removeItemsFromList (juce::SparseSet<int> selectedRows)
{
    for (int kk = getNumRows() - 1; kk >= 0; --kk)
    {
        if (selectedRows.contains (kk))
        {
            playList.remove (kk);
            aHandler->removeFile (kk);
        }
    }
    playListBox->deselectAllRows();
    playListBox->updateContent();
}

void PlaylistWindow::closeButtonPressed()
{
    this->setVisible (false);
}

void PlaylistWindow::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (juce::TextEditor::highlightColourId));
    if (rowNumber == aHandler->getCurrentFileIdx())
        g.setColour (juce::Colours::darkblue);
    else
        g.setColour (juce::Colours::black);
    g.setFont (12.0f);
    g.drawFittedText (playList[rowNumber].toRawUTF8(), 8, 0, this->getWidth() - 16, 20, juce::Justification::centredLeft, 1);
    // 8, 0, width - 16, height, Justification::centredLeft, 2);
}

void PlaylistWindow::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    // aHandler->stopPlaying();
    aHandler->setCurrentFileIdx (row, false);
    sendChangeMessage();
    // this->repaint();
}

//========================================================================================================================================
// PlaylistWindowToolbarItemFactory

PlaylistWindow::PlaylistWindowToolbarItemFactory::PlaylistWindowToolbarItemFactory (juce::Button::Listener* listener_)
    : listener (listener_)
{
}

PlaylistWindow::PlaylistWindowToolbarItemFactory::~PlaylistWindowToolbarItemFactory()
{
}

void PlaylistWindow::PlaylistWindowToolbarItemFactory::getAllToolbarItemIds (juce::Array<int>& ids)
{
    ids.add (addFiles);
    ids.add (removeSelected);
    ids.add (clearList);

    // Spacers and Separators
    ids.add (separatorBarId);
    ids.add (spacerId);
    ids.add (flexibleSpacerId);
}

void PlaylistWindow::PlaylistWindowToolbarItemFactory::getDefaultItemSet (juce::Array<int>& ids)
{
    ids.add (addFiles);
    ids.add (removeSelected);
    ids.add (clearList);
    ids.add (flexibleSpacerId);
}

juce::ToolbarItemComponent* PlaylistWindow::PlaylistWindowToolbarItemFactory::createItem (int itemId)
{
    int NumBytes;
    const char* iconData;
    juce::String buttonText, binData;

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
            binData = "removeSelected_svg";
            buttonText = "clearPlaylist";
            break;
        default:
            break;
    }

    iconData = BinaryData::getNamedResource (binData.toRawUTF8(), NumBytes);

    auto* b = new juce::ToolbarButton (itemId, buttonText, juce::Drawable::createFromImageData (iconData, NumBytes), nullptr);
    b->addListener (listener);
    return b;
}
