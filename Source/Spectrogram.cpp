/*
  ==============================================================================

    Spectrogram.cpp
    Created: 10 Feb 2015 11:06:11am
    Author:  susnak

  ==============================================================================
*/

#include "Spectrogram.h"
#include "ltfatColormaps.h"


const int Spectrogram::defaultImageWidth = 800;
const int Spectrogram::defaultImageHeight = 600;
const int Spectrogram::defaultStripWidth = 10;
const double Spectrogram::defaultMinDB = -70;
const double Spectrogram::defaultMaxDB = 40;


//==============================================================================
Spectrogram::Spectrogram():
    Spectrogram(Spectrogram::defaultImageWidth, Spectrogram::defaultImageHeight)
{
}

Spectrogram::Spectrogram(int imageWidth, int imageHeight, int stripWidth_):
    timerFired(0),
    repaintTimeMaxMs(0.0),
    audioLoopMs(0.0),
    audioLoopMaxMs(0.0),
    maxCountRefresh(100),
    stripPos(0),
    stripWidth(stripWidth_),
    image(Image(Image::ARGB, imageWidth, imageHeight, true )),
    strip(Image(Image::ARGB, stripWidth_, imageHeight, true )),
    minmaxdb(nullptr), speedSlider(nullptr),
    oldMidDB((defaultMinDB + defaultMaxDB) / 2.0),
    minDB(defaultMinDB), maxDB(defaultMaxDB), midDB(oldMidDB),
    spectrogramSourceIsValid(0)
{
    setBufferedToImage(true);
    stripBackend.malloc(stripWidth * imageHeight * sizeof(float));
    imageGraphics = new Graphics(image);

    displayFont = Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::bold);
    // Do this better
    colourmapLen = ltfat::cmap1Len;
    colourmap = HeapBlock<uint32>(colourmapLen.get());
    memcpy(colourmap.getData(), ltfat::cmap1, ltfat::cmap1Len * sizeof(uint32));
    image.clear(juce::Rectangle<int>(0, 0, imageWidth, imageHeight), Colour(colourmap[0]));
    strip.clear(juce::Rectangle<int>(0, 0, stripWidth, imageHeight), Colour(colourmap[0]));

    populatePopupMenu();
    // startThread();
    startTimer(1000.0 / 60.0);
}

void Spectrogram::populatePopupMenu()
{
    double range = defaultMaxDB - defaultMinDB;
    initMinDB = defaultMinDB - range ;
    initMaxDB = defaultMaxDB + range ;

    pm = new PopupMenu();
    minmaxdb = new Slider();
    minmaxdb->setSliderStyle(Slider::ThreeValueHorizontal);
    minmaxdb->setRange(initMinDB, initMaxDB, 1);
    minmaxdb->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    minmaxdb->setPopupMenuEnabled(true);
    minmaxdb->setMaxValue(maxDB.get(), dontSendNotification, true);
    minmaxdb->setMinValue(minDB.get(), dontSendNotification, true);
    minmaxdb->setValue(midDB.get(), dontSendNotification);
    minmaxdbLabel = new Label("Range", "Range:");
    pm->addCustomItem(1, minmaxdbLabel, 220, 30, false);
    pm->addCustomItem(2, minmaxdb, 220, 30, false);


    minmaxdb->addListener(this);
}

Spectrogram::~Spectrogram()
{
    DBG("Spectrogram desctructor");
    minmaxdbLabel = nullptr;
    minmaxdb = nullptr;
    pm = nullptr;
    // stopThread(1000);
    stopTimer();
    while (isTimerRunning()) {}
    imageGraphics = nullptr;
    spectrogramThread = nullptr;
}

void Spectrogram::stripBackendToRepaint()
{

    // This can run in separate thread except stripBackend is used further
    int totalStripElNo = strip.getWidth() * strip.getHeight();

    // Inplace convert to DB
    MathOp::toDB(stripBackend, totalStripElNo );
    // Inplace limit to range
    MathOp::toLimitedRange(stripBackend, totalStripElNo, static_cast<float>(std::max( minDB.get(), initMinDB )),
                           static_cast<float>(std::min(maxDB.get(), initMaxDB)));
    // Inplace convert to integer range [0,colourmapLen]
    MathOp::toRange(stripBackend, totalStripElNo, static_cast<float>(std::max(minDB.get(), initMinDB)),
                    static_cast<float>(std::min(maxDB.get(), initMaxDB)),
                    static_cast<float>(colourmapLen.get() - 1));


    {
        // Strip is read in repaint, we must avoid rewriting it when it is read in
        // repaint
        const ScopedLock imageGraphicsLock(objectLock);
        MathOp::imageInColourmap(strip, stripBackend, colourmap);

        int stripPosLoc = stripPos.get() + 1;
        if (stripPosLoc * stripWidth >= image.getWidth())
        {
            stripPosLoc = 0;
        }

        // This is an atomic operation
        stripPos = stripPosLoc;
    }


    // repaint should be called from the message thread
    /*
     * Using only
     *  const MessageManagerLock mmLock();
     *  fails sometimes when constructor and destructor are called
     *  rapidly.
     */
    // const MessageManagerLock mmLock(Thread::getCurrentThread());
    // if (mmLock.lockWasGained())
    repaint();
}



void Spectrogram::appendStrip(const std::complex<float>* coefs[], int Lc[], int M)
{
    // Interpolate to the stripBackend
    //MathOp::copyToBackend(coefs, Lc, M,
    //                      stripBackend, strip.getWidth(), strip.getHeight());
    stripBackendToRepaint();
}

void Spectrogram::appendStrip(const std::complex<float> coefs[], int M)
{
    // Interpolate to the stripBackend
    //
    // MathOp::copyToBackend(coefs, M,
    //                      stripBackend, strip.getWidth(), strip.getHeight());
    stripBackendToRepaint();
}

void Spectrogram::setStripWidth(int stripWidth_) {}
void Spectrogram::setColourMap(HeapBlock<uint32>& colourmap_)
{
    //colourmap = colourmap_;
}

// Just repaint according to the stripPos position
void Spectrogram::paint (Graphics& g)
{
    static int counter = 0;
    double startTime = Time::getMillisecondCounterHiRes();

    // Ensure we are using the same stripPos during the paint method
    // stripPos is atomic
    int stripPosLoc = stripPos.get();
    int stripPosLocMinOneInPix = (stripPosLoc - 1) * stripWidth;

    if ( stripPosLocMinOneInPix < 0)
    {
        stripPosLocMinOneInPix += image.getWidth();
    }

    {
        // Lock. Strip are shared between threads
        const ScopedLock imageGraphicsLock(objectLock);

        // Draw strip to the image first
        imageGraphics->drawImage(strip,
                                 stripPosLocMinOneInPix , 0, stripWidth,       image.getHeight(),
                                 0                      , 0, strip.getWidth(), strip.getHeight());
    }

    // Draw the strip itself
    int stripPosInPix = stripPosLoc * stripWidth;
    float stripPosRel = ((float)stripPosInPix) / image.getWidth();

    g.drawImage(image, 0, 0, static_cast<int>(getWidth() * (1.0f - stripPosRel)), getHeight(),
                stripPosInPix, 0, image.getWidth() - stripPosInPix, image.getHeight());

    g.drawImage(image, static_cast<int>((1.0f - stripPosRel)*getWidth()), 0, stripPosRel * getWidth(), getHeight(),
                0, 0, stripPosInPix, image.getHeight());


    double now = Time::getMillisecondCounterHiRes();
    const double elapsedMs = now - startTime;
    if (counter > maxCountRefresh)
    {
        counter = 0;
        repaintTimeMaxMs = 0.0;
    }
    counter++;
    if (elapsedMs > repaintTimeMaxMs )
    {
        repaintTimeMaxMs = elapsedMs;
    }


    GlyphArrangement ga;
    ga.addFittedText (displayFont,
                      "Repaint : " + String (elapsedMs, 1) + " ms"
                      + "\nMax. repaint : " + String(repaintTimeMaxMs, 1) + " ms"
                      + "\nReassignment : " + String(audioLoopMs.get(), 1) + " ms"
                      + "\nMax. reassignment : " + String(audioLoopMaxMs.get(), 1) + " ms",
                      0, 10.0f, getWidth() - 10.0f, (float) getHeight(), Justification::topRight, 3);

    g.setColour (Colours::white.withAlpha (0.3f));
    g.fillRect (ga.getBoundingBox (0, ga.getNumGlyphs(), true).getSmallestIntegerContainer().expanded (4));

    g.setColour (Colours::black);
    ga.draw (g);
}

void Spectrogram::resized()
{
    // There are no child Components
    // We are currently on the Message thread
    // repaint();
}

void Spectrogram::timerCallback()
{
hiResTimerCallback();
}



void Spectrogram::hiResTimerCallback()
{
    // We are on the Message thread, just signalise new data is available and exit
    // Atomic
    // timerFired = 1;
    if (spectrogramSourceIsValid.get() == 1 && nullptr != spectrogramSource.get())
    {
        double startTime = Time::getMillisecondCounterHiRes();

        // Check new data
        bool isValid = spectrogramSource.get()->getBufferCoefficientsAsAbsMatrix(stripBackend,
                       strip.getWidth(), strip.getHeight());

        double now = Time::getMillisecondCounterHiRes();

        if (isValid)
        {
            // New data is available
            stripBackendToRepaint();
            audioLoopMs.set(now - startTime);
        }
    }

}


bool Spectrogram::aboutToChangeSpectrogramSource()
{
    // Change the value to 0 only if it is 1
    return spectrogramSourceIsValid.compareAndSetBool(0, 1);
}

// No thread safety here. Can only be used for the very first call
void Spectrogram::setSpectrogramSource(SpectrogramPlottable* buf)
{
    spectrogramSource.set(buf);
    spectrogramSourceIsValid.set(1);
}

// Safely replace ringBuf
// It is sucessfull only when
bool Spectrogram::trySetSpectrogramSource(SpectrogramPlottable* buf)
{
    // Change ringBuffer only if it is already nullpt   // Change ringBuffer only if it is
    // already nullptrr
    bool retval =  spectrogramSource.compareAndSetBool(buf, nullptr);
    if (retval) spectrogramSourceIsValid = 1;
    return retval;
}

void Spectrogram::mouseDown(const MouseEvent & event)
{
    if ( event.mods == ModifierKeys::rightButtonModifier)
    {
        pm->show();
    }
}

void Spectrogram::sliderValueChanged(Slider* slider)
{
    if (slider == minmaxdb)
    {
        if (midDB.get() != slider->getValue())
        {
            double change = slider->getValue() - midDB.get();
            midDB = slider->getValue();
            minDB = minDB.get() + change;
            maxDB = maxDB.get() + change;
            slider->setMaxValue(maxDB.get(), dontSendNotification, true);
            slider->setMinValue(minDB.get(), dontSendNotification, true);
        }
        else
        {
            minDB = slider->getMinValue();
            maxDB = slider->getMaxValue();
            midDB = ( minDB.get() + maxDB.get()) / 2.0;
            slider->setValue(midDB.get(), dontSendNotification);
        }
    }
}


void Spectrogram::MathOp::copyToBackend(const std::complex<float> c[], int M,
                                        HeapBlock<float>& data, int stripWidth, int stripHeight)
{
#define DATAEL(m,ii) (*(data + stripWidth*m + ii))

    float rowsRatio = static_cast<float>(M - 1) / stripHeight;
    for (int m = 0; m < stripHeight; ++m)
    {
        float tmpyPrec = rowsRatio * m;
        int tmpy = static_cast<int>(tmpyPrec);
        tmpyPrec -= tmpy;
        tmpy = M - 1 - tmpy;
        for (int ii = 0; ii < stripWidth; ++ii)
        {
            DATAEL(m, ii) =  (1.0f - tmpyPrec) * std::abs(c[tmpy]) + tmpyPrec * std::abs(c[tmpy + 1]);
        }
    }
#undef DATAEL

}

// This is supposed to do a 2D interpolation
void Spectrogram::MathOp::copyToBackend(const std::complex<float>* c[], int Lc[], int M,
                                        HeapBlock<float>& data, int stripWidth, int stripHeight)
{
#define DATAEL(m,ii) (*(data + stripWidth*m + ii))
    // -1 to avoid getting out of bounds
    float rowsRatio = static_cast<float>(M - 1) / stripHeight;
    HeapBlock<float> colsRatios(M);
    for (int m = 0; m < M; ++m)
        colsRatios[m] = static_cast<float>(Lc[m] - 1) / stripWidth;

    for (int m = 0; m < stripHeight; ++m)
    {
        float tmpyPrec = rowsRatio * m;
        int tmpy = static_cast<int>(tmpyPrec);
        tmpyPrec -= tmpy;
        tmpy = M - 1 - tmpy;
        for (int ii = 0; ii < stripWidth; ++ii)
        {
            float tmpxPrec = colsRatios[ii] * ii;
            int tmpx = static_cast<int>(tmpxPrec);
            tmpxPrec -= tmpx;
            float intx1 =  (1.0f - tmpxPrec) * std::abs(c[tmpy][tmpx]) + tmpxPrec * std::abs(c[tmpy][tmpx + 1]);
            float intx2 =  (1.0f - tmpxPrec) * std::abs(c[tmpy + 1][tmpx]) + tmpxPrec * std::abs(c[tmpy + 1][tmpx + 1]);
            DATAEL(m, ii) = (1.0f - tmpyPrec) * intx1 + tmpyPrec * intx2;
        }
    }
#undef DATAEL
}

void Spectrogram::MathOp::toDB(HeapBlock<float>& in, int inLen)
{
    float* ptr = in.getData();
    for (int ii = 0; ii < inLen; ++ii)
    {
        *ptr = 20.0f * static_cast<float>(std::log10(*ptr **ptr + 1e-10));
        ptr++;
    }
}

void Spectrogram::MathOp::toLimitedRange(HeapBlock<float>& in, int inLen,
        float loDB, float hiDB)
{
    float* ptr = in.getData();
    for (int ii = 0; ii < inLen; ++ii)
    {
        *ptr = std::min(*ptr, hiDB);
        *ptr = std::max(*ptr, loDB);
        ptr++;
    }
}

void Spectrogram::MathOp::toRange(HeapBlock<float>& in, int inLen,
                                  float min, float max, float range)
{
    float* ptr = in.getData();
    if (min >= max)
    {
        for (int ii = 0; ii < inLen; ++ii)
        {
            ptr[ii] = min;
        }
    }
    else
    {
        float multFac = range / (max - min);
        for (int ii = 0; ii < inLen; ++ii)
        {
            *ptr = multFac * (*ptr - min);
            ptr++;
        }
    }

}

void Spectrogram::MathOp::imageInColourmap(Image& image, HeapBlock<float>& data,
        HeapBlock<uint32>& cmap)
{
    // The image is upside down
    int width = image.getWidth();
    int height = image.getHeight();
    // Jules says:
    // A BitmapData object should be used as a temporary, stack-based object.
    // Don't keep one hanging around while the image is being used elsewhere.
    Image::BitmapData idata(image, Image::BitmapData::writeOnly);


    for (int y = 0; y < height; ++y)
    {
        uint32* rowPtr = reinterpret_cast<uint32*>(idata.getLinePointer(y));
        float* dataPtr = data.getData() + (height - 1 - y) * width;

        for (int x = 0; x < width; ++x)
        {
            int dataInt = static_cast<int>(*dataPtr);
            *rowPtr = cmap[dataInt];
            rowPtr++;
            dataPtr++;
        }
    }
}
