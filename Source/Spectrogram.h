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

    void setColourMap(HeapBlock<uint32>& colourmap_);

    void paint (Graphics&);
    void resized();

    void sliderValueChanged(Slider* slider) override;
    void mouseDown(const MouseEvent & event) override;
    void hiResTimerCallback() override;

    void setSpectrogramSource(SpectrogramPlottable* buf);

    PopupMenu& getPopupMenu()
    {
        return *pm;
    };

    void setStripWidth(int stripWidth_)
    {
        jassert(stripWidth_ % partialStripWidth == 0 && "stripWidth must be divisible by 5");
        stripWidth = stripWidth_;
        stripBackend.malloc(stripWidth * image.getHeight() * sizeof(float));
        strip = Image(Image::ARGB, stripWidth, image.getHeight(), true );
    }

    void startPlotting()
    {
        startTimer(1000.0 / 60.0);
    }
    void stopPlotting()
    {
        stopTimer();
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
    double repaintTimeMaxMs;
    Atomic<double> audioLoopMs;
    Atomic<double> audioLoopMaxMs;
    int maxCountRefresh;
    Font displayFont ;

    Image image;
    Image strip;
    HeapBlock<float> stripBackend;
    HeapBlock<uint32> colourmap;
    Atomic<int> colourmapLen;
    int stripWidth;
    int stripPos;
    Atomic<SpectrogramPlottable*> spectrogramSource;

    ScopedPointer<Graphics> imageGraphics;

    // Right-click popupmenu
    ScopedPointer<PopupMenu> pm;
    ScopedPointer<Slider> minmaxdb;
    ScopedPointer<Label> minmaxdbLabel;
    ScopedPointer<Slider> speedSlider;
    double oldMidDB;
    Atomic<double> minDB, maxDB, midDB;
    double initMinDB, initMaxDB;

    CriticalSection objectLock;

    Atomic<int> spectrogramSourceIsValid;
    Atomic<int> stripIsValid;
    Atomic<int> nextStripIsValid;
    int partialStripCounter;
    int partialStripWidth;

    class MathOp
    {
    public:
        static void toDB(HeapBlock<float>& in, int inLen );
        static void toLimitedRange(HeapBlock<float>& in, int inLen, float loDB, float hiDB);
        static void toRange(HeapBlock<float>& in, int inLen, float min, float max, float range);
        static void imageInColourmap(Image& image, HeapBlock<float>& data, HeapBlock<uint32>& cmap);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectrogram)
};


#endif  // SPECTROGRAM_H_INCLUDED
