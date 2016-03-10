/*
  ==============================================================================

    MainGuiWidgets.h
    Created: 9 Mar 2016 11:47:28am
    Author:  susnak

  ==============================================================================
*/

#ifndef MAINGUIWIDGETS_H_INCLUDED
#define MAINGUIWIDGETS_H_INCLUDED

#include "JuceHeader.h"
#include "CustomToolbarButton.h"
#include "AudioHandler.h"


class MainToolbarItemFactory: public ToolbarItemFactory
{
public:
    MainToolbarItemFactory(ButtonListener* listener_);
    ~MainToolbarItemFactory();

    void getAllToolbarItemIds(Array<int> &ids) override;
    void getDefaultItemSet(Array<int> &ids) override;
    ToolbarItemComponent* createItem(int itemId) override;

private:

    enum ToolbarItemIds
    {
        back = 1,
        play,
        pause,
        stop,
        forward,
        micToggle,
        fileToggle,
        loopToggle,
        saveImg,
        playlist
    };
    ButtonListener* listener;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainToolbarItemFactory)
};


class PlaylistWindow: public DocumentWindow,
    public ListBoxModel
{
public:
    PlaylistWindow(ButtonListener* listener_, AudioHandler* ah);
    ~PlaylistWindow();

    class PlaylistWindowToolbarItemFactory: public ToolbarItemFactory
    {
    public:
        PlaylistWindowToolbarItemFactory(ButtonListener* listener_);
        ~PlaylistWindowToolbarItemFactory();

        void getAllToolbarItemIds(Array<int> &ids) override;
        void getDefaultItemSet(Array<int> &ids) override;
        ToolbarItemComponent* createItem(int itemId) override;
    private:
        enum ToolbarItemIds
        {
            addFiles = 1,
            removeSelected,
            clearList
        };

        ButtonListener* listener;
        // AudioHandler* aHandler;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistWindowToolbarItemFactory)
    };


    int getNumRows () override;
    void paintListBoxItem (int rowNumber, Graphics &g,
                           int width, int height, bool rowIsSelected) override;
    void deleteKeyPressed (int lastRowSelected) override;

    void substitutePlaylist (Array<File> newList);
    void addItemsToList (Array<File> newList);
    /*void addItemsToList (File& newList);*/

    void removeItemsFromList (SparseSet<int> selectedRows);
    void clearPlaylist();
    void listBoxItemDoubleClicked (int row, const MouseEvent &) override;

    //void removeItemsFromList (int index_);

    void resized() override;
    void closeButtonPressed() override;

    Toolbar fileControl;
private:
    ButtonListener* listener;
    AudioHandler* aHandler;
    ScopedPointer<ToolbarItemFactory> plfac;
    Array<String> playList;
    ScopedPointer<ListBox> playListBox;

};

class FilterbankSelectWindow    : public DialogWindow,
    private ButtonListener
{
public:
    FilterbankSelectWindow (File fbFile, Array<unsigned long> startingBytes,
                            Array<unsigned> blockLengths, unsigned* activeFilterbank);

    void closeButtonPressed () override;
    void buttonClicked (Button* b) override;
private:
    ScopedPointer<TextButton> confirmButton;
    ScopedPointer<Label> dialogText;
    //TextButton cancelButton("Cancel");
    unsigned filterbanksRead;
    unsigned* activeFilterbank;
    OwnedArray<ToggleButton> fbDataButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankSelectWindow)
};

class InfoWindow    : public DialogWindow
{
public:
    InfoWindow();

    void closeButtonPressed () override;
private:
    ScopedPointer<Label> dialogText;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoWindow)
};



#endif  // MAINGUIWIDGETS_H_INCLUDED
