/*
  ==============================================================================

    Main.cpp
    Created: 5 Feb 2015 11:57:03am
    Author:  susnak

  ==============================================================================
*/

#include "JuceHeader.h"
#include "StandaloneFilterWindow.h"

class StandaloneApplication  : public JUCEApplication
{
public:
   StandaloneApplication() {}
   ~StandaloneApplication()
   {
      DBG("destructor called");
 /*    
       if (nullptr != ps)
      {
         // Save the state
         StringPairArray& sp = ps->getUserSettings()->getAllProperties();
         const StringArray& sak = sp.getAllKeys();
         for (int ii = 0; ii < sp.size(); ii++)
         {
            std::cout << "Key: " << sak[ii] << " and value: " << sp.getValue(sak[ii], String("empty")) << std::endl;
         }

      }
      */
      // Clear the properties
      ps = nullptr;
   }

   const String getApplicationName() override
   {
      return ProjectInfo::projectName;
   }
   const String getApplicationVersion() override
   {
      return ProjectInfo::versionString;
   }
   bool moreThanOneInstanceAllowed() override
   {
      return true;
   }

   //==============================================================================
   void initialise (const String& commandLine) override
   {
      DBG("initialize");
      // This method is where you should put your application's initialisation code..
      PropertiesFile::Options options;
      options.applicationName     = "Standalone Plugin Runner";
      options.filenameSuffix      = "settings";
      options.osxLibrarySubFolder = "Preferences";
      // Load the state or create an empty propertyset
      ps = new ApplicationProperties();
      ps->setStorageParameters(options);

      filterWindow = new StandaloneFilterWindow (getApplicationName(),
            Colours::lightgrey,
            ps->getUserSettings(),
            false);
      filterWindow->setVisible(true);

   }

   void shutdown() override
   {
      DBG("shutdown called");
      // Add your application's shutdown code here..


      filterWindow = nullptr; // (deletes our window)
   }

   //==============================================================================
   void systemRequestedQuit() override
   {
      // This is called when the app is being asked to quit: you can ignore this
      // request and let the app carry on running, or call quit() to allow the app to close.
      DBG("system requested exit");
      quit();
   }

   void anotherInstanceStarted (const String& commandLine) override
   {
      // When another instance of the app is launched while this one is running,
      // this method is invoked, and the commandLine parameter tells you what
      // the other instance's command-line arguments were.
   }
private:
   ScopedPointer<StandaloneFilterWindow> filterWindow;
   ScopedPointer<ApplicationProperties> ps;

};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (StandaloneApplication)
