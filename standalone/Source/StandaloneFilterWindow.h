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


//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplicationBase::initialise() method, and
    let it do its work. It will create your filter object using the same createPluginFilter() function
    that the other plugin wrappers use.
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  public ButtonListener,   // (can't use Button::Listener due to VC2005 bug)
                                  public LabelListener,
                                  public MenuBarModel
{
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
    
    enum MENUITEMS { OPTIONS = 0 };
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    PopupMenu getMenuForIndex(int topLevelMenuIndex, const String &menuName) override;
    StringArray getMenuBarNames() override;
    

    
    void resized() override;

private:
    ScopedPointer<StandalonePluginHolder> pluginHolder;
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

    class FilterWindowToolbarItemFactory: public ToolbarItemFactory
    {
    public:
        FilterWindowToolbarItemFactory(ButtonListener* listener_);
        ~FilterWindowToolbarItemFactory();

        void getAllToolbarItemIds(Array<int> &ids) override;
        void getDefaultItemSet(Array<int> &ids) override;
        ToolbarItemComponent* createItem(int itemId) override;

    private:
        ButtonListener* listener;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterWindowToolbarItemFactory)
    };

    class GenericToolbarItemComponent: public ToolbarItemComponent
    {
    public:
        GenericToolbarItemComponent (int itemId,const String &labelText, bool isBeingUsedAsAButton);

        void paintButtonArea (Graphics &g, int width, int height,
                              bool isMouseOver, bool isMouseDown);
        void contentAreaChanged (const Rectangle<int> &newBounds);
        bool getToolbarItemSizes (int toolbarThickness, bool isToolbarVertical,
                                  int &preferredSize, int &minSize, int &maxSize);
    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericToolbarItemComponent)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow)
};



#endif   // JUCE_STANDALONEFILTERWINDOW_H_INCLUDED
