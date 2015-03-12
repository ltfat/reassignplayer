#ifndef FILTERBANKDATAHOLDER_H_INCLUDED
#define FILTERBANKDATAHOLDER_H_INCLUDED

#include "../../plugin/JuceLibraryCode/JuceHeader.h"

#include "fftw3.h"
#include "ltfat.h"

class FilterbankDataHolder
{
public:
    FilterbankDataHolder();
    FilterbankDataHolder(Array<File> loadedFilterbankFiles);

    bool getFilterbankData(Array<MemoryBlock> &rawFilterbankData_);
    int64 getStartingByte(int fbIndex_);
    long getBlockLength(int fbIndex_);
    /*OwnedArray<MemoryBlock> rawFilterbankData;
    OwnedArray<unsigned long> startingBytes;
    OwnedArray<unsigned> blockLengths;*/
    int getActiveFilterbank();
    bool setActiveFilterbank(int fbIndex_);

    bool isReassignable();
    void selectorWindowVisibility(bool isVisible_);
    void addChangeListenerToWindow(ChangeListener* listener);
    void removeChangeListenerFromWindow(ChangeListener* listener);

    //==============================================================================
    // Filterbank file chooser
    static Array<File> FilterbankFileLoader();

    class BLFilterbankDef
    {
    public:
        static BLFilterbankDef* createDefFromFile(File& file,
                                                  int64 byteOffset = 0);
        static BLFilterbankDef* createDefFromData(MemoryBlock& memBlock,
                                                  int64 byteOffset = 0);
        virtual ~BLFilterbankDef();

        const fftwf_complex** G;
        const int*            Gl;
        const ltfatInt*       foff;
        const int*            realonly;
        const double*         a;
        const double*         fc;
        const ltfatInt*       Lc;
        const ltfatInt*       Lchalf;
        const int             M;
        const int             L;
    private:
        static void getFilterbankBaseData (MemoryInputStream* dataPtr,
                unsigned* blockLengthPtr,
                unsigned* mPtr);
        static void getFilterbankParamData (MemoryInputStream* dataPtr,
                unsigned M, unsigned* aOne,
                unsigned a[], float fc[],
                unsigned foff[], unsigned filtLengths[]);
        static void getFilterbankFilterData (MemoryInputStream* dataPtr,
                unsigned M, unsigned filtLengths[],
                float** G);

        // Make it non-copyable and non createable
        BLFilterbankDef(const fftwf_complex** G_,
                        int*                  Gl_,
                        ltfatInt*             foff_,
                        int*                  realonly_,
                        double*               a_,
                        double*               fc_,
                        ltfatInt*             Lc_,
                        ltfatInt*             Lchalf_,
                        int                   M_,
                        int                   L_):
            G(G_), Gl(Gl_), foff(foff_), realonly(realonly_), a(a_),
            fc(fc_), Lc(Lc_), Lchalf(Lchalf_), M(M_), L(L_) {}

        BLFilterbankDef( const BLFilterbankDef& other ); // non construction-copyable
        BLFilterbankDef& operator=( const BLFilterbankDef& ); // non copyable
    };

private:
    void init(Array<File> loadedFilterbankFiles);

    class FilterbankSelectWindow : public DialogWindow,
                                   public ChangeBroadcaster,
                                   private ButtonListener
    {
    public:
        FilterbankSelectWindow (String title, Array<unsigned>& blockLengths, int* fbIndexPtr);

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

    bool reassignable;
    int numOfFilterbanks;
    int fbIndex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterbankDataHolder)
};

/*class BLFilterbankDef
{
 public:
    static BLFilterbankDef* createDefFromFile(File& file,
                                              int64 byteOffset = 0);
    static BLFilterbankDef* createDefFromData(MemoryBlock& memBlock,
                                              int64 byteOffset = 0);
    virtual ~BLFilterbankDef();

    const fftwf_complex** G;
    const int*            Gl;
    const ltfatInt*       foff;
    const int*            realonly;
    const double*         a;
    const double*         fc;
    const ltfatInt*       Lc;
    const ltfatInt*       Lchalf;
    const int             M;
    const int             L;
private:
    static void getFilterbankBaseData (MemoryInputStream* dataPtr,
            unsigned* blockLengthPtr,
            unsigned* mPtr);
    static void getFilterbankParamData (MemoryInputStream* dataPtr,
            unsigned M, unsigned* aOne,
            unsigned a[], float fc[],
            unsigned foff[], unsigned filtLengths[]);
    static void getFilterbankFilterData (MemoryInputStream* dataPtr,
            unsigned M, unsigned filtLengths[],
            float** G);

    // Make it non-copyable and non createable
    BLFilterbankDef(const fftwf_complex** G_,
                    int*                  Gl_,
                    ltfatInt*             foff_,
                    int*                  realonly_,
                    double*               a_,
                    double*               fc_,
                    ltfatInt*             Lc_,
                    ltfatInt*             Lchalf_,
                    int                   M_,
                    int                   L_):
        G(G_), Gl(Gl_), foff(foff_), realonly(realonly_), a(a_),
        fc(fc_), Lc(Lc_), Lchalf(Lchalf_), M(M_), L(L_) {}

    BLFilterbankDef( const BLFilterbankDef& other ); // non construction-copyable
    BLFilterbankDef& operator=( const BLFilterbankDef& ); // non copyable
};*/

#endif // FILTERBANKDATAHOLDER_H_INCLUDED
