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
     //optionsButton ("options"),
     micfileButton ("MIC"),
     fileChooserButton ("Open file...")
{
   menuBarComponent = new MenuBarComponent(this);

   //tbfac = new FilterWindowToolbarItemFactory(this);
   // Create the wrapped AudioProcessorEditor
   pluginHolder = new StandalonePluginHolder (settingsToUse, takeOwnershipOfSettings);
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
   setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);
   setTitleBarHeight(20);
   setSize(oldWidth, oldHeight + 5 * getTitleBarHeight());
   setResizable(true, true);
   
   // Label
   fileLabel.setText(String("No file loaded"), dontSendNotification);
   fileLabel.setColour(Label::textColourId, Colours::red);
   fileLabel.setColour(Label::backgroundColourId, backgroundColour);


   // Init toolbar
   toolbar.setStyle(Toolbar::textOnly);
   //toolbar.addDefaultItems(*tbfac);
   Component::addAndMakeVisible(toolbar);


   Component::addAndMakeVisible(menuBarComponent);
   Component::addAndMakeVisible(e, true);
   Component::addAndMakeVisible (fileLabel);
   //Component::addAndMakeVisible (optionsButton);
   Component::addAndMakeVisible (fileChooserButton);
   Component::addAndMakeVisible (micfileButton);

   micfileButton.addListener(this);
   fileLabel.addListener(this);
   fileChooserButton.addListener(this);
   //optionsButton.addListener (this);
   //optionsButton.setTriggeredOnMouseDown (true);
   fileChooserButton.setTriggeredOnMouseDown (true);
   micfileButton.setTriggeredOnMouseDown (true);
   micfileButton.setClickingTogglesState(true);



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
   pluginHolder->getPluginProcessor()->createEditorIfNeeded();
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
   if (b == &micfileButton)
   {
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

   if (b == &fileChooserButton)
   {
      FileChooser chooser ("Select a Wave file to play...",
                           File::nonexistent,
                           "*.wav;*.mp3");

      if (chooser.browseForFileToOpen())
      {
         File file (chooser.getResult());
         pluginHolder->setFile(file);
         String s = String(L"Loaded: ") + file.getFullPathName();
         fileLabel.setText(s, sendNotification);
         setName(file.getFileName());
      }
   }

   for (int ii = 0; ii < toolbar.getNumItems(); ii++)
   {
      if (b == toolbar.getItemComponent(ii))
      {
         switch (toolbar.getItemId(ii))
         {

         case 1:
            std::cout << "OK button pressed" << std::endl;
            break;

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
   //optionsButton.setBounds (8, 6, proportionOfWidth(0.1f), getTitleBarHeight() - 8);
   micfileButton.setBounds (getWidth() - 64, getTitleBarHeight()+1, 60, getTitleBarHeight() - 2);
   fileChooserButton.setBounds (getWidth() - 128, getTitleBarHeight()+1, 60, getTitleBarHeight() - 2);
   fileLabel.setBounds(0, getHeight() - getTitleBarHeight(), getWidth() - 20, getTitleBarHeight());

   menuBarComponent->setBounds(0, getTitleBarHeight(), getWidth(), getTitleBarHeight());

   AudioProcessorEditor* e = pluginHolder->getPluginEditor();
   if (e != nullptr)
      e->setBounds(0, 2 * getTitleBarHeight(), getWidth(),
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
{
   // ToolbarItemComponent* b =  new GenericToolbarItemComponent(itemId, String("OK button"),true);
   ToolbarButton* b = new ToolbarButton(itemId, String("OK button"), nullptr, nullptr);
   b->addListener(listener);
   return b;
}

void StandaloneFilterWindow::FilterWindowToolbarItemFactory
::getDefaultItemSet(Array<int> &ids)
{
   ids.add(1);
}

void StandaloneFilterWindow::FilterWindowToolbarItemFactory
::getAllToolbarItemIds(Array<int> &ids)
{
   ids.add(1);
}



//=============================================================================
// ToolbarItemComponent

StandaloneFilterWindow::GenericToolbarItemComponent
::GenericToolbarItemComponent (int itemId, const String &labelText, bool isBeingUsedAsAButton):
   ToolbarItemComponent(itemId, labelText, isBeingUsedAsAButton)  {}

void StandaloneFilterWindow::GenericToolbarItemComponent
::paintButtonArea (Graphics &g, int width, int height, bool isMouseOver, bool isMouseDown) {}

void StandaloneFilterWindow::GenericToolbarItemComponent
::contentAreaChanged (const Rectangle<int> &newBounds) {}

bool StandaloneFilterWindow::GenericToolbarItemComponent
::getToolbarItemSizes (int toolbarThickness, bool isToolbarVertical,
                       int &preferredSize, int &minSize, int &maxSize)
{
   preferredSize = 1;
   minSize = 1;
   maxSize = 1;
}

//============================================================================
// MenuBarModel related

StringArray StandaloneFilterWindow
::getMenuBarNames()
{
   const char * const nameBarNames[] =  {"Options", nullptr};
   return StringArray(nameBarNames);
}

PopupMenu StandaloneFilterWindow
::getMenuForIndex(int topLevelMenuIndex, const String& /*menuName */)
{

   PopupMenu pm;
   switch (topLevelMenuIndex)
   {
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

