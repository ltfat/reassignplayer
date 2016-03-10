/*
  ==============================================================================

    ReassignedBLFilterbank.h
    Created: 7 Mar 2016 3:28:29pm
    Author:  susnak

  ==============================================================================
*/

#ifndef REASSIGNEDBLFILTERBANK_H_INCLUDED
#define REASSIGNEDBLFILTERBANK_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "ltfat.h"
#include "fftw3.h"
#include "OverlapFifo.h"
#include "FilterbankDataHolder.h"

#ifdef USETBB
#include "tbb/tbb.h"
using namespace tbb;
#endif

namespace SpectrogramPlottableMethods
{
template<class T>
void coefsToAbsMatrix(T* coefs[], ptrdiff_t Lc[], int M,
                      float* data, int stripWidth, int stripHeight);

}


class SpectrogramPlottable
{
public:
    SpectrogramPlottable() {};
    virtual ~SpectrogramPlottable() {};
    virtual bool getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows) = 0;
};



class ReassignedBLFilterbank: public SpectrogramPlottable
{
public:
    static ReassignedBLFilterbank* makeDefault();
    static ReassignedBLFilterbank* makeFromChooser();
    ReassignedBLFilterbank(FilterbankDataHolder::BLFilterbankDef* filterbankDefs_[3], int bufLen_);

    virtual ~ReassignedBLFilterbank();
    bool getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows ) override;

    void appendSamples(const float* samples, int samplesLen);

    void setActivePlotReassigned(bool doSet = true)
    {
        doPlotReassigned.set(doSet);
    };
    
    void toggleActivePlotReassigned()
    {
        doPlotReassigned.set(!doPlotReassigned.get());
    };
private:
    void performTransform() noexcept;

    ScopedPointer<OverlapFifo> overlapFifo;
    Atomic<int> doPlotReassigned;
    // Helper arrays
    float* buf;
    fftwf_complex* bufFFTCoefs;
    Array<convsub_fftbl_plan_s*> p;

    void createPlans();
    void destroyPlans();
    void createBuffers();
    void destroyBuffers();

    int bufLen;
    int hop;

    float* win;
    fftwf_plan* plan;
    float** reassignedCoefs;

    class WindowDriver
    {
    public:
        static double pi;
        static void hann(int L, float * const win);
        static void hannsqrt(int L, float * const win);

    };

    OwnedArray<FilterbankDataHolder::BLFilterbankDef> filterbanks;
    Array<fftwf_complex**> bufFilterbankOverlaidCoefs;
    Array<fftwf_complex**> bufFilterbankCoefs;
    float** tgrad;
    float** fgrad;
    float** cs;

};







#endif  // REASSIGNEDBLFILTERBANK_H_INCLUDED
