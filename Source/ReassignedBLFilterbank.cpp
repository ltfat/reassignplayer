/*
  ==============================================================================

    ReassignedBLFilterbank.cpp
    Created: 7 Mar 2016 3:28:29pm
    Author:  susnak

  ==============================================================================
*/

#include "ReassignedBLFilterbank.h"
#define ABS(c) std::sqrt((c)[0]*(c)[0] + (c)[1]*(c)[1])


namespace SpectrogramPlottableMethods
{

template<>
void coefsToAbsMatrix<fftwf_complex>(fftwf_complex* coefs[], ltfatInt Lc[], int M,
                                     float* data, int stripWidth, int stripHeight)
{
#define DATAEL(m,ii) (*(data + stripWidth*m + ii))
    if (M == 1)
    {
        float rowsRatio = static_cast<float>(Lc[0] - 1) / stripHeight;
        fftwf_complex* c = coefs[0];
        for (int m = 0; m < stripHeight; ++m)
        {
            float tmpyPrec = rowsRatio * m;
            int tmpy = static_cast<int>(tmpyPrec);
            tmpyPrec -= tmpy;
            tmpy = Lc[0] - 2 - tmpy;
            for (int ii = 0; ii < stripWidth; ++ii)
            {
                DATAEL(m, ii) =  (1.0f - tmpyPrec) * ABS(c[tmpy]) + tmpyPrec * ABS(c[tmpy + 1]);
            }
        }
    }
    else
    {
        float rowsRatio = static_cast<float>(M - 1) / stripHeight;
        HeapBlock<float> colsRatios(M);
        for (int m = 0; m < M; ++m)
            colsRatios[m] = static_cast<float>(Lc[m] - 1) / stripWidth;

        for (int m = 0; m < stripHeight; ++m)
        {
            float tmpyPrec = rowsRatio * m;
            int tmpy = static_cast<int>(tmpyPrec);
            tmpyPrec -= tmpy;
            //tmpy = M - 2 - tmpy;
            fftwf_complex* coefstmpy = coefs[tmpy];
            fftwf_complex* coefstmpyplus1 = coefs[tmpy + 1];

            for (int ii = 0; ii < stripWidth; ++ii)
            {
                float tmpxPrec = colsRatios[tmpy] * ii;
                int tmpx = static_cast<int>(tmpxPrec);
                tmpxPrec -= tmpx;
                // tmpx = Lc[tmpy] - 2 - tmpx;
                float* coefstmpytmpx = coefstmpy[tmpx];
                float* coefstmpytmpxplus1 = coefstmpy[tmpx + 1];
                float intx1 =  (1.0f - tmpxPrec) * ABS(coefstmpytmpx) + tmpxPrec * ABS( coefstmpytmpxplus1);
                float intx2 =  (1.0f - tmpxPrec) * ABS(coefstmpyplus1[tmpx]) + tmpxPrec * ABS(coefstmpyplus1[tmpx + 1]);
                DATAEL(m, ii) = (1.0f - tmpyPrec) * intx1 + tmpyPrec * intx2;
            }

        }

    }
#undef DATAEL
}

template<>
void coefsToAbsMatrix<float>(float* coefs[], ltfatInt Lc[], int M,
                             float* data, int stripWidth, int stripHeight)
{
#define DATAEL(m,ii) (*(data + stripWidth*m + ii))

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
        float rowsRatio = static_cast<float>(Lc[0] - 1) / stripHeight;
        float* c = coefs[0];
        for (int m = 0; m < stripHeight; ++m)
        {
            float tmpyPrec = rowsRatio * m;
            int tmpy = static_cast<int>(tmpyPrec);
            tmpyPrec -= tmpy;

            for (int ii = 0; ii < stripWidth; ++ii)
            {
                DATAEL(m, ii) =  (1.0f - tmpyPrec) * (c[tmpy]) + tmpyPrec * (c[tmpy + 1]);
            }
        }
    }
    else
    {
        float rowsRatio = static_cast<float>(M - 1) / stripHeight;
        HeapBlock<float> colsRatios(M);
        for (int m = 0; m < M; ++m)
            colsRatios[m] = static_cast<float>(Lc[m] - 1) / stripWidth;

        for (int m = 0; m < stripHeight; ++m)
        {
            float tmpyPrec = rowsRatio * m;
            int tmpy = static_cast<int>(tmpyPrec);
            tmpyPrec -= tmpy;
            for (int ii = 0; ii < stripWidth; ++ii)
            {
                float tmpxPrec = colsRatios[tmpy] * ii;
                int tmpx = static_cast<int>(tmpxPrec);
                tmpxPrec -= tmpx;
                float intx1 =  (1.0f - tmpxPrec) * (coefs[tmpy][tmpx]) + tmpxPrec * (coefs[tmpy][tmpx + 1]);
                float intx2 =  (1.0f - tmpxPrec) * (coefs[tmpy + 1][tmpx]) + tmpxPrec * (coefs[tmpy + 1][tmpx + 1]);
                DATAEL(m, ii) = (1.0f - tmpyPrec) * intx1 + tmpyPrec * intx2;
            }
        }

    }
#undef DATAEL
}
} // END NAMESPACE


ReassignedBLFilterbank::
ReassignedBLFilterbank(FilterbankDataHolder::BLFilterbankDef* filterbankDefs_[3], int bufLen_):
    bufLen(bufLen_), hop(bufLen_ / 2)
{
    overlapFifo = new OverlapFifo(20 * bufLen, hop, bufLen);
    setActivePlotReassigned(false);
    for (int ii = 0; ii < 3; ++ii) filterbanks.add(filterbankDefs_[ii]);
    createBuffers();
    createPlans();
}

ReassignedBLFilterbank*
ReassignedBLFilterbank::
makeDefault()
{
    int resLen = 0;
    const char* fb = BinaryData::getNamedResource("default_lfb", resLen);
    MemoryBlock fbMemBlock(fb, resLen);
    const char* fbFgrad = BinaryData::getNamedResource("default_fgrad_lfb", resLen);
    MemoryBlock fbFgradMemBlock(fbFgrad, resLen);
    const char* fbTgrad = BinaryData::getNamedResource("default_tgrad_lfb", resLen);
    MemoryBlock fbTgradMemBlock(fbTgrad, resLen);

    Array<FilterbankDataHolder::BLFilterbankDef*> filterbankDefs;

    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(fbMemBlock, 0));
    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(fbFgradMemBlock, 0));
    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(fbTgradMemBlock, 0));

    int bufLen = filterbankDefs.getFirst()->L;
    return new ReassignedBLFilterbank(filterbankDefs.getRawDataPointer(), bufLen);
}

ReassignedBLFilterbank*
ReassignedBLFilterbank::
makeFromChooser()
{
    Array<File> loadedFilterbankFiles = FilterbankDataHolder::FilterbankFileLoader();
    
    if(loadedFilterbankFiles.size()!=3)
        return nullptr;

    FilterbankDataHolder dataHolder(loadedFilterbankFiles);

    Array<MemoryBlock> rawFbData;
    dataHolder.getFilterbankData(rawFbData);
    Array<FilterbankDataHolder::BLFilterbankDef*> filterbankDefs;

    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(rawFbData.getReference(0), 0));
    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(rawFbData.getReference(1), 0));
    filterbankDefs.add(FilterbankDataHolder::BLFilterbankDef
                       ::createDefFromData(rawFbData.getReference(2), 0));

    int bufLen = filterbankDefs.getFirst()->L;
    return new ReassignedBLFilterbank(filterbankDefs.getRawDataPointer(), bufLen);
}


ReassignedBLFilterbank::
~ReassignedBLFilterbank()
{
    destroyBuffers();
    destroyPlans();
    overlapFifo = nullptr;
}

bool ReassignedBLFilterbank::
getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows )
{
    bool retval = overlapFifo->overlapReadFromFifo(buf);

    // There is not enough samples in the fifo
    if (!retval)
        return false;

    // ... there is enough samples! Transform it!
    performTransform();

    if (doPlotReassigned.get())
        SpectrogramPlottableMethods::coefsToAbsMatrix(reassignedCoefs,
                const_cast<ltfatInt*>(filterbanks[0]->Lchalf),
                filterbanks[0]->M,
                matrix, cols, rows);
    else
        SpectrogramPlottableMethods::coefsToAbsMatrix(bufFilterbankOverlaidCoefs[0],
                const_cast<ltfatInt*>(filterbanks[0]->Lchalf),
                filterbanks[0]->M,
                matrix, cols, rows);

    // Store overlay for the next call
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        fftwf_complex** C = bufFilterbankCoefs.getUnchecked(ii);
        FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getUnchecked(ii);
        fftwf_complex** overlayback = bufFilterbankOverlaidCoefs.getUnchecked(ii);
        for (int m = 0; m < blFilt->M; ++m)
        {
            memcpy(overlayback[m], C[m] + blFilt->Lchalf[m], blFilt->Lchalf[m] * sizeof(fftwf_complex));
            //memset(overlayback[m], 0, blFilt->Lchalf[m] * sizeof(fftwf_complex));
        }
    }

    return true;
}

void
ReassignedBLFilterbank::
performTransform() noexcept
{
    for (int ii = 0; ii < bufLen; ++ii)
    {
        bufFFTCoefs[ii][0] = buf[ii] * win[ii];
        bufFFTCoefs[ii][1] = 0.0f;
    }
    fftwf_execute_dft(*plan, bufFFTCoefs, bufFFTCoefs);

    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        fftwf_complex** C = bufFilterbankCoefs.getUnchecked(ii);
        FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getUnchecked(ii);

        for (int i = 0; i < blFilt->M; ++i)
        {
            convsub_fftbl_execute_s(p[ii][i],
                                    const_cast<const fftwf_complex *>(bufFFTCoefs),
                                    const_cast<const fftwf_complex *>(blFilt->G[i]),
                                    blFilt->foff[i], 0, C[i]);
        }

        // Do the overlays
        fftwf_complex** overlayfront = bufFilterbankOverlaidCoefs.getUnchecked(ii);

        for (int m = 0; m < blFilt->M; ++m)
        {
            fftwf_complex* overlayfrontTmp = overlayfront[m];
            fftwf_complex* CTmp = C[m];
            for (int ii = 0; ii < blFilt->Lchalf[m]; ++ii)
            {
                overlayfrontTmp[ii][0] += CTmp[ii][0];
                overlayfrontTmp[ii][1] += CTmp[ii][1];
            }
        }
    }

    FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getFirst();
    const int M = blFilt->M;
    float minlvl = 1e-7f;
    fftwf_complex** c = bufFilterbankOverlaidCoefs.getUnchecked(0);
    fftwf_complex** ch = bufFilterbankOverlaidCoefs.getUnchecked(1);
    fftwf_complex** cd = bufFilterbankOverlaidCoefs.getUnchecked(2);

    filterbankphasegrad_s(const_cast<const fftwf_complex**>(c),
                          const_cast<const fftwf_complex**>(ch),
                          const_cast<const fftwf_complex**>(cd),
                          M, blFilt->Lchalf, bufLen, minlvl, tgrad, fgrad, cs);

    filterbankreassign_s(const_cast<const float**>(cs),
                         const_cast<const float**>(tgrad),
                         const_cast<const float**>(fgrad),
                         blFilt->Lchalf, blFilt->a, blFilt->fc, M, reassignedCoefs,
                         REASS_NOTIMEWRAPAROUND, NULL);

}



void
ReassignedBLFilterbank::
appendSamples(const float* samples, int samplesLen)
{
    overlapFifo->addToFifo(samples, samplesLen);
}

void
ReassignedBLFilterbank::
createBuffers()
{
    // Samples buffer
    size_t bufLenInBytes = bufLen * sizeof(float);
    buf = static_cast<float*>(fftwf_malloc( bufLenInBytes ));
    memset(buf, 0, bufLenInBytes);
    win = static_cast<float*>(fftwf_malloc( bufLenInBytes ));
    WindowDriver::hann(bufLen, win);

    // FFT buffer
    size_t cmplxBufLenInBytes = bufLen * sizeof(fftwf_complex);
    bufFFTCoefs = static_cast<fftwf_complex*>(fftwf_malloc( cmplxBufLenInBytes ));
    memset(bufFFTCoefs, 0, cmplxBufLenInBytes);

    // Filterbanks coefficient buffers
    for (FilterbankDataHolder::BLFilterbankDef * blFilt : filterbanks)
    {
        fftwf_complex** bufEl =  static_cast<fftwf_complex**>(fftwf_malloc(blFilt->M * sizeof(fftwf_complex*)));
        fftwf_complex** bufEl2 = static_cast<fftwf_complex**>(fftwf_malloc(blFilt->M * sizeof(fftwf_complex*)));
        for (int m = 0; m < blFilt->M; ++m)
        {
            bufEl[m] = static_cast<fftwf_complex*>(fftwf_malloc(blFilt->Lc[m] * sizeof(fftwf_complex)));
            bufEl2[m] = static_cast<fftwf_complex*>(fftwf_malloc(sizeof(fftwf_complex) * blFilt->Lc[m] / 2));
            memset(bufEl[m], 0, blFilt->Lc[m]*sizeof(fftwf_complex) );
            memset(bufEl2[m], 0, blFilt->Lc[m]*sizeof(fftwf_complex) / 2);
        }
        bufFilterbankCoefs.add(bufEl);
        bufFilterbankOverlaidCoefs.add(bufEl2);
    }

    // Reassigned coefs buffers
    FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getFirst();
    int M = blFilt->M;
    const ltfatInt* Lchalf = blFilt->Lchalf;
    reassignedCoefs = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));
    tgrad = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));
    fgrad = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));
    cs    = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));

    for (int m = 0; m < blFilt->M; ++m)
    {
        size_t LchalfinBytes = Lchalf[m] * sizeof(float);
        reassignedCoefs[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
        tgrad[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
        fgrad[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
        cs[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
    }

}


void
ReassignedBLFilterbank::
createPlans()
{
    // FFT plan
    plan = static_cast<fftwf_plan*>( malloc(sizeof(fftw_plan)));
    fftw_iodim fftw_dims[1];
    fftw_iodim howmanydims[1];

    fftw_dims[0].n = bufLen;
    fftw_dims[0].is = 1;
    fftw_dims[0].os = 1;

    howmanydims[0].n = 1;
    howmanydims[0].is = bufLen;
    howmanydims[0].os = bufLen;

    *plan = fftwf_plan_guru_dft(
                1, fftw_dims,
                1, howmanydims,
                bufFFTCoefs, bufFFTCoefs,
                FFTW_FORWARD, FFTW_MEASURE);

    // Clear the buffer since the plan creation might have written something to it
    size_t bufLenInBytes = bufLen * sizeof(fftwf_complex);
    memset(bufFFTCoefs, 0, bufLenInBytes);


    // Filterbank plans
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        // Actual filterbank
        FilterbankDataHolder::BLFilterbankDef * blFilt = filterbanks[ii];
        // Each filterbank has M plans, allocate space for M pointers
        convsub_fftbl_plan_s* pEl = static_cast<convsub_fftbl_plan_s*>
                                    ( fftwf_malloc(blFilt->M * sizeof(convsub_fftbl_plan_s)));
        // Add to array of plans
        p.add(pEl);
        // Temp array to be used during planning
        fftwf_complex ** bf = bufFilterbankCoefs.getUnchecked(ii);
        for (int m = 0; m < blFilt->M; ++m)
        {
            //DBG("Plannning bufLen " << bufLen << " Gl " << blFilt->Gl[m] << " nChannels " << nChannels << " a " << blFilt->a[m]  );
            // Plan m-th filter
            p.getLast()[m] = convsub_fftbl_init_s(bufLen, blFilt->Gl[m],
                                                  1, blFilt->a[m],
                                                  const_cast<const fftwf_complex*>(bf[m]));
            //DBG("After planning");
        }
    }

}

void
ReassignedBLFilterbank::
destroyBuffers()
{
    // // Destroy sample buffer
    if (nullptr != buf) fftwf_free(buf);
    // // Destroy FFT buffer
    if (nullptr != bufFFTCoefs) fftwf_free(bufFFTCoefs);
    if (nullptr != win) fftwf_free(win);

    // Destroy filterbank buffers
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks[ii];
        for (int m = 0; m < blFilt->M; ++m)
        {
            fftwf_free(bufFilterbankCoefs[ii][m]);
            fftwf_free(bufFilterbankOverlaidCoefs[ii][m]);
        }
        fftwf_free(bufFilterbankCoefs[ii]);
        fftwf_free(bufFilterbankOverlaidCoefs[ii]);
    }
    bufFilterbankCoefs.clear();
    bufFilterbankOverlaidCoefs.clear();

    // Destroy reassigned buffer
    FilterbankDataHolder::BLFilterbankDef* blFilt = filterbanks.getFirst();

    int M = blFilt->M;

    for (int m = 0; m < M; ++m)
    {
        fftwf_free(reassignedCoefs[m]);
        fftwf_free(tgrad[m]);
        fftwf_free(fgrad[m]);
        fftwf_free(cs[m]);
    }
    fftwf_free(reassignedCoefs);
    fftwf_free(tgrad);
    fftwf_free(fgrad);
    fftwf_free(cs);

}

void
ReassignedBLFilterbank::
destroyPlans()
{
    // Clear fft plan
    if (plan != nullptr)
    {
        fftwf_destroy_plan(*plan);
        free(plan);
        plan = nullptr;
    }

    // Clear filterbank plans
    for (int ii = 0; ii < p.size(); ++ii)
    {
        convsub_fftbl_plan_s* pEl = p[ii];
        for (int m = 0; m < filterbanks[ii]->M; ++m)
        {
            convsub_fftbl_done_s(pEl[m]);
        }
        fftwf_free(pEl);
    }
    p.clear();
}

void
ReassignedBLFilterbank::
WindowDriver::hann(int L, float * const win)
{
    double x = -0.5;
    double step = 1.0 / L;
    for (int ii = 0; ii < L; ++ii)
    {
        win[ii] = (0.5f + 0.5f * static_cast<float>(std::cos(2.0 * pi * x)));
        x += step;
    }
}

void
ReassignedBLFilterbank::
WindowDriver::hannsqrt(int L, float * const win)
{
    hann(L, win);
    for (int ii = 0; ii < L; ++ii)
    {
        win[ii] = std::sqrt(win[ii]);
    }
}
double
ReassignedBLFilterbank::
WindowDriver::pi = std::acos(-1);
