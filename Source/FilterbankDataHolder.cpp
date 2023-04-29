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

FilterbankDataHolder::FilterbankDataHolder()
    : numOfFilterbanks (0), reassignable (false)
{
    juce::Array<juce::File> loadedFilterbankFiles = FilterbankFileLoader();
    init (loadedFilterbankFiles);
}

FilterbankDataHolder::FilterbankDataHolder (
    juce::Array<juce::File> loadedFilterbankFiles)
    : numOfFilterbanks (0), reassignable (false)
{
    init (loadedFilterbankFiles);
}

// Initialization

void FilterbankDataHolder::init (juce::Array<juce::File> loadedFilterbankFiles)
{
    // First validity check: All files are of the same size
    switch (loadedFilterbankFiles.size())
    {
        case 0:
            throw (juce::String ("Schmorf"));
            break;
        case 1:
            rawFilterbankData.resize (1);
            break;
        case 3:
            if (loadedFilterbankFiles[0].getSize() != loadedFilterbankFiles[1].getSize() || loadedFilterbankFiles[0].getSize() != loadedFilterbankFiles[2].getSize())
            {
                throw (juce::String ("Schmorf (2)"));
            }
            rawFilterbankData.resize (3);
            reassignable = true;
            break;
        default:
            throw (juce::String ("Schmorf (3)"));
    }

    // Check file existence

    for (int kk = 0; kk < loadedFilterbankFiles.size(); ++kk)
    {
        if (!loadedFilterbankFiles[kk].existsAsFile())
            throw juce::String (juce::String ("File ") + loadedFilterbankFiles[kk].getFileName() + juce::String (" not found!"));
    }

    // Create Stream
    std::unique_ptr<juce::FileInputStream> baseFilterbankStream =
        loadedFilterbankFiles[0].createInputStream();

    loadedFilterbankFiles[0].getSize();

    for (int kk = 0; kk < loadedFilterbankFiles.size(); ++kk)
    {
        loadedFilterbankFiles[kk].loadFileAsData (
            rawFilterbankData.getReference (kk));
    }

    // Read data from stream
    juce::int64 currentPosition = 0, binFilterbankLength;

    while (currentPosition < loadedFilterbankFiles[0].getSize())
    {
        baseFilterbankStream->setPosition (currentPosition);
        startingBytes.add (currentPosition);

        if (currentPosition < loadedFilterbankFiles[0].getSize() - 6)
        {
            if (sizeof (unsigned long) > 4) // Handle standard case of 32-bit unsigned
            {
                unsigned tempInt;
                unsigned short tempIntShort;
                baseFilterbankStream->read (&tempInt, 4);
                binFilterbankLength = static_cast<unsigned long> (tempInt);

                baseFilterbankStream->read (&tempIntShort, 2);
                blockLengths.add (static_cast<unsigned> (tempIntShort));
            }
            else // Handle case of 16-bit unsigned
            {
                baseFilterbankStream->read (&binFilterbankLength, 4);
                unsigned tempInt;
                baseFilterbankStream->read (&tempInt, 2);
                blockLengths.add (tempInt);
            }
        }
        else
        {
            throw juce::String ("File read error (1)");
        }

        currentPosition += binFilterbankLength;
        numOfFilterbanks++;
    }

    if (currentPosition != loadedFilterbankFiles[0].getSize())
    {
        throw juce::String ("File read error (2)");
    }

    fbIndex = 0;
    // fbWindow = new FilterbankSelectWindow
    // (loadedFilterbankFiles[0].getFileNameWithoutExtension(), blockLengths,
    // &fbIndex);

    // if ( numOfFilterbanks > 1)
    // {
    //     //fbWindow = new FilterbankSelectWindow
    //     (loadedFilterbankFiles[0].getFileNameWithoutExtension(), blockLengths,
    //     &fbIndex);
    //     // fbWindow->setVisible(true);
    // }
}

// Parameter reading functions etc.

bool FilterbankDataHolder::getFilterbankData (
    juce::Array<juce::MemoryBlock>& rawFilterbankData_)
{
    if (rawFilterbankData.size() != 0)
    {
        rawFilterbankData_ = rawFilterbankData;
        return true;
    }
    else
    {
        return false;
    }
}

juce::int64 FilterbankDataHolder::getStartingByte (int fbIndex_)
{
    if (fbIndex_ >= 0 && fbIndex_ < numOfFilterbanks)
        return startingBytes[fbIndex_];
    else
        return -1;
}

long FilterbankDataHolder::getBlockLength (int fbIndex_)
{
    if (fbIndex_ >= 0 && fbIndex_ < numOfFilterbanks)
        return blockLengths[fbIndex_];
    else
        return -1;
}

int FilterbankDataHolder::getActiveFilterbank() { return fbIndex; }

bool FilterbankDataHolder::setActiveFilterbank (int fbIndex_)
{
    if (fbIndex_ >= 0 && fbIndex_ < numOfFilterbanks)
    {
        fbIndex = fbIndex_;
        return true;
    }
    else
        return false;
}

bool FilterbankDataHolder::isReassignable() { return reassignable; }

void FilterbankDataHolder::selectorWindowVisibility (bool isVisible_)
{
    fbWindow->setVisible (isVisible_);
}

void FilterbankDataHolder::addChangeListenerToWindow (
    juce::ChangeListener* listener)
{
    fbWindow->addChangeListener (listener);
}

void FilterbankDataHolder::removeChangeListenerFromWindow (
    juce::ChangeListener* listener)
{
    fbWindow->removeChangeListener (listener);
}

//============================================================================
// BLFilterbankDef

FilterbankDataHolder::BLFilterbankDef*
    FilterbankDataHolder::BLFilterbankDef::createDefFromFile (
        juce::File& file,
        juce::int64 byteOffset)
{
    juce::MemoryBlock fileAsData;
    file.loadFileAsData (fileAsData);
    return FilterbankDataHolder::BLFilterbankDef::createDefFromData (fileAsData,
        byteOffset);
}

FilterbankDataHolder::BLFilterbankDef*
    FilterbankDataHolder::BLFilterbankDef::createDefFromData (
        juce::MemoryBlock& memBlock,
        juce::int64 byteOffset)
{
    // Create data stream:
    juce::MemoryInputStream dataStreamPtr (memBlock, 0);
    juce::int64 dataSize = dataStreamPtr.getDataSize();

    if (dataSize < byteOffset + 8)
    {
        throw juce::String (
            "Reading error, stream does not contain the desired filterbank data");
    }

    dataStreamPtr.setPosition (
        byteOffset); // Set stream position to beginning of filterbank definition

    // Read filterbank length
    unsigned long binFilterbankLength;
    if (sizeof (unsigned long) > 4) // Handle standard case of 32-bit unsigned
    {
        unsigned tempInt;
        dataStreamPtr.read (&tempInt, 4);
        binFilterbankLength = static_cast<unsigned long> (tempInt);
    }
    else // Handle case of 16-bit unsigned
    {
        dataStreamPtr.read (&binFilterbankLength, 4);
    }

    if (dataSize < byteOffset + binFilterbankLength)
    {
        throw juce::String ("Reading error, stream does not contain the desired "
                            "filterbank data (2)");
    }

    // This is only for testing purposes
    // std::cout << byteSize << std::endl;
    unsigned blockLength;
    unsigned M;

    // Determine block length and number of channels
    BLFilterbankDef::getFilterbankBaseData (&dataStreamPtr, &blockLength, &M);

    // This is only for testing purposes
    // std::cout << "blockLength is: " << blockLength << std::endl;
    // std::cout << "M is: " << M << std::endl;

    unsigned aOne;
    unsigned* a = new unsigned[M];
    float* fc = new float[M];
    unsigned* foff = new unsigned[M];
    unsigned* filtLengths = new unsigned[M];

    // Determine filter bank parameters
    // (TODO: change order in the MATLAB files for filtLengths to be at the end)
    BLFilterbankDef::getFilterbankParamData (&dataStreamPtr, M, &aOne, a, fc, foff, filtLengths);

    // This is only for testing purposes
    // std::cout << "foff is: " << std::endl;
    // for (int kk = 0; kk < 10; kk++)
    // {std::cout << foff[kk] << std::endl;}
    //
    // std::cout << "a is: " << std::endl;
    // for (int kk = 0; kk < 10; kk++)
    // {std::cout << a[kk] << std::endl;}

    // This is only for testing purposes
    // std::cout << shouldBeAtLeast << std::endl;

    float** G = new float*[M];
    for (unsigned kk = 0; kk < M; ++kk)
    {
        G[kk] = new float[filtLengths[kk]];
    }

    // Get the filter data
    BLFilterbankDef::getFilterbankFilterData (&dataStreamPtr, M, filtLengths, G);

    // This is only for testing purposes
    //  std::cout << "G[200] is: " << std::endl;
    // for (int kk = 0; kk < 17; kk++)
    // {std::cout << G[200][kk] << std::endl;}

    int* Gl_ = static_cast<int*> (fftwf_malloc (M * sizeof (int)));
    fftwf_complex** G_ =
        static_cast<fftwf_complex**> (fftwf_malloc (M * sizeof (fftwf_complex*)));
    for (unsigned kk = 0; kk < M; ++kk)
    {
        G_[kk] = static_cast<fftwf_complex*> (
            fftwf_malloc (filtLengths[kk] * sizeof (fftwf_complex)));
    }
    ltfat_int* foff_ =
        static_cast<ltfat_int*> (fftwf_malloc (M * sizeof (ltfat_int)));
    int* realonly_ = static_cast<int*> (fftwf_malloc (M * sizeof (ltfat_int)));
    memset (realonly_, 0, M * sizeof (ltfat_int));
    double* a_ = static_cast<double*> (fftwf_malloc (M * sizeof (double)));
    double* fc_ = static_cast<double*> (fftwf_malloc (M * sizeof (double)));
    ltfat_int* Lc_ =
        static_cast<ltfat_int*> (fftwf_malloc (M * sizeof (ltfat_int)));
    ltfat_int* Lchalf_ =
        static_cast<ltfat_int*> (fftwf_malloc (M * sizeof (ltfat_int)));

    for (unsigned kk = 0; kk < M; ++kk)
    {
        Gl_[kk] = filtLengths[kk];
        for (int jj = 0; jj < Gl_[kk]; ++jj)
        {
            G_[kk][jj][0] = G[kk][jj];
            G_[kk][jj][1] = 0.0f;
        }
        foff_[kk] = foff[kk];
        a_[kk] = static_cast<double> (aOne) / a[kk];
        fc_[kk] = static_cast<double> (fc[kk]);
        Lc_[kk] = static_cast<ltfat_int> (
            std::round (static_cast<double> (blockLength) / a_[kk]));
        Lchalf_[kk] = static_cast<ltfat_int> (std::floor (Lc_[kk] / 2.0));
    }

    BLFilterbankDef* retVal = new BLFilterbankDef (
        const_cast<const fftwf_complex**> (G_), Gl_, foff_, realonly_, a_, fc_, Lc_, Lchalf_, (int) M, blockLength);

    //  Free tep variables
    delete[] a;
    delete[] fc;
    delete[] foff;
    delete[] filtLengths;
    for (unsigned kk = 0; kk < M; ++kk)
    {
        delete[] G[kk];
    }
    delete[] G;

    return retVal;
}

void FilterbankDataHolder::BLFilterbankDef::getFilterbankBaseData (
    juce::MemoryInputStream* dataStreamPtr,
    unsigned* blockLengthPtr,
    unsigned* mPtr)
{
    if (sizeof (unsigned) > 2) // Handle standard case of 32-bit unsigned
    {
        unsigned short tempInt;
        dataStreamPtr->read (&tempInt, 2);
        (*blockLengthPtr) = static_cast<unsigned> (tempInt);
        dataStreamPtr->read (&tempInt, 2);
        (*mPtr) = static_cast<unsigned> (tempInt);
    }
    else // Handle case of 16-bit unsigned
    {
        dataStreamPtr->read (blockLengthPtr, 2);
        dataStreamPtr->read (mPtr, 2);
    }
}

void FilterbankDataHolder::BLFilterbankDef::getFilterbankParamData (
    juce::MemoryInputStream* dataStreamPtr,
    unsigned M,
    unsigned* aOnePtr,
    unsigned a[],
    float fc[],
    unsigned foff[],
    unsigned filtLengths[])
{
    if (sizeof (unsigned) > 2) // Handle standard case of 32-bit unsigned
    {
        unsigned short* tempAry = new unsigned short[M];
        unsigned short tempInt;

        dataStreamPtr->read (&tempInt, 2);
        (*aOnePtr) = static_cast<unsigned> (tempInt);
        dataStreamPtr->read (tempAry, 2 * M);
        for (unsigned kk = 0; kk < M; ++kk)
        {
            a[kk] = static_cast<unsigned> (tempAry[kk]);
        }
        dataStreamPtr->read (fc, 4 * M);
        dataStreamPtr->read (tempAry, 2 * M);
        for (unsigned kk = 0; kk < M; ++kk)
        {
            foff[kk] = static_cast<unsigned> (tempAry[kk]);
        }
        dataStreamPtr->read (tempAry, 2 * M);
        for (unsigned kk = 0; kk < M; ++kk)
        {
            filtLengths[kk] = static_cast<unsigned> (tempAry[kk]);
        }

        delete[] tempAry;
    }
    else // Handle case of 16-bit unsigned
    {
        dataStreamPtr->read (filtLengths, 2 * M);
        dataStreamPtr->read (aOnePtr, 2);
        dataStreamPtr->read (a, 2 * M);
        dataStreamPtr->read (fc, 4 * M);
        dataStreamPtr->read (foff, 2 * M);
    }
}

void FilterbankDataHolder::BLFilterbankDef::getFilterbankFilterData (
    juce::MemoryInputStream* dataStreamPtr,
    unsigned M,
    unsigned filtLengths[],
    float** G)
{
    // We currently assume float to always be 32-bit (We should add a sanity check
    // here at least)
    for (unsigned kk = 0; kk < M; ++kk)
    {
        dataStreamPtr->read (G[kk], 4 * filtLengths[kk]);
    }
}

FilterbankDataHolder::BLFilterbankDef::~BLFilterbankDef()
{
    if (nullptr != G)
    {
        for (int ii = 0; ii < M; ++ii)
            if (nullptr != G[ii])
                fftwf_free (const_cast<fftwf_complex*> (G[ii]));
        fftwf_free (G);
    }
    // I know I should't, but all the C++ casts  are just horrible in this case.
    if (nullptr != Gl)
        fftwf_free ((void*) Gl);
    if (nullptr != foff)
        fftwf_free ((void*) foff);
    if (nullptr != realonly)
        fftwf_free ((void*) realonly);
    if (nullptr != a)
        fftwf_free ((void*) a);
    if (nullptr != Lc)
        fftwf_free ((void*) Lc);
    if (nullptr != Lchalf)
        fftwf_free ((void*) Lchalf);
    if (nullptr != fc)
        fftwf_free ((void*) fc);
}

//============================================================================
// FilterbankSelectWindow

FilterbankDataHolder::FilterbankSelectWindow ::FilterbankSelectWindow (
    juce::String title,
    juce::Array<unsigned>& blockLengths,
    int* fbIndexPtr)
    : DialogWindow (title, juce::Colours::lightgrey, true, true),
      confirmButton (new juce::TextButton ("Ok")),
      activeFilterbank (fbIndexPtr)
{
    // Setup window
    setTitleBarHeight (20);
    dialogText = new juce::Label ("", "Select filter bank block length...");

    for (unsigned kk = 0; kk < blockLengths.size(); ++kk)
    {
        juce::ToggleButton* newButton = new juce::ToggleButton (
            juce::String (std::to_string (blockLengths[kk])) += juce::String (" samples"));
        fbDataButtons.add (newButton);
        fbDataButtons[kk]->setToggleState (kk == 0, juce::dontSendNotification);
        fbDataButtons[kk]->setBounds (20, 55 + 25 * kk, 260, 20);
        fbDataButtons[kk]->setRadioGroupId (1119, juce::dontSendNotification);
        fbDataButtons[kk]->addListener (this);
        Component::addAndMakeVisible (fbDataButtons[kk]);
    }

    setSize (300, 5 + (blockLengths.size() + 3) * 25);
    dialogText->setBounds (20, 30, 260, 20);
    dialogText->setEditable (false);
    confirmButton->setBounds (90, getHeight() - 25, 120, 20);
    confirmButton->addListener (this);
    Component::addAndMakeVisible (dialogText);
    Component::addAndMakeVisible (confirmButton);
}

void FilterbankDataHolder::FilterbankSelectWindow::closeButtonPressed()
{
    this->setVisible (false);
}

void FilterbankDataHolder::FilterbankSelectWindow::buttonClicked (juce::Button* b)
{
    if (b == confirmButton)
    {
        for (int kk = 0; kk < fbDataButtons.size(); ++kk)
        {
            if (fbDataButtons[kk]->getToggleState())
            {
                if (*activeFilterbank != kk)
                {
                    *activeFilterbank = kk;
                    ChangeBroadcaster::sendChangeMessage();
                }
                break;
            }
        }
        closeButtonPressed();
    }
}

//============================================================================
// FilterbankFileLoader

juce::Array<juce::File> FilterbankDataHolder::FilterbankFileLoader()
{
    juce::Array<juce::File> fbData;
    juce::String tmpString = juce::String();
    juce::FileChooser fbDataChooser ("Select filter bank data for analysis...",
        juce::File(),
        "*.lfb");
    if (fbDataChooser.browseForFileToOpen())
    {
        fbData.add (fbDataChooser.getResult());
        tmpString += fbDataChooser.getResult().getFullPathName();
        tmpString = tmpString.dropLastCharacters (4);
        fbData.add (tmpString += juce::String ("_fgrad.lfb"));
        tmpString = tmpString.dropLastCharacters (10);
        fbData.add (tmpString += juce::String ("_tgrad.lfb"));

        if ((!fbData[1].existsAsFile()) || (!fbData[2].existsAsFile()))
            throw juce::String ("Failed to open filter bank data file.");
    }

    // This is empty if user pressed Cancel
    return fbData;
}
