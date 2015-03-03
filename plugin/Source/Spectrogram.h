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
#include "RingTransformBuffer.h"
#include <complex>

//==============================================================================
/*
*/
class Spectrogram    : public Component,
    public Timer,
    public Slider::Listener,
    public ModalComponentManager::Callback,
    public Thread
{
public:
    Spectrogram();
    Spectrogram(int imageWidth, int imageHeight, int stripWidth_ = Spectrogram::defaultStripWidth);
    ~Spectrogram();

    void appendStrip(const std::complex<float>* coefs[], int Lc[], int M);
    void appendStrip(const std::complex<float>  coefs[], int M);
    void setStripWidth(int stripWidth_);
    void setColourMap(HeapBlock<uint32>& colourmap_);

    void paint (Graphics&);
    void resized();

    void sliderValueChanged(Slider* slider) override;
    //void valueChanged(Value &value) override;
    void modalStateFinished(int rValue) override;
    void mouseDown(const MouseEvent & event) override;
    void timerCallback() override;
    void run() override;
    void setSpectrogramSource(SpectrogramPlottable* buf)
    {
        ringBuffer = buf;
    }
    PopupMenu& getPopupMenu()
    {
        return *pm;
    };

    void startPlotting()
    {
        startTimerHz(60);
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
    Atomic<int> stripPos;
    SpectrogramPlottable* ringBuffer;

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

    void populatePopupMenu();

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
