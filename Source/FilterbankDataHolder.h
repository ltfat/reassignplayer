#pragma once

#include "fftw3.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "ltfat.h"

class FilterbankDataHolder
{
public:
    FilterbankDataHolder();
    FilterbankDataHolder (juce::Array<juce::File> loadedFilterbankFiles);

    bool getFilterbankData (juce::Array<juce::MemoryBlock>& rawFilterbankData_);
    juce::int64 getStartingByte (int fbIndex_);
    long getBlockLength (int fbIndex_);
    /*OwnedArray<MemoryBlock> rawFilterbankData;
    OwnedArray<unsigned long> startingBytes;
    OwnedArray<unsigned> blockLengths;*/
    int getActiveFilterbank();
    bool setActiveFilterbank (int fbIndex_);

    bool isReassignable();
    void selectorWindowVisibility (bool isVisible_);
    void addChangeListenerToWindow (juce::ChangeListener* listener);
    void removeChangeListenerFromWindow (juce::ChangeListener* listener);

    //==============================================================================
    // Filterbank file chooser
    static juce::Array<juce::File> FilterbankFileLoader();

    class BLFilterbankDef
    {
    public:
        static BLFilterbankDef* createDefFromFile (juce::File& file,
            juce::int64 byteOffset = 0);
        static BLFilterbankDef* createDefFromData (juce::MemoryBlock& memBlock,
            juce::int64 byteOffset = 0);
        virtual ~BLFilterbankDef();

        const fftwf_complex** G;
        const int* Gl;
        const ltfat_int* foff;
        const int* realonly;
        const double* a;
        const double* fc;
        const ltfat_int* Lc;
        ltfat_int* Lchalf;
        const int M;
        const int L;

    private:
        static void getFilterbankBaseData (juce::MemoryInputStream* dataPtr,
            unsigned* blockLengthPtr,
            unsigned* mPtr);
        static void getFilterbankParamData (juce::MemoryInputStream* dataPtr,
            unsigned M,
            unsigned* aOne,
            unsigned a[],
            float fc[],
            unsigned foff[],
            unsigned filtLengths[]);
        static void getFilterbankFilterData (juce::MemoryInputStream* dataPtr,
            unsigned M,
            unsigned filtLengths[],
            float** G);

        // Make it non-copyable and non createable
        BLFilterbankDef (const fftwf_complex** G_,
            int* Gl_,
            ltfat_int* foff_,
            int* realonly_,
            double* a_,
            double* fc_,
            ltfat_int* Lc_,
            ltfat_int* Lchalf_,
            int M_,
            int L_) : G (G_), Gl (Gl_), foff (foff_), realonly (realonly_), a (a_), fc (fc_), Lc (Lc_), Lchalf (Lchalf_), M (M_), L (L_) {}

        BLFilterbankDef (const BLFilterbankDef& other); // non construction-copyable
        BLFilterbankDef& operator= (const BLFilterbankDef&); // non copyable
    };

private:
    void init (juce::Array<juce::File> loadedFilterbankFiles);

    class FilterbankSelectWindow : public juce::DialogWindow,
                                   public juce::ChangeBroadcaster,
                                   private juce::Button::Listener
    {
    public:
        FilterbankSelectWindow (juce::String title, juce::Array<unsigned>& blockLengths, int* fbIndexPtr);

        void closeButtonPressed() override;
        void buttonClicked (juce::Button* b) override;

    private:
        juce::ScopedPointer<juce::TextButton> confirmButton;
        juce::ScopedPointer<juce::Label> dialogText;
        int* activeFilterbank;

        juce::OwnedArray<juce::ToggleButton> fbDataButtons;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankSelectWindow)
    };

    juce::Array<juce::MemoryBlock> rawFilterbankData;
    juce::Array<juce::int64> startingBytes;
    juce::Array<unsigned> blockLengths;
    juce::ScopedPointer<FilterbankSelectWindow> fbWindow;

    bool reassignable;
    int numOfFilterbanks;
    int fbIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankDataHolder)
};
