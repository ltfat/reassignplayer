/*
  ==============================================================================

    OverlapFifo.h
    Created: 4 Mar 2016 12:32:58pm
    Author:  susnak

  ==============================================================================
*/

#pragma once
#include "juce_core/juce_core.h"

class OverlapFifo
{
public:
    OverlapFifo (int intitLen, int hop, int readLen);
    ~OverlapFifo();

    void addToFifo (const float* someData, int numItems);
    bool overlapReadFromFifo (float* someData);

private:
    juce::AbstractFifo abstractFifo;
    float* buffer;
    int bufLen;
    int hop;
    int readLen;
};
