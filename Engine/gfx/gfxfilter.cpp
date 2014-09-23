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

#include "gfx/gfxfilter.h"

// Standard do-nothing filter

namespace AGS
{
namespace Engine
{

GfxFilterInfo::GfxFilterInfo()
    : FixedScale(0)
{
}

GfxFilterInfo::GfxFilterInfo(String id, String name, uint32_t fixed_scale)
    : Id(id)
    , Name(name)
    , FixedScale(fixed_scale)
{
}

GfxFilter::~GfxFilter()
{
}

bool GfxFilter::Initialize(const int color_depth, String &err_str)
{
    return true;  // always succeeds
}

void GfxFilter::UnInitialize()
{
    // do nothing
}

} // namespace Engine
} // namespace AGS
