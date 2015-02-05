/*
  ==============================================================================

    Main.cpp
    Created: 5 Feb 2015 11:57:03am
    Author:  susnak

  ==============================================================================
*/

#include "JuceHeader.h"
#include "MainComponent.h"
#include "juce_StandaloneFilterWindow.h"

class StandaloneApplication  : public JUCEApplication
{
public:
    StandaloneApplication() {}
    
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        filterWindow = new StandaloneFilterWindow (getApplicationName(),
                                                   Colour(),
                                                   new PropertySet(),
                                                   true);
        filterWindow->setVisible(true);
        
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        filterWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
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

};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (StandaloneApplication)
