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
const int Spectrogram::defaultStripWidth = 10;

//==============================================================================
Spectrogram::Spectrogram():
   Spectrogram(Spectrogram::defaultImageWidth, Spectrogram::defaultImageHeight)
{
}

Spectrogram::Spectrogram(int imageWidth, int imageHeight, int stripWidth_):
   stripPos(0),
   stripWidth(stripWidth_),
   image(Image(Image::ARGB, imageWidth, imageHeight, true)),
   strip(Image(Image::ARGB, stripWidth_, imageHeight, true))
{
   image.clear(Rectangle<int>(0, 0, imageWidth, imageHeight), Colours::chocolate);
   strip.clear(Rectangle<int>(0, 0, stripWidth, imageHeight), Colours::lightblue);
   stripBackend.malloc(stripWidth * imageHeight * sizeof(float));
   imageGraphics = new Graphics(image);

   // Do this better
   colourmapLen = ltfat::cmap1Len;
   colourmap = HeapBlock<uint32>(colourmapLen.get());
   memcpy(colourmap.getData(), ltfat::cmap1, ltfat::cmap1Len * sizeof(uint32));
}

Spectrogram::~Spectrogram()
{
   imageGraphics = nullptr;
   spectrogramThread = nullptr;
}

void Spectrogram::appendStrip(const std::complex<float>* coefs[], int Lc[], int M)
{
   // Interpolate to the stripBackend
   MathOp::copyToBackend(coefs, Lc, M,
                         stripBackend, strip.getWidth(), strip.getHeight());

   // This can run in separate thread except stripBackend is used further
   int totalStripElNo = strip.getWidth() * strip.getHeight();

   // Inplace convert to DB
   MathOp::toDB(stripBackend, totalStripElNo );
   // Inplace limit to range
   MathOp::toLimitedRange(stripBackend, totalStripElNo, -70.0f, 20.0f);
   // Inplace convert to integer range [0,colourmapLen] 
   MathOp::toRange(stripBackend, totalStripElNo , -70.0f, 20.0f, colourmapLen.get());
   // Use colourmap to fill the strip image
   MathOp::imageInColourmap(strip, stripBackend, colourmap);

   // strip is now ready to be plotted
   // Lock the rest as we will draw to image
   const ScopedLock imageGraphicsLock(objectLock);
   imageGraphics->drawImage(strip,
                            stripPos * stripWidth, 0, stripWidth,       image.getHeight(),
                            0                    , 0, strip.getWidth(), strip.getHeight());
   stripPos++;
   if (stripPos * stripWidth >= image.getWidth())
   {
      strip.clear(Rectangle<int>(0, 0, stripWidth, image.getHeight()), Colours::red.darker());
      stripPos = 0;
   }
   // Somehow forece repaint
   const MessageManagerLock mml (Thread::getCurrentThread());
   repaint();
}
void Spectrogram::setStripWidth(int stripWidth_) {}
void Spectrogram::setColourMap(HeapBlock<uint32>& colourmap_)
{
   //colourmap = colourmap_;
}

// Just repaint according to the stripPos position
void Spectrogram::paint (Graphics& g)
{
   // Lock the rest as we read from image
   const ScopedLock imageGraphicsLock(objectLock);
   int stripPosInPix = stripPos * stripWidth;
   float stripPosRel = ((float)stripPosInPix) / image.getWidth();

   g.drawImage(image, 0, 0, getWidth() * (1 - stripPosRel), getHeight(),
               stripPosInPix, 0, image.getWidth() - stripPosInPix, image.getHeight());

   g.drawImage(image, (1 - stripPosRel)*getWidth(), 0, stripPosRel * getWidth(), getHeight(),
               0, 0, stripPosInPix, image.getHeight());
}

// There are no child Components
void Spectrogram::resized() { }

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
      *ptr = 20 * log10(*ptr **ptr + 1e-10);
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
   float multFac = range / (max - min);
   for (int ii = 0; ii < inLen; ++ii)
   {
      *ptr = multFac * (*ptr - min);
      ptr++;
   }

}

void Spectrogram::MathOp::imageInColourmap(Image& image, HeapBlock<float>& data,
      HeapBlock<uint32>& cmap)
{
   int width = image.getWidth();
   int height = image.getHeight();
   Image::BitmapData idata(image, Image::BitmapData::writeOnly);

   for (int y = 0; y < height; ++y)
   {
      uint32* rowPtr = reinterpret_cast<uint32*>(idata.getLinePointer(y));
      float* dataPtr = data.getData() + y * width;

      for (int x = 0; x < width; ++x)
      {
         *rowPtr = cmap[static_cast<int>(*dataPtr)];
         rowPtr++;
         dataPtr++;
      }
   }
}

