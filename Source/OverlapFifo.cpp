/*
  ==============================================================================

    OverlapFifo.cpp
    Created: 4 Mar 2016 12:32:58pm
    Author:  susnak

  ==============================================================================
*/

#include "OverlapFifo.h"

OverlapFifo::OverlapFifo(int initLen, int hop, int readLen)
    : abstractFifo(initLen)
    , bufLen(initLen)
    , hop(hop)
    , readLen(readLen)

{
    DBG(hop << " " << readLen << " " << initLen);
    assert(hop < readLen
           && "Hop size must be less than number of samples to be read.");
    assert(initLen > readLen
           && "Fifo length must be bigger than number of samples to be read.");
    buffer = new float[initLen];
}

OverlapFifo::~OverlapFifo()
{
    delete [] buffer;
}

void OverlapFifo::addToFifo(const float* someData, int numItems)
{
    int start1, size1, start2, size2;
    abstractFifo.prepareToWrite(numItems, start1, size1, start2, size2);

    if (size1 > 0)
        memcpy(buffer + start1, someData, size1 * sizeof(float));
    if (size2 > 0)
        memcpy(buffer + start2, someData + size1, size2 * sizeof(float));

    abstractFifo.finishedWrite(size1 + size2);
}

bool OverlapFifo::overlapReadFromFifo(float* someData)
{
    int readySamp = abstractFifo.getNumReady();
    int readyHops = readySamp / hop;

    // Not enough samples to fill a buffer
    if (!readyHops)
        return false;

    // Shift data in buffer to the left
    for (int ii = 0; ii < readLen - hop; ii++)
        someData[ii] = someData[ii + hop];

    float* someDataWritePtr = someData + readLen - hop;

    if (readyHops > 1)
    {
        // Reader is lagging, just skip to the newest part
        int skipSamples = (readyHops - 1) * hop;
        int start1, size1, start2, size2;
        int skipsize1, skipsize2;
        abstractFifo.prepareToRead(readyHops * hop, start1, size1, start2, size2);

        skipsize1 = size1 - skipSamples;
        if (skipsize1 < 0)
        {
            skipsize2 = size2 + skipsize1;
            skipsize1 = 0;
        }
        else
            skipsize2 = size2;

        if (skipsize1 > 0)
            memcpy(someDataWritePtr, buffer + start1 + skipSamples,
                   skipsize1 * sizeof(float));
        if (skipsize2 > 0)
            memcpy(someDataWritePtr + skipsize1,
                   buffer + start2 + skipSamples - skipsize1, skipsize2 * sizeof(float));

        // Mark everything as read
        abstractFifo.finishedRead(readyHops * hop);
    }
    else
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead(hop, start1, size1, start2, size2);

        if (size1 > 0)
            memcpy(someDataWritePtr, buffer + start1, size1 * sizeof(float));

        if (size2 > 0)
            memcpy(someDataWritePtr + size1, buffer + start2, size2 * sizeof(float));

        abstractFifo.finishedRead(size1 + size2);
    }
    return true;
}
