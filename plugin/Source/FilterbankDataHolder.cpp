/*
  ==============================================================================

    FilterbankChooser.cpp
    Created: 5 Feb 2015 8:38:47pm
    Author:  susnak

  ==============================================================================
*/

#include "FilterbankDataHolder.h"

//============================================================================
// FilterbankDataHolder

FilterbankDataHolder::FilterbankDataHolder():
numOfFilterbanks(0),
reassignable(false)
{
    Array<File> loadedFilterbankFiles = FilterbankFileLoader();
    init(loadedFilterbankFiles);
}

FilterbankDataHolder::FilterbankDataHolder(Array<File> loadedFilterbankFiles):
numOfFilterbanks(0),
reassignable(false)
{
    init(loadedFilterbankFiles);
}

// Initialization

void FilterbankDataHolder::init(Array<File> loadedFilterbankFiles)
{
        // First validity check: All files are of the same size
    switch ( loadedFilterbankFiles.size() )
    {
        case 0:
            throw(String("Schmorf"));
            break;
        case 1:
            rawFilterbankData.resize(1);
            break;
        case 3:
            if ( loadedFilterbankFiles[0].getSize() !=  loadedFilterbankFiles[1].getSize() ||
                 loadedFilterbankFiles[0].getSize() !=  loadedFilterbankFiles[2].getSize() )
            {
                throw(String("Schmorf (2)"));
            }
            rawFilterbankData.resize(3);
            reassignable = true;
            break;
        default:
            throw(String("Schmorf (3)"));
    }

    // Check file existence

    for (int kk=0; kk < loadedFilterbankFiles.size(); ++kk )
    {
        if ( !loadedFilterbankFiles[kk].existsAsFile() )
            throw String(String("File ") + loadedFilterbankFiles[kk].getFileName() + String(" not found!"));
    }

    // Create Stream
    FileInputStream* baseFilterbankStream = loadedFilterbankFiles[0].createInputStream();

    loadedFilterbankFiles[0].getSize();

    for (int kk=0; kk < loadedFilterbankFiles.size(); ++kk )
    {
        loadedFilterbankFiles[kk].loadFileAsData(rawFilterbankData.getReference(kk));
    }

    // Read data from stream
    int64 currentPosition = 0, binFilterbankLength;

    while ( currentPosition < loadedFilterbankFiles[0].getSize() )
    {
        baseFilterbankStream->setPosition(currentPosition);
        startingBytes.add(currentPosition);

        if ( currentPosition < loadedFilterbankFiles[0].getSize()-6 )
        {
            if (sizeof(unsigned long) > 4) // Handle standard case of 32-bit unsigned
            {
                unsigned tempInt;
                unsigned short tempIntShort;
                baseFilterbankStream->read(&tempInt, 4);
                binFilterbankLength = static_cast <unsigned long> (tempInt);

                baseFilterbankStream->read(&tempIntShort, 2);
                blockLengths.add(static_cast <unsigned> (tempIntShort));
            }
            else // Handle case of 16-bit unsigned
            {
                baseFilterbankStream->read(&binFilterbankLength, 4);
                unsigned tempInt;
                baseFilterbankStream->read(&tempInt, 2);
                blockLengths.add(tempInt);
            }
        }
        else
        {
            throw String("File read error (1)");
        }

        currentPosition += binFilterbankLength;
        numOfFilterbanks++;
    }

    if ( currentPosition != loadedFilterbankFiles[0].getSize() )
    {
            throw String("File read error (2)");
    }

    fbIndex = 0;

    if ( numOfFilterbanks > 1)
    {
    //    fbWindow = new FilterbankSelectWindow (loadedFilterbankFiles[0].getFileNameWithoutExtension(), blockLengths, &fbIndex);
    //    fbWindow->setVisible(true);
    }
}

// Parameter reading functions etc.

bool FilterbankDataHolder::getFilterbankData(Array<MemoryBlock> &rawFilterbankData_)
{
    if ( rawFilterbankData.size() != 0)
    {
        rawFilterbankData_ = rawFilterbankData;
        return true;
    }
    else
    {
        return false;
    }
}

int64 FilterbankDataHolder::getStartingByte(int fbIndex_)
{
    if ( fbIndex_ >= 0 && fbIndex_ < numOfFilterbanks )
        return startingBytes[fbIndex_];
    else
        return -1;
}

long FilterbankDataHolder::getBlockLength(int fbIndex_)
{
    if ( fbIndex_ >= 0 && fbIndex_ < numOfFilterbanks )
        return blockLengths[fbIndex_];
    else
        return -1;
}

int FilterbankDataHolder::getActiveFilterbank()
{
    return fbIndex;
}

bool FilterbankDataHolder::setActiveFilterbank(int fbIndex_)
{
    if ( fbIndex_ >= 0 && fbIndex_ < numOfFilterbanks )
    {
        fbIndex = fbIndex_;
        return true;
    }
    else
        return false;
}

bool FilterbankDataHolder::isReassignable()
{
    return reassignable;
}

//============================================================================
// FilterbankSelectWindow

FilterbankDataHolder::FilterbankSelectWindow
::FilterbankSelectWindow (String title, Array<unsigned>& blockLengths, int* fbIndexPtr)
: DialogWindow (title, Colours::lightgrey, true, true),
confirmButton(new TextButton("Ok")),
activeFilterbank(fbIndexPtr)
{
    // Setup window
    setTitleBarHeight(20);
    dialogText = new Label("","Select filter bank block length...");

    for (unsigned kk = 0 ; kk < blockLengths.size(); ++kk)
    {
        ToggleButton* newButton = new ToggleButton( String(std::to_string(blockLengths[kk])) += String(" samples"));
        fbDataButtons.add(newButton);
        fbDataButtons[kk]->setToggleState ( kk==0, dontSendNotification);
        fbDataButtons[kk]->setBounds (20, 55+25*kk, 260, 20);
        fbDataButtons[kk]->setRadioGroupId ( 1119, dontSendNotification);
        fbDataButtons[kk]->addListener(this);
        addAndMakeVisible(fbDataButtons[kk]);
    }

    setSize(300,5+(blockLengths.size()+3)*25);
    dialogText->setBounds (20, 30, 260, 20);
    dialogText->setEditable(false);
    confirmButton->setBounds (90, getHeight()-25, 120, 20);
    confirmButton->addListener(this);
    addAndMakeVisible(dialogText);
    addAndMakeVisible(confirmButton);

}

void FilterbankDataHolder::FilterbankSelectWindow::closeButtonPressed ()
{
    std::cout << *activeFilterbank << std::endl;
    this->removeFromDesktop();
}

void FilterbankDataHolder::FilterbankSelectWindow::buttonClicked (Button* b)
{
    if (b == confirmButton)
    {
        for ( unsigned kk = 0; kk < fbDataButtons.size(); ++kk)
        {
            if ( fbDataButtons[kk]->getToggleState() )
            {
                *activeFilterbank = kk;
                break;
            }
        }
        closeButtonPressed();
    }
}

//============================================================================
// FilterbankFileLoader

Array<File> FilterbankDataHolder::FilterbankFileLoader()
{
   Array<File> fbData;
   String tmpString = String();
   FileChooser fbDataChooser ("Select filter bank data for analysis...",
                               File::nonexistent,
                               "*.lfb");
   if (fbDataChooser.browseForFileToOpen())
   {
       fbData.add(fbDataChooser.getResult());
       tmpString += fbDataChooser.getResult().getFullPathName();
       tmpString = tmpString.dropLastCharacters(4);
       fbData.add(tmpString += String("_fgrad.lfb"));
       tmpString = tmpString.dropLastCharacters(10);
       fbData.add(tmpString += String("_tgrad.lfb"));

       if ( (!fbData[1].existsAsFile()) || (!fbData[2].existsAsFile()))
                fbData.removeLast(2);
   }
   else
   {
       throw String("Failed to open filter bank data file.");
   }

   return fbData;
}


/*FilterbankDataHolder::FilterbankSelectWindow
::FilterbankSelectWindow (File fbFile, Array<unsigned long> startingBytes, Array<unsigned> blockLengths, unsigned* activeFilterbank_)
: DialogWindow (fbFile.getFileNameWithoutExtension(), Colours::lightgrey, true, true),
//ResizableWindow (fbFile.getFileNameWithoutExtension(), true),
confirmButton(new TextButton("Ok")),
filterbanksRead(0),
activeFilterbank(activeFilterbank_),
fbDataButtons()
{

    // Read file and interpret fundamental data
    std::streampos dataSize, readSize;
    std::ifstream dataFile;

    unsigned long byteSize;
    dataFile.open (fbFile.getFullPathName().getCharPointer(),
                   std::ios::in | std::ios::binary | std::ios::ate);

    if (dataFile.fail())
    {
        throw String(String("File ") + fbFile.getFileName() + String(" not found!"));
    }

    dataSize = dataFile.tellg(); // Length = eof
    dataFile.seekg(0, std::ios::beg); // Reset stream to beginning of file

    byteSize = static_cast<unsigned long>(dataSize); // Recast file size as integer

    unsigned long currentPosition = 0, binFilterbankLength;

    while ( currentPosition < byteSize )
    {
        dataFile.seekg(currentPosition, std::ios::beg); // Set stream to start of next filter bank
        startingBytes.add(currentPosition);

        if ( currentPosition < byteSize-6 )
        {
            if (sizeof(unsigned long) > 4) // Handle standard case of 32-bit unsigned
            {
                unsigned tempInt;
                unsigned short tempIntShort;
                dataFile.read(reinterpret_cast <char*> (&tempInt), 4);
                binFilterbankLength = static_cast <unsigned long> (tempInt);

                dataFile.read(reinterpret_cast <char*> (&tempIntShort), 2);
                blockLengths.add(static_cast <unsigned long> (tempIntShort));
            }
            else // Handle case of 16-bit unsigned
            {
                dataFile.read(reinterpret_cast <char*> (binFilterbankLength), 4);
                unsigned tempInt;
                dataFile.read(reinterpret_cast <char*> (&tempInt), 2);
                blockLengths.add(tempInt);
            }
        }
        else
        {
            throw String("File read error (1)");
        }

        currentPosition += binFilterbankLength;
        filterbanksRead++;
    }

    if ( currentPosition != byteSize )
    {
            throw String("File read error (2)");
    }

    *activeFilterbank = 0;

    // Setup window
    dialogText = new Label("","Select filter bank block length...");


    for (unsigned kk = 0 ; kk < filterbanksRead; ++kk)
    {
        ToggleButton* newButton = new ToggleButton( String(std::to_string(blockLengths[kk])) += String(" samples"));
        fbDataButtons.add(newButton);
        fbDataButtons[kk]->setToggleState ( kk==0, dontSendNotification);
        fbDataButtons[kk]->setBounds (20, 55+25*kk, 260, 20);
        fbDataButtons[kk]->setRadioGroupId ( 1119, dontSendNotification);
        fbDataButtons[kk]->addListener(this);
        addAndMakeVisible(fbDataButtons[kk]);
    }

    setSize(300,5+(filterbanksRead+3)*25);
    dialogText->setBounds (20, 30, 260, 20);
    dialogText->setEditable(false);
    confirmButton->setBounds (90, getHeight()-25, 120, 20);
    confirmButton->addListener(this);
    //cancelButton.setBounds (160, getHeight()-25, 120, 20);
    addAndMakeVisible(dialogText);
    addAndMakeVisible(confirmButton);
    //addAndMakeVisible(cancelButton);
}*/
