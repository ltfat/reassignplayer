/*
  ==============================================================================

    OverlapFifo.h
    Created: 4 Mar 2016 12:32:58pm
    Author:  susnak

  ==============================================================================
*/

#ifndef OVERLAPFIFO_H_INCLUDED
#define OVERLAPFIFO_H_INCLUDED
#include "../JuceLibraryCode/JuceHeader.h"
#include <cstdlib>
#include <cstring>
#include <cassert>

class OverlapFifo {
public:
    OverlapFifo(int intitLen, int hop, int readLen);
    ~OverlapFifo();

    void addToFifo(const float* someData, int numItems);
    bool overlapReadFromFifo(float* someData);

private:
    AbstractFifo abstractFifo;
    float* buffer;
    int bufLen;
    int hop;
    int readLen;
};

#endif // OVERLAPFIFO_H_INCLUDED
