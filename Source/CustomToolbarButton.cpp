/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/
#include "CustomToolbarButton.h"

CustomToolbarButton::CustomToolbarButton (const int iid, const String& buttonText,
                              Array<Drawable*> imagesToUse_)
   : ToolbarItemComponent (iid, buttonText, true),
     currentImage (nullptr),
     currentState (1)
{
   jassert (imagesToUse_.size() != 0);
   DBG(imagesToUse_.size());

   for (int kk = 0; kk < imagesToUse_.size(); ++kk)
   {
       DBG(kk);
       imagesToUse.add(imagesToUse_[kk]);
   }

   currentImage = imagesToUse[0];
   //buttonStateChanged();
}

CustomToolbarButton::~CustomToolbarButton()
{
}

//==============================================================================
bool CustomToolbarButton::getToolbarItemSizes (int toolbarDepth, bool /*isToolbarVertical*/, int& preferredSize, int& minSize, int& maxSize)
{
    preferredSize = minSize = maxSize = toolbarDepth;
    return true;
}

void CustomToolbarButton::paintButtonArea (Graphics&, int /*width*/, int /*height*/, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
}

void CustomToolbarButton::contentAreaChanged (const Rectangle<int>&)
{
    buttonStateChanged();
}

void CustomToolbarButton::setCurrentImage (Drawable* const newImage)
{
    if (newImage != currentImage)
    {
        removeChildComponent (currentImage);
        currentImage = newImage;

        if (currentImage != nullptr)
        {
            enablementChanged();
            addAndMakeVisible (currentImage);
            updateDrawable();
        }
    }
}

void CustomToolbarButton::updateDrawable()
{
    if (currentImage != nullptr)
    {
        currentImage->setInterceptsMouseClicks (false, false);
        currentImage->setTransformToFit (getContentArea().toFloat(), RectanglePlacement::centred);
        currentImage->setAlpha (isEnabled() ? 1.0f : 0.5f);
    }
}

void CustomToolbarButton::resized()
{
    ToolbarItemComponent::resized();
    updateDrawable();
}

void CustomToolbarButton::enablementChanged()
{
    ToolbarItemComponent::enablementChanged();
    updateDrawable();
}

Drawable* CustomToolbarButton::getImageToUse() const
{
    if (getStyle() == Toolbar::textOnly)
        return nullptr;

    /*if (getToggleState() && toggledOnImage != nullptr)
        return toggledOnImage;

    return normalImage;*/
    DBG(currentState);
    return imagesToUse[currentState];
}

void CustomToolbarButton::buttonStateChanged()
{
    setCurrentImage (getImageToUse());
}

void CustomToolbarButton::advanceState()
{
    currentState = (currentState+1) % imagesToUse.size();
    buttonStateChanged();
}
