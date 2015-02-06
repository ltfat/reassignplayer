/*
  ==============================================================================

    StandalonePluginWindow.cpp
    Created: 5 Feb 2015 8:38:47pm
    Author:  susnak

  ==============================================================================
*/

#include "StandaloneFilterWindow.h"


StandaloneFilterWindow::StandaloneFilterWindow (const String& title,
      Colour backgroundColour,
      PropertySet* settingsToUse,
      bool takeOwnershipOfSettings)
   : DocumentWindow (title, backgroundColour, DocumentWindow::minimiseButton | DocumentWindow::closeButton),
     optionsButton ("options"),
     micfileButton ("MIC")
{
   setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);

   Component::addAndMakeVisible (optionsButton);
   Component::addAndMakeVisible (micfileButton);
   micfileButton.addListener(this);
   optionsButton.addListener (this);
   optionsButton.setTriggeredOnMouseDown (true);
   micfileButton.setTriggeredOnMouseDown (true);
   micfileButton.setClickingTogglesState(true);

   pluginHolder = new StandalonePluginHolder (settingsToUse, takeOwnershipOfSettings);

   createEditorComp();

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
   setContentOwned (getAudioProcessor()->createEditorIfNeeded(), true);
}

void StandaloneFilterWindow::deleteEditorComp()
{
   if (AudioProcessorEditor* ed = dynamic_cast<AudioProcessorEditor*> (getContentComponent()))
   {
      pluginHolder->processor->editorBeingDeleted (ed);
      clearContentComponent();
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
   if (b == &optionsButton)
   {
      PopupMenu m;
      m.addItem (1, TRANS("Audio Settings..."));
      m.addSeparator();
      m.addItem (2, TRANS("Save current state..."));
      m.addItem (3, TRANS("Load a saved state..."));
      m.addSeparator();
      m.addItem (4, TRANS("Reset to default state"));

      switch (m.showAt (&optionsButton))
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
   }
   if (b == &micfileButton)
   {
      if(b->getToggleState())
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

}

void StandaloneFilterWindow::resized()
{
   DocumentWindow::resized();
   optionsButton.setBounds (8, 6, 60, getTitleBarHeight() - 8);
   micfileButton.setBounds (78, 6, 60, getTitleBarHeight() - 8);
}
