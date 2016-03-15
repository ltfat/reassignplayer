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
#include "ReassignedBLFilterbank.h"

//==============================================================================
/*
*/


class Spectrogram    : public Component,
    public HighResolutionTimer,
    public Slider::Listener
{
public:
    Spectrogram();
    Spectrogram(int imageWidth, int imageHeight, int stripWidth_ = Spectrogram::defaultStripWidth);
    ~Spectrogram();

    void setStripWidth(int stripWidth_);
    void setColourMap(HeapBlock<uint32>& colourmap_);

    void paint (Graphics&);
    void resized();

    void sliderValueChanged(Slider* slider) override;
    void mouseDown(const MouseEvent & event) override;
    void hiResTimerCallback() override;
    
    void setSpectrogramSource(SpectrogramPlottable* buf);
    // The same as above but it can fail
    bool trySetSpectrogramSource(SpectrogramPlottable* buf);
    // When called, stop ascing for samples as soon as possible
    bool aboutToChangeSpectrogramSource();

    PopupMenu& getPopupMenu()
    {
        return *pm;
    };

    void startPlotting()
    {
        startTimer(1000.0/100.0);
    }
    void stopPlotting()
    {
        stopTimer();
    }
    void setAudioLoopMs(double ms)
    {
        static int counter = 0;
        audioLoopMs = ms;
        if (counter > maxCountRefresh)
        {
            counter = 0;
            audioLoopMaxMs = 0.0;
        }
        counter++;
        if (ms > audioLoopMaxMs.get())
        {
            audioLoopMaxMs = ms;
        }
    }

    static const int defaultImageWidth;
    static const int defaultImageHeight;
    static const int defaultStripWidth;
    static const double defaultMinDB;
    static const double defaultMaxDB;
private:
    void setImageDimensions(int width, int height);
    void stripBackendToRepaint();
    void populatePopupMenu();
    Atomic<int> timerFired;
    double repaintTimeMaxMs;
    Atomic<double> audioLoopMs;
    Atomic<double> audioLoopMaxMs;
    int maxCountRefresh;
    Font displayFont ;

    OpenGLImageType imageType;
    Image image;
    Image strip;
    HeapBlock<float> stripBackend;
    HeapBlock<uint32> colourmap;
    Atomic<int> colourmapLen;
    int stripWidth;
    int stripPos;
    Atomic<SpectrogramPlottable*> spectrogramSource;

    ScopedPointer<Thread> spectrogramThread;
    ScopedPointer<Graphics> imageGraphics;

    // Right-click popupmenu
    ScopedPointer<PopupMenu> pm;
    ScopedPointer<Slider> minmaxdb;
    ScopedPointer<Label> minmaxdbLabel;
    ScopedPointer<Slider> speedSlider;
    double oldMidDB;
    Atomic<double> minDB, maxDB, midDB;
    double initMinDB, initMaxDB;

    OwnedArray<Component> trash;

    CriticalSection objectLock;

    Atomic<int> spectrogramSourceIsValid;
    Atomic<int> stripIsValid;
    Atomic<int> nextStripIsValid;
    int partialStripCounter;
    int partialStripWidth;



    class MathOp
    {
    public:
        static void copyToBackend(const std::complex<float>* coefs[], int Lc[], int M,
                                  HeapBlock<float>& data, int stripWidth, int stripHeight);
        static void copyToBackend(const std::complex<float> coefs[], int M,
                                  HeapBlock<float>& data, int stripWidth, int stripHeight);

        static void toDB(HeapBlock<float>& in, int inLen );
        static void toLimitedRange(HeapBlock<float>& in, int inLen, float loDB, float hiDB);
        static void toRange(HeapBlock<float>& in, int inLen, float min, float max, float range);
        static void imageInColourmap(Image& image, HeapBlock<float>& data, HeapBlock<uint32>& cmap);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectrogram)
};


#endif  // SPECTROGRAM_H_INCLUDED
