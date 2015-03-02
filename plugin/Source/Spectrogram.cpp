/*
  ==============================================================================

    Spectrogram.cpp
    Created: 10 Feb 2015 11:06:11am
    Author:  susnak

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Spectrogram.h"
#include "ltfatColormaps.h"


const int Spectrogram::defaultImageWidth = 800;
const int Spectrogram::defaultImageHeight = 600;
const int Spectrogram::defaultStripWidth = 5;
const double Spectrogram::defaultMinDB = -70;
const double Spectrogram::defaultMaxDB = 40;


//==============================================================================
Spectrogram::Spectrogram():
    Spectrogram(Spectrogram::defaultImageWidth, Spectrogram::defaultImageHeight)
{
}

Spectrogram::Spectrogram(int imageWidth, int imageHeight, int stripWidth_):
    Thread("Spectrogram_thread"),
    timerFired(0),
    repaintTimeMaxMs(0.0),
    audioLoopMs(0.0),
    audioLoopMaxMs(0.0),
    maxCountRefresh(100),
    stripPos(0),
    stripWidth(stripWidth_),
    image(Image(Image::ARGB, imageWidth, imageHeight, true )),
    strip(Image(Image::ARGB, stripWidth_, imageHeight, true )),
    ringBuffer(nullptr),
    minmaxdb(nullptr), speedSlider(nullptr),
    oldMidDB((defaultMinDB + defaultMaxDB) / 2.0),
    minDB(defaultMinDB), maxDB(defaultMaxDB), midDB(oldMidDB)
{
    setBufferedToImage(true);
    stripBackend.malloc(stripWidth * imageHeight * sizeof(float));
    imageGraphics = new Graphics(image);

    displayFont = Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::bold);
    // Do this better
    colourmapLen = ltfat::cmap1Len;
    colourmap = HeapBlock<uint32>(colourmapLen.get());
    memcpy(colourmap.getData(), ltfat::cmap1, ltfat::cmap1Len * sizeof(uint32));
    image.clear(Rectangle<int>(0, 0, imageWidth, imageHeight), Colour(colourmap[0]));
    strip.clear(Rectangle<int>(0, 0, stripWidth, imageHeight), Colour(colourmap[0]));

    populatePopupMenu();
    startThread();
    startTimerHz(60);
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
    minmaxdbLabel->attachToComponent(minmaxdb, true);
    pm->addCustomItem(1, minmaxdb, 130, 30, false);

    minmaxdb->addListener(this);
}

Spectrogram::~Spectrogram()
{
    stopThread(1000);
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
    MathOp::toLimitedRange(stripBackend, totalStripElNo, std::max( minDB.get(), initMinDB ),
                           std::min( maxDB.get(), initMaxDB ));
    // Inplace convert to integer range [0,colourmapLen]
    MathOp::toRange(stripBackend, totalStripElNo , std::max( minDB.get(), initMinDB ),
                    std::min( maxDB.get(), initMaxDB ),
                    colourmapLen.get() - 1);


    // Use colourmap to fill the strip image
    MathOp::imageInColourmap(strip, stripBackend, colourmap);
    // strip is now ready to be plotted
    // Lock the rest as we will draw to image
    // const ScopedLock imageGraphicsLock(objectLock);

    //const MessageManagerLock mml (Thread::getCurrentThread());
    ++stripPos;
    if (stripPos.get() * stripWidth >= image.getWidth())
    {
        stripPos = 0;
    }

    const MessageManagerLock mmLock;
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
    int stripPosLoc = stripPos.get();

    // Draw strip to the image first
    imageGraphics->drawImage(strip,
                             stripPosLoc * stripWidth, 0, stripWidth,       image.getHeight(),
                             0                       , 0, strip.getWidth(), strip.getHeight());

    // Draw the strip itself
    int stripPosInPix = stripPos.get() * stripWidth;
    float stripPosRel = ((float)stripPosInPix) / image.getWidth();

    g.drawImage(image, 0, 0, getWidth() * (1 - stripPosRel), getHeight(),
                stripPosInPix, 0, image.getWidth() - stripPosInPix, image.getHeight());

    g.drawImage(image, (1 - stripPosRel)*getWidth(), 0, stripPosRel * getWidth(), getHeight(),
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
                      + "\nAudioloop : " + String(audioLoopMs.get(), 1) + " ms"
                      + "\nMax. audioloop : " + String(audioLoopMaxMs.get(), 1) + " ms",
                      0, 10.0f, getWidth() - 10.0f, (float) getHeight(), Justification::topRight, 3);

    g.setColour (Colours::white.withAlpha (0.3f));
    g.fillRect (ga.getBoundingBox (0, ga.getNumGlyphs(), true).getSmallestIntegerContainer().expanded (4));

    g.setColour (Colours::black);
    ga.draw (g);
}

void Spectrogram::resized()
{
    // There are no child Components
    repaint();
}
void Spectrogram::timerCallback()
{
    // We are on the Message thread, just signalise new data and exit
    timerFired = 1;
}

void Spectrogram::run()
{
    while (!threadShouldExit())
    {
        if (1 == timerFired.get())
        {
            if (nullptr != ringBuffer)
            {
                bool isValid = ringBuffer->getBufferCoefficientsAsAbsMatrix(stripBackend,
                               strip.getWidth(), strip.getHeight());
                if (isValid)
                {
                    stripBackendToRepaint();
                }
            }


            timerFired = 0;
        }
    }
}

void Spectrogram::mouseDown(const MouseEvent & event)
{
    if ( event.mods == ModifierKeys::rightButtonModifier)
    {
        pm->show();
        //pm->showMenuAsync(PopupMenu::Options(), this);
    }
}

void Spectrogram::modalStateFinished(int rValue)
{
//  DBG("Popupmenu closed");
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
            float intx1 =  (1.0f - tmpxPrec) * std::fabs(c[tmpy][tmpx]) + tmpxPrec * std::fabs(c[tmpy][tmpx + 1]);
            float intx2 =  (1.0f - tmpxPrec) * std::fabs(c[tmpy + 1][tmpx]) + tmpxPrec * std::fabs(c[tmpy + 1][tmpx + 1]);
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
        *ptr = 20.0 * std::log10(*ptr **ptr + 1e-10);
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
        float* dataPtr = data.getData() + (height-1 - y) * width;

        for (int x = 0; x < width; ++x)
        {
            int dataInt = static_cast<int>(*dataPtr);
            *rowPtr = cmap[dataInt];
            rowPtr++;
            dataPtr++;
        }
    }
}

