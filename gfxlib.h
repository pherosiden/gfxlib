#pragma once

//===============================================================//
//                 GFXLIB Graphics Library                       //
//               Use SDL2 for render system                      //
//               SDL2_image for load images                      //
//            Target OS: cross-platform (x32_64)                 //
//               Author: Nguyen Ngoc Van                         //
//               Create: 22/10/2018                              //
//              Version: 1.2.3                                   //
//          Last Update: 2021-09-29                              //
//              Website: http://codedemo.net                     //
//                Email: pherosiden@gmail.com                    //
//           References: https://crossfire-designs.de            //
//                       https://lodev.org                       //
//                       https://permadi.com                     //
//                       https://sources.ru                      //
//                       http://eyecandyarchive.com              //
//              License: GNU GPL                                 //
//===============================================================//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <float.h>
#ifdef __APPLE__
#include <libgen.h>
#include <x86intrin.h>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#else
#include "SDL.h"
#include "SDL_image.h"
#endif

//just for x32 and old compiler only
//optimize by used MMX technology
#if !defined(__APPLE__) && !defined(_WIN64)
#define _USE_ASM
#pragma message("Build with assembly code for maximize performance...")
#endif

//GFX version string
#define GFX_VERSION             "v21.09.29"
#define GFX_BUILD_ID            20210929

//MIXED mode constants
#define SCREEN_WIDTH            640     //default screen size
#define SCREEN_HEIGHT           400
#define SCREEN_MIDX             320     //center size
#define SCREEN_MIDY             200
#define SCREEN_SIZE             256000  //screen size in bytes (SCREEN_WIDTH * SCREEN_HEIGHT)
#define IMAGE_WIDTH             320     //texture size for mode 13h
#define IMAGE_HEIGHT            200
#define MAX_HEIGHT              199     //screen size for mode 13h
#define MAX_WIDTH               319
#define MAX_SIZE                63999   //max buffer size for mode 13h
#define IMAGE_MIDX              160     //center size for mode 13h
#define IMAGE_MIDY              100
#define MAX_MIDX                159     //center screen size for mode 13h
#define MAX_MIDY                99
#define IMAGE_SIZE              64000   //image size in bytes (IMAGE_WIDTH * IMAGE_HEIGHT)

//common buffer size
#define SIZE_128                128
#define SIZE_256                256
#define SIZE_512                512
#define SIZE_16K                16384   //128 * 128 size
#define SIZE_32K                32768   //128 * 256 size
#define SIZE_64K                65536   //256 * 256 size

//default frame rate
#define FPS_30                  33      //30 frames per second
#define FPS_60                  17      //60 frames per second
#define FPS_90                  11      //90 frames per second

//projection constant
#define ECHE                    0.77    //must change for each monitor

//radian const
#define RAD                     0.017453293

//XFN font style
#define GFX_FONT_FIXED          0x01    //fixed font (all characters have same size)
#define GFX_FONT_MULTI          0x02    //multiple font
#define GFX_FONT_ANIMATE        0x04    //animation font
#define GFX_FONT_ANIPOS         0x08    //random position font
#define GFX_FONT_SCALEABLE      0x10    //scaleable font
#define GFX_FONT_VECTOR         0x20    //vector font (like CHR, BGI font)
#define GFX_BUFF_SIZE           131072  //maximum GFX buffer
#define GFX_MAX_FONT            5       //maximum GFX font loaded at same time

//GFX error type
#define GFX_ERROR               0x01    //raise error message and quit program
#define GFX_WARNING             0x02    //raise warning message and program will be continued
#define GFX_INFO                0x03    //raise info message

#define MOUSE_LEFT_BUTTON       0       //mouse left button pressed
#define MOUSE_MIDDLE_BUTTON     1       //mouse middle button pressed
#define MOUSE_RIGHT_BUTTON      2       //mouse right button pressed

//button state
#define BUTTON_STATE_NORMAL     0       //mouse state nornal
#define BUTTON_STATE_ACTIVE     1       //mouse state active
#define BUTTON_STATE_PRESSED    2       //mouse state pressed
#define BUTTON_STATE_WAITING    3       //mouse state waiting
#define BUTTON_STATE_DISABLED   4       //mouse state diasble

//button and mouse sprite count
#define BUTTON_HANDLE_COUNT     2       //number of button to handle
#define MOUSE_SPRITE_COUNT      9       //number of mouse sprite
#define BUTTON_STATE_COUNT      4       //number of button state

//fill poly constant
#define MAX_POLY_CORNERS        200     //max polygon corners

//user input filter type
#define INPUT_KEY_PRESSED       0x01    //filter keyboard pressed
#define INPUT_MOUSE_CLICK       0x02    //filter mouse click
#define INPUT_MOUSE_MOTION      0x04    //filter mouse move
#define INPUT_MOUSE_WHEEL       0x08    //filter mouse wheel
#define INPUT_WIN_RESIZED       0x10    //filter windows resize

//re-defined, some compiler does not define yet
#define sqr(a)                  ((a) * (a))
#define max(a, b)               ((a) > (b) ? (a) : (b))
#define min(a, b)               ((a) < (b) ? (a) : (b))
#define sign(x)                 (((x) >= 0) ? (1) : (-1))
#define swap(a, b)              {a ^= b; b ^= a; a ^= b;}
#define swapf(a, b)             {double t = a; a = b; b = t;}
#define clamp(x, lo, hi)        (min(max(x, lo), hi))

//common routines
#ifdef __APPLE__
#define _rotr8(v, n)            __rorb(v, n)
#define _rotl8(v, n)            __rolb(v, n)
#define LOWORD(a)               ((a) & 0xFFFF)
#define HIWORD(a)               (((a) >> 16) & 0xFFFF)
#endif

//RGB common colors
#define RGB_BLACK               0x000000
#define RGB_WHITE               0xFFFFFF
#define RGB_RED                 0xFF0000
#define RGB_GREEN               0x00FF00
#define RGB_BLUE                0x0000FF
#define RGB_YELLOW              0xFFFF00
#define RGB_CYAN                0x00FFFF
#define RGB_MAGENTA             0xFF00FF
#define RGB_PURPLE              0x800080
#define RGB_MAROON              0x800000
#define RGB_DARK_RED            0xC00000
#define RGB_DARK_GREY           0x808080
#define RGB_LIGHT_GREY          0xC0C0C0
#define RGB_DARK_GREEN          0x008000
#define RGB_NAVY                0x000080
#define RGB_TEAL                0x008080
#define RGB_OLIVE               0x808000
#define RGB_GREY32              0x202020
#define RGB_GREY64              0x404040
#define RGB_GREY127             0x7F7F7F
#define RGB_GREY191             0xBFBFBF

//benchmarks snipping code
#define BENCH_START()           {startClock = clock();}
#define BENCH_END()             {messageBox(GFX_INFO, "Total time: %lf(s)", double(clock() - startClock) / CLOCKS_PER_SEC);}

#pragma pack(push, 1)

//redefine RGB color
typedef SDL_Color RGB;

//double point struct
typedef struct
{
    double x, y;
} POINT2D;

//GFX stroke vector info
typedef struct
{
    uint8_t         code;                       //stroke code (0: unuse, 1: moveto, 2: lineto)
    uint8_t         x, y;                       //stroke coordinates
} GFX_STROKE_INFO;

//GFX stroke vector data
typedef struct
{
    uint8_t         width;                      //stroke width
    uint8_t         height;                     //stroke height
    uint16_t        numOfLines;                 //number of strokes
} GFX_STROKE_DATA;

//GFX font info table
typedef struct
{
    uint32_t        startOffset;                //offset of the font start
    uint8_t         bitsPerPixel;               //bits per pixel
    uint16_t        bytesPerLine;               //bytes per line (BMP-font)
    uint16_t        width;                      //font width
    uint16_t        height;                     //font height
    uint16_t        baseLine;                   //baseLine of the character
    uint16_t        descender;                  //font desender
    uint16_t        startChar;                  //start of character
    uint16_t        endChar;                    //end of character
    uint8_t         distance;                   //distance between characters
    uint8_t         randomX;                    //only <> 0 if flag anipos on
    uint8_t         randomY;                    //only <> 0 if flag anipos on
    uint32_t        usedColors;                 //only use for BMP8 font
    uint32_t        spacer;                     //distance for non-existing chars
    uint8_t         reserved[10];               //reserved for later use
} GFX_CHAR_HEADER;

//GFX font
typedef struct
{
    uint8_t         signature[4];               //font signature 'Fnt2'
    uint16_t        version;                    //version number 0x0101
    uint8_t         name[32];                   //name of font
    uint8_t         copyRight[32];              //font copy-right (use for BGI font)
    uint8_t         fontType[4];                //font type BMP1, BMP8, VECT, ...
    uint16_t        subFonts;                   //number of sub-fonts (difference size)
    uint32_t        memSize;                    //bytes on load raw data (use this to alloc memory)
    uint32_t        flags;                      //font flags (ANIPOS, ANIMATION, MULTI, ...)
    GFX_CHAR_HEADER subData;                    //sub-fonts data info
} GFX_FONT_HEADER;

//GFX loaded font memory
typedef struct
{
    GFX_FONT_HEADER header;                     //font header
    uint8_t*        dataPtr;                    //font raw data
} GFX_FONT;

//the structure of image data (base image for GX2LIB)
typedef struct
{
    int32_t         mWidth;                     //image width
    int32_t         mHeight;                    //image height
    uint32_t        mSize;                      //image size in bytes
    uint32_t        mRowBytes;                  //bytes per scanline
    uint8_t*        mData;                      //image raw data
} GFX_IMAGE;

//the structure for animated mouse pointers
typedef struct tagMOUSEBITMAP GFX_BITMAP;
struct tagMOUSEBITMAP
{
    int32_t         mbHotX;                     //mouse hotspot x
    int32_t         mbHotY;                     //mouse hotspot y
    uint8_t*        mbData;                     //mouse bitmap data
    GFX_BITMAP*     mbNext;                     //point to next mouse data
};

//the structure for a bitmap mouse pointer.
typedef struct
{
    int32_t         msPosX;                     //current pos x
    int32_t         msPosY;                     //current pos y
    int32_t         msWidth;                    //mouse image width
    int32_t         msHeight;                   //mouse image height
    uint8_t*        msUnder;                    //mouse under bacground
    GFX_BITMAP*     msBitmap;                   //hold mouse bitmap info
} GFX_MOUSE;

//the structure for a bitmap button.
typedef struct
{
    int32_t         btPosX;                     //button x
    int32_t         btPosY;                     //button y
    int32_t         btState;                    //button state (normal, hover, click, disable)
    int32_t         btWidth;                    //button width (each state)
    int32_t         btHeight;                   //button height (each state)
    uint8_t*        btData[BUTTON_STATE_COUNT]; //hold button data for each button state
} GFX_BUTTON;

//HSL color type
typedef struct {
    int32_t         h;
    int32_t         s;
    int32_t         l;
} HSL;

//HSV color type
typedef struct
{
    int32_t         h;
    int32_t         s;
    int32_t         v;
} HSV;

//memory mapping color structure
typedef struct
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} ARGB;

#pragma pack(pop)

//pixel blending mode
enum BLEND_MODE {
    BLEND_MODE_NORMAL,                          //this's a normal mode
    BLEND_MODE_ADD,                             //add with background color
    BLEND_MODE_SUB,                             //sub with background color
    BLEND_MODE_ALPHA,                           //alpha blending with background color
    BLEND_MODE_ANTIALIASED                      //anti-aliased edge (use for line, circle, ellipse, cubic, bezier)
};

//image interpolation type (apply for scale, rotate, ...)
enum INTERPOLATION_TYPE
{
    INTERPOLATION_TYPE_NORMAL,                  //Bresenham interpolcation (nearest and smooth)
    INTERPOLATION_TYPE_NEARST,                  //nearest neighbor (low quality)
    INTERPOLATION_TYPE_SMOOTH,                  //use average to smooth image (still low quality)
    INTERPOLATION_TYPE_BILINEAR,                //bi-linear interpolation (good quality)
    INTERPOLATION_TYPE_BICUBIC,                 //bi-cubic interpolation (best quality)
    INTERPOLATION_TYPE_UNKNOWN                  //error type
};

//3D projection type
enum PROJECTION_TYPE
{
    PROJECTION_TYPE_PERSPECTIVE,                //perspective projection
    PROJECTION_TYPE_PARALLELE,                  //parallele perjection
    PROJECTION_TYPE_UNKNOWN,                    //error perjection
};

//projection parameters
enum PROJECTION_PARAMS
{
    PROJECTION_PARAMS_THETA,                    //theta angle
    PROJECTION_PARAMS_PHI,                      //phi angle
    PROJECTION_PARAMS_DE,                       //deplace ending
    PROJECTION_PARAMS_UNKNOWN,                  //error params
};

//filled pattern type
enum PATTERN_TYPE
{
    PATTERN_TYPE_LINE,                          //line fill style
    PATTERN_TYPE_LITE_SLASH,                    //line with slash style
    PATTERN_TYPE_SLASH,                         //slash style
    PATTERN_TYPE_BACK_SLASH,                    //back slash
    PATTERN_TYPE_LITE_BACK_SLASH,               //lite back slash
    PATTERN_TYPE_HATCH,                         //hatch style
    PATTERN_TYPE_HATCH_X,                       //hatch x style
    PATTERN_TYPE_INTER_LEAVE,                   //inter leaving style
    PATTERN_TYPE_WIDE_DOT,                      //wide dot style
    PATTERN_TYPE_CLOSE_DOT,                     //closed dot style
    PATTERN_TYPE_UNKNOWN,                       //error type
};

//benchmarks record time
extern clock_t  startClock;                     //recording start clock time

//load texture and image functions
int32_t     loadTexture(uint32_t** texture, int32_t* txw, int32_t* txh, const char* fname);
int32_t     loadPNG(uint8_t* raw, RGB* pal, const char* fname);
int32_t     loadImage(const char* fname, GFX_IMAGE* im);
void        freeImage(GFX_IMAGE* im);

//GFXLIB font functions
GFX_FONT*   getFont(int32_t type = 0);
int32_t     getFontWidth(const char* str);
int32_t     getFontHeight(const char* str);
int32_t     getFontType();
void        setFontType(int32_t type);
void        setFontSize(uint32_t size);
void        makeFont(char* str);
int32_t     loadFont(const char* fname, int32_t type);
void        freeFont(int32_t type);


//program keyboard input handler
void        quit();
void        readKeys();
void        delay(uint32_t miliseconds);
int32_t     keyDown(int32_t key);
int32_t     keyPressed(int32_t key);
int32_t     waitUserInput(int32_t inputMask = INPUT_KEY_PRESSED);
int32_t     finished(int32_t key);
int32_t     getInputDataX();
int32_t     getInputDataY();

//some customize random functions
void        randomBuffer(void* buff, int32_t count, int32_t range);

//system info
int32_t     initSystemInfo();
uint32_t    getTotalMemory();
uint32_t    getAvailableMemory();

//CPU info
uint32_t    getCpuSpeed();
const char* getCpuType();
const char* getCpuName();
const char* getCpuFeatures();

//graphic info
uint32_t    getVideoMemory();
const char* getVideoModeInfo();
const char* getVideoName();
const char* getDriverVersion();
const char* getRenderVersion();
const char* getImageVersion();

//mouse handler functions
void        showMouseCursor(int32_t show);
void        getMouseState(int32_t* mx, int32_t* my, int32_t* lmb = NULL, int32_t* rmb = NULL);
void        setMousePosition(int32_t x, int32_t y);

//timer and FPS functions
uint32_t    getTime();
uint32_t    getElapsedTime(uint32_t tmstart);
void        waitFor(uint32_t tmstart, uint32_t ms);
void        sleepFor(uint32_t ms);

//video and render functions
int32_t     initScreen(int32_t width = SCREEN_WIDTH, int32_t height = SCREEN_HEIGHT, int32_t bpp = 8, int32_t scaled = 0, const char* text = "");
int32_t     getCenterX();
int32_t     getCenterY();
int32_t     getMaxX();
int32_t     getMaxY();
int32_t     getMinX();
int32_t     getMinY();
int32_t     getBitsPerPixel();
int32_t     getBytesPerPixel();
int32_t     getBytesPerScanline();

void        getViewPort(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2);
void        changeViewPort(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void        restoreViewPort();
void        cleanup();
void        render();
void        renderBuffer(const void* buffer, int32_t width, int32_t height);
void*       getDrawBuffer(int32_t* width = NULL, int32_t* height = NULL);
void        changeDrawBuffer(void* newBuff, int32_t newWidth, int32_t newHeight);
void        restoreDrawBuffer();
int32_t     getDrawBufferWidth();
int32_t     getDrawBufferHeight();

//handle program message
void        messageBox(int32_t type, const char* fmt, ...);
void        writeText(int32_t x, int32_t y, uint32_t txtColor, uint32_t mode, const char* format, ...);
int32_t     drawText(int32_t ypos, int32_t size, const char** str);

void        clearScreen(uint32_t color);

//pixels function
uint32_t    getPixel(int32_t x, int32_t y);
void        putPixel(int32_t x, int32_t y, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);

//drawing functions
void        clipLine(int32_t* xs, int32_t* ys, int32_t* xe, int32_t* ye);
void        horizLine(int32_t x, int32_t y, int32_t sx, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        vertLine(int32_t x, int32_t y, int32_t sy, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        moveTo(int32_t x, int32_t y);
void        lineTo(int32_t x, int32_t y, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        drawCircle(int32_t xc, int32_t yc, int32_t rad, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        drawEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        drawRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        drawRoundRect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t rd, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);
void        drawBox(int32_t x, int32_t y, int32_t width, int32_t height, int32_t dx, int32_t dy, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        drawCubicBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);
void        drawQuadBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);
void        drawQuadRationalBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, int32_t col, int32_t mode = BLEND_MODE_NORMAL);
void        drawRotatedEllipse(int32_t x, int32_t y, int32_t ra, int32_t rb, double angle, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        drawLineWidthAA(int32_t x0, int32_t y0, int32_t x1, int32_t y1, double wd, uint32_t col);
void        drawRoundBox(int32_t x, int32_t y, int32_t width, int32_t height, int32_t rd, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);
void        drawPoly(POINT2D* point, int32_t num, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        initProjection(double theta, double phi, double de, double rho = 0);
void        resetProjectionParams();
void        setProjection(PROJECTION_TYPE type);
void        projette(double x, double y, double z, double *px, double *py);
void        deplaceEn(double x, double y, double z);
void        traceVers(double x, double y, double z, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        fillRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        fillRectPattern(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, uint8_t* pattern, int32_t mode = BLEND_MODE_NORMAL);
uint8_t*    getPattern(int32_t type);

void        fillCircle(int32_t xc, int32_t yc, int32_t radius, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        fillEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        fillPolygon(POINT2D* point, int32_t num, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        setActivePage(GFX_IMAGE* page);
void        setVisualPage(GFX_IMAGE* page);

int32_t     newImage(int32_t width, int32_t height, GFX_IMAGE* img);
int32_t     updateImage(int32_t width, int32_t height, GFX_IMAGE* img);
void        freeImage(GFX_IMAGE* img);
void        clearImage(GFX_IMAGE* img);

void        getImage(int32_t x, int32_t y, int32_t width, int32_t height, GFX_IMAGE* img);
void        putImage(int32_t x, int32_t y, GFX_IMAGE* img, int32_t mode = BLEND_MODE_NORMAL);
void        putSprite(int32_t x, int32_t y, uint32_t keyColor, GFX_IMAGE* img, int32_t mode = BLEND_MODE_NORMAL);

//image interpolation
void        scaleImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t type = INTERPOLATION_TYPE_SMOOTH);
void        rotateImage(GFX_IMAGE* dst, GFX_IMAGE* src, double degree, int32_t type = INTERPOLATION_TYPE_SMOOTH);

//palette function (use for mixed mode - 256 colors)
void        getPalette(RGB* pal);
void        setPalette(RGB* pal);
void        shiftPalette(RGB* pal);
void        convertPalette(const uint8_t* palette, RGB* color);
void        getBasePalette(RGB* pal);

void        clearPalette();
void        whitePalette();
void        makeRainbowPalette();
void        makeLinearPalette();
void        makeFunkyPalette();

void        scrollPalette(int32_t from, int32_t to, int32_t step);
void        rotatePalette(int32_t from, int32_t to, int32_t loop, int32_t wtime);

void        fadeIn(RGB* dest, uint32_t wtime);
void        fadeOut(RGB* dest, uint32_t wtime);
void        fadeMax(uint32_t wtime);
void        fadeMin(uint32_t wtime);
void        fadeDown(RGB* pal);
void        fadeCircle(int32_t dir, uint32_t col);
void        fadeRollo(int32_t dir, uint32_t col);
void        fadeOutImage(GFX_IMAGE* img, uint8_t step);

//some FX-effect functiosn
void        prepareTunnel(GFX_IMAGE* dimg, uint8_t* buf1, uint8_t* buf2);
void        drawTunnel(GFX_IMAGE* dimg, GFX_IMAGE* simg, uint8_t* buf1, uint8_t* buf2, uint8_t* mov, uint8_t step);
void        blurImageEx(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t blur);
void        brightnessImage(GFX_IMAGE* dst, GFX_IMAGE* src, uint8_t bright);
void        brightnessAlpha(GFX_IMAGE* img, uint8_t bright);
void        blockOutMidImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t xb, int32_t yb);
void        fadeOutCircle(double pc, int32_t size, int32_t type, uint32_t col);
void        scaleUpImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t* tables, int32_t xfact, int32_t yfact);
void        blurImage(GFX_IMAGE* img);
void        blendImage(GFX_IMAGE* dst, GFX_IMAGE* src1, GFX_IMAGE* src2, int32_t cover);
void        rotateImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t* tables, int32_t axisx, int32_t axisy, double angle, double scale);
void        bumpImage(GFX_IMAGE* dst, GFX_IMAGE* src1, GFX_IMAGE* src2, int32_t lx, int32_t ly);

void        initPlasma(uint8_t* sint, uint8_t* cost);
void        createPlasma(uint8_t* dx, uint8_t* dy, uint8_t* sint, uint8_t* cost, GFX_IMAGE* img);

//show image and mouse activity simulation
void        showPNG(const char* fname);
void        showBMP(const char* fname);
void        handleMouseButton();
void        setWindowTitle(const char* title);

//other pixels fx effects
void        putPixelBob(int32_t x, int32_t y);
void        drawLineBob(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

//export demo function (not included in GFXLIB)
void        gfxDemoMix();
void        gfxDemo();
void        gfxEffectsMix();
void        gfxEffects();
void        gfxFontView();
void        gfxFractals();

//convert r,g,b values to 32bits integer value
static __forceinline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

//merge rgb and alpha channel to packed color
static __forceinline uint32_t rgba(uint32_t col, uint8_t alpha)
{
    return (uint32_t(alpha) << 24) | col;
}

//HSL to RGB convert
static __forceinline uint32_t hsl2rgb(int32_t hi, int32_t si, int32_t li)
{
    double r = 0.0, g = 0.0, b = 0.0;
    double temp1 = 0.0, temp2 = 0.0;
    double tempr = 0.0, tempg = 0.0, tempb = 0.0;

    const double h = hi / 256.0;
    const double s = si / 256.0;
    const double l = li / 256.0;

    //if saturation is 0, the color is a shade of grey
    if (s == 0.0) r = g = b = l;

    //if saturation > 0, more complex calculations are needed
    else
    {
        //set the temporary values
        if (l < 0.5) temp2 = l * (1 + s);
        else temp2 = (l + s) - (l * s);

        temp1 = 2 * l - temp2;
        tempr = h + 1.0 / 3.0;

        if (tempr > 1.0) tempr--;

        tempg = h;
        tempb = h - 1.0 / 3.0;

        if (tempb < 0.0) tempb++;

        //red
        if (tempr < 1.0 / 6.0) r = temp1 + (temp2 - temp1) * 6.0 * tempr;
        else if (tempr < 0.5) r = temp2;
        else if (tempr < 2.0 / 3.0) r = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempr) * 6.0;
        else r = temp1;

        //green
        if (tempg < 1.0 / 6.0) g = temp1 + (temp2 - temp1) * 6.0 * tempg;
        else if (tempg < 0.5) g = temp2;
        else if (tempg < 2.0 / 3.0) g = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempg) * 6.0;
        else g = temp1;

        //blue
        if (tempb < 1.0 / 6.0) b = temp1 + (temp2 - temp1) * 6.0 * tempb;
        else if (tempb < 0.5) b = temp2;
        else if (tempb < 2.0 / 3.0) b = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempb) * 6.0;
        else b = temp1;
    }

    uint32_t col = 0;
    uint8_t* pcol = (uint8_t*)&col;
    pcol[2] = uint8_t(r * 255);
    pcol[1] = uint8_t(g * 255);
    pcol[0] = uint8_t(b * 255);
    return col;
}

//HSV to RGB convert
static __forceinline uint32_t hsv2rgb(int32_t hi, int32_t si, int32_t vi)
{
    double h = hi / 256.0;
    const double s = si / 256.0;
    const double v = vi / 256.0;
    double r = 0.0, g = 0.0, b = 0.0;

    //if saturation is 0, the color is a shade of grey
    if (s == 0.0) r = g = b = v;

    //if saturation > 0, more complex calculations are needed
    else
    {
        //to bring hue to a number between 0 and 6, better for the calculations
        h *= 6.0;

        const int32_t i = int32_t(floor(h));

        //the fractional part of h
        const double f = h - i;
        const double p = v * (1.0 - s);
        const double q = v * (1.0 - (s * f));
        const double t = v * (1.0 - (s * (1.0 - f)));

        switch (i)
        {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: r = g = b = 0.0; break;
        }
    }

    uint32_t col = 0;
    uint8_t* pcol = (uint8_t*)&col;
    pcol[2] = uint8_t(r * 255);
    pcol[1] = uint8_t(g * 255);
    pcol[0] = uint8_t(b * 255);
    return col;
}

//converts an RGB color to HSV color
static __forceinline HSV rgb2hsv(uint8_t ri, uint8_t gi, uint8_t bi)
{
    double r = ri / 256.0;
    double g = gi / 256.0;
    double b = bi / 256.0;

    double dmax = max(r, max(g, b));
    double dmin = min(r, min(g, b));

    double v = dmax;
    double h = 0, s = 0;

    if (dmax != 0.0)
    {
        s = (dmax - dmin) / dmax;
    }

    if (s == 0.0)
    {
        h = 0.0;
    }
    else
    {
        if (r == dmax) h = (g - b) / (dmax - dmin);
        if (g == dmax) h = 2.0 + (b - r) / (dmax - dmin);
        if (b == dmax) h = 4.0 + (r - g) / (dmax - dmin);
        h /= 6.0;
        if (h < 0.0) h++;
    }

    HSV col = { 0 };
    col.h = uint32_t(h * 255.0);
    col.s = uint32_t(s * 255.0);
    col.v = uint32_t(v * 255.0);
    return col;
}

//convert an RGB color to HSL color
static __forceinline HSL rgb2hsl(uint8_t ri, uint8_t gi, uint8_t bi)
{
    double r = ri / 255.0;
    double g = gi / 255.0;
    double b = bi / 255.0;

    double dmax = max(r, max(g, b));
    double dmin = min(r, min(g, b));
    double l = (dmax + dmin) / 2;

    double h = 0, s = 0;

    if (dmax == dmin)
    {
        h = s = 0;
    }
    else
    {
        double d = dmax - dmin;
        s = (l > 0.5) ? d / (2 - dmax - dmin) : d / (dmax + dmin);
        if (r == dmax) h = (g - b) / d + (g < b ? 6 : 0);
        if (g == dmax) h = (b - r) / d + 2;
        if (b == dmax) h = (r - g) / d + 4;
        h /= 6;
    }

    HSL col = { 0 };
    col.h = int32_t(h * 360);
    col.s = int32_t(s * 100);
    col.l = int32_t(l * 100);
    return col;
}

//generate random value from number
static __forceinline int32_t random(int32_t a)
{
    return a ? rand() % a : 0;
}

//generate random value in range
static __forceinline int32_t random(int32_t a, int32_t b)
{
    return (a < b) ? (a + (rand() % (b - a + 1))) : (b + (rand() % (a - b + 1)));
}

//generate double random in ranage
static __forceinline double frand(double fmin, double fmax)
{
    const double fn = double(rand()) / RAND_MAX;
    return fmin + fn * (fmax - fmin);
}

//round-up function
static __forceinline int32_t fround(double x)
{
    return (x > 0) ? int32_t(x + 0.5) : int32_t(x - 0.5);
}

//clip point at (x,y)
static __forceinline bool clipPoint(const int32_t width, const int32_t height, int32_t* x, int32_t* y)
{
    bool ret = true;

    if (*x < 0)
    { 
        *x = 0;
        ret = false;
    }
    else if (*x >= width)
    { 
        *x = width - 1;
        ret = false;
    }

    if (*y < 0)
    {
        *y = 0;
        ret = false;
    }
    else if (*y >= height)
    {
        *y = height - 1;
        ret = false;
    }

    return ret;
}

//get source pixel
static __forceinline uint32_t clampOffset(const int32_t width, const int32_t height, const int32_t x, const int32_t y)
{
    //x-range check
    const int32_t xx = clamp(x, 0, width - 1);
    const int32_t yy = clamp(y, 0, height - 1);

    //return offset at (x,y)
    return yy * width + xx;
}

//clamp pixels at offset (x,y)
static __forceinline uint32_t clampPixels(const GFX_IMAGE* img, int32_t x, int32_t y)
{
    const uint32_t* psrc = (const uint32_t*)img->mData;
    bool insrc = clipPoint(img->mWidth, img->mHeight, &x, &y);
    uint32_t result = psrc[y * img->mWidth + x];
    if (!insrc)
    {
        uint8_t* pcol = (uint8_t*)&result;
        pcol[3] = 0;
    }
    return result;
}

//alpha-blending pixel
static __forceinline uint32_t alphaBlend(uint32_t dst, uint32_t src)
{
#ifdef _USE_ASM
    __asm {
        pxor        mm7, mm7
        movd        mm0, src
        movd        mm2, dst
        punpcklbw   mm0, mm7
        punpcklbw   mm2, mm7
        movq        mm1, mm0
        punpckhwd   mm1, mm1
        psubw       mm0, mm2
        punpckhdq   mm1, mm1
        psllw       mm2, 8
        pmullw      mm0, mm1
        paddw       mm2, mm0
        psrlw       mm2, 8
        packuswb    mm2, mm7
        movd        eax, mm2
        emms
    }
#else
    uint32_t cover = src >> 24;
    uint32_t rcover = 255 - cover;
    uint32_t rb = ((dst & 0x00ff00ff) * rcover + (src & 0x00ff00ff) * cover);
    uint32_t ag = (((dst & 0xff00ff00) >> 8) * rcover + ((src & 0xff00ff00) >> 8) * cover);
    return ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
#endif
}

//smooth get pixel
static __forceinline uint32_t smoothGetPixel(const GFX_IMAGE* img, const int32_t sx, const int32_t sy)
{
    int32_t lx = sx >> 16;
    int32_t ly = sy >> 16;
    const uint32_t* psrc = (const uint32_t*)img->mData;
    const uint8_t* p0 = (const uint8_t*)&psrc[clampOffset(img->mWidth, img->mHeight, lx, ly)];
    const uint8_t* p1 = (const uint8_t*)&psrc[clampOffset(img->mWidth, img->mHeight, lx + 1, ly)];

    uint32_t col = 0;
    uint8_t* pcol = (uint8_t*)&col;
    pcol[2] = (p0[2] + p1[2]) >> 1;
    pcol[1] = (p0[1] + p1[1]) >> 1;
    pcol[0] = (p0[0] + p1[0]) >> 1;
    return col;
}

//bilinear get pixel with FIXED-POINT (signed 16.16)
static __forceinline uint32_t bilinearGetPixelCenter(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    const uint32_t* pixel = (uint32_t*)psrc->mData;
    const uint32_t* pixel0 = &pixel[(sy >> 16) * psrc->mWidth + (sx >> 16)];
    const uint32_t* pixel1 = pixel0 + psrc->mWidth;
    
    const uint8_t pu = sx >> 8;
    const uint8_t pv = sy >> 8;
    const uint32_t w3 = (pu * pv) >> 8;
    const uint32_t w2 = pu - w3;
    const uint32_t w1 = pv - w3;
    const uint32_t w0 = 256 - w1 - w2 - w3;

    //load 4 pixels [(x, y),(x + 1, y),(x, y + 1),(x + 1, y + 1)]
    __m128i p12 = _mm_loadl_epi64((const __m128i*)pixel0);
    __m128i p34 = _mm_loadl_epi64((const __m128i*)pixel1);

    //convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA
    p12 = _mm_unpacklo_epi8(p12, p34);
    p34 = _mm_unpackhi_epi64(p12, _mm_setzero_si128());
    p12 = _mm_unpacklo_epi8(p12, p34);

    //extend to 16bits
    __m128i rg = _mm_unpacklo_epi8(p12, _mm_setzero_si128());
    __m128i ba = _mm_unpackhi_epi8(p12, _mm_setzero_si128());

    //convert floating point weights to 16bits integer w4 w3 w2 w1
    __m128i weight = _mm_set_epi32(w3, w1, w2, w0);

    //make 32bit -> 2 x 16bits
    weight = _mm_packs_epi32(weight, weight);

    //rg = [w1*r1 + w2*r2 | w3*r3 + w4*r4 | w1*g1 + w2*g2 | w3*g3 + w4*g4]
    rg = _mm_madd_epi16(rg, weight);

    //ba = [w1*b1 + w2*b2 | w3*b3 + w4*b4 | w1*a1 + w2*a2 | w3*a3 + w4*a4]
    ba = _mm_madd_epi16(ba, weight);

    //horizontal add that will produce the output values (in 32bit)
    weight = _mm_hadd_epi32(rg, ba);
    weight = _mm_srli_epi32(weight, 8);

    //convert 32bit->8bit
    weight = _mm_packus_epi32(weight, _mm_setzero_si128());
    weight = _mm_packus_epi16(weight, _mm_setzero_si128());
    return _mm_cvtsi128_si32(weight);
}

//bilinear get pixel with FIXED-POINT (signed 16.16)
static __forceinline uint32_t bilinearGetPixelBorder(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    //convert to fixed point
    const int32_t lx = sx >> 16;
    const int32_t ly = sy >> 16;

    //load the 4 neighboring pixels
    uint32_t pixels[4] = { 0 };
    pixels[0] = clampPixels(psrc, lx    , ly    );
    pixels[1] = clampPixels(psrc, lx + 1, ly    );
    pixels[2] = clampPixels(psrc, lx    , ly + 1);
    pixels[3] = clampPixels(psrc, lx + 1, ly + 1);

    GFX_IMAGE img = { 0 };
    img.mData = (uint8_t*)pixels;
    img.mWidth = 2;
    img.mHeight = 2;
    img.mRowBytes = 8;
    return bilinearGetPixelCenter(&img, sx & 0xffff, sy & 0xffff);
}

//bilinear get pixel with FIXED-POINT (signed 16.16)
//general optimize version, fast speed
static __forceinline uint32_t bilinearGetPixelFixed(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    //convert to fixed point
    const int32_t lx = sx >> 16;
    const int32_t ly = sy >> 16;
    const uint8_t u = (sx & 0xffff) >> 8;
    const uint8_t v = (sy & 0xffff) >> 8;

    //load the 4 neighboring pixels
    const uint32_t p0 = clampPixels(psrc, lx    , ly    );
    const uint32_t p1 = clampPixels(psrc, lx + 1, ly    );
    const uint32_t p2 = clampPixels(psrc, lx    , ly + 1);
    const uint32_t p3 = clampPixels(psrc, lx + 1, ly + 1);

    //calculate the weights for each pixel
    const uint32_t w3 = (u * v) >> 8;
    const uint32_t w2 = u - w3;
    const uint32_t w1 = v - w3;
    const uint32_t w0 = 256 - w1 - w2 - w3;

    uint32_t rb = (p0 & 0x00ff00ff) * w0;
    uint32_t ga = ((p0 & 0xff00ff00) >> 8) * w0;
    rb += (p1 & 0x00ff00ff) * w2;
    ga += ((p1 & 0xff00ff00) >> 8) * w2;
    rb += (p2 & 0x00ff00ff) * w1;
    ga += ((p2 & 0xff00ff00) >> 8) * w1;
    rb += (p3 & 0x00ff00ff) * w3;
    ga += ((p3 & 0xff00ff00) >> 8) * w3;
    return (ga & 0xff00ff00) | ((rb & 0xff00ff00) >> 8);
}

//constant values that will be needed
static const __m128 CONST_1 = _mm_set_ps1(1);
static const __m128 CONST_256 = _mm_set_ps1(256);

//calculate weight of pixel at (x,y)
static __forceinline __m128 calcWeights(const double x, const double y)
{
    __m128 xmm0 = _mm_set_ps1(float(x));
    __m128 xmm1 = _mm_set_ps1(float(y));
    __m128 xmm2 = _mm_unpacklo_ps(xmm0, xmm1);

    xmm0 = _mm_floor_ps(xmm2);
    xmm1 = _mm_sub_ps(xmm2, xmm0);
    xmm2 = _mm_sub_ps(CONST_1, xmm1);

    __m128 xmm3 = _mm_unpacklo_ps(xmm2, xmm1);
    xmm3 = _mm_movelh_ps(xmm3, xmm3);

    __m128 xmm4 = _mm_shuffle_ps(xmm2, xmm1, _MM_SHUFFLE(1, 1, 1, 1));
    xmm4 = _mm_mul_ps(xmm3, xmm4);

    return _mm_mul_ps(xmm4, CONST_256);
}

//get pixels bilinear with SSE2
static __forceinline uint32_t bilinearGetPixelSSE2(const GFX_IMAGE* psrc, const double x, const double y)
{
    //calculate offset at (x,y)
    const int32_t lx = int32_t(x);
    const int32_t ly = int32_t(y);

    //clamp 4 neighboring pixels
    uint32_t pixels[4] = { 0 };
    pixels[0] = clampPixels(psrc, lx    , ly    );
    pixels[1] = clampPixels(psrc, lx + 1, ly    );
    pixels[2] = clampPixels(psrc, lx    , ly + 1);
    pixels[3] = clampPixels(psrc, lx + 1, ly + 1);

    //load 4 pixels [(x, y),(x + 1, y),(x, y + 1),(x + 1, y + 1)]
    __m128i p12 = _mm_loadl_epi64((const __m128i*)&pixels[0]);
    __m128i p34 = _mm_loadl_epi64((const __m128i*)&pixels[2]);

    //convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA
    p12 = _mm_unpacklo_epi8(p12, p34);
    p34 = _mm_unpackhi_epi64(p12, _mm_setzero_si128());
    p12 = _mm_unpacklo_epi8(p12, p34);

    //extend to 16bits
    __m128i rg = _mm_unpacklo_epi8(p12, _mm_setzero_si128());
    __m128i ba = _mm_unpackhi_epi8(p12, _mm_setzero_si128());

    //convert floating point weights to 16bits integer w4 w3 w2 w1
    __m128i weight = _mm_cvtps_epi32(calcWeights(x, y));

    //make 32bit -> 2 x 16bits
    weight = _mm_packs_epi32(weight, weight);

    //rg = [w1*r1 + w2*r2 | w3*r3 + w4*r4 | w1*g1 + w2*g2 | w3*g3 + w4*g4]
    rg = _mm_madd_epi16(rg, weight);

    //ba = [w1*b1 + w2*b2 | w3*b3 + w4*b4 | w1*a1 + w2*a2 | w3*a3 + w4*a4]
    ba = _mm_madd_epi16(ba, weight);

    //horizontal add that will produce the output values (in 32bit)
    weight = _mm_hadd_epi32(rg, ba);
    weight = _mm_srli_epi32(weight, 8);

    //convert 32bit->8bit
    weight = _mm_packus_epi32(weight, _mm_setzero_si128());
    weight = _mm_packus_epi16(weight, _mm_setzero_si128());
    return _mm_cvtsi128_si32(weight);
}

//bicubic helper
static __forceinline double cubicHermite(const double a, const double b, const double c, const double d, const double fract)
{
    const double aa = -a / 2.0 + 1.5 * b - 1.5 * c + d / 2.0;
    const double bb = a - 2.5 * b + 2.0 * c - d / 2.0;
    const double cc = -a / 2.0 + c / 2.0;
    return aa * fract * fract * fract + bb * fract * fract + cc * fract + b;
}

//calculate function sin(x)/x replace for cubicHermite
//so this will add to lookup table for speedup improvement
static __forceinline double sinXDivX(const double b)
{
    const double a = -1;
    const double x = (b < 0) ? -b : b;
    const double x2 = x * x, x3 = x2 * x;

    if (x <= 1) return (a + 2) * x3 - (a + 3) * x2 + 1;
    else if (x <= 2) return a * x3 - (5 * a) * x2 + (8 * a) * x - (4 * a);
    return 0;
}

//4 signed 32bits sum of bits of data (simulation for _mm_madd_epi32)
static __forceinline int32_t _mm_hsum_epi32(const __m128i val)
{
    //_mm_extract_epi32 is slower
    __m128i tmp = _mm_add_epi32(val, _mm_srli_si128(val, 8));
    tmp = _mm_add_epi32(tmp, _mm_srli_si128(tmp, 4));
    return _mm_cvtsi128_si32(tmp);
}

//calculate pixel by bicubic interpolation
static __forceinline uint32_t bicubicGetPixel(const GFX_IMAGE* img, const double sx, const double sy)
{
    const int32_t px = int32_t(sx);
    const double fx = sx - int32_t(sx);

    const int32_t py = int32_t(sy);
    const double fy = sy - int32_t(sy);

    const uint32_t width = img->mWidth;
    const uint32_t height = img->mHeight;
    const uint32_t* psrc = (const uint32_t*)img->mData;

    const uint8_t* p00 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py - 1)];
    const uint8_t* p10 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py - 1)];
    const uint8_t* p20 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py - 1)];
    const uint8_t* p30 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py - 1)];
    const uint8_t* p01 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py    )];
    const uint8_t* p11 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py    )];
    const uint8_t* p21 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py    )];
    const uint8_t* p31 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py    )];
    const uint8_t* p02 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py + 1)];
    const uint8_t* p12 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py + 1)];
    const uint8_t* p22 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py + 1)];
    const uint8_t* p32 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py + 1)];
    const uint8_t* p03 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py + 2)];
    const uint8_t* p13 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py + 2)];
    const uint8_t* p23 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py + 2)];
    const uint8_t* p33 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py + 2)];

    //mapping destination pointer
    uint32_t dst = 0;
    uint8_t* pdst = (uint8_t*)&dst;

    //start interpolate bicubically
    for (int32_t i = 0; i < 4; i++)
    {
        const double col0 = cubicHermite(p00[i], p10[i], p20[i], p30[i], fx);
        const double col1 = cubicHermite(p01[i], p11[i], p21[i], p31[i], fx);
        const double col2 = cubicHermite(p02[i], p12[i], p22[i], p32[i], fx);
        const double col3 = cubicHermite(p03[i], p13[i], p23[i], p33[i], fx);
        const double pcol = cubicHermite(col0, col1, col2, col3, fy);

        //saturation check
        pdst[i] = uint8_t(clamp(pcol, 0.0, 255.0));
    }

    return dst;
}

//this calculate pixel with boundary so quite slowly
static __forceinline uint32_t bicubicGetPixelFixed(const GFX_IMAGE* img, const int16_t *sintab, const int32_t sx, const int32_t sy)
{
    //peek offset at (px,py)
    const int32_t px = sx >> 16, py = sy >> 16;

    const uint32_t width = img->mWidth;
    const uint32_t height = img->mHeight;
    const uint32_t* psrc = (const uint32_t*)img->mData;

    //calculate around pixels
    const uint8_t *p00 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py - 1)];
    const uint8_t *p01 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py - 1)];
    const uint8_t *p02 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py - 1)];
    const uint8_t *p03 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py - 1)];
    const uint8_t *p10 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py    )];
    const uint8_t *p11 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py    )];
    const uint8_t *p12 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py    )];
    const uint8_t *p13 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py    )];
    const uint8_t *p20 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py + 1)];
    const uint8_t *p21 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py + 1)];
    const uint8_t *p22 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py + 1)];
    const uint8_t *p23 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py + 1)];
    const uint8_t *p30 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py + 2)];
    const uint8_t *p31 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py + 2)];
    const uint8_t *p32 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py + 2)];
    const uint8_t *p33 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py + 2)];

    //4 pixels weigths
    const uint8_t u = sx >> 8, v = sy >> 8;
    const int32_t u0 = sintab[256 + u], u1 = sintab[u];
    const int32_t u2 = sintab[256 - u], u3 = sintab[512 - u];
    const int32_t v0 = sintab[256 + v], v1 = sintab[v];
    const int32_t v2 = sintab[256 - v], v3 = sintab[512 - v];

    uint32_t dst = 0;
    uint8_t* pdst = (uint8_t*)&dst;

    //calculate each pixel channel
    for (int32_t i = 0; i < 4; i++)
    {
        const int32_t s1 = (p00[i] * u0 + p01[i] * u1 + p02[i] * u2 + p03[i] * u3) * v0;
        const int32_t s2 = (p10[i] * u0 + p11[i] * u1 + p12[i] * u2 + p13[i] * u3) * v1;
        const int32_t s3 = (p20[i] * u0 + p21[i] * u1 + p22[i] * u2 + p23[i] * u3) * v2;
        const int32_t s4 = (p30[i] * u0 + p31[i] * u1 + p22[i] * u2 + p33[i] * u3) * v3;
        pdst[i] = clamp((s1 + s2 + s3 + s4) >> 16, 0, 255);
    }

    return dst;
}

//calculate sin&cos of an angle
static __forceinline void sincos(double angle, double* sina, double* cosa)
{
#ifdef _USE_ASM
    __asm {
        fld     angle
        mov     eax, sina
        mov     edx, cosa
        fsincos   
        fstp    qword ptr [edx]   
        fstp    qword ptr [eax]  
    }
#else
    *sina = sin(angle);
    *cosa = cos(angle);
#endif // _USE_ASM
}

//rotate clip data
struct TClipData
{
public:
    int32_t srcw;
    int32_t srch;
    int32_t dstw;
    int32_t dsth;
    int32_t ax; 
    int32_t ay; 
    int32_t bx; 
    int32_t by; 
    int32_t cx;
    int32_t cy; 
    int32_t boundWidth;

private:
    int32_t currUpX0;
    int32_t currUpX1;
    int32_t currDownX0;
    int32_t currDownX1;

    __forceinline bool pointInBound(int32_t scx, int32_t scy)
    {
        return  (((scx >= (-(boundWidth << 16))) && ((scx >> 16) < (srcw + boundWidth))) &&
                 ((scy >= (-(boundWidth << 16))) && ((scy >> 16) < (srch + boundWidth))));
    }

    __forceinline bool pointInSrc(int32_t scx, int32_t scy)
    {
        return  (((scx >= (boundWidth << 16)) &&
                 ((scx >> 16) < (srcw - boundWidth))) &&
                 ((scy >= (boundWidth << 16)) &&
                 ((scy >> 16) < (srch - boundWidth))));
    }

    void findBeginIn(int32_t dsty, int32_t* dstx, int32_t* scx, int32_t* scy)
    {
        *scx -= ax;
        *scy -= ay;

        while (pointInBound(*scx, *scy))
        {
            (*dstx)--;
            *scx -= ax;
            *scy -= ay;
        }

        *scx += ax;
        *scy += ay;
    }

    bool findBegin(int32_t dsty, int32_t* dstx0, int32_t dstx1)
    {
        int32_t testx0 = *dstx0 - 1;
        int32_t scx = ax * testx0 + bx * dsty + cx;
        int32_t scy = ay * testx0 + by * dsty + cy;

        for (int32_t i = testx0; i <= dstx1; i++)
        {
            if (pointInBound(scx, scy))
            {
                *dstx0 = i;

                if (i == testx0) findBeginIn(dsty, dstx0, &scx, &scy);

                if (*dstx0 < 0)
                {
                    scx -= ax * (*dstx0);
                    scy -= ay * (*dstx0);
                }

                srcx = scx;
                srcy = scy;

                return true;
            }
            else
            {
                scx += ax;
                scy += ay;
            }
        }

        return false;
    }

    void findEnd(int32_t dsty, int32_t dstx0, int32_t* dstx1)
    {
        int32_t testx1 = *dstx1;
        if (testx1 < dstx0) testx1 = dstx0;

        int32_t scx = ax * testx1 + bx * dsty + cx;
        int32_t scy = ay * testx1 + by * dsty + cy;

        if (pointInBound(scx, scy))
        {
            testx1++;
            scx += ax;
            scy += ay;

            while (pointInBound(scx, scy))
            {
                testx1++;
                scx += ax;
                scy += ay;
            }

            *dstx1 = testx1;
        }
        else
        {
            scx -= ax;
            scy -= ay;
            while (!pointInBound(scx, scy))
            {
                testx1--;
                scx -= ax;
                scy -= ay;
            }

            *dstx1 = testx1;
        }
    }

    __forceinline void updateInX()
    {
        if (!boundWidth || boundx0 >= boundx1)
        {
            inx0 = boundx0;
            inx1 = boundx1;
        }
        else
        {
            int32_t scx = srcx;
            int32_t scy = srcy;
            int32_t i = boundx0;

            while (i < boundx1)
            {
                if (pointInSrc(scx, scy)) break;
                scx += ax;
                scy += ay;
                i++;
            }

            inx0 = i;

            scx = srcx + (boundx1 - boundx0) * ax;
            scy = srcy + (boundx1 - boundx0) * ay;

            i = boundx1;

            while (i > inx0)
            {
                scx -= ax;
                scy -= ay;
                if (pointInSrc(scx, scy)) break;
                i--;
            }

            inx1 = i;
        }
    }

    __forceinline void updateUpX()
    {
        if (currUpX0 < 0) boundx0 = 0;
        else boundx0 = currUpX0;
        
        if (currUpX1 >= dstw) boundx1 = dstw;
        else boundx1 = currUpX1;

        updateInX();
    }

    __forceinline void updateDownX()
    {
        if (currDownX0 < 0) boundx0 = 0;
        else boundx0 = currDownX0;

        if (currDownX1 >= dstw) boundx1 = dstw;
        else boundx1 = currDownX1;

        updateInX();
    }

public:
    int32_t srcx;
    int32_t srcy;

    int32_t dstUpY;
    int32_t dstDownY;

    int32_t boundx0;
    int32_t inx0;
    int32_t inx1;
    int32_t boundx1;

public:
    bool intiClip(int32_t dcx, int32_t dcy, int32_t bwidth)
    {
        boundWidth = bwidth;
        dstDownY = dcx;
        currDownX0 = dcy;
        currDownX1 = currDownX0;

        if (findBegin(dstDownY, &currDownX0, currDownX1)) findEnd(dstDownY, currDownX0, &currDownX1);

        dstUpY = dstDownY;
        currUpX0 = currDownX0;
        currUpX1 = currDownX1;
        
        updateUpX();

        return currDownX0 < currDownX1;
    }

    bool nextLineDown()
    {
        dstDownY++;
        if (!findBegin(dstDownY, &currDownX0, currDownX1)) return false;
        findEnd(dstDownY, currDownX0, &currDownX1);
        updateDownX();
        return currDownX0 < currDownX1;
    }

    bool nextLineUp()
    {
        dstUpY--;
        if (!findBegin(dstUpY, &currUpX0, currUpX1)) return false;
        findEnd(dstUpY, currUpX0, &currUpX1);
        updateUpX();
        return currUpX0 < currUpX1;
    }
};

//fast calculate pixel at center, don't care boundary
static __forceinline uint32_t bicubicGetPixelCenter(const GFX_IMAGE* img, const int16_t* stable, const int32_t sx, const int32_t sy)
{
    const uint8_t px = sx >> 8, py = sy >> 8;
    const int16_t u0 = stable[256 + px], u1 = stable[px];
    const int16_t u2 = stable[256 - px], u3 = stable[512 - px];

    const __m128i xpart = _mm_setr_epi16(u0, u1, u2, u3, u0, u1, u2, u3); //U0 U1 U2 U3 U0 U1 U2 U3
    const __m128i ypart = _mm_setr_epi32(stable[256 + py], stable[py], stable[256 - py], stable[512 - py]);
    
    const uint32_t* psrc = (const uint32_t*)img->mData;
    const uint32_t *pixel0 = (const uint32_t*)&psrc[((sy >> 16) - 1) * img->mWidth + ((sx >> 16) - 1)];
    const uint32_t *pixel1 = &pixel0[img->mWidth];
    const uint32_t *pixel2 = &pixel1[img->mWidth];
    const uint32_t *pixel3 = &pixel2[img->mWidth];

    __m128i p0 = _mm_load_si128((const __m128i *)pixel0), p1 = _mm_load_si128((const __m128i *)pixel1); //P00 P01 P02 P03 P10 P11 P12 P13
    __m128i p2 = _mm_load_si128((const __m128i *)pixel2), p3 = _mm_load_si128((const __m128i *)pixel3); //P20 P21 P22 P23 P30 P31 P32 P33

    p0 = _mm_shuffle_epi8(p0, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15)); //B0 G0 R0 A0
    p1 = _mm_shuffle_epi8(p1, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15)); //B1 G1 R1 A1
    p2 = _mm_shuffle_epi8(p2, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15)); //B2 G2 R2 A2
    p3 = _mm_shuffle_epi8(p3, _mm_setr_epi8(0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15)); //B3 G3 R3 A3

    const __m128i bg01 = _mm_unpacklo_epi32(p0, p1); //B0 B1 G0 G1
    const __m128i ra01 = _mm_unpackhi_epi32(p0, p1); //R0 R1 A0 A1
    const __m128i bg23 = _mm_unpacklo_epi32(p2, p3); //B2 B3 G2 G3
    const __m128i ra23 = _mm_unpackhi_epi32(p2, p3); //R2 R3 A2 A3

    const __m128i b01 = _mm_unpacklo_epi8(bg01, _mm_setzero_si128());
    const __m128i b23 = _mm_unpacklo_epi8(bg23, _mm_setzero_si128());

    //P00 * U0 + P01 * U1 + P02 * U2 + P03 * U3
    const __m128i sb = _mm_hadd_epi32(_mm_madd_epi16(b01, xpart), _mm_madd_epi16(b23, xpart));

    const __m128i g01 = _mm_unpackhi_epi8(bg01, _mm_setzero_si128());
    const __m128i g23 = _mm_unpackhi_epi8(bg23, _mm_setzero_si128());

    //P10 * U0 + P11 * U1 + P12 * U2 + P13 * U3
    const __m128i sg = _mm_hadd_epi32(_mm_madd_epi16(g01, xpart), _mm_madd_epi16(g23, xpart));

    const __m128i r01 = _mm_unpacklo_epi8(ra01, _mm_setzero_si128());
    const __m128i r23 = _mm_unpacklo_epi8(ra23, _mm_setzero_si128());

    //P20 * U0 + P21 * U1 + P22 * U2 + P23 * U3
    const __m128i sr = _mm_hadd_epi32(_mm_madd_epi16(r01, xpart), _mm_madd_epi16(r23, xpart));

    const __m128i a01 = _mm_unpackhi_epi8(ra01, _mm_setzero_si128());
    const __m128i a23 = _mm_unpackhi_epi8(ra23, _mm_setzero_si128());

    //P30 * U0 + P31 * U1 + P32 * U2 + P33 * U3
    const __m128i sa = _mm_hadd_epi32(_mm_madd_epi16(a01, xpart), _mm_madd_epi16(a23, xpart));

    //P00 * U0 + P01 * U1 + P02 * U2 + P03 * U3 
    //P10 * U0 + P11 * U1 + P12 * U2 + P13 * U3 
    //P20 * U0 + P21 * U1 + P22 * U2 + P23 * U3 
    //P30 * U0 + P31 * U1 + P32 * U2 + P33 * U3 
    __m128i result = _mm_setr_epi32(
        _mm_hsum_epi32(_mm_mullo_epi32(sb, ypart)), //SB * V0
        _mm_hsum_epi32(_mm_mullo_epi32(sg, ypart)), //SG * V1
        _mm_hsum_epi32(_mm_mullo_epi32(sr, ypart)), //SR * V2
        _mm_hsum_epi32(_mm_mullo_epi32(sa, ypart))  //SA * V3
    );

    //SUM >> 16
    result = _mm_srai_epi32(result, 16);

    //CLAMP 0, 255
    return _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packus_epi32(result, result), result));
}

//this calculate pixel with boundary so quite slowly
static __forceinline uint32_t bicubicGetPixelBorder(const GFX_IMAGE* img, const int16_t *sintab, const int32_t sx, const int32_t sy)
{
    //peek offset at (px,py)
    const int32_t px = (sx >> 16) - 1, py = (sy >> 16) - 1;

    //calculate 16 pixels start (px-1, py-1), (px+2, py+2)
    uint32_t pixels[16] = { 0 };
    for (int32_t i = 0; i < 4; i++)
    {
        const int32_t y = py + i;
        for (int32_t j = 0; j < 4; j++)
        {
            const int32_t x = px + j;
            pixels[(i << 2) + j] = clampPixels(img, x, y);
        }
    }

    //construct matrix 16x16 pixels data
    GFX_IMAGE mpic;
    mpic.mData = (uint8_t*)pixels;
    mpic.mWidth = 4;
    mpic.mHeight = 4;
    mpic.mRowBytes = 16;

    //optimize function
    return bicubicGetPixelCenter(&mpic, sintab, (sx & 0xffff) + 0x10000, (sy & 0xffff) + 0x10000);
}
