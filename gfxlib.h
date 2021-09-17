#pragma once

//===============================================================//
//                 GFXLIB Graphics Library                       //
//               Use SDL2 for render system                      //
//               SDL2_image for load images                      //
//            Target OS: cross-platform (x32_64)                 //
//               Author: Nguyen Ngoc Van                         //
//               Create: 22/10/2018                              //
//              Version: 1.2.1                                   //
//          Last Update: 2021-09-11                              //
//              Website: http://codedemo.net                     //
//                Email: pherosiden@gmail.com                    //
//           References: https://crossfire-designs.de            //
//                       https://lodev.org                       //
//                       https://permadi.com                     //
//                       https://sources.ru                      //
//                       http://eyecandyarchive.com              //
//              License: MIT                                     //
//===============================================================//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
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

//use this to optimize performance
//(some effects will work smoothly)
//just for x32 and old compiler only
//default mode is used MMX technology
#if !defined(_WIN64) && !defined(__APPLE__)
#define _USE_ASM
#pragma message("Build with assembly code for maximum performance...")
#endif

//GFX version string
#define GFX_VERSION             "v21.09.11"
#define GFX_BUILD_ID            20210911

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

//re-defined, some compiler does not define yet
#define sqr(a)                  ((a) * (a))
#define max(a, b)               ((a) > (b) ? (a) : (b))
#define min(a, b)               ((a) < (b) ? (a) : (b))
#define sign(x)                 (((x) >= 0) ? (1) : (-1))
#define swap(a, b)              {a ^= b; b ^= a; a ^= b;}

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

//benchmark snip code
#define BENCH_START()           {startClock = clock();}
#define BENCH_END()             {messageBox(GFX_INFO, "Total time:%lf", double(clock() - startClock) / CLOCKS_PER_SEC);}

#pragma pack(push, 1)

//redefine RGB color
typedef SDL_Color RGB;

//2D point struct
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
    INTERPOLATION_TYPE_NORMAL,                  //Bresenham scale (nearest and smooth, very fast)
    INTERPOLATION_TYPE_NEARST,                  //nearest neighbor (fastest, but low quality)
    INTERPOLATION_TYPE_SMOOTH,                  //use average to smooth image (fast, still low quality)
    INTERPOLATION_TYPE_BILINEAR,                //bi-linear interpolation (slow, good quality)
    INTERPOLATION_TYPE_BICUBIC                  //bi-cubic interpolation (very slow, best quality)
};

#pragma pack(pop)
extern int32_t  texWidth;                       //current texture width
extern int32_t  texHeight;                      //current texture height
extern int32_t  bitsPerPixel;                   //bits per pixel (8/15/16/24/32)
extern int32_t  bytesPerPixel;                  //bytes per pixel (1/2/3/4)
extern int32_t  bytesPerScanline;               //bytes per scanline

//clip cordinate handle
extern int32_t  centerX, centerY;               //center screen points
extern int32_t  cminX, cminY, cmaxX, cmaxY;     //view port clip points

//3D projection
enum PROJ_TYPE { PERSPECTIVE, PARALLELE };      //projection type values
extern double   DE, rho, theta, phi;            //projection angles
extern double   aux1, aux2, aux3, aux4;         //temponary values
extern double   aux5, aux6, aux7, aux8;         //temponary values
extern double   obsX, obsY, obsZ;               //X,Y,Z coordinate
extern double   projX, projY;                   //projection X,Y
extern uint8_t  projection;                     //projection type

extern GFX_FONT gfxFonts[GFX_MAX_FONT];         //GFX font loadable at the same time
extern uint8_t* fontPalette[GFX_MAX_FONT];      //GFX font palette data (BMP8 type)
extern uint8_t* gfxBuff;                        //GFX buffer
extern uint32_t subFonts;                       //GFX sub-fonts
extern uint32_t fontType;                       //current selected font (use for multiple loaded font)
extern uint32_t randSeed;                       //global random seed
extern uint32_t factor;                         //global factor

//pattern filled styles
extern uint8_t  ptnLine[];                      //line fill style
extern uint8_t  ptnLiteSlash[];                 //line with slash style
extern uint8_t  ptnSlash[];                     //slash style
extern uint8_t  ptnBackSlash[];                 //back slash
extern uint8_t  ptnLiteBackSlash[];             //lite back slash
extern uint8_t  ptnHatch[];                     //hatch style
extern uint8_t  ptnHatchX[];                    //hatch x style
extern uint8_t  ptnInterLeave[];                //inter leaving style
extern uint8_t  ptnWideDot[];                   //wide dot style
extern uint8_t  ptnCloseDot[];                  //closed dot style

//benchmart record time
extern clock_t  startClock;                     //recording start clock time

//load texture and image functions
int32_t     loadTexture(uint32_t** texture, int32_t* txw, int32_t* txh, const char* fname);
int32_t     loadPNG(uint8_t* raw, RGB* pal, const char* fname);
int32_t     loadImage(const char* fname, GFX_IMAGE* im);
void        freeImage(GFX_IMAGE* im);

//GFXLIB font functions
int32_t     getFontWidth(const char* str);
int32_t     getFontHeight(const char* str);
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
int32_t     waitKeyPressed();
int32_t     finished(int32_t key);

//some customize random functions
void        randomBuffer(void* buff, int32_t count, int32_t range);

//system info
void        initSystemInfo();
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
void        changeViewPort(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void        restoreViewPort();
void        render();
void        cleanup();
void        renderBuffer(const void* buffer, uint32_t size);
void*       getDrawBuffer(int32_t* width = NULL, int32_t* height = NULL);
void        changeDrawBuffer(void* newBuff, int32_t newWidth, int32_t newHeight);
void        restoreDrawBuffer();

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

void        initProjection();
void        projette(double x, double y, double z);
void        deplaceEn(double x, double y, double z);
void        traceVers(double x, double y, double z, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        fillRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        fillRectPattern(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, uint8_t* pattern, int32_t mode = BLEND_MODE_NORMAL);

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
void        rotateImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t degree, int32_t type = INTERPOLATION_TYPE_SMOOTH);

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
void        handleMouseButton();
void        showPNG(const char* fname);
void        showBMP(const char* fname);

//other pixels fx
void        putPixelBob(int32_t x, int32_t y);
void        drawLineBob(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

//export demo function (not included in GFXLIB)
void        gfxDemoMix();
void        gfxDemo();
void        gfxEffectsMix();
void        gfxEffects();
void        gfxFontView();

/*=============================================================================*/
/*                             INLINE FUNCTIONS                                */
/*=============================================================================*/

// Fixed-point math routines (signed 19.12)
// Largest positive value:  524287.999755859375
// Smallest positive value: 0.000244140625

#define FRAC_BITS       12           //bits number
#define INT_MASK        0x7FFFF000   // 20 bits
#define FIXED_1         4096         // (1 << FRAC_BITS)
#define FIXED_255       1044480      // (255 << FRAC_BITS)
#define FIXED_HALF      2048         // (fixed_t)(0.5 * (float)(1L << FRAC_BITS) + 0.5)
#define FIXED_EPSILON   1
#define FIXED_TO_INT(x) ((int32_t)(x < 0 ? 0 : (x > FIXED_255) ? 255 : fixtoi(x + FIXED_HALF)))

//new fixed type
typedef int32_t fixed_t;

inline fixed_t itofix(int32_t x)
{
    return x << FRAC_BITS;
}

inline int32_t fixtoi(fixed_t x)
{
    return x >> FRAC_BITS;
}

inline fixed_t ftofix(float x)
{
    return ((fixed_t)((x) * (float)(1L << FRAC_BITS) + 0.5));
}

inline float fixtof(fixed_t x)
{
    return ((float)((x) / (float)(1L << FRAC_BITS)));
}

#if defined(__APPLE__)
#if defined(__arm__)
inline fixed_t fixmul(fixed_t x, fixed_t y)
{
    fixed_t __hi, __lo, __result;

    __asm__ __volatile__(
        "smull %0, %1, %3, %4\n\t"
        "movs %0, %0, lsr %5\n\t"
        "add %2, %0, %1, lsl %6"
        : "=&r" (__lo), "=&r" (__hi), "=r" (__result)
        : "%r" (x), "r" (y), "M" (FRAC_BITS), "M" (32 - (FRAC_BITS))
        : "cc"
    );

    return __result;
}
#elif defined(__i386__) || defined(__x86_64__)
// This improves fixed-point performance about 15-20% on x86
inline fixed_t fixmul(fixed_t x, fixed_t y)
{
    fixed_t __hi, __lo;

    __asm__ __volatile__(
        "imull %3\n"
        "shrdl %4, %1, %0"
        : "=a"(__lo), "=d"(__hi)
        : "%a"(x), "rm"(y), "I"(FRAC_BITS)
        : "cc"
    );

    return __lo;
}
#else
inline fixed_t fixmul(fixed_t x, fixed_t y)
{
    return (fixed_t)(((int64_t)x * y) >> FRAC_BITS);
}
#endif
#elif !defined(_WIN64) //x86 Windows
inline fixed_t fixmul(fixed_t x, fixed_t y)
{
    __asm {
        mov eax, x
        imul y
        shrd eax, edx, FRAC_BITS
    }
    //eax is returned automatically
}
#else //x64 Windows
inline fixed_t fixmul(fixed_t x, fixed_t y)
{
    return (fixed_t)(((int64_t)x * y) >> FRAC_BITS);
}
#endif

inline fixed_t fixdiv(fixed_t x, fixed_t y)
{
    return (fixed_t)(((int64_t)x << FRAC_BITS) / y);
}

inline fixed_t fixfloor(fixed_t x)
{
    return x & INT_MASK;
}

inline fixed_t fixceil(fixed_t x)
{
    return (fixed_t)(x < 0 ? 0 : (x > FIXED_255) ? 255 : fixtoi(x + FIXED_HALF));
}

//convert r,g,b values to 32bits integer value
inline uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

//merge rgb and alpha channel to packed color
inline uint32_t rgba(uint32_t col, uint8_t alpha)
{
    return (uint32_t(alpha) << 24) | col;
}

//HSL to RGB convert
inline uint32_t hsl(int32_t hi, int32_t si, int32_t li)
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

    return rgb(int32_t(r * 255), int32_t(g * 255), int32_t(b * 255));
}

//HSV to RGB convert
inline uint32_t hsv(int32_t hi, int32_t si, int32_t vi)
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

    return rgb(int32_t(r * 255), int32_t(g * 255), int32_t(b * 255));
}

//generate random value from number
inline int32_t random(int32_t a)
{
    return a ? rand() % a : 0;
}

//generate random value in range
inline int32_t randomRange(int32_t a, int32_t b)
{
    return (a < b) ? (a + (rand() % (b - a + 1))) : (b + (rand() % (a - b + 1)));
}

//generate double random in ranage
inline double frand(double fmin, double fmax)
{
    const double fn = double(rand()) / RAND_MAX;
    return fmin + fn * (fmax - fmin);
}

//round-up function
inline int32_t roundf(double x)
{
    return int32_t(ceil(x));
}

//bicubic helper
inline float cubicHermite(float a, float b, float c, float d, float fract)
{
    const float aa = -a / 2.0f + 1.5f * b - 1.5f * c + d / 2.0f;
    const float bb = a - 2.5f * b + 2.0f * c - d / 2.0f;
    const float cc = -a / 2.0f + c / 2.0f;
    return aa * fract * fract * fract + bb * fract * fract + cc * fract + b;
}

//get source pixel
inline uint32_t clampOffset(const int32_t width, const int32_t height, int32_t x, int32_t y)
{
    //x-range check
    if (x < 0) x = 0;
    if (x > width - 1) x = width - 1;

    //y-range check
    if (y < 0) y = 0;
    if (y > height - 1) y = height - 1;

    //return image offset
    return y * width + x;
}

//calculate pixel by bicubic interpolation
inline uint32_t bicubicGetPixel(const uint32_t* psrc, const int32_t width, const int32_t height, const float sx, const float sy)
{
    const int32_t px = int32_t(sx);
    const float fx = sx - int32_t(sx);
   
    const int32_t py = int32_t(sy);
    const float fy = sy - int32_t(sy);

    //1st row
    const uint8_t* p00 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py - 1)];
    const uint8_t* p10 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py - 1)];
    const uint8_t* p20 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py - 1)];
    const uint8_t* p30 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py - 1)];

    //2nd row
    const uint8_t* p01 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py)];
    const uint8_t* p11 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py)];
    const uint8_t* p21 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py)];
    const uint8_t* p31 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py)];

    //2th row
    const uint8_t* p02 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py + 1)];
    const uint8_t* p12 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py + 1)];
    const uint8_t* p22 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py + 1)];
    const uint8_t* p32 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py + 1)];

    //4th row
    const uint8_t* p03 = (const uint8_t*)&psrc[clampOffset(width, height, px - 1, py + 2)];
    const uint8_t* p13 = (const uint8_t*)&psrc[clampOffset(width, height, px    , py + 2)];
    const uint8_t* p23 = (const uint8_t*)&psrc[clampOffset(width, height, px + 1, py + 2)];
    const uint8_t* p33 = (const uint8_t*)&psrc[clampOffset(width, height, px + 2, py + 2)];

    //mapping destination pointer
    uint32_t dst = 0;
    uint8_t* pdst = (uint8_t*)&dst;

    //start interpolate bicubically
    for (int32_t i = 0; i < 3; i++)
    {
        const float col0 = cubicHermite(p00[i], p10[i], p20[i], p30[i], fx);
        const float col1 = cubicHermite(p01[i], p11[i], p21[i], p31[i], fx);
        const float col2 = cubicHermite(p02[i], p12[i], p22[i], p32[i], fx);
        const float col3 = cubicHermite(p03[i], p13[i], p23[i], p33[i], fx);
        float val = cubicHermite(col0, col1, col2, col3, fy);

        //saturation check
        if (val < 0.0f) val = 0.0f;
        if (val > 255.0f) val = 255.0f;
        pdst[i] = uint8_t(val);
    }

    return dst;
}

//bilinear get pixel with FIXED-POINT
inline uint32_t bilinearGetPixelFIXED(const uint32_t* psrc, const int32_t width, const float sx, const float sy)
{
    /*//convert to fixed point
    const fixed_t dx = ftofix(sx);
    const fixed_t dy = ftofix(sy);

    const int32_t mx = int32_t(sx);
    const int32_t my = int32_t(sy);

    //calculate fraction
    const fixed_t fx  = ftofix(sx) - dx;
    const fixed_t fy  = ftofix(sy) - dy;
    const fixed_t fx1 = ftofix(1.0)  - fx;
    const fixed_t fy1 = ftofix(1.0)  - fy;

    //calculate the weights for each pixel
    const fixed_t w1 = fixmul(fixmul(fx1, fy1), ftofix(256.0));
    const fixed_t w2 = fixmul(fixmul(fx,  fy1), ftofix(256.0));
    const fixed_t w3 = fixmul(fixmul(fx1,  fy), ftofix(256.0));
    const fixed_t w4 = fixmul(fixmul(fx,   fy), ftofix(256.0));

    //pointer to first pixel
    const uint32_t* p0 = psrc + intptr_t(my) * width + intptr_t(mx);

    //load the 4 neighboring pixels
    const uint8_t* p1 = (const uint8_t*)&p0[0];
    const uint8_t* p2 = (const uint8_t*)&p0[1];
    const uint8_t* p3 = (const uint8_t*)&p0[width];
    const uint8_t* p4 = (const uint8_t*)&p0[width + 1];

    //mapping destination pointer
    uint32_t col = 0;
    uint8_t* pdst = (uint8_t*)&col;

    //sum of pixels with weighted (for each channel)
    pdst[2] = (p1[2] * fixtoi(w1) + p2[2] * fixtoi(w2) + p3[2] * fixtoi(w3) + p4[2] * fixtoi(w4)) >> 8;
    pdst[1] = (p1[1] * fixtoi(w1) + p2[1] * fixtoi(w2) + p3[1] * fixtoi(w3) + p4[1] * fixtoi(w4)) >> 8;
    pdst[0] = (p1[0] * fixtoi(w1) + p2[0] * fixtoi(w2) + p3[0] * fixtoi(w3) + p4[0] * fixtoi(w4)) >> 8;
    return col;*/

    //convert to fixed point
    const int32_t dx = int32_t(sx * 256);
    const int32_t dy = int32_t(sy * 256);

    //calculate fraction
    const int32_t fx  = dx & 0xff;
    const int32_t fy  = dy & 0xff;
    const int32_t fx1 = 256 - fx;
    const int32_t fy1 = 256 - fy;

    //calculate the weights for each pixel
    const int32_t w1 = (fx1 * fy1) >> 8;
    const int32_t w2 = (fx  * fy1) >> 8;
    const int32_t w3 = (fx1 *  fy) >> 8;
    const int32_t w4 = (fx  *  fy) >> 8;

    //pointer to first pixel
    const uint32_t* p0 = &psrc[intptr_t(sy) * width + intptr_t(sx)];

    //load the 4 neighboring pixels
    const uint8_t* p1 = (const uint8_t*)&p0[0];
    const uint8_t* p2 = (const uint8_t*)&p0[1];
    const uint8_t* p3 = (const uint8_t*)&p0[width];
    const uint8_t* p4 = (const uint8_t*)&p0[width + 1];

    //mapping destination pointer
    uint32_t col = 0;
    uint8_t* pdst = (uint8_t*)&col;

    //sum of pixels with weighted (for each channel)
    pdst[2] = (p1[2] * w1 + p2[2] * w2 + p3[2] * w3 + p4[2] * w4) >> 8;
    pdst[1] = (p1[1] * w1 + p2[1] * w2 + p3[1] * w3 + p4[1] * w4) >> 8;
    pdst[0] = (p1[0] * w1 + p2[0] * w2 + p3[0] * w3 + p4[0] * w4) >> 8;
    return col;
}

//constant values that will be needed
static const __m128 CONST_1 = _mm_set_ps1(1);
static const __m128 CONST_256 = _mm_set_ps1(256);

//calculate weight of pixel at (x,y)
inline __m128 calcWeights(const float x, const float y)
{
    __m128 xmm0 = _mm_set_ps1(x);
    __m128 xmm1 = _mm_set_ps1(y);
    __m128 xmm2 = _mm_unpacklo_ps(xmm0, xmm1);

    xmm0 = _mm_cvtepi32_ps(_mm_cvttps_epi32(xmm2));
    xmm1 = _mm_sub_ps(xmm2, xmm0);
    xmm2 = _mm_sub_ps(CONST_1, xmm1);

    __m128 xmm3 = _mm_unpacklo_ps(xmm2, xmm1);
    xmm3 = _mm_movelh_ps(xmm3, xmm3);

    __m128 xmm4 = _mm_shuffle_ps(xmm2, xmm1, _MM_SHUFFLE(1, 1, 1, 1));
    xmm4 = _mm_mul_ps(xmm3, xmm4);

    return _mm_mul_ps(xmm4, CONST_256);
}

//get pixels bilinear with SSE2
inline uint32_t bilinearGetPixelSSE2(const uint32_t* img, const int32_t width, const float x, const float y)
{
    //calculate offset at (x,y)
    const uint32_t* p0 = &img[intptr_t(y) * width + intptr_t(x)];

    //load 4 pixels [(x, y),(x + 1, y),(x, y + 1),(x + 1, y + 1)]
    __m128i p12 = _mm_loadl_epi64((const __m128i*)&p0[0]);
    __m128i p34 = _mm_loadl_epi64((const __m128i*)&p0[width]);

    //extend to 16bits
    p12 = _mm_unpacklo_epi8(p12, _mm_setzero_si128());
    p34 = _mm_unpacklo_epi8(p34, _mm_setzero_si128());

    //convert floating point weights to 16bits integer
    __m128i weight = _mm_cvtps_epi32(calcWeights(x, y));
    weight = _mm_packs_epi32(weight, _mm_setzero_si128());

    //prepare the weights
    __m128i w12 = _mm_shufflelo_epi16(weight, _MM_SHUFFLE(1, 1, 0, 0));
    __m128i w34 = _mm_shufflelo_epi16(weight, _MM_SHUFFLE(3, 3, 2, 2));

    //extend to 32bits
    w12 = _mm_unpacklo_epi16(w12, w12);
    w34 = _mm_unpacklo_epi16(w34, w34);

    //multiply each pixel with its weight (2 pixel per SSE mul)
    p12 = _mm_mullo_epi16(p12, w12);
    p34 = _mm_mullo_epi16(p34, w34);

    //sum the results
    w12 = _mm_add_epi16(p12, p34);
    w34 = _mm_shuffle_epi32(w12, _MM_SHUFFLE(3, 2, 3, 2));
    weight = _mm_add_epi16(w12, w34);

    //convert back to 8bits
    weight = _mm_srli_epi16(weight, 8);
    weight = _mm_packus_epi16(weight, _mm_setzero_si128());

    //return this pixels
    return _mm_cvtsi128_si32(weight);
}
