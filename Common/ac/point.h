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

#ifndef __AC_POINT_H
#define __AC_POINT_H

namespace AGS { namespace Common { class Stream; } }
using namespace AGS; // FIXME later

struct _Point {
    short x, y;

    _Point()
        : x(0)
        , y(0)
    {
    }
};

#define MAXPOINTS 30
struct PolyPoints {
    int x[MAXPOINTS];
    int y[MAXPOINTS];
    int numpoints;
    void add_point(int xxx,int yyy);
    PolyPoints() { numpoints = 0; }

    void ReadFromFile(Common::Stream *in);
    void WriteToFile(Common::Stream *out) const;
};

#endif // __AC_POINT_H