/*
  ==============================================================================

    MainGuiWidgets.h
    Created: 9 Mar 2016 11:47:28am
    Author:  susnak

  ==============================================================================
*/

#pragma once

#include "AudioHandler.h"
#include "CustomToolbarButton.h"

class MainToolbarItemFactory : public juce::ToolbarItemFactory
{
public:
    MainToolbarItemFactory (juce::Button::Listener* listener_);
    ~MainToolbarItemFactory();

    void getAllToolbarItemIds (juce::Array<int>& ids) override;
    void getDefaultItemSet (juce::Array<int>& ids) override;
    juce::ToolbarItemComponent* createItem (int itemId) override;

private:
    enum ToolbarItemIds {
        back = 1,
        play,
        pause,
        stop,
        forward,
        micToggle,
        fileToggle,
        loopToggle,
        saveImg,
        playlist,
        fbfile,
        switchreass
    };
    juce::Button::Listener* listener;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainToolbarItemFactory)
};

class PlaylistWindow : public juce::DocumentWindow,
                       public juce::ChangeBroadcaster,
                       public juce::ListBoxModel
{
public:
    PlaylistWindow (juce::Button::Listener* listener_, AudioHandler* ah);
    ~PlaylistWindow();

    class PlaylistWindowToolbarItemFactory : public juce::ToolbarItemFactory
    {
    public:
        PlaylistWindowToolbarItemFactory (juce::Button::Listener* listener_);
        ~PlaylistWindowToolbarItemFactory();

        void getAllToolbarItemIds (juce::Array<int>& ids) override;
        void getDefaultItemSet (juce::Array<int>& ids) override;
        juce::ToolbarItemComponent* createItem (int itemId) override;

    private:
        enum ToolbarItemIds { addFiles = 1,
            removeSelected,
            clearList };

        juce::Button::Listener* listener;
        // AudioHandler* aHandler;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (
            PlaylistWindowToolbarItemFactory)
    };

    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void deleteKeyPressed (int lastRowSelected) override;

    void substitutePlaylist (juce::Array<juce::File> newList);
    void addItemsToList (juce::Array<juce::File> newList);
    /*void addItemsToList (File& newList);*/

    void removeItemsFromList (juce::SparseSet<int> selectedRows);
    void clearPlaylist();
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;

    // void removeItemsFromList (int index_);

    void resized() override;
    void closeButtonPressed() override;

    juce::Toolbar fileControl;

private:
    juce::Button::Listener* listener;
    AudioHandler* aHandler;
    juce::ScopedPointer<juce::ToolbarItemFactory> plfac;
    juce::Array<juce::String> playList;
    juce::ScopedPointer<juce::ListBox> playListBox;
};

class FilterbankSelectWindow : public juce::DialogWindow,
                               private juce::Button::Listener
{
public:
    FilterbankSelectWindow (juce::File fbFile,
        juce::Array<unsigned long> startingBytes,
        juce::Array<unsigned> blockLengths,
        unsigned* activeFilterbank);

    void closeButtonPressed() override;
    void buttonClicked (juce::Button* b) override;

private:
    juce::ScopedPointer<juce::TextButton> confirmButton;
    juce::ScopedPointer<juce::Label> dialogText;
    // TextButton cancelButton("Cancel");
    unsigned filterbanksRead;
    unsigned* activeFilterbank;
    juce::OwnedArray<juce::ToggleButton> fbDataButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankSelectWindow)
};

class InfoWindow : public juce::DialogWindow
{
public:
    InfoWindow();

    void closeButtonPressed() override;

private:
    juce::ScopedPointer<juce::Label> dialogText;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoWindow)
};
