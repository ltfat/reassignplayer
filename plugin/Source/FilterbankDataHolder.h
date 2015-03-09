#ifndef FILTERBANKDATAHOLDER_H_INCLUDED
#define FILTERBANKDATAHOLDER_H_INCLUDED

#include "../../plugin/JuceLibraryCode/JuceHeader.h"

class FilterbankDataHolder
{
public:
    //FilterbankDataHolder();
    FilterbankDataHolder(Array<File> loadedFilterbankFiles);

    bool getFilterbankData(Array<MemoryBlock> rawFilterbankData_);
    int64 getStartingByte(int fbIndex_);
    long getBlockLengths(int fbIndex_);
    /*OwnedArray<MemoryBlock> rawFilterbankData;
    OwnedArray<unsigned long> startingBytes;
    OwnedArray<unsigned> blockLengths;*/
    int getActiveFilterbank();
    bool setActiveFilterbank(int fbIndex_);

    //==============================================================================
    // Filterbank file chooser
    static Array<File> FilterbankFileLoader();

private:
    void init(Array<File> loadedFilterbankFiles);

    class FilterbankSelectWindow : public DialogWindow,
                                   private ButtonListener
    {
    public:
        FilterbankSelectWindow (String title, Array<unsigned> blockLengths, int* fbIndexPtr);

        void closeButtonPressed () override;
        void buttonClicked (Button* b) override;

    private:
        ScopedPointer<TextButton> confirmButton;
        ScopedPointer<Label> dialogText;
        ScopedPointer<int> activeFilterbank;

        OwnedArray<ToggleButton> fbDataButtons;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankSelectWindow)
    };

    Array<MemoryBlock> rawFilterbankData;
    Array<int64> startingBytes;
    Array<unsigned> blockLengths;
    ScopedPointer<FilterbankSelectWindow> fbWindow;

    int numOfFilterbanks;
    int fbIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankDataHolder)
};

#endif // FILTERBANKCHOOSER_H_INCLUDED
