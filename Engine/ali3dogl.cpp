#include <stdio.h>
#include <allegro.h>
#if defined(WINDOWS_VERSION)
#include <winalleg.h>
#include <allegro/platform/aintwin.h>
#include <ali3d.h>
#include <GL/gl.h>
#elif defined(ANDROID_VERSION)
#include <ali3d.h>
#include <GLES/gl.h>

#define HDC void*
#define HGLRC void*
#define HWND void*
#define HINSTANCE void*

#define glOrtho glOrthof
#define GL_CLAMP GL_CLAMP_TO_EDGE

#define Sleep(x) usleep(x * 1000)

// Defined in Allegro
extern "C" 
{
  void android_swap_buffers();
  void android_create_screen(int width, int height, int color_depth);
}

#endif
extern "C" void android_debug_printf(char* format, ...);

extern int psp_gfx_smoothing;
extern int psp_gfx_scaling;

extern unsigned int android_screen_physical_width;
extern unsigned int android_screen_physical_height;
extern int android_screen_initialized;


void ogl_dummy_vsync() { }


typedef struct _OGLVECTOR {
    float x;
    float y;
} OGLVECTOR2D;


struct OGLCUSTOMVERTEX
{
    OGLVECTOR2D position;
    float tu;
    float tv;
};



#define MAX_DRAW_LIST_SIZE 200

#define algetr32(xx) ((xx >> _rgb_r_shift_32) & 0xFF)
#define algetg32(xx) ((xx >> _rgb_g_shift_32) & 0xFF)
#define algetb32(xx) ((xx >> _rgb_b_shift_32) & 0xFF)
#define algeta32(xx) ((xx >> _rgb_a_shift_32) & 0xFF)
#define algetr15(xx) ((xx >> _rgb_r_shift_15) & 0x1F)
#define algetg15(xx) ((xx >> _rgb_g_shift_15) & 0x1F)
#define algetb15(xx) ((xx >> _rgb_b_shift_15) & 0x1F)

#define GFX_OPENGL  AL_ID('O','G','L',' ')

GFX_DRIVER gfx_opengl =
{
   GFX_OPENGL,
   empty_string,
   empty_string,
   "OpenGL",
   NULL,    // init
   NULL,   // exit
   NULL,                        // AL_METHOD(int, scroll, (int x, int y)); 
   ogl_dummy_vsync,   // vsync
   NULL,  // setpalette
   NULL,                        // AL_METHOD(int, request_scroll, (int x, int y));
   NULL,                        // AL_METHOD(int, poll_scroll, (void));
   NULL,                        // AL_METHOD(void, enable_triple_buffer, (void));
   NULL,  //create_video_bitmap
   NULL,  //destroy_video_bitmap
   NULL,   //show_video_bitmap
   NULL,
   NULL,  //gfx_directx_create_system_bitmap,
   NULL, //gfx_directx_destroy_system_bitmap,
   NULL, //gfx_directx_set_mouse_sprite,
   NULL, //gfx_directx_show_mouse,
   NULL, //gfx_directx_hide_mouse,
   NULL, //gfx_directx_move_mouse,
   NULL,                        // AL_METHOD(void, drawing_mode, (void));
   NULL,                        // AL_METHOD(void, save_video_state, (void*));
   NULL,                        // AL_METHOD(void, restore_video_state, (void*));
   NULL,                        // AL_METHOD(void, set_blender_mode, (int mode, int r, int g, int b, int a));
   NULL,                        // AL_METHOD(int, fetch_mode_list, (void));
   0, 0,                        // int w, h;
   FALSE,                        // int linear;
   0,                           // long bank_size;
   0,                           // long bank_gran;
   0,                           // long vid_mem;
   0,                           // long vid_phys_base;
   TRUE                         // int windowed;
};


struct TextureTile
{
  int x, y;
  int width, height;
  unsigned int texture;
};

class OGLBitmap : public IDriverDependantBitmap
{
public:
  // Transparency is a bit counter-intuitive
  // 0=not transparent, 255=invisible, 1..254 barely visible .. mostly visible
  virtual void SetTransparency(int transparency) { _transparency = transparency; }
  virtual void SetFlippedLeftRight(bool isFlipped) { _flipped = isFlipped; }
  virtual void SetStretch(int width, int height) 
  {
    _stretchToWidth = width;
    _stretchToHeight = height;
  }
  virtual int GetWidth() { return _width; }
  virtual int GetHeight() { return _height; }
  virtual int GetColorDepth() { return _colDepth; }
  virtual void SetLightLevel(int lightLevel)  { _lightLevel = lightLevel; }
  virtual void SetTint(int red, int green, int blue, int tintSaturation) 
  {
    _red = red;
    _green = green;
    _blue = blue;
    _tintSaturation = tintSaturation;
  }

  int _width, _height;
  int _colDepth;
  bool _flipped;
  int _stretchToWidth, _stretchToHeight;
  int _red, _green, _blue;
  int _tintSaturation;
  int _lightLevel;
  bool _opaque;
  bool _hasAlpha;
  int _transparency;
  OGLCUSTOMVERTEX* _vertex;
  TextureTile *_tiles;
  int _numTiles;

  OGLBitmap(int width, int height, int colDepth, bool opaque)
  {
    _width = width;
    _height = height;
    _colDepth = colDepth;
    _flipped = false;
    _hasAlpha = false;
    _stretchToWidth = 0;
    _stretchToHeight = 0;
    _tintSaturation = 0;
    _lightLevel = 0;
    _transparency = 0;
    _opaque = opaque;
    _vertex = NULL;
    _tiles = NULL;
    _numTiles = 0;
  }

  int GetWidthToRender() { return (_stretchToWidth > 0) ? _stretchToWidth : _width; }
  int GetHeightToRender() { return (_stretchToHeight > 0) ? _stretchToHeight : _height; }

  void Dispose()
  {
    if (_tiles != NULL)
    {
      for (int i = 0; i < _numTiles; i++)
        glDeleteTextures(1, &(_tiles[i].texture));

      free(_tiles);
      _tiles = NULL;
      _numTiles = 0;
    }
    if (_vertex != NULL)
    {
      free(_vertex);
      _vertex = NULL;
    }
  }

  ~OGLBitmap()
  {
    Dispose();
  }
};

struct SpriteDrawListEntry
{
  OGLBitmap *bitmap;
  int x, y;
  bool skip;
};

class OGLGraphicsDriver : public IGraphicsDriver
{
public:
  virtual const char*GetDriverName() { return "OpenGL"; }
  virtual const char*GetDriverID() { return "OGL"; }
  virtual void SetTintMethod(TintMethod method);
  virtual bool Init(int width, int height, int colourDepth, bool windowed, volatile int *loopTimer);
  virtual bool Init(int virtualWidth, int virtualHeight, int realWidth, int realHeight, int colourDepth, bool windowed, volatile int *loopTimer);
  virtual int  FindSupportedResolutionWidth(int idealWidth, int height, int colDepth, int widthRangeAllowed);
  virtual void SetCallbackForPolling(ALI3DCLIENTCALLBACK callback) { _pollingCallback = callback; }
  virtual void SetCallbackToDrawScreen(ALI3DCLIENTCALLBACK callback) { _drawScreenCallback = callback; }
  virtual void SetCallbackOnInit(ALI3DCLIENTCALLBACKINITGFX callback) { _initGfxCallback = callback; }
  virtual void SetCallbackForNullSprite(ALI3DCLIENTCALLBACKXY callback) { _nullSpriteCallback = callback; }
  virtual void UnInit();
  virtual void ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse);
  virtual BITMAP *ConvertBitmapToSupportedColourDepth(BITMAP *allegroBitmap);
  virtual IDriverDependantBitmap* CreateDDBFromBitmap(BITMAP *allegroBitmap, bool hasAlpha, bool opaque);
  virtual void UpdateDDBFromBitmap(IDriverDependantBitmap* bitmapToUpdate, BITMAP *allegroBitmap, bool hasAlpha);
  virtual void DestroyDDB(IDriverDependantBitmap* bitmap);
  virtual void DrawSprite(int x, int y, IDriverDependantBitmap* bitmap);
  virtual void ClearDrawList();
  virtual void RenderToBackBuffer();
  virtual void Render();
  virtual void Render(GlobalFlipType flip);
  virtual void SetRenderOffset(int x, int y);
  virtual void GetCopyOfScreenIntoBitmap(BITMAP* destination);
  virtual void EnableVsyncBeforeRender(bool enabled) { }
  virtual void Vsync();
  virtual void FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
  virtual void FadeIn(int speed, PALETTE p, int targetColourRed, int targetColourGreen, int targetColourBlue);
  virtual void BoxOutEffect(bool blackingOut, int speed, int delay);
  virtual bool PlayVideo(const char *filename, bool useSound, VideoSkipType skipType, bool stretchToFullScreen);
  virtual bool SupportsGammaControl();
  virtual void SetGamma(int newGamma);
  virtual void UseSmoothScaling(bool enabled) { _smoothScaling = enabled; }
  virtual bool RequiresFullRedrawEachFrame() { return true; }
  virtual bool HasAcceleratedStretchAndFlip() { return true; }
  virtual bool UsesMemoryBackBuffer() { return false; }
  virtual BITMAP* GetMemoryBackBuffer() { return NULL; }
  virtual void SetMemoryBackBuffer(BITMAP *backBuffer) {  }
  virtual void SetScreenTint(int red, int green, int blue);

  // Internal
  int _initDLLCallback();
  int _resetDeviceIfNecessary();
  void _render(GlobalFlipType flip, bool clearDrawListAfterwards);
  void _reDrawLastFrame();
  OGLGraphicsDriver(D3DGFXFilter *filter);
  virtual ~OGLGraphicsDriver();

  D3DGFXFilter *_filter;

private:
  HDC _hDC;
  HGLRC _hRC;
  HWND _hWnd;
  HINSTANCE _hInstance;
  int _newmode_width, _newmode_height;
  int _newmode_screen_width, _newmode_screen_height;
  int _newmode_depth, _newmode_refresh;
  bool _newmode_windowed;
  int _global_x_offset, _global_y_offset;
  unsigned int availableVideoMemory;
  ALI3DCLIENTCALLBACK _pollingCallback;
  ALI3DCLIENTCALLBACK _drawScreenCallback;
  ALI3DCLIENTCALLBACKXY _nullSpriteCallback;
  ALI3DCLIENTCALLBACKINITGFX _initGfxCallback;
  int _tint_red, _tint_green, _tint_blue;
  OGLCUSTOMVERTEX defaultVertices[4];
  char previousError[ALLEGRO_ERROR_SIZE];
  bool _smoothScaling;
  bool _legacyPixelShader;
  float _pixelRenderOffset;
  volatile int *_loopTimer;
  BITMAP* _screenTintLayer;
  OGLBitmap* _screenTintLayerDDB;
  SpriteDrawListEntry _screenTintSprite;

  float _scale_width;
  float _scale_height;

  SpriteDrawListEntry drawList[MAX_DRAW_LIST_SIZE];
  int numToDraw;
  SpriteDrawListEntry drawListLastTime[MAX_DRAW_LIST_SIZE];
  int numToDrawLastTime;
  GlobalFlipType flipTypeLastTime;

  bool EnsureDirect3D9IsCreated();
  void InitOpenGl();
  void set_up_default_vertices();
  void AdjustSizeToNearestSupportedByCard(int *width, int *height);
  void UpdateTextureRegion(TextureTile *tile, BITMAP *bitmap, OGLBitmap *target, bool hasAlpha);
  void do_fade(bool fadingOut, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue);
  bool IsModeSupported(int width, int height, int colDepth);
  void create_screen_tint_bitmap();
  void _renderSprite(SpriteDrawListEntry *entry, bool globalLeftRightFlip, bool globalTopBottomFlip);
};

static OGLGraphicsDriver *_ogl_driver = NULL;

IGraphicsDriver* GetOGLGraphicsDriver(GFXFilter *filter)
{
  D3DGFXFilter* d3dfilter = (D3DGFXFilter*)filter;
  if (_ogl_driver == NULL)
  {
    _ogl_driver = new OGLGraphicsDriver(d3dfilter);
  }
  else if (_ogl_driver->_filter != filter)
  {
    _ogl_driver->_filter = d3dfilter;
  }

  return _ogl_driver;
}

OGLGraphicsDriver::OGLGraphicsDriver(D3DGFXFilter *filter) 
{
  numToDraw = 0;
  numToDrawLastTime = 0;
  _pollingCallback = NULL;
  _drawScreenCallback = NULL;
  _initGfxCallback = NULL;
  _tint_red = 0;
  _tint_green = 0;
  _tint_blue = 0;
  _global_x_offset = 0;
  _global_y_offset = 0;
  _newmode_screen_width = 0;
  _newmode_screen_height = 0;
  _filter = filter;
  _screenTintLayer = NULL;
  _screenTintLayerDDB = NULL;
  _screenTintSprite.skip = true;
  _screenTintSprite.x = 0;
  _screenTintSprite.y = 0;
  previousError[0] = 0;
  _legacyPixelShader = false;
  _scale_width = 1.0f;
  _scale_height = 1.0f;
  set_up_default_vertices();
}


void OGLGraphicsDriver::set_up_default_vertices()
{
  defaultVertices[0].position.x = 0.0f;
  defaultVertices[0].position.y = 0.0f;
  defaultVertices[0].tu=0.0;
  defaultVertices[0].tv=0.0;

  defaultVertices[1].position.x = 1.0f;
  defaultVertices[1].position.y = 0.0f;
  defaultVertices[1].tu=1.0;
  defaultVertices[1].tv=0.0;

  defaultVertices[2].position.x = 0.0f;
  defaultVertices[2].position.y = -1.0f;
  defaultVertices[2].tu=0.0;
  defaultVertices[2].tv=1.0;

  defaultVertices[3].position.x = 1.0f;
  defaultVertices[3].position.y = -1.0f;
  defaultVertices[3].tu=1.0;
  defaultVertices[3].tv=1.0;
}


void OGLGraphicsDriver::Vsync() 
{
  // do nothing on D3D
}

bool OGLGraphicsDriver::IsModeSupported(int width, int height, int colDepth)
{
  return true;
}

int OGLGraphicsDriver::FindSupportedResolutionWidth(int idealWidth, int height, int colDepth, int widthRangeAllowed)
{
  return idealWidth;
}

bool OGLGraphicsDriver::SupportsGammaControl() 
{
  return false;
}

void OGLGraphicsDriver::SetGamma(int newGamma)
{
}




void OGLGraphicsDriver::SetRenderOffset(int x, int y)
{
  _global_x_offset = x;
  _global_y_offset = y;
}

void OGLGraphicsDriver::SetTintMethod(TintMethod method) 
{
  _legacyPixelShader = (method == TintReColourise);
}

void OGLGraphicsDriver::InitOpenGl()
{
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_FLAT);

    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, android_screen_physical_width, android_screen_physical_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, android_screen_physical_width, 0, android_screen_physical_height, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (psp_gfx_scaling)
    {
      float android_screen_ar = (float)_newmode_width / (float)_newmode_height;
      float android_device_ar = (float)android_screen_physical_width / (float)android_screen_physical_height;

      if (android_device_ar <= android_screen_ar)
      {
         _scale_width = (float)android_screen_physical_width / (float)_newmode_width;
         _scale_height = _scale_width;
      }
      else
      {
         _scale_height = (float)android_screen_physical_height / (float)_newmode_height;
         _scale_width = _scale_height;
      }
    }
    else
    {
      _scale_width = _scale_height = 1.0f;
    }

    glEnable(GL_SCISSOR_TEST);
    glScissor((int)(((float)android_screen_physical_width - _scale_width * (float)_newmode_width) / 2.0f + 1.0f), (int)(((float)android_screen_physical_height - _scale_height * (float)_newmode_height) / 2.0f), (int)(_scale_width * (float)_newmode_width), (int)(_scale_height * (float)_newmode_height));
}

bool OGLGraphicsDriver::Init(int width, int height, int colourDepth, bool windowed, volatile int *loopTimer) 
{
  return this->Init(width, height, width, height, colourDepth, windowed, loopTimer);
}

bool OGLGraphicsDriver::Init(int virtualWidth, int virtualHeight, int realWidth, int realHeight, int colourDepth, bool windowed, volatile int *loopTimer)
{
  android_create_screen(realWidth, realHeight, colourDepth);

  if (colourDepth < 15)
  {
    ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Direct3D driver does not support 256-colour games"));
    return false;
  }

  _newmode_width = virtualWidth;
  _newmode_height = virtualHeight;
  _newmode_screen_width = realWidth;
  _newmode_screen_height = realHeight;
  _newmode_depth = colourDepth;
  _newmode_refresh = 0;
  _newmode_windowed = windowed;
  _loopTimer = loopTimer;

  _filter->GetRealResolution(&_newmode_screen_width, &_newmode_screen_height);

  try
  {
    int i = 0;

#if defined(WINDOWS_VERSION)
    HWND allegro_wnd = hWnd = win_get_window();

    if (!_newmode_windowed)
    {
      // Remove the border in full-screen mode, otherwise if the player
      // clicks near the edge of the screen it goes back to Windows
      LONG windowStyle = WS_POPUP;
      SetWindowLong(allegro_wnd, GWL_STYLE, windowStyle);
    }
#endif

    gfx_driver = &gfx_opengl;

#if defined(WINDOWS_VERSION)
    if (_newmode_windowed)
    {
      if (adjust_window(_newmode_screen_width, _newmode_screen_height) != 0) 
      {
        ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text("Window size not supported"));
        return -1;
      }
    }

    win_grab_input();


      GLuint PixelFormat;

      static PIXELFORMATDESCRIPTOR pfd =
      {
         sizeof(PIXELFORMATDESCRIPTOR),
         1,
         PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
         PFD_TYPE_RGBA,
         colourDepth,
         0, 0, 0, 0, 0, 0,
         0,
         0,
         0,
         0, 0, 0, 0,
         0,
         0,
         0,
         PFD_MAIN_PLANE,
         0,
         0, 0, 0
      };

      if (!(_hDC = GetDC(_hWnd)))
      return false;

      if (!(PixelFormat = ChoosePixelFormat(_hDC, &pfd)))
      return false;

      if (!SetPixelFormat(_hDC, PixelFormat, &pfd))
      return false;

      if (!(hRC = wglCreateContext(_hDC)))
      return false;

      if(!wglMakeCurrent(_hDC, _hRC))
      return false;
#endif

    InitOpenGl();

    this->create_screen_tint_bitmap();

  }
  catch (Ali3DException exception)
  {
    if (exception._message != allegro_error)
      ustrzcpy(allegro_error, ALLEGRO_ERROR_SIZE, get_config_text(exception._message));
    return false;
  }
  // create dummy screen bitmap
  screen = ConvertBitmapToSupportedColourDepth(create_bitmap_ex(colourDepth, virtualWidth, virtualHeight));

  return true;
}

void OGLGraphicsDriver::UnInit() 
{
  if (_screenTintLayerDDB != NULL) 
  {
    this->DestroyDDB(_screenTintLayerDDB);
    _screenTintLayerDDB = NULL;
    _screenTintSprite.bitmap = NULL;
  }
  if (_screenTintLayer != NULL)
  {
    destroy_bitmap(_screenTintLayer);
    _screenTintLayer = NULL;
  }

  gfx_driver = NULL;
}

OGLGraphicsDriver::~OGLGraphicsDriver()
{
  UnInit();
}

void OGLGraphicsDriver::ClearRectangle(int x1, int y1, int x2, int y2, RGB *colorToUse)
{

}

void OGLGraphicsDriver::GetCopyOfScreenIntoBitmap(BITMAP *destination)
{
}

void OGLGraphicsDriver::RenderToBackBuffer()
{
  throw Ali3DException("D3D driver does not have a back buffer");
}

void OGLGraphicsDriver::Render()
{
  Render(None);
}

void OGLGraphicsDriver::Render(GlobalFlipType flip)
{
  _render(flip, true);
}

void OGLGraphicsDriver::_reDrawLastFrame()
{
  memcpy(&drawList[0], &drawListLastTime[0], sizeof(SpriteDrawListEntry) * numToDrawLastTime);
  numToDraw = numToDrawLastTime;
}

void OGLGraphicsDriver::_renderSprite(SpriteDrawListEntry *drawListEntry, bool globalLeftRightFlip, bool globalTopBottomFlip)
{
  OGLBitmap *bmpToDraw = drawListEntry->bitmap;

  if (bmpToDraw->_transparency >= 255)
    return;

  float width = bmpToDraw->GetWidthToRender();
  float height = bmpToDraw->GetHeightToRender();
  float xProportion = (float)width / (float)bmpToDraw->_width;
  float yProportion = (float)height / (float)bmpToDraw->_height;

  bool flipLeftToRight = globalLeftRightFlip ^ bmpToDraw->_flipped;
  int drawAtX = drawListEntry->x + _global_x_offset;
  int drawAtY = drawListEntry->y + _global_y_offset;

  for (int ti = 0; ti < bmpToDraw->_numTiles; ti++)
  {
    width = bmpToDraw->_tiles[ti].width * xProportion;
    height = bmpToDraw->_tiles[ti].height * yProportion;
    float xOffs;
    float yOffs = bmpToDraw->_tiles[ti].y * yProportion;
    if (flipLeftToRight != globalLeftRightFlip)
    {
      xOffs = (bmpToDraw->_width - (bmpToDraw->_tiles[ti].x + bmpToDraw->_tiles[ti].width)) * xProportion;
    }
    else
    {
      xOffs = bmpToDraw->_tiles[ti].x * xProportion;
    }
    int thisX = drawAtX + xOffs;
    int thisY = drawAtY + yOffs;

    if (globalLeftRightFlip)
    {
      thisX = (_newmode_width - thisX) - width;
    }
    if (globalTopBottomFlip) 
    {
      thisY = (_newmode_height - thisY) - height;
    }

    thisX = (-(_newmode_width / 2)) + thisX;
    thisY = (_newmode_height / 2) - thisY;


    //Setup translation and scaling matrices
    float widthToScale = (float)width;
    float heightToScale = (float)height;
    if (flipLeftToRight)
    {
      // The usual transform changes 0..1 into 0..width
      // So first negate it (which changes 0..w into -w..0)
      widthToScale = -widthToScale;
      // and now shift it over to make it 0..w again
      thisX += width;
    }
    if (globalTopBottomFlip) 
    {
      heightToScale = -heightToScale;
      thisY -= height;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (bmpToDraw->_transparency == 0)
      glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    else
      glColor4f(1.0f, 1.0f, 1.0f, ((float)bmpToDraw->_transparency / 255.0f));

    glTranslatef(android_screen_physical_width / 2.0f, android_screen_physical_height / 2.0f, 0.0f);

    glTranslatef((float)thisX * _scale_width, (float)thisY * _scale_height, 0.0f);
    glScalef(widthToScale * _scale_width, heightToScale * _scale_height, 1.0f);

    glBindTexture(GL_TEXTURE_2D, bmpToDraw->_tiles[ti].texture);

    if ((psp_gfx_smoothing) || (_smoothScaling) && (bmpToDraw->_stretchToHeight > 0) &&
        ((bmpToDraw->_stretchToHeight != bmpToDraw->_height) ||
         (bmpToDraw->_stretchToWidth != bmpToDraw->_width)))
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else 
    {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    if (bmpToDraw->_vertex != NULL)
    {
      glTexCoordPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &(bmpToDraw->_vertex[ti * 4].tu));
      glVertexPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &(bmpToDraw->_vertex[ti * 4].position));  
    }
    else
    {
      glTexCoordPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &defaultVertices[0].tu);
      glVertexPointer(2, GL_FLOAT, sizeof(OGLCUSTOMVERTEX), &defaultVertices[0].position);  
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}

void OGLGraphicsDriver::_render(GlobalFlipType flip, bool clearDrawListAfterwards)
{
  if (!android_screen_initialized)
  {
    InitOpenGl();
    android_screen_initialized = 1;
  }

  SpriteDrawListEntry *listToDraw = drawList;
  int listSize = numToDraw;

  bool globalLeftRightFlip = (flip == Vertical) || (flip == Both);
  bool globalTopBottomFlip = (flip == Horizontal) || (flip == Both);

  glDisable(GL_SCISSOR_TEST);
  glClear(GL_COLOR_BUFFER_BIT);
  glEnable(GL_SCISSOR_TEST);

  // Setup virtual screen
  glViewport(0, 0, android_screen_physical_width, android_screen_physical_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, android_screen_physical_width, 0, android_screen_physical_height, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  for (int i = 0; i < listSize; i++)
  {
    if (listToDraw[i].skip)
      continue;

    if (listToDraw[i].bitmap == NULL)
    {
      if (_nullSpriteCallback)
        _nullSpriteCallback(listToDraw[i].x, listToDraw[i].y);
      else
        throw Ali3DException("Unhandled attempt to draw null sprite");

      continue;
    }

    this->_renderSprite(&listToDraw[i], globalLeftRightFlip, globalTopBottomFlip);
  }

  if (!_screenTintSprite.skip)
  {
    this->_renderSprite(&_screenTintSprite, false, false);
  }

#if defined(WINDOWS_VERSION)
  SwapBuffers(_hDC);
#elif defined(ANDROID_VERSION)
  android_swap_buffers();
#endif

  if (clearDrawListAfterwards)
  {
    numToDrawLastTime = numToDraw;
    memcpy(&drawListLastTime[0], &drawList[0], sizeof(SpriteDrawListEntry) * listSize);
    flipTypeLastTime = flip;
    ClearDrawList();
  }
}

void OGLGraphicsDriver::ClearDrawList()
{
  numToDraw = 0;
}

void OGLGraphicsDriver::DrawSprite(int x, int y, IDriverDependantBitmap* bitmap)
{
  if (numToDraw >= MAX_DRAW_LIST_SIZE)
  {
    throw Ali3DException("Too many sprites to draw in one frame");
  }

  drawList[numToDraw].bitmap = (OGLBitmap*)bitmap;
  drawList[numToDraw].x = x;
  drawList[numToDraw].y = y;
  drawList[numToDraw].skip = false;
  numToDraw++;
}

void OGLGraphicsDriver::DestroyDDB(IDriverDependantBitmap* bitmap)
{
  for (int i = 0; i < numToDrawLastTime; i++)
  {
    if (drawListLastTime[i].bitmap == bitmap)
    {
      drawListLastTime[i].skip = true;
    }
  }
  delete ((OGLBitmap*)bitmap);
}

__inline void get_pixel_if_not_transparent15(unsigned short *pixel, unsigned short *red, unsigned short *green, unsigned short *blue, unsigned short *divisor)
{
  if (pixel[0] != MASK_COLOR_15)
  {
    *red += algetr15(pixel[0]);
    *green += algetg15(pixel[0]);
    *blue += algetb15(pixel[0]);
    divisor[0]++;
  }
}

__inline void get_pixel_if_not_transparent32(unsigned long *pixel, unsigned long *red, unsigned long *green, unsigned long *blue, unsigned long *divisor)
{
  if (pixel[0] != MASK_COLOR_32)
  {
    *red += algetr32(pixel[0]);
    *green += algetg32(pixel[0]);
    *blue += algetb32(pixel[0]);
    divisor[0]++;
  }
}



#define D3DCOLOR_RGBA(r,g,b,a) \
  (((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))


void OGLGraphicsDriver::UpdateTextureRegion(TextureTile *tile, BITMAP *allegroBitmap, OGLBitmap *target, bool hasAlpha)
{
  bool usingLinearFiltering = (psp_gfx_smoothing == 1); //_filter->NeedToColourEdgeLines();
  bool lastPixelWasTransparent = false;
  char *origPtr = (char*)malloc(4 * tile->width * tile->height);
  char *memPtr = origPtr;
  for (int y = 0; y < tile->height; y++)
  {
    lastPixelWasTransparent = false;
    for (int x = 0; x < tile->width; x++)
    {
      if (target->_colDepth == 15)
      {
        unsigned short* memPtrShort = (unsigned short*)memPtr;
        unsigned short* srcData = (unsigned short*)&allegroBitmap->line[y + tile->y][(x + tile->x) * 2];
        if (*srcData == MASK_COLOR_15) 
        {
          if (target->_opaque)  // set to black if opaque
            memPtrShort[x] = 0x8000;
          else if (!usingLinearFiltering)
            memPtrShort[x] = 0;
          // set to transparent, but use the colour from the neighbouring 
          // pixel to stop the linear filter doing black outlines
          else
          {
            unsigned short red = 0, green = 0, blue = 0, divisor = 0;
            if (x > 0)
              get_pixel_if_not_transparent15(&srcData[-1], &red, &green, &blue, &divisor);
            if (x < tile->width - 1)
              get_pixel_if_not_transparent15(&srcData[1], &red, &green, &blue, &divisor);
            if (y > 0)
              get_pixel_if_not_transparent15((unsigned short*)&allegroBitmap->line[y + tile->y - 1][(x + tile->x) * 2], &red, &green, &blue, &divisor);
            if (y < tile->height - 1)
              get_pixel_if_not_transparent15((unsigned short*)&allegroBitmap->line[y + tile->y + 1][(x + tile->x) * 2], &red, &green, &blue, &divisor);
            if (divisor > 0)
              memPtrShort[x] = ((red / divisor) << 10) | ((green / divisor) << 5) | (blue / divisor);
            else
              memPtrShort[x] = 0;
          }
          lastPixelWasTransparent = true;
        }
        else
        {
          memPtrShort[x] = 0x8000 | (algetr15(*srcData) << 10) | (algetg15(*srcData) << 5) | algetb15(*srcData);
          if (lastPixelWasTransparent)
          {
            // update the colour of the previous tranparent pixel, to
            // stop black outlines when linear filtering
            memPtrShort[x - 1] = memPtrShort[x] & 0x7FFF;
            lastPixelWasTransparent = false;
          }
        }
      }
      else if (target->_colDepth == 32)
      {
        unsigned long* memPtrLong = (unsigned long*)memPtr;
        unsigned long* srcData = (unsigned long*)&allegroBitmap->line[y + tile->y][(x + tile->x) * 4];
        if (*srcData == MASK_COLOR_32)
        {
          if (target->_opaque)  // set to black if opaque
            memPtrLong[x] = 0xFF000000;
          else if (!usingLinearFiltering)
            memPtrLong[x] = 0;
          // set to transparent, but use the colour from the neighbouring 
          // pixel to stop the linear filter doing black outlines
          else
          {
            unsigned long red = 0, green = 0, blue = 0, divisor = 0;
            if (x > 0)
              get_pixel_if_not_transparent32(&srcData[-1], &red, &green, &blue, &divisor);
            if (x < tile->width - 1)
              get_pixel_if_not_transparent32(&srcData[1], &red, &green, &blue, &divisor);
            if (y > 0)
              get_pixel_if_not_transparent32((unsigned long*)&allegroBitmap->line[y + tile->y - 1][(x + tile->x) * 4], &red, &green, &blue, &divisor);
            if (y < tile->height - 1)
              get_pixel_if_not_transparent32((unsigned long*)&allegroBitmap->line[y + tile->y + 1][(x + tile->x) * 4], &red, &green, &blue, &divisor);
            if (divisor > 0)
              memPtrLong[x] = ((red / divisor) << 16) | ((green / divisor) << 8) | (blue / divisor);
            else
              memPtrLong[x] = 0;
          }
          lastPixelWasTransparent = true;
        }
        else if (hasAlpha)
        {
          memPtrLong[x] = D3DCOLOR_RGBA(algetr32(*srcData), algetg32(*srcData), algetb32(*srcData), algeta32(*srcData));
        }
        else
        {
          memPtrLong[x] = D3DCOLOR_RGBA(algetr32(*srcData), algetg32(*srcData), algetb32(*srcData), 0xff);
          if (lastPixelWasTransparent)
          {
            // update the colour of the previous tranparent pixel, to
            // stop black outlines when linear filtering
            memPtrLong[x - 1] = memPtrLong[x] & 0x00FFFFFF;
            lastPixelWasTransparent = false;
          }
        }
      }
    }

    memPtr += tile->width * 4;
  }

  unsigned int newTexture = tile->texture;

  glBindTexture(GL_TEXTURE_2D, tile->texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tile->width, tile->height, GL_RGBA, GL_UNSIGNED_BYTE, origPtr);

  free(origPtr);
}

void OGLGraphicsDriver::UpdateDDBFromBitmap(IDriverDependantBitmap* bitmapToUpdate, BITMAP *allegroBitmap, bool hasAlpha)
{
  OGLBitmap *target = (OGLBitmap*)bitmapToUpdate;
  if ((target->_width == allegroBitmap->w) &&
     (target->_height == allegroBitmap->h))
  {
    if (bitmap_color_depth(allegroBitmap) != target->_colDepth)
    {
      throw Ali3DException("Mismatched colour depths");
    }

    target->_hasAlpha = hasAlpha;

    for (int i = 0; i < target->_numTiles; i++)
    {
      UpdateTextureRegion(&target->_tiles[i], allegroBitmap, target, hasAlpha);
    }
  }
}

BITMAP* OGLGraphicsDriver::ConvertBitmapToSupportedColourDepth(BITMAP *allegroBitmap)
{
   int colorConv = get_color_conversion();
   set_color_conversion(COLORCONV_KEEP_TRANS | COLORCONV_TOTAL);

   int colourDepth = bitmap_color_depth(allegroBitmap);
/*   if ((colourDepth == 8) || (colourDepth == 16))
   {
     // Most 3D cards don't support 8-bit; and we need 15-bit colour
     BITMAP* tempBmp = create_bitmap_ex(15, allegroBitmap->w, allegroBitmap->h);
     blit(allegroBitmap, tempBmp, 0, 0, 0, 0, tempBmp->w, tempBmp->h);
     destroy_bitmap(allegroBitmap);
     set_color_conversion(colorConv);
     return tempBmp;
   }
*/   if (colourDepth != 32)
   {
     // we need 32-bit colour
     BITMAP* tempBmp = create_bitmap_ex(32, allegroBitmap->w, allegroBitmap->h);
     blit(allegroBitmap, tempBmp, 0, 0, 0, 0, tempBmp->w, tempBmp->h);
     destroy_bitmap(allegroBitmap);
     set_color_conversion(colorConv);
     return tempBmp;
   }
   return allegroBitmap;
}

void OGLGraphicsDriver::AdjustSizeToNearestSupportedByCard(int *width, int *height)
{
  int allocatedWidth = *width, allocatedHeight = *height;

    bool foundWidth = false, foundHeight = false;
    int tryThis = 2;
    while ((!foundWidth) || (!foundHeight))
    {
      if ((tryThis >= allocatedWidth) && (!foundWidth))
      {
        allocatedWidth = tryThis;
        foundWidth = true;
      }

      if ((tryThis >= allocatedHeight) && (!foundHeight))
      {
        allocatedHeight = tryThis;
        foundHeight = true;
      }

      tryThis = tryThis << 1;
    }

  *width = allocatedWidth;
  *height = allocatedHeight;
}



IDriverDependantBitmap* OGLGraphicsDriver::CreateDDBFromBitmap(BITMAP *allegroBitmap, bool hasAlpha, bool opaque)
{
  int allocatedWidth = allegroBitmap->w;
  int allocatedHeight = allegroBitmap->h;
  BITMAP *tempBmp = NULL;
  int colourDepth = bitmap_color_depth(allegroBitmap);
/*  if ((colourDepth == 8) || (colourDepth == 16))
  {
    // Most 3D cards don't support 8-bit; and we need 15-bit colour
    tempBmp = create_bitmap_ex(15, allegroBitmap->w, allegroBitmap->h);
    blit(allegroBitmap, tempBmp, 0, 0, 0, 0, tempBmp->w, tempBmp->h);
    allegroBitmap = tempBmp;
    colourDepth = 15;
  }
*/  if (colourDepth != 32)
  {
    // we need 32-bit colour
    tempBmp = create_bitmap_ex(32, allegroBitmap->w, allegroBitmap->h);
    blit(allegroBitmap, tempBmp, 0, 0, 0, 0, tempBmp->w, tempBmp->h);
    allegroBitmap = tempBmp;
    colourDepth = 32;
  }

  OGLBitmap *ddb = new OGLBitmap(allegroBitmap->w, allegroBitmap->h, colourDepth, opaque);

  AdjustSizeToNearestSupportedByCard(&allocatedWidth, &allocatedHeight);
  int tilesAcross = 1, tilesDown = 1;

  // *************** REMOVE THESE LINES *************
  //direct3ddevicecaps.MaxTextureWidth = 64;
  //direct3ddevicecaps.MaxTextureHeight = 256;
  // *************** END REMOVE THESE LINES *************

  // Calculate how many textures will be necessary to
  // store this image

  int MaxTextureWidth = 512;
  int MaxTextureHeight = 512;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxTextureWidth);
  MaxTextureHeight = MaxTextureWidth;

  tilesAcross = (allocatedWidth + MaxTextureWidth - 1) / MaxTextureWidth;
  tilesDown = (allocatedHeight + MaxTextureHeight - 1) / MaxTextureHeight;
  int tileWidth = allegroBitmap->w / tilesAcross;
  int lastTileExtraWidth = allegroBitmap->w % tilesAcross;
  int tileHeight = allegroBitmap->h / tilesDown;
  int lastTileExtraHeight = allegroBitmap->h % tilesDown;
  int tileAllocatedWidth = tileWidth;
  int tileAllocatedHeight = tileHeight;

  AdjustSizeToNearestSupportedByCard(&tileAllocatedWidth, &tileAllocatedHeight);

  int numTiles = tilesAcross * tilesDown;
  TextureTile *tiles = (TextureTile*)malloc(sizeof(TextureTile) * numTiles);
  memset(tiles, 0, sizeof(TextureTile) * numTiles);

  OGLCUSTOMVERTEX *vertices = NULL;

  if ((numTiles == 1) &&
      (allocatedWidth == allegroBitmap->w) &&
      (allocatedHeight == allegroBitmap->h))
  {
    // use default whole-image vertices
  }
  else
  {
     // The texture is not the same as the bitmap, so create some custom vertices
     // so that only the relevant portion of the texture is rendered
     int vertexBufferSize = numTiles * 4 * sizeof(OGLCUSTOMVERTEX);

     ddb->_vertex = vertices = (OGLCUSTOMVERTEX*)malloc(vertexBufferSize);
  }

  for (int x = 0; x < tilesAcross; x++)
  {
    for (int y = 0; y < tilesDown; y++)
    {
      TextureTile *thisTile = &tiles[y * tilesAcross + x];
      int thisAllocatedWidth = tileAllocatedWidth;
      int thisAllocatedHeight = tileAllocatedHeight;
      thisTile->x = x * tileWidth;
      thisTile->y = y * tileHeight;
      thisTile->width = tileWidth;
      thisTile->height = tileHeight;
      if (x == tilesAcross - 1) 
      {
        thisTile->width += lastTileExtraWidth;
        thisAllocatedWidth = thisTile->width;
        AdjustSizeToNearestSupportedByCard(&thisAllocatedWidth, &thisAllocatedHeight);
      }
      if (y == tilesDown - 1) 
      {
        thisTile->height += lastTileExtraHeight;
        thisAllocatedHeight = thisTile->height;
        AdjustSizeToNearestSupportedByCard(&thisAllocatedWidth, &thisAllocatedHeight);
      }

      if (vertices != NULL)
      {
        for (int vidx = 0; vidx < 4; vidx++)
        {
          int i = (y * tilesAcross + x) * 4 + vidx;
          vertices[i] = defaultVertices[vidx];
          if (vertices[i].tu > 0.0)
          {
            vertices[i].tu = (float)thisTile->width / (float)thisAllocatedWidth;
          }
          if (vertices[i].tv > 0.0)
          {
            vertices[i].tv = (float)thisTile->height / (float)thisAllocatedHeight;
          }
        }
      }

      char* empty = (char*)malloc(thisAllocatedWidth * thisAllocatedHeight * 4);
      memset(empty, 0, thisAllocatedWidth * thisAllocatedHeight * 4);

      glGenTextures(1, &thisTile->texture);
      glBindTexture(GL_TEXTURE_2D, thisTile->texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, thisAllocatedWidth, thisAllocatedHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, empty);

      free(empty);
    }
  }

  ddb->_numTiles = numTiles;
  ddb->_tiles = tiles;

  UpdateDDBFromBitmap(ddb, allegroBitmap, hasAlpha);

  if (tempBmp) destroy_bitmap(tempBmp);

  return ddb;
}

void OGLGraphicsDriver::do_fade(bool fadingOut, int speed, int targetColourRed, int targetColourGreen, int targetColourBlue)
{
  if (fadingOut)
  {
    this->_reDrawLastFrame();
  }
  else if (_drawScreenCallback != NULL)
    _drawScreenCallback();
  
  BITMAP *blackSquare = create_bitmap_ex(32, 16, 16);
  clear_to_color(blackSquare, makecol32(targetColourRed, targetColourGreen, targetColourBlue));
  IDriverDependantBitmap *d3db = this->CreateDDBFromBitmap(blackSquare, false, false);
  destroy_bitmap(blackSquare);

  d3db->SetStretch(_newmode_width, _newmode_height);
  this->DrawSprite(-_global_x_offset, -_global_y_offset, d3db);

  if (speed <= 0) speed = 16;
  speed *= 2;  // harmonise speeds with software driver which is faster
  for (int a = 1; a < 255; a += speed)
  {
    int timerValue = *_loopTimer;
    d3db->SetTransparency(fadingOut ? a : (255 - a));
    this->_render(flipTypeLastTime, false);

    do
    {
      if (_pollingCallback)
        _pollingCallback();
      Sleep(1);
    }
    while (timerValue == *_loopTimer);

  }

  if (fadingOut)
  {
    d3db->SetTransparency(0);
    this->_render(flipTypeLastTime, false);
  }

  this->DestroyDDB(d3db);
  this->ClearDrawList();
}

void OGLGraphicsDriver::FadeOut(int speed, int targetColourRed, int targetColourGreen, int targetColourBlue) 
{
  do_fade(true, speed, targetColourRed, targetColourGreen, targetColourBlue);
}

void OGLGraphicsDriver::FadeIn(int speed, PALLETE p, int targetColourRed, int targetColourGreen, int targetColourBlue) 
{
  do_fade(false, speed, targetColourRed, targetColourGreen, targetColourBlue);
}

void OGLGraphicsDriver::BoxOutEffect(bool blackingOut, int speed, int delay)
{
  if (blackingOut)
  {
    this->_reDrawLastFrame();
  }
  else if (_drawScreenCallback != NULL)
    _drawScreenCallback();
  
  BITMAP *blackSquare = create_bitmap_ex(32, 16, 16);
  clear(blackSquare);
  IDriverDependantBitmap *d3db = this->CreateDDBFromBitmap(blackSquare, false, false);
  destroy_bitmap(blackSquare);

  d3db->SetStretch(_newmode_width, _newmode_height);
  this->DrawSprite(0, 0, d3db);
  if (!blackingOut)
  {
    // when fading in, draw four black boxes, one
    // across each side of the screen
    this->DrawSprite(0, 0, d3db);
    this->DrawSprite(0, 0, d3db);
    this->DrawSprite(0, 0, d3db);
  }

  int yspeed = _newmode_height / (_newmode_width / speed);
  int boxWidth = speed;
  int boxHeight = yspeed;

  while (boxWidth < _newmode_width)
  {
    boxWidth += speed;
    boxHeight += yspeed;
    if (blackingOut)
    {
      this->drawList[this->numToDraw - 1].x = _newmode_width / 2- boxWidth / 2;
      this->drawList[this->numToDraw - 1].y = _newmode_height / 2 - boxHeight / 2;
      d3db->SetStretch(boxWidth, boxHeight);
    }
    else
    {
      this->drawList[this->numToDraw - 4].x = _newmode_width / 2 - boxWidth / 2 - _newmode_width;
      this->drawList[this->numToDraw - 3].y = _newmode_height / 2 - boxHeight / 2 - _newmode_height;
      this->drawList[this->numToDraw - 2].x = _newmode_width / 2 + boxWidth / 2;
      this->drawList[this->numToDraw - 1].y = _newmode_height / 2 + boxHeight / 2;
      d3db->SetStretch(_newmode_width, _newmode_height);
    }
    
    this->_render(flipTypeLastTime, false);

    if (_pollingCallback)
      _pollingCallback();
    Sleep(delay);
  }

  this->DestroyDDB(d3db);
  this->ClearDrawList();
}

bool OGLGraphicsDriver::PlayVideo(const char *filename, bool useAVISound, VideoSkipType skipType, bool stretchToFullScreen)
{
 return true;
}

void OGLGraphicsDriver::create_screen_tint_bitmap() 
{
  _screenTintLayer = create_bitmap_ex(this->_newmode_depth, 16, 16);
  _screenTintLayer = this->ConvertBitmapToSupportedColourDepth(_screenTintLayer);
  _screenTintLayerDDB = (OGLBitmap*)this->CreateDDBFromBitmap(_screenTintLayer, false, false);
  _screenTintSprite.bitmap = _screenTintLayerDDB;
}

void OGLGraphicsDriver::SetScreenTint(int red, int green, int blue)
{ 
  if ((red != _tint_red) || (green != _tint_green) || (blue != _tint_blue))
  {
    _tint_red = red; 
    _tint_green = green; 
    _tint_blue = blue;

    clear_to_color(_screenTintLayer, makecol_depth(bitmap_color_depth(_screenTintLayer), red, green, blue));
    this->UpdateDDBFromBitmap(_screenTintLayerDDB, _screenTintLayer, false);
    _screenTintLayerDDB->SetStretch(_newmode_width, _newmode_height);
    _screenTintLayerDDB->SetTransparency(128);

    _screenTintSprite.skip = ((red == 0) && (green == 0) && (blue == 0));
  }
}
