//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "util/wgt2allg.h"
#include "gfx/gfxfilter.h"
#include "gfx/gfxdriverbase.h"
#include "gfx/bitmap.h"


namespace AGS
{
namespace Engine
{

GraphicsDriverBase::GraphicsDriverBase()
    : _global_x_offset(0)
    , _global_y_offset(0)
    , _loopTimer(NULL)
    , _pollingCallback(NULL)
    , _drawScreenCallback(NULL)
    , _nullSpriteCallback(NULL)
    , _initGfxCallback(NULL)
{
}

bool GraphicsDriverBase::IsModeSet() const
{
    return _mode.Width != 0 && _mode.Height != 0 && _mode.ColorDepth != 0;
}

bool GraphicsDriverBase::IsNativeSizeValid() const
{
    return !_srcRect.IsEmpty();
}

bool GraphicsDriverBase::IsRenderFrameValid() const
{
    return !_srcRect.IsEmpty() && !_dstRect.IsEmpty();
}

DisplayMode GraphicsDriverBase::GetDisplayMode() const
{
    return _mode;
}

Size GraphicsDriverBase::GetNativeSize() const
{
    return _srcRect.GetSize();
}

Rect GraphicsDriverBase::GetRenderDestination() const
{
    return _dstRect;
}

void GraphicsDriverBase::SetRenderOffset(int x, int y)
{
    _global_x_offset = x;
    _global_y_offset = y;
}

void GraphicsDriverBase::OnInit(volatile int *loopTimer)
{
    _loopTimer = loopTimer;
}

void GraphicsDriverBase::OnUnInit()
{
}

void GraphicsDriverBase::OnModeSet(const DisplayMode &mode)
{
    _mode = mode;
}

void GraphicsDriverBase::OnModeReleased()
{
    _mode = DisplayMode();
    _dstRect = Rect();
}

void GraphicsDriverBase::OnScalingChanged()
{
    PGfxFilter filter = GetGraphicsFilter();
    if (filter)
        _filterRect = filter->SetTranslation(_srcRect.GetSize(), _dstRect);
    else
        _filterRect = Rect();
    _scaling.Init(_srcRect.GetSize(), _dstRect);
}

void GraphicsDriverBase::OnSetNativeSize(const Size &src_size)
{
    _srcRect = RectWH(0, 0, src_size.Width, src_size.Height);
    OnScalingChanged();
}

void GraphicsDriverBase::OnSetRenderFrame(const Rect &dst_rect)
{
    _dstRect = dst_rect;
    OnScalingChanged();
}

void GraphicsDriverBase::OnSetFilter()
{
    _filterRect = GetGraphicsFilter()->SetTranslation(Size(_srcRect.GetSize()), _dstRect);
}

Bitmap *GraphicsDriverBase::ReplaceBitmapWithSupportedFormat(Bitmap *old_bmp)
{
    Bitmap *new_bitmap = ConvertBitmapToSupportedColourDepth(old_bmp);
    if (new_bitmap != old_bmp)
        delete old_bmp;
    return new_bitmap;
}

} // namespace Engine
} // namespace AGS
