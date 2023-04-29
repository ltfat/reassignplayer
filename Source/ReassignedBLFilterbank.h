/*
  ==============================================================================

    ReassignedBLFilterbank.h
    Created: 7 Mar 2016 3:28:29pm
    Author:  susnak

  ==============================================================================
*/

#pragma once

#include "fftw3.h"
#include "ltfat.h"

#include "FilterbankDataHolder.h"
#include "OverlapFifo.h"

#ifdef USETBB
    #include "tbb/tbb.h"
using namespace tbb;
#endif

namespace SpectrogramPlottableMethods
{
    template <class T>
    void coefsToAbsMatrix (T* coefs[], ltfat_int Lc[], int M, float* data, int stripWidth, int stripHeight);

}

class SpectrogramPlottable
{
public:
    SpectrogramPlottable() {};
    virtual ~SpectrogramPlottable() {};
    virtual bool getBufferCoefficientsAsAbsMatrix (float* matrix, int cols, int rows) = 0;
};

class ReassignedBLFilterbank : public SpectrogramPlottable
{
public:
    static ReassignedBLFilterbank* makeDefault();
    static ReassignedBLFilterbank* makeFromChooser();
    ReassignedBLFilterbank (FilterbankDataHolder::BLFilterbankDef* filterbankDefs_[3], int bufLen_);

    virtual ~ReassignedBLFilterbank();
    bool getBufferCoefficientsAsAbsMatrix (float* matrix, int cols, int rows) override;

    void appendSamples (const float* samples, int samplesLen);

    void setActivePlotReassigned (bool doSet = true)
    {
        doPlotReassigned.set (doSet);
    };

    bool getActivePlotReassigned()
    {
        return static_cast<bool> (doPlotReassigned.get());
    };

    void toggleActivePlotReassigned()
    {
        doPlotReassigned.set (!doPlotReassigned.get());
    };

    int getBufLen() { return bufLen; }

private:
    void performTransform() noexcept;

    juce::ScopedPointer<OverlapFifo> overlapFifo;
    juce::Atomic<int> doPlotReassigned;
    // Helper arrays
    float* buf;
    fftwf_complex* bufFFTCoefs;
    juce::Array<ltfat_convsub_fftbl_plan_s*> p;

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
        static void hann (int L, float* const win);
        static void hannsqrt (int L, float* const win);
    };

    juce::OwnedArray<FilterbankDataHolder::BLFilterbankDef> filterbanks;
    juce::Array<ltfat_complex_s**> bufFilterbankOverlaidCoefs;
    juce::Array<ltfat_complex_s**> bufFilterbankCoefs;
    float** tgrad;
    float** fgrad;
    float** cs;
};
