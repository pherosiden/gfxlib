#pragma once

/*---------------------------------------------------------------*/
/*                 GFXLIB Graphics Library                       */
/*               Use SDL2 for render system                      */
/*               SDL2_image for load images                      */
/*            Target OS: cross-platform (x32_64)                 */
/*               Author: Nguyen Ngoc Van                         */
/*               Create: 22/10/2018                              */
/*              Version: 1.2.6                                   */
/*          Last Update: 2022-06-11                              */
/*              Website: http://codedemo.net                     */
/*                Email: pherosiden@gmail.com                    */
/*           References: https://crossfire-designs.de            */
/*                       https://lodev.org                       */
/*                       https://permadi.com                     */
/*                       https://sources.ru                      */
/*                       http://eyecandyarchive.com              */
/*              License: GNU GPL                                 */
/*---------------------------------------------------------------*/

#include <random>
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
//optimize by using MMX technology
//on modern system, don't use this option
#if !defined(__APPLE__) && !defined(_WIN64)
#define _USE_ASM
#pragma message("MMX technology is turned on. On modern system don't use this option!")
#endif

//GFX version string
#define GFX_VERSION             "v22.06.11"
#define GFX_BUILD_ID            20220611

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
#define GFX_FONT_SCALEABLE      0x10    //scalable font
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
#define BUTTON_STATE_NORMAL     0       //mouse state normal
#define BUTTON_STATE_ACTIVE     1       //mouse state active
#define BUTTON_STATE_PRESSED    2       //mouse state pressed
#define BUTTON_STATE_WAITING    3       //mouse state waiting
#define BUTTON_STATE_DISABLED   4       //mouse state disable

//button and mouse sprite count
#define BUTTON_HANDLE_COUNT     2       //number of button to handle
#define MOUSE_SPRITE_COUNT      9       //number of mouse sprite
#define BUTTON_STATE_COUNT      4       //number of button state

//fill polygon constant
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
#define LOWORD(a)               ((a) & 0xffff)
#define HIWORD(a)               (((a) >> 16) & 0xffff)
#endif

//inline common
#ifdef __APPLE__
#define must_inline             __attribute__((always_inline))
#else
#define must_inline             __forceinline
#endif

//RGB common colors
#define RGB_BLACK               0x000000
#define RGB_WHITE               0xffffff
#define RGB_RED                 0xff0000
#define RGB_GREEN               0x00ff00
#define RGB_BLUE                0x0000ff
#define RGB_YELLOW              0xffff00
#define RGB_CYAN                0x00ffff
#define RGB_MAGENTA             0xff00ff
#define RGB_PURPLE              0x800080
#define RGB_MAROON              0x800000
#define RGB_DARK_RED            0xc00000
#define RGB_DARK_GREY           0x808080
#define RGB_LIGHT_GREY          0xc0c0c0
#define RGB_DARK_GREEN          0x008000
#define RGB_NAVY                0x000080
#define RGB_TEAL                0x008080
#define RGB_OLIVE               0x808000
#define RGB_GREY32              0x202020
#define RGB_GREY64              0x404040
#define RGB_GREY127             0x7f7f7f
#define RGB_GREY191             0xbfbfbf

//benchmarks snipping code
#define BENCH_START()           clock_t startClock = clock();
#define BENCH_END()             messageBox(GFX_INFO, "Total time: %lu(ms)", clock() - startClock);

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
    uint8_t         code;                       //stroke code (0: unused, 1: move to, 2: line to)
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
    uint16_t        descender;                  //font descender
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
    uint8_t         copyright[32];              //font copy-right (use for BGI font)
    uint8_t         fontType[4];                //font type BMP1, BMP8, VECT, ...
    uint16_t        subFonts;                   //number of sub-fonts (difference size)
    uint32_t        memSize;                    //bytes on load raw data (use this to alloc memory)
    uint32_t        flags;                      //font flags (ANIPOS, ANIMATION, MULTI, ...)
    GFX_CHAR_HEADER subData;                    //sub-fonts data info
} GFX_FONT_HEADER;

//GFX loaded font memory
typedef struct
{
    GFX_FONT_HEADER hdr;                        //font header
    uint8_t*        dataPtr;                    //font raw data
} GFX_FONT;

//the structure of image data (base image for GFXLIB)
typedef struct
{
    int32_t         mWidth;                     //image width
    int32_t         mHeight;                    //image height
    uint32_t        mSize;                      //image size in bytes
    uint32_t        mRowBytes;                  //bytes per scan line
    void*           mData;                      //image raw data
} GFX_IMAGE;

//the structure for animated mouse pointers
typedef struct tagMOUSEBITMAP GFX_BITMAP;
struct tagMOUSEBITMAP
{
    int32_t         mbHotX;                     //mouse hot spot x
    int32_t         mbHotY;                     //mouse hot spot y
    uint8_t*        mbData;                     //mouse bitmap data
    GFX_BITMAP*     mbNext;                     //point to next mouse data
};

//the structure for a bitmap mouse pointer.
typedef struct
{
    int32_t         msPosX;                     //current position x
    int32_t         msPosY;                     //current position y
    int32_t         msWidth;                    //mouse image width
    int32_t         msHeight;                   //mouse image height
    uint8_t*        msUnder;                    //mouse under background
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

// rotate clip data
typedef struct
{
    int32_t srcw, srch;                         // source width and height
    int32_t dstw, dsth;                         // destination width and height
    int32_t srcx, srcy;                         // source strart x, y

    int32_t ax, ay;                             // left, right
    int32_t bx, by;                             // top, bottom
    int32_t cx, cy;                             // center x, y

    int32_t boundWidth;                         // boundary width (pixels units)
    int32_t currUp0, currUp1;                   // current up
    int32_t currDown0, currDown1;               // current down

    int32_t yUp, yDown;                         // y top and down

    int32_t outBound0, outBound1;               // in-bound and out-bound
    int32_t inBound0, inBound1;                 // in-bound and out-bound
} ROTATE_CLIP;

#pragma pack(pop)

//pixel blending mode
enum BLEND_MODE {
    BLEND_MODE_NORMAL,                          //this is a normal mode
    BLEND_MODE_ADD,                             //add with background color
    BLEND_MODE_SUB,                             //sub with background color
    BLEND_MODE_AND,                             //logical and with background color (only for fillRect and putImage functions)
    BLEND_MODE_XOR,                             //logical x-or with background color (only for fillRect and putImage functions)
    BLEND_MODE_ALPHA,                           //alpha blending with background color
    BLEND_MODE_ANTIALIASED                      //anti-aliased edge (use for line, circle, ellipse, cubic, bezier curve)
};

//image interpolation type (apply for scale, rotate, ...)
enum INTERPOLATION_TYPE
{
    INTERPOLATION_TYPE_NORMAL,                  //Bresenham interpolation (nearest and smooth)
    INTERPOLATION_TYPE_NEARST,                  //nearest neighbor (low quality)
    INTERPOLATION_TYPE_SMOOTH,                  //use average pixels to smooth image (normal quality)
    INTERPOLATION_TYPE_BILINEAR,                //bi-linear interpolation (good quality)
    INTERPOLATION_TYPE_BICUBIC,                 //bi-cubic interpolation (best quality)
    INTERPOLATION_TYPE_UNKNOWN                  //error type
};

//3D projection type
enum PROJECTION_TYPE
{
    PROJECTION_TYPE_PERSPECTIVE,                //perspective projection
    PROJECTION_TYPE_PARALLELE,                  //paralleled projection
    PROJECTION_TYPE_UNKNOWN,                    //error projection
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
int32_t     getDataX();
int32_t     getDataY();

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
int32_t     initScreen(int32_t width = SCREEN_WIDTH, int32_t height = SCREEN_HEIGHT, int32_t bpp = 8, int32_t scaled = 0, const char* text = "", int32_t resizeable = 0);
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
int32_t     getBufferWidth();
int32_t     getBufferHeight();

//handle program message
void        messageBox(int32_t type, const char* fmt, ...);
void        writeText(int32_t x, int32_t y, uint32_t txtColor, uint32_t mode, const char* format, ...);
int32_t     drawText(int32_t ypos, int32_t size, const char** str);

void        clearScreen(uint32_t color = 0);

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
void        drawPolygon(POINT2D* point, int32_t num, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        initProjection(double theta, double phi, double de, double rho = 0);
void        resetProjection();
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
void        randomPolygon(const int32_t cx, const int32_t cy, const int32_t avgRadius, double irregularity, double spikeyness, const int32_t numVerts, POINT2D* points);

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
void        fadeCircle(int32_t dir, uint32_t col, uint32_t mswait = FPS_90);
void        fadeRollo(int32_t dir, uint32_t col, uint32_t mswait = FPS_90);
void        fadeOutImage(GFX_IMAGE* img, uint8_t step);

//some FX-effect functions
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

//8/32-pixels alignment for AVX2 use (size = (32 / bytesPerPixel) - 1)
static must_inline int32_t alignedSize(int32_t msize)
{
    return (getBytesPerPixel() == 1) ? (msize + 31) & ~31 : (msize + 7) & ~7;
}

//32-bytes alignment for AVX2 use (all memory must be 32 bytes aligned)
static must_inline uint32_t alignedBytes(uint32_t msize)
{
    return (msize + 31) & ~31;
}

//convert r,g,b values to 32bits integer value
static must_inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

//merge rgb and alpha channel to packed color
static must_inline uint32_t rgba(uint32_t col, uint8_t alpha)
{
    return (uint32_t(alpha) << 24) | col;
}

//HSL to RGB convert
static must_inline uint32_t hsl2rgb(int32_t hi, int32_t si, int32_t li)
{
    double r = 0.0, g = 0.0, b = 0.0;
    double t1 = 0.0, t2 = 0.0;
    double tr = 0.0, tg = 0.0, tb = 0.0;

    const double h = hi / 256.0;
    const double s = si / 256.0;
    const double l = li / 256.0;

    //if saturation is 0, the color is a shade of gray
    if (s == 0.0) r = g = b = l;

    //if saturation > 0, more complex calculations are needed
    else
    {
        //set the temporary values
        if (l < 0.5) t2 = l * (1 + s);
        else t2 = (l + s) - (l * s);

        t1 = 2 * l - t2;
        tr = h + 1.0 / 3.0;

        if (tr > 1.0) tr--;

        tg = h;
        tb = h - 1.0 / 3.0;

        if (tb < 0.0) tb++;

        //red
        if (tr < 1.0 / 6.0) r = t1 + (t2 - t1) * 6.0 * tr;
        else if (tr < 0.5) r = t2;
        else if (tr < 2.0 / 3.0) r = t1 + (t2 - t1) * ((2.0 / 3.0) - tr) * 6.0;
        else r = t1;

        //green
        if (tg < 1.0 / 6.0) g = t1 + (t2 - t1) * 6.0 * tg;
        else if (tg < 0.5) g = t2;
        else if (tg < 2.0 / 3.0) g = t1 + (t2 - t1) * ((2.0 / 3.0) - tg) * 6.0;
        else g = t1;

        //blue
        if (tb < 1.0 / 6.0) b = t1 + (t2 - t1) * 6.0 * tb;
        else if (tb < 0.5) b = t2;
        else if (tb < 2.0 / 3.0) b = t1 + (t2 - t1) * ((2.0 / 3.0) - tb) * 6.0;
        else b = t1;
    }

    uint32_t col = 0;
    ARGB* pcol = (ARGB*)&col;
    pcol->r = uint8_t(r * 255);
    pcol->g = uint8_t(g * 255);
    pcol->b = uint8_t(b * 255);
    return col;
}

//HSV to RGB convert
static must_inline uint32_t hsv2rgb(int32_t hi, int32_t si, int32_t vi)
{
    double h = hi / 256.0;
    const double s = si / 256.0;
    const double v = vi / 256.0;
    double r = 0.0, g = 0.0, b = 0.0;

    //if saturation is 0, the color is a shade of gray
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
    ARGB* pcol = (ARGB*)&col;
    pcol->r = uint8_t(r * 255);
    pcol->g = uint8_t(g * 255);
    pcol->b = uint8_t(b * 255);
    return col;
}

//converts an RGB color to HSV color
static must_inline HSV rgb2hsv(uint8_t ri, uint8_t gi, uint8_t bi)
{
    const double r = ri / 256.0;
    const double g = gi / 256.0;
    const double b = bi / 256.0;

    const double dmax = max(r, max(g, b));
    const double dmin = min(r, min(g, b));
    const double v = dmax;
    
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
static must_inline HSL rgb2hsl(uint8_t ri, uint8_t gi, uint8_t bi)
{
    const double r = ri / 255.0;
    const double g = gi / 255.0;
    const double b = bi / 255.0;

    const double dmax = max(r, max(g, b));
    const double dmin = min(r, min(g, b));
    const double l = (dmax + dmin) / 2;

    double h = 0, s = 0;

    if (dmax == dmin)
    {
        h = s = 0;
    }
    else
    {
        const double dt = dmax - dmin;
        s = (l > 0.5) ? dt / (2 - dmax - dmin) : dt / (dmax + dmin);
        if (r == dmax) h = (g - b) / dt + (g < b ? 6 : 0);
        if (g == dmax) h = (b - r) / dt + 2;
        if (b == dmax) h = (r - g) / dt + 4;
        h /= 6;
    }

    HSL col = { 0 };
    col.h = int32_t(h * 360);
    col.s = int32_t(s * 100);
    col.l = int32_t(l * 100);
    return col;
}

//generate random value from number
static must_inline int32_t random(int32_t a)
{
    return a ? rand() % a : 0;
}

//generate random value in range
static must_inline int32_t random(int32_t a, int32_t b)
{
    return (a < b) ? (a + (rand() % (b - a + 1))) : (b + (rand() % (a - b + 1)));
}

//generate double random in range
static must_inline double frand(double fmin, double fmax)
{
    const double fn = double(rand()) / RAND_MAX;
    return fmin + fn * (fmax - fmin);
}

//round-up function
static must_inline int32_t fround(double x)
{
    return (x > 0) ? int32_t(x + 0.5) : int32_t(x - 0.5);
}

static must_inline double uniformRand(const double from, const double to)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distr(from, to);
    return distr(gen);
}

static must_inline double gaussianRand(const double min, const double max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> distr(min, max);
    return distr(gen);
}

static must_inline bool pointInBound(const ROTATE_CLIP* clip, const int32_t scx, const int32_t scy)
{
    return  (((scx >= (-(clip->boundWidth << 16))) && ((scx >> 16) < (clip->srcw + clip->boundWidth))) &&
             ((scy >= (-(clip->boundWidth << 16))) && ((scy >> 16) < (clip->srch + clip->boundWidth))));
}

static must_inline bool pointInSrc(const ROTATE_CLIP* clip, const int32_t scx, const int32_t scy)
{
    return  (((scx >= (clip->boundWidth << 16)) && ((scx >> 16) < (clip->srcw - clip->boundWidth))) &&
             ((scy >= (clip->boundWidth << 16)) && ((scy >> 16) < (clip->srch - clip->boundWidth))));
}

static must_inline void findBeginIn(const ROTATE_CLIP* clip, int32_t* dstx, int32_t* scx, int32_t* scy)
{
    *scx -= clip->ax;
    *scy -= clip->ay;

    while (pointInBound(clip, *scx, *scy))
    {
        (*dstx)--;
        *scx -= clip->ax;
        *scy -= clip->ay;
    }

    *scx += clip->ax;
    *scy += clip->ay;
}

static must_inline bool findBegin(ROTATE_CLIP* clip, const int32_t dsty, int32_t* dstx0, const int32_t dstx1)
{
    const int32_t testx0 = *dstx0 - 1;
    int32_t scx = clip->ax * testx0 + clip->bx * dsty + clip->cx;
    int32_t scy = clip->ay * testx0 + clip->by * dsty + clip->cy;

    for (int32_t i = testx0; i <= dstx1; i++)
    {
        if (pointInBound(clip, scx, scy))
        {
            *dstx0 = i;

            if (i == testx0) findBeginIn(clip, dstx0, &scx, &scy);

            if (*dstx0 < 0)
            {
                scx -= clip->ax * (*dstx0);
                scy -= clip->ay * (*dstx0);
            }

            clip->srcx = scx;
            clip->srcy = scy;

            return true;
        }
        else
        {
            scx += clip->ax;
            scy += clip->ay;
        }
    }

    return false;
}

static must_inline void findEnd(const ROTATE_CLIP* clip, const int32_t dsty, const int32_t dstx0, int32_t* dstx1)
{
    int32_t testx1 = *dstx1;
    if (testx1 < dstx0) testx1 = dstx0;

    int32_t scx = clip->ax * testx1 + clip->bx * dsty + clip->cx;
    int32_t scy = clip->ay * testx1 + clip->by * dsty + clip->cy;

    if (pointInBound(clip, scx, scy))
    {
        testx1++;
        scx += clip->ax;
        scy += clip->ay;

        while (pointInBound(clip, scx, scy))
        {
            testx1++;
            scx += clip->ax;
            scy += clip->ay;
        }

        *dstx1 = testx1;
    }
    else
    {
        scx -= clip->ax;
        scy -= clip->ay;
        while (!pointInBound(clip, scx, scy))
        {
            testx1--;
            scx -= clip->ax;
            scy -= clip->ay;
        }

        *dstx1 = testx1;
    }
}

static must_inline void updateInX(ROTATE_CLIP* clip)
{
    if (!clip->boundWidth || clip->outBound0 >= clip->outBound1)
    {
        clip->inBound0 = clip->outBound0;
        clip->inBound1 = clip->outBound1;
    }
    else
    {
        int32_t scx = clip->srcx;
        int32_t scy = clip->srcy;
        int32_t i = clip->outBound0;

        while (i < clip->outBound1)
        {
            if (pointInSrc(clip, scx, scy)) break;
            scx += clip->ax;
            scy += clip->ay;
            i++;
        }

        clip->inBound0 = i;

        scx = clip->srcx + (clip->outBound1 - clip->outBound0) * clip->ax;
        scy = clip->srcy + (clip->outBound1 - clip->outBound0) * clip->ay;

        i = clip->outBound1;

        while (i > clip->inBound0)
        {
            scx -= clip->ax;
            scy -= clip->ay;
            if (pointInSrc(clip, scx, scy)) break;
            i--;
        }

        clip->inBound1 = i;
    }
}

static must_inline void updateUpX(ROTATE_CLIP* clip)
{
    if (clip->currUp0 < 0) clip->outBound0 = 0;
    else clip->outBound0 = clip->currUp0;
        
    if (clip->currUp1 >= clip->dstw) clip->outBound1 = clip->dstw;
    else clip->outBound1 = clip->currUp1;

    updateInX(clip);
}

static must_inline void updateDownX(ROTATE_CLIP* clip)
{
    if (clip->currDown0 < 0) clip->outBound0 = 0;
    else clip->outBound0 = clip->currDown0;

    if (clip->currDown1 >= clip->dstw) clip->outBound1 = clip->dstw;
    else clip->outBound1 = clip->currDown1;

    updateInX(clip);
}

static must_inline bool intiClip(ROTATE_CLIP* clip, const int32_t dcx, const int32_t dcy, const int32_t bwidth)
{
    clip->boundWidth = bwidth;
    clip->yDown = dcx;
    clip->currDown0 = dcy;
    clip->currDown1 = dcy;

    if (findBegin(clip, clip->yDown, &clip->currDown0, clip->currDown1)) findEnd(clip, clip->yDown, clip->currDown0, &clip->currDown1);

    clip->yUp = clip->yDown;
    clip->currUp0 = clip->currDown0;
    clip->currUp1 = clip->currDown1;
        
    updateUpX(clip);

    return clip->currDown0 < clip->currDown1;
}

static must_inline bool nextLineDown(ROTATE_CLIP* clip)
{
    clip->yDown++;
    if (!findBegin(clip, clip->yDown, &clip->currDown0, clip->currDown1)) return false;
    findEnd(clip, clip->yDown, clip->currDown0, &clip->currDown1);
    updateDownX(clip);
    return clip->currDown0 < clip->currDown1;
}

static must_inline bool nextLineUp(ROTATE_CLIP* clip)
{
    clip->yUp--;
    if (!findBegin(clip, clip->yUp, &clip->currUp0, clip->currUp1)) return false;
    findEnd(clip, clip->yUp, clip->currUp0, &clip->currUp1);
    updateUpX(clip);
    return clip->currUp0 < clip->currUp1;
}
