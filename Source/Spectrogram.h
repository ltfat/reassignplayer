/*
  ==============================================================================

    Spectrogram.h
    Created: 10 Feb 2015 11:06:11am
    Author:  susnak

  ==============================================================================
*/

#pragma once

#include "ReassignedBLFilterbank.h"
#include <complex>

class Spectrogram : public juce::Component,
                    public juce::HighResolutionTimer,
                    public juce::Slider::Listener
{
public:
    Spectrogram();
    Spectrogram (int imageWidth, int imageHeight, int stripWidth_ = Spectrogram::defaultStripWidth);
    ~Spectrogram();

    void setColourMap (juce::HeapBlock<juce::uint32>& colourmap_);

    void paint (juce::Graphics&);
    void resized();

    void sliderValueChanged (juce::Slider* slider) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void hiResTimerCallback() override;

    void setSpectrogramSource (SpectrogramPlottable* buf);

    juce::PopupMenu& getPopupMenu()
    {
        return *pm;
    };

    void setStripWidth (int stripWidth_)
    {
        jassert (stripWidth_ % partialStripWidth == 0 && "stripWidth must be divisible by 5");
        stripWidth = stripWidth_;
        stripBackend.malloc (stripWidth * image.getHeight() * sizeof (float));
        strip = juce::Image (juce::Image::ARGB, stripWidth, image.getHeight(), true);
    }

    void startPlotting()
    {
        startTimer (1000.0 / 60.0);
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
    void setImageDimensions (int width, int height);
    void stripBackendToRepaint();
    void populatePopupMenu();
    double repaintTimeMaxMs;
    juce::Atomic<double> audioLoopMs;
    juce::Atomic<double> audioLoopMaxMs;
    int maxCountRefresh;
    juce::Font displayFont;

    juce::Image image;
    juce::Image strip;
    juce::HeapBlock<float> stripBackend;
    juce::HeapBlock<juce::uint32> colourmap;
    juce::Atomic<int> colourmapLen;
    int stripWidth;
    int stripPos;
    juce::Atomic<SpectrogramPlottable*> spectrogramSource;

    juce::ScopedPointer<juce::Graphics> imageGraphics;

    // Right-click popupmenu
    juce::ScopedPointer<juce::PopupMenu> pm;
    juce::ScopedPointer<juce::Slider> minmaxdb;
    juce::ScopedPointer<juce::Label> minmaxdbLabel;
    juce::ScopedPointer<juce::Slider> speedSlider;
    double oldMidDB;
    juce::Atomic<double> minDB, maxDB, midDB;
    double initMinDB, initMaxDB;

    juce::CriticalSection objectLock;

    juce::Atomic<int> spectrogramSourceIsValid;
    juce::Atomic<int> stripIsValid;
    juce::Atomic<int> nextStripIsValid;
    int partialStripCounter;
    int partialStripWidth;

    class MathOp
    {
    public:
        static void toDB (juce::HeapBlock<float>& in, int inLen);
        static void toLimitedRange (juce::HeapBlock<float>& in, int inLen, float loDB, float hiDB);
        static void toRange (juce::HeapBlock<float>& in, int inLen, float min, float max, float range);
        static void imageInColourmap (juce::Image& image, juce::HeapBlock<float>& data, juce::HeapBlock<juce::uint32>& cmap);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectrogram)
};
