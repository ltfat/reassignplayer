#pragma once
#include "AudioHandler.h"
#include "MainGuiWidgets.h"
#include "ReassignedBLFilterbank.h"
#include "Spectrogram.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_opengl/juce_opengl.h>

class MainContentComponent : public juce::Component,
                             private juce::Button::Listener,
                             private juce::ChangeListener,
                             private juce::Label::Listener,
                             private juce::MenuBarModel
{
public:
    MainContentComponent();
    ~MainContentComponent() override;

    enum MENUITEMS { FILE = 0,
        OPTIONS,
        INFO };
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex,
        const juce::String& menuName) override;
    juce::StringArray getMenuBarNames() override;

    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void buttonClicked (juce::Button* b) override;
    void labelTextChanged (juce::Label* l) override;

    void replaceFilterbank (ReassignedBLFilterbank* fb);
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::ScopedPointer<AudioHandler> aHandler;
    juce::ScopedPointer<Spectrogram> spectrogram;
    juce::ScopedPointer<ReassignedBLFilterbank> filterbank;
    juce::ScopedPointer<juce::OpenGLContext> ogl;
    juce::ScopedPointer<FilterbankDataHolder> dataHolder;

    juce::ScopedPointer<MenuBarModel> menuBarModel;
    juce::ScopedPointer<juce::MenuBarComponent> menuBarComponent;
    juce::Toolbar toolbar;
    juce::ScopedPointer<juce::ToolbarItemFactory> tbfac;
    juce::Label fileLabel;

    juce::ScopedPointer<PlaylistWindow> plWindow;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
