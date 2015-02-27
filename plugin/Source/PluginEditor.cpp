/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
PluginEditor::PluginEditor (PluginAudioProcessor& p)
    : AudioProcessorEditor(p),
      processor(p)
{
    addAndMakeVisible (spectrogram = new Spectrogram());
    spectrogram->setName ("spectrogram");


    //[UserPreSize]

    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
   reassignToggle = new TextButton("Reassig toggle");
   reassignToggle->setToggleState(true, sendNotification);
   reassignToggle->setClickingTogglesState(true);
   reassignToggle->addListener(this); 

   channelChooser = new ComboBox();
   for (int ii = 1; ii <= processor.getNumInputChannels(); ++ii)
   {
      channelChooser->addItem(String(ii), ii);
   }
   channelChooser->setSelectedId(1, sendNotification);
   Label* l = new Label("ch", "Channel:");
   trash.add(l);
   l->attachToComponent(channelChooser, true);
   channelChooser->addListener(this);


   spectrogram->getPopupMenu().addSeparator();
   spectrogram->getPopupMenu().addSectionHeader("Plugin options");
   spectrogram->getPopupMenu().addCustomItem(0, channelChooser, 60, 30, false);
   spectrogram->getPopupMenu().addCustomItem(0, reassignToggle, 60, 30, false);


  ogl = new OpenGLContext();
  ogl->attachTo(*spectrogram);

    //ogl->setSwapInterval(1);
   /* DBG("PluginEditor constructor");
    settings = nullptr;
    fileFilter = new JSONFilterbankFileFilter("JSONFilterbankFileFilter");
    tsThread = new TimeSliceThread("DirectoryContentsList thread");
    DirectoryContentsList* currDirContents =
       new DirectoryContentsList(fileFilter,*tsThread);

    currDirContents->setDirectory(File::getSpecialLocation(
                                      File::currentApplicationFile).
                                          getParentDirectory(),false,true);
    currDirContents->addChangeListener(this);
    dirContents.add(currDirContents);
    // Thread is already started in setDirectory
    //tsThread->startThread();
    //loadFilters();
    // DBG("PluginEditor constructor end");
    */
    //[/Constructor]
}

PluginEditor::~PluginEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    ogl->detach();
    ogl = nullptr;
    //[/Destructor_pre]

    spectrogram = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
   // tsThread->stopThread(0);
   // tsThread = nullptr;
    //[/Destructor]
}

//==============================================================================
void PluginEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void PluginEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    spectrogram->setBounds (0, 0, proportionOfWidth (1.0000f), proportionOfHeight (1.0000f));
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void PluginEditor::mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel)
{
   // HeapBlock<float> tmp = HeapBlock<float>();

   //spectrogram->appendStrip(tmp,0,0);
}

void PluginEditor::loadFilters()
{
   if (nullptr != settings)
   {
      // find whether there is a filterbankPath entry
      // and add it to dirContents if it is not there already
   }

   filterbankFiles.clear(true);
   for (DirectoryContentsList * d : dirContents)
   {

      // Block until the dir search is finished
      while (d->isStillLoading()) {}

      if (d->getNumFiles() != 0)
      {
         for (int ii = 0; ii < d->getNumFiles(); ++ii)
         {
            File f = d->getFile(ii);
            filterbankFiles.add(new File(f));
            DBG("Filterbank file found!" << f.getFileName());
         }
      }
   }

}

void PluginEditor::changeListenerCallback(ChangeBroadcaster *source)
{
   // We are now on the main message thread
   /*  RingTransformBuffer* rbuf = processor.getRingBuffer();
     if(source == rbuf)
     {
        // Consume 1 buffer from FFT buffer
        const fftwf_complex* fbuf = rbuf->getBuffer();
        if(nullptr != fbuf)
        {
           spectrogram->appendStrip(reinterpret_cast<const std::complex<float>*>(fbuf),rbuf->getBufLen()/3);
           DBG("Consumed Buffer");
        }
     }
     */
}
void PluginEditor::comboBoxChanged (ComboBox* comboBox)
{
   if (comboBox == channelChooser)
   {
      processor.setParameterNotifyingHost (PluginAudioProcessor::kActChannel, comboBox->getSelectedId() - 1);
   }

}

void PluginEditor::buttonClicked (Button* button)
{
   if (button == reassignToggle)
   {
      processor.setParameterNotifyingHost (PluginAudioProcessor::kReassignedSwitch, reassignToggle->isDown());
   }

}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PluginEditor" componentName=""
                 parentClasses="public AudioProcessorEditor, public ButtonListener, public ComboBoxListener, public ChangeListener"
                 constructorParams="PluginAudioProcessor&amp; p" variableInitialisers="AudioProcessorEditor(p)&#10;processor(p)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <GENERICCOMPONENT name="spectrogram" id="7c4cfc3cd1d13dfb" memberName="spectrogram"
                    virtualName="" explicitFocusOrder="0" pos="0 0 100% 100%" class="Spectrogram"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
