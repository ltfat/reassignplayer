/*
LLast year, the became
LLast year, the became
LLast year, the became
LLast year, the became
LLast year, the became
  ==============================================================================

    RingTransformBuffer.h
    Created: 18 Feb 2015 12:28:54pm
    Author:  susnak

  ==============================================================================
*/

#ifndef RINGTRANSFORMBUFFER_H_INCLUDED
#define RINGTRANSFORMBUFFER_H_INCLUDED

#ifdef STANDALONE
#define JUCE_DONT_DECLARE_PROJECTINFO 1
#endif

#include "../../plugin/JuceLibraryCode/JuceHeader.h"

#ifdef STANDALONE
#undef JUCE_DONT_DECLARE_PROJECTINFO
#endif

#include <complex>
#include "fftw3.h"
#include "ltfat.h"
#include <type_traits>
#include <cstring>
#include <fstream>
#include <tbb/tbb.h>
using namespace tbb;

namespace SpectrogramPlottableMethods
{
template<class T>
void coefsToAbsMatrix(T* coefs[], ltfatInt Lc[], int M,
                      float* data, int stripWidth, int stripHeight);

}


class SpectrogramPlottable
{
public:
    SpectrogramPlottable() {};
    virtual ~SpectrogramPlottable() {};
    virtual bool getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows) = 0;
};

class RingTransformBuffer
{
public:
    enum bufLenChooser
    {
        b512 =  512,
        b1024 = 1024,
        b2048 = 2048,
        b3072 = 3072,
        b4096 = 4096,
        emptyBufLen = 0
    };

    RingTransformBuffer(int bufLen_, int nChannels_ = 1, int nBuf_ = 2);
    virtual ~RingTransformBuffer();

    void appendSamples(const float* samples[], int samplesLen);
    const float* getBuffer();
    int getBufLen()
    {
        return bufLen;
    }
    bool isEmpty();
    bool isFull();

    virtual void performTransform() {};
protected:

    int bufLen;
    int bufLenHalf;
    int nChannels;
    int nBuf;
    Array<float*> buf;
    // Index of an active buffer
    int head;
    // Index of the buffer to be read
    int tail;
    // Write position in the active buffer
    int pos;
    //
    const bool transformOnRead;
private:
    void createBuffers();
    void destroyBuffers();
};

class RingFFTBuffer: public RingTransformBuffer,
    public SpectrogramPlottable
{
public:
    enum winType
    {
        hann,
        sqrthann,
        rect,
        noWinType
    };
    RingFFTBuffer(int bufLen_, winType winType_ = winType::hann,
                  int nChannels_ = 1, int nBuf_ = 2);
    virtual ~RingFFTBuffer();
    virtual void performTransform() noexcept override;
    virtual bool getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows) override;
    fftwf_complex* getFFTCoefficients(bool doConsume = true);
protected:
    void createPlan();
    void destroyPlan();
    void createFFTBuffers();
    void destroyFFTBuffers();

    Array<fftwf_complex*> bufFFTCoefs;
private:
    fftwf_plan* plan;
    HeapBlock<float> win;
    class WindowDriver
    {
    public:
        static double pi;
        static void hann(int L, float * const win);
        static void hannsqrt(int L, float * const win);

    };
};

class RingBLFilterbankBuffer: public RingFFTBuffer
{
protected:
    class BLFilterbankDef
    {
    public:
        static BLFilterbankDef* createDefFromFile(File& f);
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
        static void getFilterbankBaseData (std::ifstream* dataFilePtr,
                unsigned* blockLengthPtr,
                unsigned* mPtr);
        static void getFilterbankParamData (std::ifstream* dataFilePtr,
                unsigned M, unsigned* aOne,
                unsigned a[], float fc[],
                unsigned foff[], unsigned filtLengths[]);
        static void getFilterbankFilterData (std::ifstream* dataFilePtr,
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

public:

    RingBLFilterbankBuffer(File filterbankFile_, int bufLen_,
                           winType winType_ = winType::hann,
                           int nChannels_ = 1, int nBuf_ = 2);
    RingBLFilterbankBuffer(Array<File> filterbankFiles_, int bufLen_,
                           winType winType_ = winType::hann,
                           int nChannels_ = 1, int nBuf_ = 2);
    /*RingBLFilterbankBuffer(Array<BLFilterbankDef*> filterbankFiles_, int bufLen_,
                           winType winType_ = winType::hann,
                           int nChannels_ = 1, int nBuf_ = 2);*/
    virtual ~RingBLFilterbankBuffer();
    void performTransform() noexcept override;
    virtual bool getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows ) override;
    fftwf_complex** getFilterbankCoefficients(int filterbankId = 0, bool doConsume = true);
    fftwf_complex** getFilterbankOverlaidCoefficients(int filterbankId = 0, bool doConsume = true);

    void setActivePlotFilterbank(int filterbankId, bool plotOverlaiedCoefficients_ = true )
    {
        jassert(filterbankId >= 0 && filterbankId < filterbanks.size());
        // Atomic operations
        activePlottingFilterbank = filterbankId;
        plotOverlaiedCoefficients = plotOverlaiedCoefficients_;
    }

protected:
    OwnedArray<BLFilterbankDef> filterbanks;
    OwnedArray<Array<fftwf_complex**>> bufFilterbankOverlaidCoefs;
    OwnedArray<Array<fftwf_complex**>> bufFilterbankCoefs;
    fftwf_complex** dummy;


private:
    Atomic<int> activePlottingFilterbank;
    Atomic<int> plotOverlaiedCoefficients;

    void readFilterbankDefinitions();
    void createFilterbankPlan();
    void destroyFilterbankPlan();
    void createFilterbankBuffers();
    void destroyFilterbankBuffers();

    // Array of FB definitions
    //
    Array<convsub_fftbl_plan_s*> p;
    Array<File> filterbankFiles;

};

class RingReassignedBLFilterbankBuffer: public RingBLFilterbankBuffer
{
public:
    RingReassignedBLFilterbankBuffer(File filterbankFiles_[3], int bufLen_,
                                     winType winType_ = winType::hann,
                                     int nChannels_ = 1, int nBuf_ = 2);
    virtual ~RingReassignedBLFilterbankBuffer();
    virtual bool getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows ) override;
    float** getReassignedCoefficients(bool doConsume = true);
    void setActivePlotReassigned(bool doSet = true)
    {
        doPlotReassigned = doSet;
    };
    void performTransform() noexcept override;
private:
    Atomic<int> doPlotReassigned;
    Array<float**> reassignedCoefs;
    // Helper arrays
    float** tgrad;
    float** fgrad;
    float** cs;
    ltfatInt* Lchalf;
    double* cFreq;

    void createReassignPlan();
    void destroyReassignPlan();
    void createReassignBuffers();
    void destroyReassignBuffers();
};



#endif  // RINGTRANSFORMBUFFER_H_INCLUDED
