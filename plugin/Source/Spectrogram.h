/*
  ==============================================================================

    Spectrogram.h
    Created: 10 Feb 2015 11:06:11am
    Author:  susnak

  ==============================================================================
*/

#ifndef SPECTROGRAM_H_INCLUDED
#define SPECTROGRAM_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <complex>

//==============================================================================
/*
*/
class Spectrogram    : public Component
{
public:
    Spectrogram();
    Spectrogram(int imageWidth, int imageHeight, int stripWidth_ = Spectrogram::defaultStripWidth);
    ~Spectrogram();
    
    void appendStrip(const std::complex<float>* coefs[], int Lc[], int M);
    void setStripWidth(int stripWidth_);
    void setColourMap(HeapBlock<uint32>& colourmap_);
    
    void paint (Graphics&);
    void resized();

    static const int defaultImageWidth;
    static const int defaultImageHeight;
    static const int defaultStripWidth;
private:
    void setImageDimensions(int width, int height);

    Image image;
    Image strip;
    HeapBlock<float> stripBackend;
    HeapBlock<uint32> colourmap;
    Atomic<int> colourmapLen;
    int stripWidth;
    int stripPos;
    
    ScopedPointer<Thread> spectrogramThread;
    ScopedPointer<Graphics> imageGraphics;
    CriticalSection objectLock;

    class MathOp
    {
    public:
       static void copyToBackend(const std::complex<float>* coefs[],int Lc[],int M,
                                 HeapBlock<float>& data, int stripWidth, int stripHeight);
       static void toDB(HeapBlock<float>& in, int inLen );
       static void toLimitedRange(HeapBlock<float>& in, int inLen, float loDB, float hiDB);
       static void toRange(HeapBlock<float>& in, int inLen, float min, float max, float range);
       static void imageInColourmap(Image& image, HeapBlock<float>& data, HeapBlock<uint32>& cmap);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectrogram)
};


#endif  // SPECTROGRAM_H_INCLUDED
