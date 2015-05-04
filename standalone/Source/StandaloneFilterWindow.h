/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_STANDALONEFILTERWINDOW_H_INCLUDED
#define JUCE_STANDALONEFILTERWINDOW_H_INCLUDED

#include "JuceHeader.h"
#include "StandalonePluginHolder.h"
#include "../../plugin/Source/FilterbankDataHolder.h"
#include <fstream>


//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplicationBase::initialise() method, and
    let it do its work. It will create your filter object using the same createPluginFilter() function
    that the other plugin wrappers use.
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  private ButtonListener,   // (can't use Button::Listener due to VC2005 bug)
                                  private LabelListener,
                                  private ChangeListener,
                                  private MenuBarModel
{
private:
    class PlaylistWindow: public DocumentWindow,
                          public ListBoxModel
    {
    public:
        PlaylistWindow(ButtonListener* listener_, StandalonePluginHolder* pluginHolder_);
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
            StandalonePluginHolder* pluginHolder;
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaylistWindowToolbarItemFactory)
        };


       int getNumRows () override;
       void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
       void deleteKeyPressed (int lastRowSelected) override;

       void substitutePlaylist (Array<File> newList);
       void addItemsToList (Array<File> newList);
       /*void addItemsToList (File& newList);*/

       void removeItemsFromList (SparseSet<int> selectedRows);
       void clearPlaylist();
       void listBoxItemDoubleClicked (int row, const MouseEvent &) override;

       //void removeItemsFromList (int index_);

       void resized() override;

       Toolbar fileControl;
    private:
       ScopedPointer<ToolbarItemFactory> plfac;
       Array<String> playList;
       ScopedPointer<ListBox> playListBox;
       ButtonListener* listener;
       StandalonePluginHolder* pluginHolder;

       void closeButtonPressed() override;
    };

    class FilterbankSelectWindow    : public DialogWindow,
    private ButtonListener
    {
        public:
            FilterbankSelectWindow (File fbFile, Array<unsigned long> startingBytes, Array<unsigned> blockLengths, unsigned* activeFilterbank);

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

public:
    //==============================================================================
    /** Creates a window with a given title and colour.
        The settings object can be a PropertySet that the class should use to
        store its settings (it can also be null). If takeOwnershipOfSettings is
        true, then the settings object will be owned and deleted by this object.
    */
    StandaloneFilterWindow (const String& title,
                            Colour backgroundColour,
                            PropertySet* settingsToUse,
                            bool takeOwnershipOfSettings);

    ~StandaloneFilterWindow();

    //==============================================================================
    AudioProcessor* getAudioProcessor() const noexcept    { return pluginHolder->getPluginProcessor(); }
    AudioDeviceManager& getDeviceManager() const noexcept { return pluginHolder->getDeviceManager(); }
    //==============================================================================

    void createEditorComp();

    void deleteEditorComp();

    /** Deletes and re-creates the plugin, resetting it to its default state. */
    void resetToDefaultState();

    //==============================================================================
    void closeButtonPressed() override;

    void buttonClicked (Button* b) override;
    void labelTextChanged (Label* l) override;

    //==============================================================================
    // MenuBarModel related

    enum MENUITEMS { FILE = 0, OPTIONS, INFO };
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName) override;
    StringArray getMenuBarNames() override;

    void resized() override;

    //==============================================================================
    // Filterbank file chooser
    Array<File> FilterbankFileLoader();

    //==============================================================================
    // Playlist Window
    ScopedPointer<PlaylistWindow> plWindow;
    ScopedPointer<InfoWindow> infoWindow;

private:
    unsigned activeFilterbank;
    ScopedPointer<OpenGLContext> ogl;
    ScopedPointer<StandalonePluginHolder> pluginHolder;
    ScopedPointer<FilterbankSelectWindow> fbWindow;
    //==============================================================================

    // Graphic components
    TextButton optionsButton;
    TextButton micfileButton;
    TextButton fileChooserButton;
    Label fileLabel;

    ScopedPointer<MenuBarModel> menuBarModel;
    ScopedPointer<MenuBarComponent> menuBarComponent;
    // Toolbar
    Toolbar toolbar;
    ScopedPointer<ToolbarItemFactory> tbfac;

    // Filterbank to use
    unsigned selectedFilterbank;

    // ChangeListener required
    void changeListenerCallback(ChangeBroadcaster* source);

    class FilterWindowToolbarItemFactory: public ToolbarItemFactory
    {
    public:
        FilterWindowToolbarItemFactory(ButtonListener* listener_);
        ~FilterWindowToolbarItemFactory();

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
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterWindowToolbarItemFactory)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow)
};

#endif   // JUCE_STANDALONEFILTERWINDOW_H_INCLUDED

