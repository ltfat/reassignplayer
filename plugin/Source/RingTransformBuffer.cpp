/*
  ==============================================================================

    RingTransformBuffer.cpp
    Created: 18 Feb 2015 12:28:54pm
    Author:  susnak

  ==============================================================================
*/

#include "RingTransformBuffer.h"
#define ABS(c) std::sqrt((c)[0]*(c)[0] + (c)[1]*(c)[1])

RingTransformBuffer::RingTransformBuffer(int bufLen_, int nChannels_, int nBuf_):
    bufLen(bufLen_), bufLenHalf(bufLen / 2), nChannels(std::max(nChannels_, 1)),
    nBuf( std::max(nBuf_, 2) + 1), head(0), tail(0), transformOnRead(false)
{
    DBG("RingTransformBuffer constructor start");
    pos = bufLenHalf;
    createBuffers();

    DBG("RingTransformBuffer constructor end");
}

RingTransformBuffer::~RingTransformBuffer()
{
    destroyBuffers();
}

void RingTransformBuffer::appendSamples(const float* samples[], int samplesLen)
{
    int remainingSamples = samplesLen;
    int samplesIdx = 0;
    float* actBuf = buf.getUnchecked(head);

    // First handle situation when samples overflow
    while (pos + remainingSamples >= bufLen)
    {
        if (isFull())
        {
            // DBG("BUFFER IS FULL");
            return;
        }
        // Number of samples we can write to the actual buffer
        int toWriteLen = bufLen - pos;
        for (int chIdx = 0; chIdx < nChannels; ++chIdx)
        {
            float* actBufTmp = actBuf + chIdx * bufLen;
            for (int ii = 0; ii < toWriteLen; ++ii)
            {
                // Write to the real part // equivalent to actBuf[ii][0]
                actBufTmp[pos + ii] = samples[chIdx][ii + samplesIdx];
            }
        }
        /* Now, the act buffer is full and we can transform it.
         * We will do it after copyint the higher half to the lower half
         * of the next buffer.  */

        // Move the index in samples
        samplesIdx += toWriteLen;
        // Shrink the remaining samples
        remainingSamples -= toWriteLen;
        // Index of the next buffer to write into

        // Copy the upper half of the buffer to lower half of the next buffer
        float* nextBuf = buf.getUnchecked((head + 1) % nBuf);
        memset(nextBuf, 0, bufLen * sizeof(float));

        for (int chIdx = 0; chIdx < nChannels; ++chIdx)
            memcpy(nextBuf, actBuf + chIdx * bufLen + bufLenHalf, bufLenHalf * sizeof(float));

        // This is a virtual method expected to be overloaded
        if (!transformOnRead)
            performTransform();


        // WARNING: HEAD MODIFICATION
        // --------------------------
        head = (head + 1) % nBuf;
        // --------------------------

        // Notify listeners
        // sendChangeMessage();

        actBuf = nextBuf;
        pos = bufLenHalf;
    }

    // Now the remaining samples fit in the act buffer
    for (int chIdx = 0; chIdx < nChannels; ++chIdx)
    {
        float* actBufTmp = actBuf + chIdx * bufLen;
        for (int ii = 0; ii < remainingSamples; ++ii)
        {
            actBufTmp[pos + ii] = samples[chIdx][ii + samplesIdx];
        }
    }
    pos += remainingSamples;
}

void RingTransformBuffer::createBuffers()
{
    destroyBuffers();
    size_t bufLenInBytes = nChannels * bufLen * sizeof(float);
    for (int ii = 0; ii < nBuf; ++ii)
    {
        float *tmp = static_cast<float*>(fftwf_malloc(nChannels * bufLen * sizeof(float)));
        memset(tmp, 0, bufLenInBytes);
        buf.add(tmp);
    }
}
void RingTransformBuffer::destroyBuffers()
{
    for (float * a : buf)
        if (nullptr != a) fftwf_free(a);
}

const float* RingTransformBuffer::getBuffer()
{
    if ( ! isEmpty() )
    {
        float* retval = buf.getUnchecked(tail);
        tail = (tail + 1) % nBuf;
        return retval;
    }
    else
    {
        return nullptr;
    }
}

bool RingTransformBuffer::isEmpty()
{
    return tail == head;
}

bool RingTransformBuffer::isFull()
{
    return tail == ((head + 1) % nBuf);
}

void RingFFTBuffer::WindowDriver::hann(int L, float * const win)
{
    double x = -0.5;
    double step = 1.0 / L;
    for (int ii = 0; ii < L; ++ii)
    {
        win[ii] = (0.5f + 0.5f * std::cos(2.0 * pi * x));
        x += step;
    }
}

void RingFFTBuffer::WindowDriver::hannsqrt(int L, float * const win)
{
    hann(L, win);
    for (int ii = 0; ii < L; ++ii)
    {
        win[ii] = std::sqrt(win[ii]);
    }
}
double RingFFTBuffer::WindowDriver::pi = std::acos(-1);

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
                float tmpxPrec = colsRatios[ii] * ii;
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
                float tmpxPrec = colsRatios[ii] * ii;
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


RingFFTBuffer::RingFFTBuffer(int bufLen_, winType winType_,  int nChannels_, int nBuf_)
    : RingTransformBuffer(bufLen_, nChannels_, nBuf_),
      plan(nullptr)
{
    createFFTBuffers();
    createPlan();

    win.malloc(bufLen);
    WindowDriver::hannsqrt(bufLen, win.getData());
}

RingFFTBuffer::~RingFFTBuffer()
{
    win.free();
    destroyPlan();
    destroyFFTBuffers();
}


// Create fftw plan using the first buffer
void RingFFTBuffer::createPlan()
{
    destroyPlan();
    plan = static_cast<fftwf_plan*>( malloc(sizeof(fftw_plan)));
    fftw_iodim fftw_dims[1];
    fftw_iodim howmanydims[1];

    fftw_dims[0].n = bufLen;
    fftw_dims[0].is = 1;
    fftw_dims[0].os = 1;

    howmanydims[0].n = nChannels;
    howmanydims[0].is = bufLen;
    howmanydims[0].os = bufLen;

    fftwf_complex* tmp = bufFFTCoefs.getFirst();
    *plan = fftwf_plan_guru_dft(
                1, fftw_dims,
                1, howmanydims,
                tmp, tmp,
                FFTW_FORWARD, FFTW_PATIENT);

    // Clear the buffer since the plan creation might have written something to it
    size_t bufLenInBytes = nChannels * bufLen * sizeof(fftwf_complex);
    memset(tmp, 0, bufLenInBytes);
}


void RingFFTBuffer::destroyPlan()
{
    if (plan != nullptr)
    {
        fftwf_destroy_plan(*plan);
        free(plan);
    }
    plan = nullptr;
}

void RingFFTBuffer::createFFTBuffers()
{
    destroyFFTBuffers();
    size_t bufLenInBytes = nChannels * bufLen * sizeof(fftwf_complex);
    for (int ii = 0; ii < nBuf; ++ii)
    {
        fftwf_complex *tmp = static_cast<fftwf_complex*>(fftwf_malloc(nChannels * bufLen * sizeof(fftwf_complex)));
        memset(tmp, 0, bufLenInBytes);
        bufFFTCoefs.add(tmp);
    }
}
void RingFFTBuffer::destroyFFTBuffers()
{
    for (fftwf_complex * a : bufFFTCoefs)
    {
        if (nullptr != a)  fftwf_free(a);
    }
}

void RingFFTBuffer::performTransform() noexcept
{
    float* in = buf[head];
    fftwf_complex* out = bufFFTCoefs.getUnchecked(head);

    for (int chIdx = 0; chIdx < nChannels; ++chIdx)
    {
        float* inTmp = in + chIdx * bufLen;
        fftwf_complex* outTmp = out + chIdx * bufLen;
        for (int ii = 0; ii < bufLen; ++ii)
        {
            outTmp[ii][0] = inTmp[ii];
            outTmp[ii][1] = 0.0f;
        }
    }
    // Pre-multiply with a window
    if (nullptr != win.getData())
    {
        float* winPtr = win.getData();
        for (int chIdx = 0; chIdx < nChannels; ++chIdx)
        {
            fftwf_complex* outTmp = out + chIdx * bufLen;
            for (int ii = 0; ii < bufLen; ++ii)
            {
                outTmp[ii][0] *= winPtr[ii];
            }
        }
    }

    // Do fftshift here? To get the phase in the middle...
    fftwf_execute_dft(*plan, out, out);
}

fftwf_complex* RingFFTBuffer::getFFTCoefficients(bool doConsume)
{
    if ( ! isEmpty() )
    {
        fftwf_complex* retval = bufFFTCoefs.getUnchecked(tail);
        if (doConsume)
            tail = (tail + 1) % nBuf;
        return retval;
    }
    else
    {
        return nullptr;
    }
}

bool RingFFTBuffer::getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows)
{
    fftwf_complex* cEl = getFFTCoefficients(true);
    if (nullptr == cEl) return false;

    ltfatInt bufLenPtr[1] = { bufLen / 2 };
    SpectrogramPlottableMethods::coefsToAbsMatrix(&cEl, bufLenPtr , 1, matrix, cols, rows);
    return true;
}

RingBLFilterbankBuffer::RingBLFilterbankBuffer(File filterbankFile_, int bufLen_,
        winType winType_,  int nChannels_, int nBuf_)
    : RingBLFilterbankBuffer(Array<File>(&filterbankFile_, 1), bufLen_, winType_, nChannels_, nBuf_)
{}


RingBLFilterbankBuffer::RingBLFilterbankBuffer(Array<File> filterbankFiles_, int bufLen_,
        winType winType_,  int nChannels_, int nBuf_)
    : RingFFTBuffer(bufLen_, winType_, nChannels_, nBuf_),
      plotOverlaiedCoefficients(true), filterbankFiles(filterbankFiles_)
{
    readFilterbankDefinitions();
    createFilterbankBuffers();
    createFilterbankPlan();
}

RingBLFilterbankBuffer::RingBLFilterbankBuffer(Array<BLFilterbankDef*> filterbankDefs_, int bufLen_,
        winType winType_,  int nChannels_, int nBuf_)
    : RingFFTBuffer(bufLen_, winType_, nChannels_, nBuf_),
      plotOverlaiedCoefficients(true)
{
    for (BLFilterbankDef* f : filterbankDefs_)  filterbanks.add(f);

    createFilterbankBuffers();
    createFilterbankPlan();
}


RingBLFilterbankBuffer::~RingBLFilterbankBuffer()
{
    destroyFilterbankPlan();
    destroyFilterbankBuffers();
}

void RingBLFilterbankBuffer::readFilterbankDefinitions()
{
    for (File f : filterbankFiles)
    {
        filterbanks.add(BLFilterbankDef::createDefFromFile(f));
    }
}

void RingBLFilterbankBuffer::createFilterbankPlan()
{
    for (int ii = 0; ii < filterbanks.size(); ++ii)
    {
        // Actual filterbank
        BLFilterbankDef * blFilt = filterbanks[ii];
        // Each filterbank has M plans, allocate space for M pointers
        convsub_fftbl_plan_s* pEl = static_cast<convsub_fftbl_plan_s*>
                                    ( fftwf_malloc(blFilt->M * sizeof(convsub_fftbl_plan_s)));
        // Add to array of plans
        p.add(pEl);
        // Temp array to be used during planning
        fftwf_complex ** bf = bufFilterbankCoefs.getFirst()->getUnchecked(ii);
        for (int m = 0; m < blFilt->M; ++m)
        {
            DBG("Plannning bufLen " << bufLen << " Gl " << blFilt->Gl[m] << " nChannels " << nChannels << " a " << blFilt->a[m]  );
            // Plan m-th filter
            p.getLast()[m] = convsub_fftbl_init_s(bufLen, blFilt->Gl[m],
                                                  nChannels, blFilt->a[m],
                                                  reinterpret_cast<const _Complex float*>(bf[m]));
            DBG("After planning");
        }
    }
}

void RingBLFilterbankBuffer::destroyFilterbankPlan()
{
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

void RingBLFilterbankBuffer::createFilterbankBuffers()
{
    for (int ii = 0; ii < nBuf; ++ii)
    {
        Array<fftwf_complex**>* arrEl = new Array<fftwf_complex**>();
        Array<fftwf_complex**>* arrEl2 = new Array<fftwf_complex**>();
        for (BLFilterbankDef * blFilt : filterbanks)
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
            arrEl->add(bufEl);
            arrEl2->add(bufEl2);
        }
        bufFilterbankCoefs.add(arrEl);
        bufFilterbankOverlaidCoefs.add(arrEl2);
    }

    dummy = static_cast<fftwf_complex**>(fftwf_malloc(filterbanks[0]->M * sizeof(fftwf_complex*)));
    for (int m = 0; m < filterbanks[0]->M; ++m)
    {
        dummy[m] = static_cast<fftwf_complex*>(fftwf_malloc(sizeof(fftwf_complex) * filterbanks[0]->Lc[m] / 2));
        memset(dummy[m], 0, filterbanks[0]->Lc[m]*sizeof(fftwf_complex) / 2);
    }


}
void RingBLFilterbankBuffer::destroyFilterbankBuffers()
{
    for (int jj = 0; jj < bufFilterbankCoefs.size(); ++jj)
    {
        Array<fftwf_complex**>* a = bufFilterbankCoefs[jj];
        Array<fftwf_complex**>* a2 = bufFilterbankOverlaidCoefs[jj];
        for (int ii = 0; ii < filterbanks.size(); ++ii)
        {
            BLFilterbankDef* blFilt = filterbanks[ii];
            for (int m = 0; m < blFilt->M; ++m)
            {
                fftwf_free((*a)[ii][m]);
                fftwf_free((*a2)[ii][m]);
            }
            fftwf_free((*a)[ii]);
            fftwf_free((*a2)[ii]);
        }
        a->clear();
        a2->clear();
    }
    bufFilterbankCoefs.clear(true);
    bufFilterbankOverlaidCoefs.clear(true);
}

void RingBLFilterbankBuffer::performTransform() noexcept
{
    int locHead = head;
    // Do FFT of the block
    RingFFTBuffer::performTransform();
    // bufFFTCoefs[head] contains fresh FFT samples

    //for (int ii = 0; ii < filterbanks.size(); ++ii)

    parallel_for( blocked_range<size_t>(0, filterbanks.size(), 1), [ = ](const blocked_range<size_t>& rbig)
    {
        for (size_t ii = rbig.begin(); ii != rbig.end(); ++ii)

        {
            fftwf_complex** C = bufFilterbankCoefs.getUnchecked(locHead)->getUnchecked(ii);
            BLFilterbankDef* blFilt = filterbanks.getUnchecked(ii);

            // #pragma omp parallel for private(m)

            //for (int m = 0; m < blFilt->M; ++m)
            parallel_for( blocked_range<size_t>(0, blFilt->M, 10), [ = ](const blocked_range<size_t>& r)
            {

                for (size_t i = r.begin(); i != r.end(); ++i)
                {
                    convsub_fftbl_execute_s(p[ii][i],
                    reinterpret_cast<const float _Complex *>(bufFFTCoefs.getUnchecked(locHead)),
                    reinterpret_cast<const float _Complex *>(blFilt->G[i]),
                    blFilt->foff[i], 0 , reinterpret_cast<float _Complex *>(C[i]));
                }
            }
                        );

            /*
                    filterbank_fftbl_execute_s(p[ii],
                    reinterpret_cast<const float _Complex *>(bufFFTCoefs.getUnchecked(locHead)),
                    reinterpret_cast<const float _Complex **>(blFilt->G),
                    blFilt->M, blFilt->foff, blFilt->realonly,
                    reinterpret_cast<float _Complex **>(C));
            */
            // Do the overlays
            fftwf_complex** overlayfront = bufFilterbankOverlaidCoefs.getUnchecked(locHead)->getUnchecked(ii);

            //for (int m = 0; m < blFilt->M; ++m)
            parallel_for( blocked_range<size_t>(0, blFilt->M, 10), [ = ](const blocked_range<size_t>& r)
            {
                for (size_t m = r.begin(); m != r.end(); ++m)
                {

                    fftwf_complex* overlayfrontTmp = overlayfront[m];
                    fftwf_complex* CTmp = C[m];
                    for (int ii = 0; ii < blFilt->Lchalf[m] ; ++ii)
                    {
                        overlayfrontTmp[ii][0] += CTmp[ii][0];
                        overlayfrontTmp[ii][1] += CTmp[ii][1];
                    }
                }
            });
            // This is actually safe, because we keep one slot open in the ring buffer and it
            // is not even accessed by the producer

            int headplusone = ((locHead + 1) % nBuf);
            fftwf_complex** overlayback = bufFilterbankOverlaidCoefs.getUnchecked(headplusone)->getUnchecked(ii);

            //for (int m = 0; m < blFilt->M; ++m)
            parallel_for( blocked_range<size_t>(0, blFilt->M, 10), [ = ](const blocked_range<size_t>& r)
            {
                for (size_t m = r.begin(); m != r.end(); ++m)
                {
                    memcpy(overlayback[m], C[m] + blFilt->Lchalf[m], blFilt->Lchalf[m] * sizeof(fftwf_complex));
                }
            });

        }
    }
                );

}

fftwf_complex** RingBLFilterbankBuffer::
getFilterbankCoefficients(int filterbankId, bool doConsume)
{
    if ( ! RingTransformBuffer::isEmpty() )
    {
        fftwf_complex** retval = bufFilterbankCoefs.getUnchecked(tail)->getUnchecked(filterbankId);
        if ( doConsume)
            tail = (tail + 1) % nBuf;
        return retval;
    }
    else
    {
        return nullptr;
    }

}

fftwf_complex** RingBLFilterbankBuffer::
getFilterbankOverlaidCoefficients(int filterbankId, bool doConsume)
{
    if ( !RingTransformBuffer::isEmpty() )
    {
        fftwf_complex** retval = bufFilterbankOverlaidCoefs.getUnchecked(tail)->getUnchecked(filterbankId);
        //fftwf_complex** retval = dummy;
        if ( doConsume)
            tail = (tail + 1) % nBuf;
        return retval;
    }
    else
    {
        return nullptr;
    }
}

bool RingBLFilterbankBuffer::getBufferCoefficientsAsAbsMatrix(float * matrix, int cols, int rows)
{
    // activePlottingFilterbank is atomic
    int activeFb = activePlottingFilterbank.get();
    fftwf_complex** cEl;
    ltfatInt* Lcact;

    if (plotOverlaiedCoefficients.get())
    {
        // Lc is only half here
        cEl = getFilterbankOverlaidCoefficients(activeFb, true);
        Lcact = const_cast<ltfatInt*>(filterbanks[activeFb]->Lchalf);
    }
    else
    {
        cEl = getFilterbankCoefficients(activeFb, true);
        Lcact = const_cast<ltfatInt*>(filterbanks[activeFb]->Lc);
    }

    if (nullptr == cEl) return false;


    SpectrogramPlottableMethods::coefsToAbsMatrix(cEl, Lcact,
            filterbanks[activeFb]->M,
            matrix, cols, rows);

    return true;
}

RingBLFilterbankBuffer::BLFilterbankDef* RingBLFilterbankBuffer::BLFilterbankDef::
createDefFromFile(File& file, std::streamoff byteOffset)
{
    // Read the file:
    std::streampos dataSize, readSize;
    std::ifstream dataFile;

    unsigned long byteSize, shouldBeAtLeast;
    dataFile.open (file.getFullPathName().getCharPointer(),
                   std::ios::in | std::ios::binary | std::ios::ate);

    if (dataFile.fail())
    {
        throw String(String("File ") + file.getFileName() + String(" not found!"));
    }

    dataSize = dataFile.tellg(); // Length = eof
    dataFile.seekg(byteOffset, std::ios::beg); // Reset stream to beginning of file

    byteSize = static_cast<unsigned long>(dataSize); // Recast file size as integer

    // This is only for testing purposes
    // std::cout << byteSize << std::endl;
    shouldBeAtLeast = 8;
    if (byteSize < shouldBeAtLeast)
    {
        throw String(String("Reading error, no filterbank data file") +
                     file.getFileName());
    }
    unsigned long binFilterbankLength;
    unsigned blockLength;
    unsigned M;

    // Read filterbank length
    if (sizeof(unsigned long) > 4) // Handle standard case of 32-bit unsigned
    {
        unsigned* tempInt = new unsigned;
        dataFile.read(reinterpret_cast <char*> (tempInt), 4);
        binFilterbankLength = static_cast <unsigned long> (*tempInt);
    }
    else // Handle case of 16-bit unsigned
    {
        dataFile.read(reinterpret_cast <char*> (binFilterbankLength), 4);
    }

    // Determine block length and number of channels
    BLFilterbankDef::getFilterbankBaseData (&dataFile, &blockLength, &M);

    // This is only for testing purposes
    // std::cout << "blockLength is: " << blockLength << std::endl;
    // std::cout << "M is: " << M << std::endl;

    // This is the second check whether the file has the correct length
    shouldBeAtLeast += 10 * M + 2;

    // This is only for testing purposes
    // std::cout << shouldBeAtLeast << std::endl;

    if (byteSize < shouldBeAtLeast)
    {
        throw String("Reading error, no filterbank data file (2)");
    }

    unsigned aOne;
    unsigned* a = new unsigned[M];
    float* fc = new float[M];
    unsigned* foff = new unsigned[M];
    unsigned* filtLengths = new unsigned[M];

    // Determine filter bank parameters
    // (TODO: change order in the MATLAB files for filtLengths to be at the end)
    BLFilterbankDef::getFilterbankParamData (&dataFile, M, &aOne, a, fc, foff, filtLengths);

    // This is only for testing purposes
    // std::cout << "foff is: " << std::endl;
    // for (int kk = 0; kk < 10; kk++)
    // {std::cout << foff[kk] << std::endl;}
    //
    // std::cout << "a is: " << std::endl;
    // for (int kk = 0; kk < 10; kk++)
    // {std::cout << a[kk] << std::endl;}

    // This is the final check whether the file has the correct length
    for ( unsigned kk = 0; kk < M; ++kk)
    {
        shouldBeAtLeast += 4 * (filtLengths[kk]);
    }


    // This is only for testing purposes
    // std::cout << shouldBeAtLeast << std::endl;

    if (byteSize < shouldBeAtLeast)
    {
        dataFile.close();
        delete [] a;
        delete [] fc;
        delete [] foff;
        delete [] filtLengths;
        throw String("Reading error, no filterbank data file (3)");
    }

    float** G = new float*[M];
    for (unsigned kk = 0; kk < M; ++kk)
    {
        G[kk] = new float[filtLengths[kk]];
    }

    // Get the filter data
    BLFilterbankDef::getFilterbankFilterData (&dataFile, M, filtLengths, G);

    // Close the file stream
    dataFile.close();

    // This is only for testing purposes
    //  std::cout << "G[200] is: " << std::endl;
    // for (int kk = 0; kk < 17; kk++)
    // {std::cout << G[200][kk] << std::endl;}


    int *Gl_ = static_cast<int*>(fftwf_malloc(M * sizeof(int)));
    fftwf_complex ** G_ = static_cast<fftwf_complex**>( fftwf_malloc(M * sizeof(fftwf_complex*)));
    for (unsigned kk = 0; kk < M; ++kk)
    {
        G_[kk] = static_cast<fftwf_complex*>(fftwf_malloc(filtLengths[kk] * sizeof(fftwf_complex)));
    }
    ltfatInt *foff_ = static_cast<ltfatInt*>( fftwf_malloc(M * sizeof(ltfatInt)));
    int * realonly_ = static_cast<int*>(fftwf_malloc(M * sizeof(ltfatInt)));
    memset(realonly_, 0, M * sizeof(ltfatInt));
    double *a_ = static_cast<double*>(fftwf_malloc(M * sizeof(double)));
    double *fc_ = static_cast<double*>(fftwf_malloc(M * sizeof(double)));
    ltfatInt *Lc_ = static_cast<ltfatInt*>(fftwf_malloc(M * sizeof(ltfatInt)));
    ltfatInt *Lchalf_ = static_cast<ltfatInt*>(fftwf_malloc(M * sizeof(ltfatInt)));

    for (unsigned kk = 0; kk < M; ++kk)
    {
        Gl_[kk] = filtLengths[kk];
        for (int jj = 0; jj < Gl_[kk]; ++jj)
        {
            G_[kk][jj][0] = G[kk][jj];
            G_[kk][jj][1] = 0.0f;
        }
        foff_[kk] = foff[kk];
        a_[kk] = static_cast<double>(aOne) / a[kk];
        fc_[kk] = static_cast<double>(fc[kk]);
        Lc_[kk] = static_cast<ltfatInt>(std::round(static_cast<double>(blockLength) / a_[kk]));
        Lchalf_[kk] = static_cast<ltfatInt>(std::floor(Lc_[kk] / 2.0));
    }

    BLFilterbankDef* retVal = new BLFilterbankDef(const_cast<const fftwf_complex**>(G_),
            Gl_, foff_, realonly_, a_, fc_, Lc_, Lchalf_, (int)M, blockLength);

    //  Free tep variables
    delete [] a;
    delete [] fc;
    delete [] foff;
    delete [] filtLengths;
    for (unsigned kk = 0; kk < M; ++kk)
    {
        delete [] G[kk];
    }
    delete [] G;

    return retVal;
}

void RingBLFilterbankBuffer::BLFilterbankDef::
getFilterbankBaseData(std::ifstream* dataFilePtr, unsigned* blockLengthPtr, unsigned* mPtr)
{
    if (sizeof(unsigned) > 2) // Handle standard case of 32-bit unsigned
    {
        unsigned short* tempInt = new unsigned short;
        (*dataFilePtr).read(reinterpret_cast <char*> (tempInt), 2);
        (*blockLengthPtr) = static_cast <unsigned> (*tempInt);
        (*dataFilePtr).read(reinterpret_cast <char*> (tempInt), 2);
        (*mPtr) = static_cast <unsigned> (*tempInt);
    }
    else // Handle case of 16-bit unsigned
    {
        (*dataFilePtr).read(reinterpret_cast <char*> (blockLengthPtr), 2);
        (*dataFilePtr).read(reinterpret_cast <char*> (mPtr), 2);
    }
}

void RingBLFilterbankBuffer::BLFilterbankDef::
getFilterbankParamData(std::ifstream* dataFilePtr,
                       unsigned M, unsigned* aOnePtr,
                       unsigned a[], float fc[],
                       unsigned foff[], unsigned filtLengths[])
{
    if (sizeof(unsigned) > 2) // Handle standard case of 32-bit unsigned
    {
        unsigned short* tempAry = new unsigned short[M];
        unsigned short* tempInt = new unsigned short;

        (*dataFilePtr).read(reinterpret_cast <char*> (tempInt), 2);
        (*aOnePtr) = static_cast <unsigned> (*tempInt);
        (*dataFilePtr).read(reinterpret_cast <char*> (tempAry), 2 * M);
        for (unsigned kk = 0; kk < M; ++kk)
        {
            a[kk] = static_cast <unsigned> (tempAry[kk]);
        }
        (*dataFilePtr).read(reinterpret_cast <char*> (fc), 4 * M);
        (*dataFilePtr).read(reinterpret_cast <char*> (tempAry), 2 * M);
        for (unsigned kk = 0; kk < M; ++kk)
        {
            foff[kk] = static_cast <unsigned> (tempAry[kk]);
        }
        (*dataFilePtr).read(reinterpret_cast <char*> (tempAry), 2 * M);
        for (unsigned kk = 0; kk < M; ++kk)
        {
            filtLengths[kk] = static_cast <unsigned> (tempAry[kk]);
        }

    }
    else // Handle case of 16-bit unsigned
    {
        (*dataFilePtr).read(reinterpret_cast <char*> (filtLengths), 2 * M);
        (*dataFilePtr).read(reinterpret_cast <char*> (aOnePtr), 2);
        (*dataFilePtr).read(reinterpret_cast <char*> (a), 2 * M);
        (*dataFilePtr).read(reinterpret_cast <char*> (fc), 4 * M);
        (*dataFilePtr).read(reinterpret_cast <char*> (foff), 2 * M);
    }
}

void RingBLFilterbankBuffer::BLFilterbankDef::
getFilterbankFilterData(std::ifstream* dataFilePtr,
                        unsigned M, unsigned filtLengths[],
                        float** G)
{
    // We currently assume float to always be 32-bit (We should add a sanity check here at least)
    for (unsigned kk = 0; kk < M; ++kk)
    {
        (*dataFilePtr).read(reinterpret_cast <char*> (G[kk]), 4 * filtLengths[kk]);
    }

}


RingBLFilterbankBuffer::BLFilterbankDef::~BLFilterbankDef()
{
    if (nullptr != G)
    {
        for (int ii = 0; ii < M; ++ii)
            if (nullptr != G[ii])
                fftwf_free(const_cast<fftwf_complex*>(G[ii]));
        fftwf_free(G);
    }
    // I know I should't, but all the C++ casts  are just horrible in this case.
    if (nullptr != Gl) fftwf_free((void*)Gl);
    if (nullptr != foff) fftwf_free((void*)foff);
    if (nullptr != realonly) fftwf_free((void*)realonly);
    if (nullptr != a) fftwf_free((void*)a);
    if (nullptr != Lc) fftwf_free((void*)Lc);
}

RingReassignedBLFilterbankBuffer::
RingReassignedBLFilterbankBuffer(File filterbankFiles_[3], int bufLen_,
                                 winType winType,
                                 int nChannels_, int nBuf):
    RingBLFilterbankBuffer(Array<File>(filterbankFiles_, 3), bufLen_, winType, nChannels_, nBuf)
{
    // There are now 3 filters in filterbanks
    // We need to check whether thea are all the same..
    setActivePlotReassigned( true);
    createReassignBuffers();
    createReassignPlan();
}

RingReassignedBLFilterbankBuffer::
~RingReassignedBLFilterbankBuffer()
{
    destroyReassignBuffers();
    destroyReassignPlan();
}

void RingReassignedBLFilterbankBuffer::
createReassignPlan()
{

}

void RingReassignedBLFilterbankBuffer::
destroyReassignPlan()
{

}

void RingReassignedBLFilterbankBuffer::
createReassignBuffers()
{
    // tgrad,fgrad,cs,
    // reassignedCoefs
    BLFilterbankDef* blFilt = filterbanks.getFirst();
    int M = blFilt->M;
    const ltfatInt* Lchalf = blFilt->Lchalf;
    for (int ii = 0; ii < nBuf; ++ii)
    {
        reassignedCoefs.add(static_cast<float**>(fftwf_malloc(M * sizeof(float*))));
    }

    tgrad = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));
    fgrad = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));
    cs    = static_cast<float**>(fftwf_malloc(M * sizeof(float*)));

    for (int m = 0; m < blFilt->M; ++m)
    {
        size_t LchalfinBytes = Lchalf[m]*sizeof(float);
        for (float** a: reassignedCoefs)
        {
            a[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
            memset(a[m],0,LchalfinBytes);
        }
        tgrad[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
        fgrad[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
        cs[m] = static_cast<float*>(fftwf_malloc(LchalfinBytes));
        memset(tgrad[m],0,LchalfinBytes);
        memset(fgrad[m],0,LchalfinBytes);
        memset(cs[m],0,LchalfinBytes);
    }

}
void RingReassignedBLFilterbankBuffer::
destroyReassignBuffers()
{
    BLFilterbankDef* blFilt = filterbanks.getFirst();
    int M = blFilt->M;

    for (auto a : reassignedCoefs)
    {
        for (int m = 0; m < M; ++m) fftwf_free(a[m]);

        fftwf_free(a);
    }

    for (int m = 0; m < M; ++m)
    {
        fftwf_free(tgrad[m]);
        fftwf_free(fgrad[m]);
        fftwf_free(cs[m]);
    }

    fftwf_free(tgrad);
    fftwf_free(fgrad);
    fftwf_free(cs);

}

void RingReassignedBLFilterbankBuffer::performTransform() noexcept
{

    RingBLFilterbankBuffer::performTransform();
    // bufFilterbankOverlaidCoefs[head] now contains (in order)
    // 1. coefficients to be reassigned
    // 2. coefficients obtained with frequency-weighted window
    // 3. coefficients obtained with time-weighted window

    BLFilterbankDef* blFilt = filterbanks.getFirst();
    const int M = blFilt->M;
    float minlvl = 1e-6f;

    fftwf_complex** c = bufFilterbankOverlaidCoefs.getUnchecked(head)->getUnchecked(0);
    fftwf_complex** ch = bufFilterbankOverlaidCoefs.getUnchecked(head)->getUnchecked(1);
    fftwf_complex** cd = bufFilterbankOverlaidCoefs.getUnchecked(head)->getUnchecked(2);
    // tgrad,fgrad,cs

    // Super easy type casing !! :)
    filterbankphasegrad_s(reinterpret_cast<const _Complex float**>(const_cast<const fftwf_complex**>(c)),
                          reinterpret_cast<const _Complex float**>(const_cast<const fftwf_complex**>(ch)),
                          reinterpret_cast<const _Complex float**>(const_cast<const fftwf_complex**>(cd)),
                          M, blFilt->Lchalf, bufLen, minlvl, tgrad, fgrad, cs);
        // And do the reassignment
        float** sr = reassignedCoefs.getUnchecked(head);
        filterbankreassign_s(const_cast<const float**>(cs),
        const_cast<const float**>(tgrad),
        const_cast<const float**>(fgrad),
        blFilt->Lchalf, blFilt->a, blFilt->fc, M, sr, NULL);
        
}

float** RingReassignedBLFilterbankBuffer::
getReassignedCoefficients( bool doConsume)
{
    if ( !RingTransformBuffer::isEmpty() )
    {
        float** retval = reassignedCoefs.getUnchecked(tail);
        if ( doConsume)
            tail = (tail + 1) % nBuf;
        return retval;
    }
    else
    {
        return nullptr;
    }
}

bool RingReassignedBLFilterbankBuffer::
getBufferCoefficientsAsAbsMatrix(float* matrix, int cols, int rows )
{
    if (!doPlotReassigned.get())
    {
        return RingBLFilterbankBuffer::getBufferCoefficientsAsAbsMatrix(matrix, cols, rows);
    }

    float** cEl = getReassignedCoefficients(true);
    if (nullptr == cEl) return false;

    SpectrogramPlottableMethods::coefsToAbsMatrix(cEl, const_cast<ltfatInt*>(filterbanks[0]->Lchalf),
            filterbanks[0]->M,
            matrix, cols, rows);
    return true;
}



#undef ABS
