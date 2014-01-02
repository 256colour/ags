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
//
// Helper math functions
//
//=============================================================================
#ifndef __AGS_CN_UTIL__MATH_H
#define __AGS_CN_UTIL__MATH_H

namespace AGS
{
namespace Common
{

namespace Math
{
    template <class T>
    inline int Max(T a, T b)
    {
        return a > b ? a : b;
    }

    template <class T>
    inline int Min(T a, T b)
    {
        return a > b ? b : a;
    }

    template <class T>
    inline void Clamp(T floor, T ceil, T &val)
    {
        val = Max<T>(floor, Min<T>(val, ceil));
    }

    template <class T>
    inline void ClampLength(T floor, T height, T &from, T &length)
    {
        if (from < floor)
        {
            length -= floor - from;
            from = floor;
        }
        else if (from >= floor + height)
        {
            from = 0;
            length = 0;
        }

        length = Max<T>(length, 0);
        length = Min<T>(length, height - from);
    }

    template <class T>
    inline T Surplus(T larger, T smaller)
    {
        return larger > smaller ? larger - smaller : 0;
    }

} // namespace Math

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_UTIL__MATH_H
