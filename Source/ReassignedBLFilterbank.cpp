/*
  ==============================================================================

    ReassignedBLFilterbank.cpp
    Created: 7 Mar 2016 3:28:29pm
    Author:  susnak

  ==============================================================================
*/

#include "ReassignedBLFilterbank.h"
#include "BinaryData.h"
#include <complex>
#ifdef USETBB
    #include "tbb/tbb.h"
#endif

namespace SpectrogramPlottableMethods
{

    template <>
    void coefsToAbsMatrix<ltfat_complex_s> (ltfat_complex_s* coefs[], ltfat_int Lc[], int M, float* data, int stripWidth, int stripHeight)
    {
#define DATAEL(m, ii) (*(data + stripWidth * m + ii))
        if (M == 1)
        {
            float rowsRatio = static_cast<float> (Lc[0] - 1) / stripHeight;
            ltfat_complex_s* c = coefs[0];
            for (int m = 0; m < stripHeight; ++m)
            {
                float tmpyPrec = rowsRatio * m;
                int tmpy = static_cast<int> (tmpyPrec);
                tmpyPrec -= tmpy;
                tmpy = Lc[0] - 2 - tmpy;
                for (int ii = 0; ii < stripWidth; ++ii)
                {
                    DATAEL (m, ii) =
                        (1.0f - tmpyPrec) * std::abs (c[tmpy]) + tmpyPrec * std::abs (c[tmpy + 1]);
                }
            }
        }
        else
        {
            float rowsRatio = static_cast<float> (M - 1) / stripHeight;
            juce::HeapBlock<float> colsRatios (M);
            for (int m = 0; m < M; ++m)
                colsRatios[m] = static_cast<float> (Lc[m] - 1) / stripWidth;

            for (int m = 0; m < stripHeight; ++m)
            {
                float tmpyPrec = rowsRatio * m;
                int tmpy = static_cast<int> (tmpyPrec);
                tmpyPrec -= tmpy;
                // tmpy = M - 2 - tmpy;
                ltfat_complex_s* coefstmpy = coefs[tmpy];
                ltfat_complex_s* coefstmpyplus1 = coefs[tmpy + 1];

                for (int ii = 0; ii < stripWidth; ++ii)
                {
                    float tmpxPrec = colsRatios[tmpy] * ii;
                    int tmpx = static_cast<int> (tmpxPrec);
                    tmpxPrec -= tmpx;
                    // tmpx = Lc[tmpy] - 2 - tmpx;
                    float coefstmpytmpx = coefstmpy[tmpx].real();
                    float coefstmpytmpxplus1 = coefstmpy[tmpx + 1].imag();
                    float intx1 = (1.0f - tmpxPrec) * std::abs (coefstmpytmpx) + tmpxPrec * std::abs (coefstmpytmpxplus1);
                    float intx2 = (1.0f - tmpxPrec) * std::abs (coefstmpyplus1[tmpx]) + tmpxPrec * std::abs (coefstmpyplus1[tmpx + 1]);
                    DATAEL (m, ii) = (1.0f - tmpyPrec) * intx1 + tmpyPrec * intx2;
                }
            }
        }
#undef DATAEL
    }

    template <>
    void coefsToAbsMatrix<float> (float* coefs[], ltfat_int Lc[], int M, float* data, int stripWidth, int stripHeight)
    {
#define DATAEL(m, ii) (*(data + stripWidth * m + ii))

        // for (int ii = 0; ii < stripWidth; ii++)
        //     for (int m = 0; m < stripHeight; ++m)
        //         DATAEL(m, ii) = 0.0;
        //
        // // for (int ii = 0; ii < stripWidth; ii++)
        // //         DATAEL(50, ii) = 1.0;
        // //
        // // return;

        if (M == 1)
        {
            float rowsRatio = static_cast<float> (Lc[0] - 1) / stripHeight;
            float* c = coefs[0];
            for (int m = 0; m < stripHeight; ++m)
            {
                float tmpyPrec = rowsRatio * m;
                int tmpy = static_cast<int> (tmpyPrec);
                tmpyPrec -= tmpy;

                for (int ii = 0; ii < stripWidth; ++ii)
                {
                    DATAEL (m, ii) =
                        (1.0f - tmpyPrec) * (c[tmpy]) + tmpyPrec * (c[tmpy + 1]);
                }
            }
        }
        else
        {
            float rowsRatio = static_cast<float> (M - 1) / stripHeight;
            juce::HeapBlock<float> colsRatios (M);
            for (int m = 0; m < M; ++m)
                colsRatios[m] = static_cast<float> (Lc[m] - 1) / stripWidth;

            for (int m = 0; m < stripHeight; ++m)
            {
                float tmpyPrec = rowsRatio * m;
                int tmpy = static_cast<int> (tmpyPrec);
                tmpyPrec -= tmpy;
                for (int ii = 0; ii < stripWidth; ++ii)
                {
                    float tmpxPrec = colsRatios[tmpy] * ii;
                    int tmpx = static_cast<int> (tmpxPrec);
                    tmpxPrec -= tmpx;
                    float intx1 = (1.0f - tmpxPrec) * (coefs[tmpy][tmpx]) + tmpxPrec * (coefs[tmpy][tmpx + 1]);
                    float intx2 = (1.0f - tmpxPrec) * (coefs[tmpy + 1][tmpx]) + tmpxPrec * (coefs[tmpy + 1][tmpx + 1]);
                    DATAEL (m, ii) = (1.0f - tmpyPrec) * intx1 + tmpyPrec * intx2;
                }
            }
        }
#undef DATAEL
    }
} // namespace SpectrogramPlottableMethods

ReassignedBLFilterbank::ReassignedBLFilterbank (
    FilterbankDataHolder::BLFilterbankDef* filterbankDefs_[3],
    int bufLen_)
    : bufLen (bufLen_), hop (bufLen_ / 2)
{
    overlapFifo = new OverlapFifo (20 * bufLen, hop, bufLen);
    setActivePlotReassigned (false);
    for (int ii = 0; ii < 3; ++ii)
        filterbanks.add (filterbankDefs_[ii]);
    createBuffers();
    createPlans();
}

ReassignedBLFilterbank* ReassignedBLFilterbank::makeDefault()
{
    int resLen = 0;
    const char* fb = BinaryData::getNamedResource ("default_lfb", resLen);
    juce::MemoryBlock fbMemBlock (fb, resLen);
    const char* fbFgrad =
        BinaryData::getNamedResource ("default_fgrad_lfb", resLen);
    juce::MemoryBlock fbFgradMemBlock (fbFgrad, resLen);
    const char* fbTgrad =
        BinaryData::getNamedResource ("default_tgrad_lfb", resLen);
    juce::MemoryBlock fbTgradMemBlock (fbTgrad, resLen);

    juce::Array<FilterbankDataHolder::BLFilterbankDef*> filterbankDefs;

    filterbankDefs.add (
        FilterbankDataHolder::BLFilterbankDef ::createDefFromData (fbMemBlock, 0));
    filterbankDefs.add (FilterbankDataHolder::BLFilterbankDef ::createDefFromData (
        fbFgradMemBlock, 0));
    filterbankDefs.add (FilterbankDataHolder::BLFilterbankDef ::createDefFromData (
        fbTgradMemBlock, 0));

    int bufLen = filterbankDefs.getFirst()->L;
    return new ReassignedBLFilterbank (filterbankDefs.getRawDataPointer(), bufLen);
}

ReassignedBLFilterbank* ReassignedBLFilterbank::makeFromChooser()
{
    juce::Array<juce::File> loadedFilterbankFiles =
        FilterbankDataHolder::FilterbankFileLoader();

    if (loadedFilterbankFiles.size() != 3)
        return nullptr;

    FilterbankDataHolder dataHolder (loadedFilterbankFiles);

    juce::Array<juce::MemoryBlock> rawFbData;
    dataHolder.getFilterbankData (rawFbData);
    juce::Array<FilterbankDataHolder::BLFilterbankDef*> filterbankDefs;

    filterbankDefs.add (FilterbankDataHolder::BLFilterbankDef ::createDefFromData (
        rawFbData.getReference (0), 0));
    filterbankDefs.add (FilterbankDataHolder::BLFilterbankDef ::createDefFromData (
        rawFbData.getReference (1), 0));
    filterbankDefs.add (FilterbankDataHolder::BLFilterbankDef ::createDefFromData (
        rawFbData.getReference (2), 0));

    int bufLen = filterbankDefs.getFirst()->L;
    return new ReassignedBLFilterbank (filterbankDefs.getRawDataPointer(), bufLen);
}

ReassignedBLFilterbank::~ReassignedBLFilterbank()
{
    destroyBuffers();
    destroyPlans();
    overlapFifo = nullptr;
}

bool ReassignedBLFilterbank::getBufferCoefficientsAsAbsMatrix (float* matrix,
    int cols,
    int rows)
{
    bool retval = overlapFifo->overlapReadFromFifo (buf);

    // There is not enough samples in the fifo
    if (!retval)
        return false;

    // ... there is enough samples! Transform it!
    performTransform();

    if (doPlotReassigned.get())
        SpectrogramPlottableMethods::coefsToAbsMatrix (
            reassignedCoefs, const_cast<ltfat_int*> (filterbanks[0]->Lchalf), filterbanks[0]->M, matrix, cols, rows);
    else
        SpectrogramPlottableMethods::coefsToAbsMatrix (
            bufFilterbankOverlaidCoefs[0],
            const_cast<ltfat_int*> (filterbanks[0]->Lchalf),
            filterbanks[0]->M,
            matrix,
            cols,
            rows);

    // Store overlay for the next call
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        ltfat_complex_s** C = bufFilterbankCoefs.getUnchecked (ii);
        FilterbankDataHolder::BLFilterbankDef* blFilt =
            filterbanks.getUnchecked (ii);
        ltfat_complex_s** overlayback = bufFilterbankOverlaidCoefs.getUnchecked (ii);
        for (int m = 0; m < blFilt->M; ++m)
        {
            memcpy (overlayback[m], C[m] + blFilt->Lchalf[m], blFilt->Lchalf[m] * sizeof (fftwf_complex));
            // memset(overlayback[m], 0, blFilt->Lchalf[m] * sizeof(fftwf_complex));
        }
    }

    return true;
}

void ReassignedBLFilterbank::performTransform() noexcept
{
    for (int ii = 0; ii < bufLen; ++ii)
    {
        bufFFTCoefs[ii][0] = buf[ii] * win[ii];
        bufFFTCoefs[ii][1] = 0.0f;
    }
    fftwf_execute_dft (*plan, bufFFTCoefs, bufFFTCoefs);

#ifdef USETBB
    parallel_for (
        blocked_range<size_t> (0, filterbanks.size(), 1),
        [=] (const blocked_range<size_t>& rbig) {
            for (size_t ii = rbig.begin(); ii != rbig.end(); ++ii)
            {
                fftwf_complex** C = bufFilterbankCoefs.getUnchecked (ii);
                FilterbankDataHolder::BLFilterbankDef* blFilt =
                    filterbanks.getUnchecked (ii);

                parallel_for (blocked_range<size_t> (0, blFilt->M, 10),
                    [=] (const blocked_range<size_t>& r) {
                        for (size_t i = r.begin(); i != r.end(); ++i)
                        {
                            convsub_fftbl_execute_s (
                                p[ii][i],
                                const_cast<const fftwf_complex*> (bufFFTCoefs),
                                const_cast<const fftwf_complex*> (blFilt->G[i]),
                                blFilt->foff[i],
                                0,
                                C[i]);
                        }
                    });

                // Do the overlays
                fftwf_complex** overlayfront =
                    bufFilterbankOverlaidCoefs.getUnchecked (ii);

                parallel_for (blocked_range<size_t> (0, blFilt->M, 10),
                    [=] (const blocked_range<size_t>& r) {
                        for (size_t m = r.begin(); m != r.end(); ++m)
                        {
                            fftwf_complex* overlayfrontTmp = overlayfront[m];
                            fftwf_complex* CTmp = C[m];
                            for (int ii = 0; ii < blFilt->Lchalf[m]; ++ii)
                            {
                                overlayfrontTmp[ii][0] += CTmp[ii][0];
                                overlayfrontTmp[ii][1] += CTmp[ii][1];
                            }
                        }
                    });
            }
        });
#else
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        ltfat_complex_s** C = bufFilterbankCoefs.getUnchecked (ii);
        FilterbankDataHolder::BLFilterbankDef* blFilt =
            filterbanks.getUnchecked (ii);

        for (int i = 0; i < blFilt->M; ++i)
        {
            ltfat_convsub_fftbl_execute_s (
                p[ii][i], reinterpret_cast<const ltfat_complex_s*> (bufFFTCoefs), reinterpret_cast<const ltfat_complex_s*> (blFilt->G[i]), blFilt->foff[i], 0, reinterpret_cast<ltfat_complex_s*> (C[i]));
        }

        // Do the overlays
        ltfat_complex_s** overlayfront =
            bufFilterbankOverlaidCoefs.getUnchecked (ii);

        for (int m = 0; m < blFilt->M; ++m)
        {
            ltfat_complex_s* overlayfrontTmp = overlayfront[m];
            ltfat_complex_s* CTmp = C[m];
            for (int ii = 0; ii < blFilt->Lchalf[m]; ++ii)
            {
                overlayfrontTmp[ii] += CTmp[ii];
            }
        }
    }
#endif

    FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getFirst();
    const int M = blFilt->M;
    float minlvl = 1e-7f;
    auto** c = const_cast<const ltfat_complex_s**> (
        bufFilterbankOverlaidCoefs.getUnchecked (0));
    auto** ch = const_cast<const ltfat_complex_s**> (
        bufFilterbankOverlaidCoefs.getUnchecked (1));
    auto** cd = const_cast<const ltfat_complex_s**> (
        bufFilterbankOverlaidCoefs.getUnchecked (2));

    ltfat_filterbankphasegrad_s (c, ch, cd, M, blFilt->Lchalf, bufLen, minlvl, tgrad, fgrad, cs);

    ltfat_filterbankreassign_s (
        const_cast<const float**> (cs), const_cast<const float**> (tgrad), const_cast<const float**> (fgrad), blFilt->Lchalf, blFilt->a, blFilt->fc, M, reassignedCoefs, REASS_NOTIMEWRAPAROUND, NULL);
}

void ReassignedBLFilterbank::appendSamples (const float* samples,
    int samplesLen)
{
    overlapFifo->addToFifo (samples, samplesLen);
}

void ReassignedBLFilterbank::createBuffers()
{
    // Samples buffer
    size_t bufLenInBytes = bufLen * sizeof (float);
    buf = static_cast<float*> (fftwf_malloc (bufLenInBytes));
    memset (buf, 0, bufLenInBytes);
    win = static_cast<float*> (fftwf_malloc (bufLenInBytes));
    WindowDriver::hann (bufLen, win);

    // FFT buffer
    size_t cmplxBufLenInBytes = bufLen * sizeof (ltfat_complex_s);
    bufFFTCoefs = static_cast<fftwf_complex*> (fftwf_malloc (cmplxBufLenInBytes));
    memset (bufFFTCoefs, 0, cmplxBufLenInBytes);

    // Filterbanks coefficient buffers
    for (FilterbankDataHolder::BLFilterbankDef* blFilt : filterbanks)
    {
        ltfat_complex_s** bufEl = static_cast<ltfat_complex_s**> (
            fftwf_malloc (blFilt->M * sizeof (ltfat_complex_s*)));
        ltfat_complex_s** bufEl2 = static_cast<ltfat_complex_s**> (
            fftwf_malloc (blFilt->M * sizeof (ltfat_complex_s*)));
        for (int m = 0; m < blFilt->M; ++m)
        {
            bufEl[m] = static_cast<ltfat_complex_s*> (
                fftwf_malloc (blFilt->Lc[m] * sizeof (ltfat_complex_s)));
            bufEl2[m] = static_cast<ltfat_complex_s*> (
                fftwf_malloc (sizeof (ltfat_complex_s) * blFilt->Lc[m] / 2));
            memset (bufEl[m], 0, blFilt->Lc[m] * sizeof (fftwf_complex));
            memset (bufEl2[m], 0, blFilt->Lc[m] * sizeof (fftwf_complex) / 2);
        }
        bufFilterbankCoefs.add (bufEl);
        bufFilterbankOverlaidCoefs.add (bufEl2);
    }

    // Reassigned coefs buffers
    FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getFirst();
    int M = blFilt->M;
    const ltfat_int* Lchalf = blFilt->Lchalf;
    reassignedCoefs = static_cast<float**> (fftwf_malloc (M * sizeof (float*)));
    tgrad = static_cast<float**> (fftwf_malloc (M * sizeof (float*)));
    fgrad = static_cast<float**> (fftwf_malloc (M * sizeof (float*)));
    cs = static_cast<float**> (fftwf_malloc (M * sizeof (float*)));

    for (int m = 0; m < blFilt->M; ++m)
    {
        size_t LchalfinBytes = Lchalf[m] * sizeof (float);
        reassignedCoefs[m] = static_cast<float*> (fftwf_malloc (LchalfinBytes));
        tgrad[m] = static_cast<float*> (fftwf_malloc (LchalfinBytes));
        fgrad[m] = static_cast<float*> (fftwf_malloc (LchalfinBytes));
        cs[m] = static_cast<float*> (fftwf_malloc (LchalfinBytes));
    }
}

void ReassignedBLFilterbank::createPlans()
{
    // FFT plan
    plan = static_cast<fftwf_plan*> (malloc (sizeof (fftw_plan)));
    fftw_iodim fftw_dims[1];
    fftw_iodim howmanydims[1];

    fftw_dims[0].n = bufLen;
    fftw_dims[0].is = 1;
    fftw_dims[0].os = 1;

    howmanydims[0].n = 1;
    howmanydims[0].is = bufLen;
    howmanydims[0].os = bufLen;

    *plan = fftwf_plan_guru_dft (1, fftw_dims, 1, howmanydims, bufFFTCoefs, bufFFTCoefs, FFTW_FORWARD, FFTW_MEASURE);

    // Clear the buffer since the plan creation might have written something to it
    size_t bufLenInBytes = bufLen * sizeof (fftwf_complex);
    memset (bufFFTCoefs, 0, bufLenInBytes);

    // Filterbank plans
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        // Actual filterbank
        FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks[ii];
        // Each filterbank has M plans, allocate space for M pointers
        auto* pEl = static_cast<ltfat_convsub_fftbl_plan_s*> (
            fftwf_malloc (blFilt->M * sizeof (ltfat_convsub_fftbl_plan_s)));
        // Add to array of plans
        p.add (pEl);
        // Temp array to be used during planning
        ltfat_complex_s** bf = bufFilterbankCoefs.getUnchecked (ii);
        for (int m = 0; m < blFilt->M; ++m)
        {
            // DBG("Plannning bufLen " << bufLen << " Gl " << blFilt->Gl[m] << "
            // nChannels " << nChannels << " a " << blFilt->a[m]  );
            //  Plan m-th filter
            p.getLast()[m] = ltfat_convsub_fftbl_init_s (bufLen, blFilt->Gl[m], 1, blFilt->a[m], bf[m]);
            // DBG("After planning");
        }
    }
}

void ReassignedBLFilterbank::destroyBuffers()
{
    // // Destroy sample buffer
    if (nullptr != buf)
        fftwf_free (buf);
    // // Destroy FFT buffer
    if (nullptr != bufFFTCoefs)
        fftwf_free (bufFFTCoefs);
    if (nullptr != win)
        fftwf_free (win);

    // Destroy filterbank buffers
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks[ii];
        for (int m = 0; m < blFilt->M; ++m)
        {
            fftwf_free (bufFilterbankCoefs[ii][m]);
            fftwf_free (bufFilterbankOverlaidCoefs[ii][m]);
        }
        fftwf_free (bufFilterbankCoefs[ii]);
        fftwf_free (bufFilterbankOverlaidCoefs[ii]);
    }
    bufFilterbankCoefs.clear();
    bufFilterbankOverlaidCoefs.clear();

    // Destroy reassigned buffer
    FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getFirst();

    int M = blFilt->M;

    for (int m = 0; m < M; ++m)
    {
        fftwf_free (reassignedCoefs[m]);
        fftwf_free (tgrad[m]);
        fftwf_free (fgrad[m]);
        fftwf_free (cs[m]);
    }
    fftwf_free (reassignedCoefs);
    fftwf_free (tgrad);
    fftwf_free (fgrad);
    fftwf_free (cs);
}

void ReassignedBLFilterbank::destroyPlans()
{
    // Clear fft plan
    if (plan != nullptr)
    {
        fftwf_destroy_plan (*plan);
        free (plan);
        plan = nullptr;
    }

    // Clear filterbank plans
    for (int ii = 0; ii < p.size(); ++ii)
    {
        ltfat_convsub_fftbl_plan_s* pEl = p[ii];
        for (int m = 0; m < filterbanks[ii]->M; ++m)
        {
            ltfat_convsub_fftbl_done_s (pEl[m]);
        }
        fftwf_free (pEl);
    }
    p.clear();
}

void ReassignedBLFilterbank::WindowDriver::hann (int L, float* const win)
{
    double x = -0.5;
    double step = 1.0 / L;
    for (int ii = 0; ii < L; ++ii)
    {
        win[ii] = (0.5f + 0.5f * static_cast<float> (std::cos (2.0 * pi * x)));
        x += step;
    }
}

void ReassignedBLFilterbank::WindowDriver::hannsqrt (int L, float* const win)
{
    hann (L, win);
    for (int ii = 0; ii < L; ++ii)
    {
        win[ii] = std::sqrt (win[ii]);
    }
}
double ReassignedBLFilterbank::WindowDriver::pi = std::acos (-1);
