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
    uint8_t*        btData[BUTTON_STATE_COUNT];   //hold button data for each button state
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

#pragma pack(pop)

extern int32_t  texWidth;                       //current texture width
extern int32_t  texHeight;                      //current texture height
extern uint32_t bitsPerPixel;                   //bits per pixel (8/16/32)

//clip cordinate handle
extern int32_t  currX, currY;                   //current draw cursor
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

//convert color functions
uint32_t    hsl(int32_t h, int32_t s, int32_t l);
uint32_t    hsv(int32_t h, int32_t s, int32_t v);
uint32_t    rgb(uint8_t r, uint8_t g, uint8_t b);
uint32_t    rgba(uint32_t col, uint8_t alpha);

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
int32_t     random(int32_t a);
int32_t     randomRange(int32_t a, int32_t b);
void        randomBuffer(void* buff, int32_t count, int32_t range);
double      frand(double fmin, double fmax);

//system info functions
void        initSystemInfo();
uint32_t    getTotalMemory();
uint32_t    getAvailableMemory();

uint32_t    getCpuSpeed();
const char* getCpuType();
const char* getCpuName();
const char* getCpuFeatures();

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
void*       getDrawBuffer(int32_t *width = NULL, int32_t *height = NULL);
void        changeDrawBuffer(void* newBuff, int32_t newWidth, int32_t newHeight);
void        restoreDrawBuffer();

//handle program message
void        messageBox(int32_t type, const char* fmt, ...);
void        writeText(int32_t x, int32_t y, uint32_t txtColor, uint32_t mode, const char* format, ...);
int32_t     drawText(int32_t ypos, int32_t size, const char** str);

//drawing functions
void        clearScreen(uint32_t color);

uint32_t    getPixel(int32_t x, int32_t y);
void        putPixel(int32_t x, int32_t y, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);

void        horizLine(int32_t x, int32_t y, int32_t sx, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        vertLine(int32_t x, int32_t y, int32_t sy, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);

void        clipLine(int32_t* xs, int32_t* ys, int32_t* xe, int32_t* ye);
void        drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);

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

void        moveTo(int32_t x, int32_t y);
void        lineTo(int32_t x, int32_t y, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        initProjection();
void        projette(double x, double y, double z);
void        deplaceEn(double x, double y, double z);
void        traceVers(double x, double y, double z, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        fillRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        fillRectPattern(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, uint8_t* pattern, int32_t mode = BLEND_MODE_NORMAL);

void        calcCircle(int32_t r, int32_t* point);
void        fillCircle(int32_t xc, int32_t yc, int32_t radius, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);
void        calcEllipse(int32_t rx, int32_t ry, int32_t* point);
void        fillEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color, int32_t mode = BLEND_MODE_NORMAL);

void        fillPolygon(POINT2D* point, int32_t num, uint32_t col, int32_t mode = BLEND_MODE_NORMAL);

void        scaleLine(void* dst, void* src, int32_t dw, int32_t sw, int32_t smooth);
void        scaleImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t smooth);

void        setActivePage(GFX_IMAGE* page);
void        setVisualPage(GFX_IMAGE* page);

int32_t     newImage(int32_t width, int32_t height, GFX_IMAGE* img);
int32_t     updateImage(int32_t width, int32_t height, GFX_IMAGE* img);
void        freeImage(GFX_IMAGE* img);
void        clearImage(GFX_IMAGE* img);

void        getImage(int32_t x, int32_t y, int32_t width, int32_t height, GFX_IMAGE* img);
void        putImage(int32_t x, int32_t y, GFX_IMAGE* img, int32_t mode = BLEND_MODE_NORMAL);

void        putSprite(int32_t x, int32_t y, uint32_t keyColor, GFX_IMAGE* img, int32_t mode = BLEND_MODE_NORMAL);

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
void        rotatePalette(int32_t from, int32_t to, int32_t loop, int32_t delay);

void        fadeIn(RGB* dest, uint32_t delay);
void        fadeOut(RGB* dest, uint32_t delay);
void        fadeMax(uint32_t delay);
void        fadeMin(uint32_t delay);
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
void        bilinearRotateImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t angle);
void        bilinearScaleImage(GFX_IMAGE* dst, GFX_IMAGE* src);
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
