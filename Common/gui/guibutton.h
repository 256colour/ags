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

#ifndef __AC_GUIBUTTON_H
#define __AC_GUIBUTTON_H

#include <vector>
#include "gui/guiobject.h"
#include "util/string.h"

#define GUIBUTTON_TEXTLENGTH 50

namespace AGS
{
namespace Common
{

enum MouseButton
{
    kMouseNone  = -1,
    kMouseLeft  =  0,
    kMouseRight =  1,
};

enum GUIClickAction
{
    kGUIAction_None       = 0,
    kGUIAction_SetMode    = 1,
    kGUIAction_RunScript  = 2,
};

// TODO: generic alignment
enum GUIButtonAlignment
{
    kButtonAlign_TopCenter     = 0,
    kButtonAlign_TopLeft       = 1,
    kButtonAlign_TopRight      = 2,
    kButtonAlign_CenterLeft    = 3,
    kButtonAlign_Centered      = 4,
    kButtonAlign_CenterRight   = 5,
    kButtonAlign_BottomLeft    = 6,
    kButtonAlign_BottomCenter  = 7,
    kButtonAlign_BottomRight   = 8,
};

class GUIButton : public GUIObject
{
public:
    GUIButton();

    const String &GetText() const;

    // Operations
    virtual void Draw(Bitmap *ds) override;
    void         SetText(const String &text);

    // Events
    virtual bool OnMouseDown() override;
    virtual void OnMouseEnter() override;
    virtual void OnMouseLeave() override;
    virtual void OnMouseUp() override;
  
    // Serialization
    virtual void WriteToFile(Stream *out) override;
    virtual void ReadFromFile(Stream *in, GuiVersion gui_version) override;

// TODO: these members are currently public; hide them later
public:
    int32_t     Image;
    int32_t     MouseOverImage;
    int32_t     PushedImage;
    int32_t     CurrentImage;
    int32_t     Font;
    color_t     TextColor;
    int32_t     TextAlignment;
    // Click actions for left and right mouse buttons
    // NOTE: only left click is currently in use
    static const int ClickCount = kMouseRight + 1;
    GUIClickAction ClickAction[ClickCount];
    int32_t        ClickData[ClickCount];

    bool        IsPushed;
    bool        IsMouseOver;

private:
    void DrawImageButton(Bitmap *ds, bool draw_disabled);
    void DrawText(Bitmap *ds, bool draw_disabled);
    void DrawTextButton(Bitmap *ds, bool draw_disabled);
    void PrepareTextToDraw();

    // Defines button placeholder mode; the mode is set
    // depending on special tags found in button text
    enum GUIButtonPlaceholder
    {
        kButtonPlace_None,
        kButtonPlace_InvItemStretch,
        kButtonPlace_InvItemCenter,
        kButtonPlace_InvItemAuto
    };

    // Text property set by user
    String _text;
    // type of content placeholder, if any
    GUIButtonPlaceholder _placeholder;
    // A flag indicating unnamed button; this is a convenience trick:
    // buttons are created named "New Button" in the editor, and users
    // often do not clear text when they want a graphic button.
    bool _unnamed;
    // Prepared text buffer/cache
    String _textToDraw;
};

} // namespace Common
} // namespace AGS

extern std::vector<AGS::Common::GUIButton> guibuts;
extern int numguibuts;

int UpdateAnimatingButton(int bu);
void StopButtonAnimation(int idxn);

#endif // __AC_GUIBUTTON_H
