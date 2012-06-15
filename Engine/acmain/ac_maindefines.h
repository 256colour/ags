
//
// Here I put only explicit defines made in the start of ac.cpp
//

#if defined(PSP_VERSION)
#define cos(a) vfpu_cosf(a)
#define sin(a) vfpu_sinf(a)
#define tan(a) vfpu_tanf(a)
#define cos(a) vfpu_acosf(a)
#define sin(a) vfpu_asinf(a)
#define atan(a) vfpu_atanf(a)
#define atan2(a,b) vfpu_atan2f(a,b)
#define log(a) vfpu_logf(a)
#define exp(a) vfpu_expf(a)
#define cosh(a) vfpu_coshf(a)
#define sinh(a) vfpu_sinhf(a)
#define tanh(a) vfpu_tanhf(a)
#endif

#ifdef NO_MP3_PLAYER
#define SPECIAL_VERSION "NMP"
#else
#define SPECIAL_VERSION ""
#endif

// Version and build numbers
#define AC_VERSION_TEXT "3.21 "
#define ACI_VERSION_TEXT "3.21.1115"SPECIAL_VERSION
// this needs to be updated if the "play" struct changes
#define LOWEST_SGVER_COMPAT "3.20.1103"SPECIAL_VERSION

#define UNICODE

#define memcpyfast memcpy

#define USE_CLIB

#define IS_ANTIALIAS_SPRITES usetup.enable_antialiasing && (play.disable_antialiasing == 0)

// Allegro 4 has switched 15-bit colour to BGR instead of RGB, so
// in this case we need to convert the graphics on load
#if ALLEGRO_DATE > 19991010
#define USE_15BIT_FIX
#endif


#ifdef WINDOWS_VERSION

#elif defined(LINUX_VERSION) || defined(MAC_VERSION)

#define HWND long
#define _getcwd getcwd
#define strnicmp strncasecmp

#else   // it's DOS (DJGPP)

#define _getcwd getcwd

#endif

#define getr32(xx) ((xx >> _rgb_r_shift_32) & 0xFF)
#define getg32(xx) ((xx >> _rgb_g_shift_32) & 0xFF)
#define getb32(xx) ((xx >> _rgb_b_shift_32) & 0xFF)
#define geta32(xx) ((xx >> _rgb_a_shift_32) & 0xFF)
#define makeacol32(r,g,b,a) ((r << _rgb_r_shift_32) | (g << _rgb_g_shift_32) | (b << _rgb_b_shift_32) | (a << _rgb_a_shift_32))


#define LOADROOM_DO_POLL

#define INI_READONLY


#if defined(WINDOWS_VERSION) && !defined(_DEBUG)
#define USE_CUSTOM_EXCEPTION_HANDLER
#endif



// archive attributes to search for - al_findfirst breaks with 0
#define FA_SEARCH -1

// MACPORT FIX 9/6/5: undef M_PI first
#undef M_PI
#define M_PI 3.14159265358979323846


#ifndef MAX_PATH
#define MAX_PATH 260
#endif



#ifdef WINDOWS_VERSION

#else

#define wArgc argc
#define wArgv argv
#define LPWSTR char*
#define LPCWSTR const char*
#define WCHAR char
#define StrCpyW strcpy

#endif



#define WOUTTEXT_REVERSE wouttext_reverseifnecessary

#define MAX_SCRIPT_MODULES 50