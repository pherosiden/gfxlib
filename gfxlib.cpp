//===============================================================//
//                 GFXLIB Graphics Library                       //
//               Use SDL2 for render system                      //
//               SDL2_image for load images                      //
//            Target OS: cross-platform (x32_64)                 //
//               Author: Nguyen Ngoc Van                         //
//               Create: 22/10/2018                              //
//              Version: 1.1.6                                   //
//          Last Update: 2021-07-24                              //
//              Website: http://codedemo.net                     //
//                Email: pherosiden@gmail.com                    //
//           References: https://crossfire-designs.de            //
//                       https://lodev.org                       //
//                       https://permadi.com                     //
//                       https://sources.ru                      //
//                       http://eyecandyarchive.com              //
//              License: MIT                                     //
//===============================================================//

#include <map>
#include "gfxlib.h"
#ifdef __APPLE__
#include <cpuid.h>
#include <sys/sysctl.h>
#else
#include <dxgi.h>
#include <tchar.h>
#pragma comment (lib, "dxgi")
#pragma comment (lib, "sdl2")
#pragma comment (lib, "sdl2main")
#pragma comment (lib, "sdl2_image")
#endif

//drawing buffer
void*           drawBuff = NULL;            //current render buffer
int32_t         texWidth = 0;               //current buffer width
int32_t         texHeight = 0;              //current buffer height

//save current buffer
void*           oldBuffer = NULL;           //saved render buffer
int32_t         oldWidth = 0;               //saved buffer width
int32_t         oldHeight = 0;              //saved buffer height

//pixels attributes
uint32_t        bitsPerPixel = 0;           //bits per pixel (8/15/16/24/32)
uint32_t        bytesPerScanline = 0;       //bytes per scanline

//mid-screen coordinate
int32_t         centerX = 0;                //x center screen
int32_t         centerY = 0;                //y center screen

//current screen view-port
int32_t         cminX = 0;                  //current left-top
int32_t         cminY = 0;                  //current left-top
int32_t         cmaxX = 0;                  //current right-bottom
int32_t         cmaxY = 0;                  //current right-bottom

//saved screen view-port
int32_t         oldMinX = 0;                //saved left-top
int32_t         oldMinY = 0;                //saved left-top
int32_t         oldMaxX = 0;                //saved right-bottom
int32_t         oldMaxY = 0;                //saved right-bottom

//current screen resolution
int32_t         cresX = 0;                  //X screen resolution
int32_t         cresY = 0;                  //Y screen resolution

//current draw cursor (2D)
int32_t         currX = 0;                  //current cursor x
int32_t         currY = 0;                  //current cursor y

//3D projection
double          DE = 0.0;

//trigonometric angle
double          rho = 0.0, theta = 0.0, phi = 0.0;

//saved transform values
double          aux1 = 0.0, aux2 = 0.0, aux3 = 0.0, aux4 = 0.0;
double          aux5 = 0.0, aux6 = 0.0, aux7 = 0.0, aux8 = 0.0;

//3D coordinator (x,y,z)
double          obsX = 0.0, obsY = 0.0, obsZ = 0.0;

//projection points
double          projX = 0.0;                //x projection
double          projY = 0.0;                //y projection

//current draw cursor (3D)
int32_t         cranX = 0;                  //x cursor
int32_t         cranY = 0;                  //y cursor

//3D projection type
uint8_t         projection = 0;             //current projection type

//GFX font data
GFX_FONT        gfxFonts[GFX_MAX_FONT] = { 0 };     //GFX font loadable at the same time
uint8_t*        fontPalette[GFX_MAX_FONT] = { 0 };  //GFX font palette data (BMP8 type)
uint8_t*        gfxBuff  = NULL;                    //GFX buffer
uint32_t        subFonts = 0;                       //GFX sub-fonts
uint32_t        fontType = 0;                       //current selected font (use for multiple loaded font)
uint32_t        randSeed = 0;                       //global random seed
uint32_t        factor   = 0x8088405;               //global factor

//pattern filled styles
uint8_t         ptnLine[]          = { 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00 };
uint8_t         ptnLiteSlash[]     = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
uint8_t         ptnSlash[]         = { 0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC1, 0x83 };
uint8_t         ptnBackSlash[]     = { 0x07, 0x83, 0xC1, 0xE0, 0x70, 0x38, 0x1C, 0x0E };
uint8_t         ptnLiteBackSlash[] = { 0x5A, 0x2D, 0x96, 0x4B, 0xA5, 0xD2, 0x69, 0xB4 };
uint8_t         ptnHatch[]         = { 0xFF, 0x88, 0x88, 0x88, 0xFF, 0x88, 0x88, 0x88 };
uint8_t         ptnHatchX[]        = { 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18 };
uint8_t         ptnInterLeave[]    = { 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33 };
uint8_t         ptnWideDot[]       = { 0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00 };
uint8_t         ptnCloseDot[]      = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };

//CPU and video card parameters
uint32_t        cpuSpeed = 0;               //CPU speed in MHz
char            cpuName[48] = { 0 };        //full CPU name string
char            cpuType[13] = { 0 };        //CPU type (INTEL, AMD, ...)
char            cpuFeatures[48] = { 0 };    //CPU features (MMX, 3DNow!, SSE, SSE2, SSE3, ...)
char            videoName[128] = { 0 };     //full name of graphic card
char            driverVersion[32] = { 0 };  //graphic driver version string
char            renderVersion[32] = { 0 };  //SDL2 version string
char            imageVersion[32] = { 0 };   //SDL2_image version string
char            modeInfo[32] = { 0 };       //current display mode info string

//system memory profiles
uint32_t        totalMemory = 0;            //total physical memory in MB
uint32_t        availableMemory = 0;        //available physical memory in MB
uint32_t        videoMemory = 0;            //total video memory in MB

//pointer function handlers
void            (*clearScreen)(uint32_t) = NULL;
void            (*putPixel)(int32_t, int32_t, uint32_t) = NULL;
void            (*putPixelAdd)(int32_t, int32_t, uint32_t) = NULL;
void            (*putPixelSub)(int32_t, int32_t, uint32_t) = NULL;
void            (*putPixelAnd)(int32_t, int32_t, uint32_t) = NULL;
void            (*putPixelXor)(int32_t, int32_t, uint32_t) = NULL;
uint32_t        (*getPixel)(int32_t, int32_t) = NULL;
void            (*horizLine)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*horizLineAdd)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*horizLineSub)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*horizLinePattern)(int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void            (*horizLinePatternAdd)(int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void            (*horizLinePatternSub)(int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void            (*vertLine)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*vertLineAdd)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*vertLineSub)(int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*fillRect)(int32_t, int32_t, int32_t, int32_t, uint32_t) = NULL;
void            (*fillRectPattern)(int32_t, int32_t, int32_t, int32_t, uint32_t, uint8_t*) = NULL;
void            (*getImage)(int32_t, int32_t, int32_t, int32_t, GFX_IMAGE*) = NULL;
void            (*putImage)(int32_t, int32_t, GFX_IMAGE*) = NULL;
void            (*putSprite)(int32_t, int32_t, uint32_t, GFX_IMAGE*) = NULL;
void            (*scaleImage)(GFX_IMAGE*, GFX_IMAGE*, int32_t) = NULL;

//global SDL objects
SDL_Window*     sdlWindow = NULL;
SDL_Surface*	sdlSurface = NULL;
SDL_Surface*	sdlScreen = NULL;
SDL_Texture*	sdlTexture = NULL;
SDL_Renderer*   sdlRenderer = NULL;
SDL_Event		sdlEvent = { 0 };

//keyboard status
uint8_t*        keyStates = 0;

//for the "keyPressed" function to detect a keypress only once
std::map<int32_t, int32_t> keyStatus;

//default 8-bits palette entries, SDL2 init with black palette, no default here
RGB basePalette[256] = {
    {0,0,0,255},{0,0,42,255},{0,42,0,255},{0,42,42,255},{42,0,0,255},{42,0,42,255},{42,21,0,255},{42,42,42,255},{21,21,21,255},{21,21,63,255},{21,63,21,255},{21,63,63,255},{63,21,21,255},{63,21,63,255},{63,63,21,255},{63,63,63,255},
    {0,0,0,255},{5,5,5,255},{8,8,8,255},{11,11,11,255},{14,14,14,255},{17,17,17,255},{20,20,20,255},{24,24,24,255},{28,28,28,255},{32,32,32,255},{36,36,36,255},{40,40,40,255},{45,45,45,255},{50,50,50,255},{56,56,56,255},{63,63,63,255},
    {0,0,63,255},{16,0,63,255},{31,0,63,255},{47,0,63,255},{63,0,63,255},{63,0,47,255},{63,0,31,255},{63,0,16,255},{63,0,0,255},{63,16,0,255},{63,31,0,255},{63,47,0,255},{63,63,0,255},{47,63,0,255},{31,63,0,255},{16,63,0,255},
    {0,63,0,255},{0,63,16,255},{0,63,31,255},{0,63,47,255},{0,63,63,255},{0,47,63,255},{0,31,63,255},{0,16,63,255},{31,31,63,255},{39,31,63,255},{47,31,63,255},{55,31,63,255},{63,31,63,255},{63,31,55,255},{63,31,47,255},{63,31,39,255},
    {63,31,31,255},{63,39,31,255},{63,47,31,255},{63,55,31,255},{63,63,31,255},{55,63,31,255},{47,63,31,255},{39,63,31,255},{31,63,31,255},{31,63,39,255},{31,63,47,255},{31,63,55,255},{31,63,63,255},{31,55,63,255},{31,47,63,255},{31,39,63,255},
    {45,45,63,255},{49,45,63,255},{54,45,63,255},{58,45,63,255},{63,45,63,255},{63,45,58,255},{63,45,54,255},{63,45,49,255},{63,45,45,255},{63,49,45,255},{63,54,45,255},{63,58,45,255},{63,63,45,255},{58,63,45,255},{54,63,45,255},{49,63,45,255},
    {45,63,45,255},{45,63,49,255},{45,63,54,255},{45,63,58,255},{45,63,63,255},{45,58,63,255},{45,54,63,255},{45,49,63,255},{0,0,28,255},{7,0,28,255},{14,0,28,255},{21,0,28,255},{28,0,28,255},{28,0,21,255},{28,0,14,255},{28,0,7,255},
    {28,0,0,255},{28,7,0,255},{28,14,0,255},{28,21,0,255},{28,28,0,255},{21,28,0,255},{14,28,0,255},{7,28,0,255},{0,28,0,255},{0,28,7,255},{0,28,14,255},{0,28,21,255},{0,28,28,255},{0,21,28,255},{0,14,28,255},{0,7,28,255},
    {14,14,28,255},{17,14,28,255},{21,14,28,255},{24,14,28,255},{28,14,28,255},{28,14,24,255},{28,14,21,255},{28,14,17,255},{28,14,14,255},{28,17,14,255},{28,21,14,255},{28,24,14,255},{28,28,14,255},{24,28,14,255},{21,28,14,255},{17,28,14,255},
    {14,28,14,255},{14,28,17,255},{14,28,21,255},{14,28,24,255},{14,28,28,255},{14,24,28,255},{14,21,28,255},{14,17,28,255},{20,20,28,255},{22,20,28,255},{24,20,28,255},{26,20,28,255},{28,20,28,255},{28,20,26,255},{28,20,24,255},{28,20,22,255},
    {28,20,20,255},{28,22,20,255},{28,24,20,255},{28,26,20,255},{28,28,20,255},{26,28,20,255},{24,28,20,255},{22,28,20,255},{20,28,20,255},{20,28,22,255},{20,28,24,255},{20,28,26,255},{20,28,28,255},{20,26,28,255},{20,24,28,255},{20,22,28,255},
    {0,0,16,255},{4,0,16,255},{8,0,16,255},{12,0,16,255},{16,0,16,255},{16,0,12,255},{16,0,8,255},{16,0,4,255},{16,0,0,255},{16,4,0,255},{16,8,0,255},{16,12,0,255},{16,16,0,255},{12,16,0,255},{8,16,0,255},{4,16,0,255},
    {0,16,0,255},{0,16,4,255},{0,16,8,255},{0,16,12,255},{0,16,16,255},{0,12,16,255},{0,8,16,255},{0,4,16,255},{8,8,16,255},{10,8,16,255},{12,8,16,255},{14,8,16,255},{16,8,16,255},{16,8,14,255},{16,8,12,255},{16,8,10,255},
    {16,8,8,255},{16,10,8,255},{16,12,8,255},{16,14,8,255},{16,16,8,255},{14,16,8,255},{12,16,8,255},{10,16,8,255},{8,16,8,255},{8,16,10,255},{8,16,12,255},{8,16,14,255},{8,16,16,255},{8,14,16,255},{8,12,16,255},{8,10,16,255},
    {11,11,16,255},{12,11,16,255},{13,11,16,255},{15,11,16,255},{16,11,16,255},{16,11,15,255},{16,11,13,255},{16,11,12,255},{16,11,11,255},{16,12,11,255},{16,13,11,255},{16,15,11,255},{16,16,11,255},{15,16,11,255},{13,16,11,255},{12,16,11,255},
    {11,16,11,255},{11,16,12,255},{11,16,13,255},{11,16,15,255},{11,16,16,255},{11,15,16,255},{11,13,16,255},{11,12,16,255},{0,0,0,255},{0,0,0,255},{0,0,0,255},{0,0,0,255},{0,0,0,255},{0,0,0,255},{0,0,0,255},{0,0,0,255},
};

//read current keyboard state
void readKeys()
{
    SDL_PumpEvents();
    keyStates = (uint8_t*)SDL_GetKeyboardState(NULL);
}

//this checks if the key is held down, returns true all the time until the key is up
int32_t keyDown(int32_t key)
{
    if (!keyStates) return 0;
    return (keyStates[key] != 0);
}

//this checks if the key is *just* pressed, returns true only once until the key is up again
int32_t keyPressed(int32_t key)
{
    if (!keyStates) return 0;

    if (keyStatus.find(key) == keyStatus.end()) keyStatus[key] = 0;

    if (keyStates[key])
    {
        if (keyStatus[key] == 0)
        {
            keyStatus[key] = 1;
            return 1;
        }
    }
    else
    {
        keyStatus[key] = 0;
    }

    return 0;
}

//read input key from user
int32_t waitKeyPressed()
{
    int32_t done = 0;
    int32_t pressedKey = 0;
    
    //flush all queue events
    while (SDL_PollEvent(&sdlEvent))
    {
        if (sdlEvent.type == SDL_QUIT) break;
    }

    while (!done)
    {
        while (SDL_PollEvent(&sdlEvent))
        {
            if (sdlEvent.type == SDL_QUIT || sdlEvent.type == SDL_KEYDOWN)
            {
                done = 1;
                pressedKey = sdlEvent.key.keysym.scancode;
            }
        }
        SDL_Delay(1);
    }

    return pressedKey;
}

//sleep CPU execution
void delay(uint32_t miliseconds)
{
    SDL_Delay(miliseconds);
}

//only return 1 when exitkey scancode (not escape) is givent, ESCAPE key to exit program
int32_t finished(int32_t keyCode)
{
    //flush pending event
    while (SDL_PollEvent(&sdlEvent))
    {
        if (sdlEvent.type == SDL_QUIT) return 1;
    }

    //what the user input key?
    readKeys();
    if (keyStates[SDL_SCANCODE_ESCAPE]) quit();
    if (keyStates[keyCode])
    {
        //clear buffer
        keyStates[keyCode] = 0;
        return 1;
    }

    SDL_Delay(1);
    return 0;
}

//exit program
void quit()
{
    cleanup();
    exit(1);
}

//get current mouse state
void getMouseState(int32_t* mx, int32_t* my)
{
    SDL_GetMouseState(mx, my);
}

//show or hide mouse cursor
void showMouseCursor(int32_t show)
{
    SDL_ShowCursor(show);
}

//get current mouse state
void getMouseState(int32_t* mx, int32_t* my, int32_t* lmb, int32_t* rmb)
{
    uint8_t mstate = SDL_GetMouseState(mx, my);
    if (lmb)
    {
        if (mstate & 1) *lmb = 1;
        else *lmb = 0;
    }
    if (rmb)
    {
        if (mstate & 4) *rmb = 1;
        else *rmb = 0;
    }
}

//set mouse position
void setMousePosition(int32_t x, int32_t y)
{
    SDL_SetWindowGrab(sdlWindow, SDL_TRUE);
    SDL_WarpMouseInWindow(sdlWindow, x, y);
}

//returns the time in milliseconds since the program started
double getTime()
{
    return SDL_GetTicks();
}

//return passing time from start time
double getElapsedTime(double tmstart)
{
    return getTime() - tmstart;
}

//wait until time wait is passed, program exit when ESCAPE key pressed
void waitFor(double tmstart, double tmwait)
{
    while (getElapsedTime(tmstart) < tmwait)
    {
        SDL_PollEvent(&sdlEvent);
        if (sdlEvent.type == SDL_QUIT) quit();
        keyStates = (uint8_t*)SDL_GetKeyboardState(NULL);
        if (keyStates[SDL_SCANCODE_ESCAPE]) quit();
        SDL_Delay(1);
    }
}

//sleep until time wait is passed or enter key, program exit when ESCAPE key pressed
void sleepFor(double tmwait)
{
    int32_t done = 0;
    const double tmstart = getTime();
    while (getElapsedTime(tmstart) < tmwait && !done)
    {
        SDL_PollEvent(&sdlEvent);
        if (sdlEvent.type == SDL_QUIT) quit();
        keyStates = (uint8_t*)SDL_GetKeyboardState(NULL);
        if (keyStates[SDL_SCANCODE_ESCAPE]) quit();
        if (keyStates[SDL_SCANCODE_RETURN])
        {
            keyStates[SDL_SCANCODE_RETURN] = 0;
            done = 1;
        }
        SDL_Delay(1);
    }
}

//initialize graphic video system
void initScreen(int32_t width, int32_t height, int32_t bpp, int32_t scaled, const char* title)
{
    //initialize SDL video mode only
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        messageBox(GFX_ERROR, "Failed init SDL2: %s", SDL_GetError());
        return;
    }

    //initialize SDL2 image lib
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        messageBox(GFX_ERROR, "Failed init SDL image: %s", IMG_GetError());
        return;
    }

    //create screen to display contents
    sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scaled ? SCREEN_WIDTH : width, scaled ? SCREEN_HEIGHT : height, SDL_WINDOW_SHOWN);
    if (!sdlWindow)
    {
        messageBox(GFX_ERROR, "Failed to create window: %s", SDL_GetError());
        return;
    }

    //set windows icon
    SDL_Surface* icon = IMG_Load("assets/gfxicon-128x.png");
    if (icon)
    {
        SDL_SetColorKey(icon, true, SDL_MapRGB(icon->format, 0, 0, 0));
        SDL_SetWindowIcon(sdlWindow, icon);
        SDL_FreeSurface(icon);
    }

    //create render windows
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer)
    {
        messageBox(GFX_ERROR, "Failed to create renderer: %s", SDL_GetError());
        return;
    }

    //create 32bits texture for render
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!sdlTexture)
    {
        messageBox(GFX_ERROR, "Failed to create texture: %s", SDL_GetError());
        return;
    }

    //use palette color for 8 bits?
    if (bpp == 8)
    {
        //create 32bits surface and use this to convert to texture before render to screen
        sdlScreen = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
        if (!sdlScreen)
        {
            messageBox(GFX_ERROR, "Failed to create 32 bits surface: %s", SDL_GetError());
            return;
        }

        //create 8bits surface
        sdlSurface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
        if (!sdlSurface)
        {
            messageBox(GFX_ERROR, "Failed to create 8 bits surface: %s", SDL_GetError());
            return;
        }

        //initialize drawing buffer (use current surface pixel buffer)
        drawBuff = sdlSurface->pixels;
        if (!drawBuff)
        {
            messageBox(GFX_ERROR, "Failed to create render buffer!");
            return;
        }

        //initialze raster function
        bitsPerPixel = 8;
        bytesPerScanline = sdlSurface->pitch;

        //set default palette
        shiftPalette(basePalette);
        setPalette(basePalette);

        //mapping raster function
        putPixel            = putPixel8;
        getPixel            = getPixel8;
        horizLine           = horizLine8;
        vertLine            = vertLine8;
        clearScreen         = clearScreen8;
        fillRect            = fillRect8;
        fillRectPattern     = fillRectPattern8;
        putImage            = putImage8;
        getImage            = getImage8;
        putSprite           = putSprite8;
        scaleImage          = scaleImage8;
    }
    else
    {
        //initialize drawing buffer for 32 bits RGBA
        drawBuff = (uint32_t*)calloc(intptr_t(width) * height, sizeof(int32_t));
        if (!drawBuff)
        {
            messageBox(GFX_ERROR, "Failed to create render buffer!");
            return;
        }

        //initialize raster function
        bitsPerPixel = 32;
        bytesPerScanline = width << 2;

        //mapping raster functions
        putPixel        = putPixel32;
        putPixelAdd     = putPixelAdd32;
        putPixelSub     = putPixelSub32;
        putPixelAnd     = putPixelAnd32;
        putPixelXor     = putPixelXor32;
        getPixel        = getPixel32;
        horizLine       = horizLine32;
        horizLineAdd    = horizLineAdd32;
        horizLineSub    = horizLineSub32;
        vertLine        = vertLine32;
        vertLineAdd     = vertLineAdd32;
        vertLineSub     = vertLineSub32;
        clearScreen     = clearScreen32;
        fillRect        = fillRect32;
        fillRectPattern = fillRectPattern32;
        putImage        = putImage32;
        getImage        = getImage32;
        putSprite       = putSprite32;
        scaleImage      = scaleImage32;
    }

    //initialize random number generation
    randSeed = uint32_t(time(NULL));
    srand(randSeed);

    //initialize GFXLIB buffer
    gfxBuff = (uint8_t*)calloc(GFX_BUFF_SIZE, 1);
    if (!gfxBuff)
    {
        messageBox(GFX_ERROR, "Cannot init GFXLIB memory!");
        return;
    }

    //initialize screen buffer size
    texWidth    = width;
    texHeight   = height;
    centerX     = texWidth >> 1;
    centerY     = texHeight >> 1;
    cresX       = width;
    cresY       = height;

    //initialize view port size
    cminX   = 0;
    cminY   = 0;
    cmaxX   = texWidth - 1;
    cmaxY   = texHeight - 1;
}

//cleanup function must call after graphics operations ended
void cleanup()
{
    if (bitsPerPixel == 8)
    {
        if (sdlScreen)
        {
            SDL_FreeSurface(sdlScreen);
            sdlScreen = NULL;
        }

        if (sdlSurface)
        {
            SDL_FreeSurface(sdlSurface);
            sdlSurface = NULL;
        }
    }
    else
    {
        if (drawBuff)
        {
            free(drawBuff);
            drawBuff = NULL;
        }
    }

    if (gfxBuff)
    {
        free(gfxBuff);
        gfxBuff = NULL;
    }

    if (sdlTexture)
    {
        SDL_DestroyTexture(sdlTexture);
        sdlTexture = NULL;
    }

    if (sdlRenderer)
    {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = NULL;
    }

    if (sdlWindow)
    {
        SDL_DestroyWindow(sdlWindow);
        sdlWindow = NULL;
    }

    SDL_Quit();
    IMG_Quit();
}

//render function, use this to render draw buffer to video memory
void render()
{
    if (bitsPerPixel == 32)
    {
        //RGBA mode, just render texture to video memory without any conversation
        SDL_UpdateTexture(sdlTexture, NULL, drawBuff, bytesPerScanline);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    }
    else if (bitsPerPixel == 8)
    {
        //256 colors palette, we must convert 8 bits surface to 32 bits surface
        SDL_UpperBlit(sdlSurface, NULL, sdlScreen, NULL);
        SDL_UpdateTexture(sdlTexture, NULL, sdlScreen->pixels, sdlScreen->pitch);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    }
    else messageBox(GFX_ERROR, "GFXLIB only support render for 32 or 8 bits pixels!");
}

//generate random value from number
int32_t random(int32_t a)
{
    return (a == 0) ? 0 : rand() % a;
}

//generate random value in range
int32_t randomRange(int32_t a, int32_t b)
{
    return (a < b) ? (a + (rand() % (b - a + 1))) : (b + (rand() % (a - b + 1)));
}

//raise a message box
void messageBox(int32_t type, const char* fmt, ...)
{
    char buffer[1024] = { 0 };
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    switch (type)
    {
    case GFX_ERROR:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GFX Error!", buffer, NULL);
        quit();
        break;

    case GFX_WARNING:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "GFX Warning!", buffer, NULL);
        break;

    default:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "GFX Info", buffer, NULL);
        break;
    }
}

//RGB to HSL convert
HSL RGB2HSL(uint8_t ur, uint8_t ug, uint8_t ub)
{
    double h = 0, s = 0, l = 0;

    const double r = ur / 256.0;
    const double g = ug / 256.0;
    const double b = ub / 256.0;

    const double maxColor = max(r, max(g, b));
    const double minColor = min(r, min(g, b));

    //R = G = B, so it's a shade of grey
    if (minColor == maxColor)
    {
        h = 0;
        s = 0;
        l = r;
    }
    else
    {
        l = (minColor + maxColor) / 2.0;

        if (l < 0.5) s = (maxColor - minColor) / (maxColor + minColor);
        if (l >= 0.5) s = (maxColor - minColor) / (2.0 - maxColor - minColor);

        if (r == maxColor) h = (g - b) / (maxColor - minColor);
        if (g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
        if (b == maxColor) h = 4.0 + (r - g) / (maxColor - minColor);

        //to bring it to a number between 0 and 1
        h /= 6;
        if (h < 0) h += 1;
    }

    HSL hsl = { 0 };
    hsl.h = int32_t(h * 255.0);
    hsl.s = int32_t(s * 255.0);
    hsl.l = int32_t(l * 255.0);
    return hsl;
}

//HSL to RGB convert
RGB HSL2RGB(int32_t hi, int32_t si, int32_t li)
{
    double r = 0, g = 0, b = 0;
    double temp1 = 0, temp2 = 0;
    double tempr = 0, tempg = 0, tempb = 0;

    const double h = hi / 256.0;
    const double s = si / 256.0;
    const double l = li / 256.0;

    //if saturation is 0, the color is a shade of grey
    if (s == 0) r = g = b = l;

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

    RGB rgb = { 0 };
    rgb.r = int32_t(r * 255.0);
    rgb.g = int32_t(g * 255.0);
    rgb.b = int32_t(b * 255.0);
    return rgb;
}

//RGB to HSV convert
HSV RGB2HSV(uint8_t ur, uint8_t ug, uint8_t ub)
{
    const double r = ur / 256.0;
    const double g = ug / 256.0;
    const double b = ub / 256.0;

    const double maxColor = max(r, max(g, b));
    const double minColor = min(r, min(g, b));
    const double v = maxColor;

    double h = 0.0, s = 0.0;
    
    //avoid division by zero when the color is black
    if (maxColor != 0.0)
    {
        s = (maxColor - minColor) / maxColor;
    }

    if (s == 0.0)
    {
        //it doesn't matter what value it has
        h = 0.0;
    }
    else
    {
        if (r == maxColor) h = (g - b) / (maxColor - minColor);
        if (g == maxColor) h = 2.0 + (b - r) / (maxColor - minColor);
        if (b == maxColor) h = 4.0 + (r - g) / (maxColor - minColor);

        //to bring it to a number between 0 and 1
        h /= 6.0;
        if (h < 0.0) h++;
    }

    HSV hsv = { 0 };
    hsv.h = int32_t(h * 255.0);
    hsv.s = int32_t(s * 255.0);
    hsv.v = int32_t(v * 255.0);
    return hsv;
}

//HSV to RGB convert
RGB HSV2RGB(int32_t hi, int32_t si, int32_t vi)
{
    double h = hi / 256.0;
    const double s = si / 256.0;
    const double v = vi / 256.0;
    double r = 0, g = 0, b = 0;

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
        default: r = g = b = 0; break;
        }
    }

    RGB rgb = { 0 };
    rgb.r = int32_t(r * 255.0);
    rgb.g = int32_t(g * 255.0);
    rgb.b = int32_t(b * 255.0);
    return rgb;
}

//convert r,g,b values to integer (pixel color)
uint32_t RGB2INT(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

//convert pixel color to r,g,b
RGB INT2RGB(uint32_t color)
{
    RGB rgb = { 0 };
    rgb.r = (color >> 16) & 0xFF;
    rgb.g = (color >>  8) & 0xFF;
    rgb.b = (color      ) & 0xFF;
    return rgb;
}

//retrive raw pixels data
void* getDrawBuffer(int32_t *width, int32_t *height)
{
    if (width) *width = texWidth;
    if (height) *height = texHeight;
    return drawBuff;
}

//set the draw buffer
void setDrawBuffer(void* newBuff, int32_t newWidth, int32_t newHeight)
{
    oldBuffer = drawBuff;
    oldWidth  = texWidth;
    oldHeight = texHeight;
    drawBuff  = newBuff;
    texWidth  = newWidth;
    texHeight = newHeight;
    setViewPort(0, 0, newWidth - 1, newHeight - 1);
}

//must call after setDrawBuffer call
void restoreDrawBuffer()
{
    drawBuff  = oldBuffer;
    texWidth  = oldWidth;
    texHeight = oldHeight;
    restoreViewPort();
}

//render user-buffer to current drawing buffer
void renderBuffer(const void* buffer, uint32_t size)
{
    memcpy(drawBuff, buffer, size);
    render();
}

//set current screen view port for clipping
void setViewPort(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //check correct range
    if (x1 > x2) swap(x1, x2);
    if (y1 > y2) swap(y1, y2);
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 < 0) x2 = 0;
    if (y2 < 0) y2 = 0;

    if (x1 >= texWidth)  x1 = texWidth - 1;
    if (y1 >= texHeight) y1 = texHeight - 1;
    if (x2 >= texWidth)  x2 = texWidth - 1;
    if (y2 >= texHeight) y2 = texHeight - 1;

    //save current view port
    oldMinX = cminX;
    oldMinY = cminY;
    oldMaxX = cmaxX;
    oldMaxY = cmaxY;

    //update clip point
    cminX = x1;
    cminY = y1;
    cmaxX = x2;
    cmaxY = y2;

    //update center x,y
    centerX = cminX + ((cmaxX - cminX + 1) >> 1);
    centerY = cminY + ((cmaxY - cminY + 1) >> 1);
}

//must call after setViewPort call
void restoreViewPort()
{
    cminX = oldMinX;
    cminY = oldMinY;
    cmaxX = oldMaxX;
    cmaxY = oldMaxY;
    centerX = cminX + ((cmaxX - cminX + 1) >> 1);
    centerY = cminY + ((cmaxY - cminY + 1) >> 1);
}

//plot a pixel at (x,y) with color
void putPixel8(int32_t x, int32_t y, uint32_t color)
{
    if (bitsPerPixel != 8) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     edi, drawBuff
        add     edi, eax
        mov     eax, color
        stosb
    }
#else
    uint8_t* pixels = (uint8_t*)drawBuff;
    pixels[y * texWidth + x] = color;
#endif
}

//plot a pixel at (x,y) with color
void putPixel32(int32_t x, int32_t y, uint32_t color)
{
    if (bitsPerPixel != 32) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     eax, color
        stosd
    }
#else
    uint32_t* pixel = (uint32_t*)drawBuff;
    pixel[y * texWidth + x] = color;
#endif
}

//plot a pixel at (x,y) with color and alpha channel
void putPixelAlpha(int32_t x, int32_t y, uint32_t col, uint8_t alpha)
{
    if (bitsPerPixel != 32) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff   
        add     edi, eax
        mov     al, [edi]
        mul     alpha
        mov     bx, ax
        mov     al, byte ptr[col]
        mov     cl, 255
        sub     cl, alpha
        mul     cl
        add     ax, bx
        shr     ax, 8
        stosb
        mov     al, [edi]
        mul     alpha
        mov     bx, ax
        mov     al, byte ptr[col + 1]
        mov     cl, 255
        sub     cl, alpha
        mul     cl
        add     ax, bx
        shr     ax, 8
        stosb
        mov     al, [edi]
        mul     alpha
        mov     bx, ax
        mov     al, byte ptr[col + 2]
        mov     cl, 255
        sub     cl, alpha
        mul     cl
        add     ax, bx
        shr     ax, 8
        stosb
    }
#else
    uint8_t* rgb = (uint8_t*)&col;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    pixels[0] = (alpha * pixels[0] + rgb[0] * (255 - alpha)) >> 8;
    pixels[1] = (alpha * pixels[1] + rgb[1] * (255 - alpha)) >> 8;
    pixels[2] = (alpha * pixels[2] + rgb[2] * (255 - alpha)) >> 8;
#endif
}

//plot a pixel at (x,y) with background color
void putPixelBob(int32_t x, int32_t y)
{
    if (bitsPerPixel != 8) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     esi, drawBuff
        add     esi, eax
        mov     ebx, eax
        lodsb
        mov     edi, drawBuff
        add     edi, ebx
        inc     al
        jnc     plot
        mov     al, 255
    plot:
        stosb
    }
#else
    uint8_t col = min(getPixel(x, y) + 1, 255);
    putPixel(x, y, col);
#endif
}

//plot a pixel at (x,y) with add color
void putPixelAdd32(int32_t x, int32_t y, uint32_t color)
{
    if (bitsPerPixel != 32) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     eax, [edi]
        add     al, byte ptr[color]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr[color + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, byte ptr[color + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
    }
#else
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    pixels[0] = min(pixels[0] + rgb[0], 255);
    pixels[1] = min(pixels[1] + rgb[1], 255);
    pixels[2] = min(pixels[2] + rgb[2], 255);
#endif
}

//plot a pixel at (x,y) with sub color
void putPixelSub32(int32_t x, int32_t y, uint32_t color)
{
    if (bitsPerPixel != 32) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     eax, [edi]
        sub     al, byte ptr[color]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr[color + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, byte ptr[color + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
    }
#else
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    pixels[0] = max(pixels[0] - rgb[0], 0);
    pixels[1] = max(pixels[1] - rgb[1], 0);
    pixels[2] = max(pixels[2] - rgb[2], 0);
#endif
}

//plot a pixel at (x,y) with and color
void putPixelAnd32(int32_t x, int32_t y, uint32_t color)
{
    if (bitsPerPixel != 32) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     eax, [edi]
        and     al, byte ptr[color]
        jnc     bstep
        xor     al, al
    bstep:
        and     ah, byte ptr[color + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        and     bl, byte ptr[color + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
    }
#else
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    pixels[0] = pixels[0] & rgb[0];
    pixels[1] = pixels[1] & rgb[1];
    pixels[2] = pixels[2] & rgb[2];
#endif
}

//plot a pixel at (x,y) with xor color
void putPixelXor32(int32_t x, int32_t y, uint32_t color)
{
    if (bitsPerPixel != 32) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     eax, [edi]
        xor     al, byte ptr[color]
        jnc     bstep
        xor     al, al
    bstep:
        xor     ah, byte ptr[color + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        xor     bl, byte ptr[color + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
    }
#else
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    pixels[0] = pixels[0] ^ rgb[0];
    pixels[1] = pixels[1] ^ rgb[1];
    pixels[2] = pixels[2] ^ rgb[2];
#endif
}

//peek a pixel at (x,y)
uint32_t getPixel8(int32_t x, int32_t y)
{
    if (bitsPerPixel != 8) return 0;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return 0;
#ifdef _USE_ASM
    uint8_t col = 0;
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     esi, drawBuff
        add     esi, eax
        lodsb
        mov     col, al
    }
    return col;
#else
    uint8_t* pixels = (uint8_t*)drawBuff;
    return pixels[y * texWidth + x];
#endif
}

//peek a pixel at (x,y)
uint32_t getPixel32(int32_t x, int32_t y)
{
    if (bitsPerPixel != 32) return 0;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return 0;
#ifdef _USE_ASM
    uint32_t col = 0;
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     esi, drawBuff
        add     esi, eax
        lodsd
        mov     col, eax
    }
    return col;
#else
    uint32_t* pixel = (uint32_t*)drawBuff;
    return pixel[y * texWidth + x];
#endif
}

//clear screen with color
void clearScreen8(uint32_t color)
{
    if (bitsPerPixel != 8) return;
    const uint32_t lfbSize = texHeight * texWidth;
#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     ecx, lfbSize
        shr     ecx, 2
        mov     ebx, lfbSize
        and     ebx, 3
        mov     al, byte ptr color
        mov     ah, al
        mov     dx, ax
        shl     eax, 16
        mov     ax, dx
        rep     stosd
        mov     ecx, ebx
        rep     stosb
    }
#else
    uint8_t* pixels = (uint8_t*)drawBuff;
    for (uint32_t i = 0; i < lfbSize; i++) *pixels++ = color;
#endif
    render();
}

//clear screen with color
void clearScreen32(uint32_t color)
{
    if (bitsPerPixel != 32) return;
    const uint32_t lfbSize = texHeight * texWidth;
#ifdef _USE_ASM
    _asm {
        mov    edi, drawBuff
        mov    ecx, lfbSize
        mov    eax, color
        rep    stosd
    }
#else    
    uint32_t* pixels = (uint32_t*)drawBuff;
    for (uint32_t i = 0; i < lfbSize; i++) *pixels++ = color;
#endif
    render();
}

//fast horizontal line from (x,y) with sx length, and color
void horizLine8(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
    if (bitsPerPixel != 8) return;

    //check for clip-y
    if (y > cmaxY || y < cminY) return;
    if (x > cmaxX || sx <= 0) return;

    //check clip boundary
    if (x < cminX)
    {
        //re-calculate sx
        sx -= (cminX - x) + 1;
        x = cminX;
    }

    //inbound check
    if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
    if (sx <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     edi, drawBuff
        add     edi, eax
        mov     al, byte ptr color
        mov     ah, al
        mov     dx, ax
        shl     eax, 16
        mov     ax, dx
        mov     edx, sx
        shr     edx, 2
        mov     ebx, sx
        and     ebx, 3
        mov     ecx, edx
        rep     stosd
        mov     ecx, ebx
        rep     stosb
    }
#else
    uint8_t* pixels = (uint8_t*)drawBuff + intptr_t(texWidth) * y + x;
    for (int32_t i = 0; i < sx; i++) *pixels++ = color;
#endif
}

//fast horizontal line from (x,y) with sx length, and color
void horizLine32(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //check for clip-y
    if (y > cmaxY || y < cminY) return;
    if (x > cmaxX || sx <= 0) return;

    //check clip boundary
    if (x < cminX)
    {
        //re-calculate sx
        sx -= (cminX - x) + 1;
        x = cminX;
    }

    //inbound check
    if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
    if (sx <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sx
        mov     eax, color
        rep     stosd
    }
#else
    uint32_t* pixels = (uint32_t*)drawBuff + intptr_t(texWidth) * y + x;
    for (int32_t i = 0; i < sx; i ++) *pixels++ = color;
#endif
}

//fast horizontal line from (x,y) with sx length, and add color
void horizLineAdd32(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //check for clip-y
    if (y > cmaxY || y < cminY) return;
    if (x > cmaxX || sx <= 0) return;

    //check clip boundary
    if (x < cminX)
    {
        //re-calculate sx
        sx -= (cminX - x) + 1;
        x = cminX;
    }

    //inbound check
    if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
    if (sx <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sx
    next:
        mov     eax, [edi]
        add     al, byte ptr [color]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr [color + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, byte ptr [color + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        loop    next
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    for (int32_t i = 0; i < sx; i++)
    {
        pixels[0] = min(pixels[0] + rgb[0], 255);
        pixels[1] = min(pixels[1] + rgb[1], 255);
        pixels[2] = min(pixels[2] + rgb[2], 255);
        pixels += 4;
    }
#endif
}

//fast horizontal line from (x,y) with sx length, and sub color
void horizLineSub32(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //check for clip-y
    if (y > cmaxY || y < cminY) return;
    if (x > cmaxX || sx <= 0) return;

    //check clip boundary
    if (x < cminX)
    {
        //re-calculate sx
        sx -= (cminX - x) + 1;
        x = cminX;
    }

    //inbound check
    if (sx > cmaxX - x) sx = (cmaxX - x) + 1;
    if (sx <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sx
    next:
        mov     eax, [edi]
        sub     al, byte ptr [color]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr [color + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, byte ptr [color + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        loop    next
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    for (int32_t i = 0; i < sx; i++)
    {
        pixels[0] = max(pixels[0] - rgb[0], 0);
        pixels[1] = max(pixels[1] - rgb[1], 0);
        pixels[2] = max(pixels[2] - rgb[2], 0);
        pixels += 4;
    }
#endif
}

//fast vertical line from (x,y) with sy length, and palette color
void vertLine8(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
    if (bitsPerPixel != 8) return;

    //check for clip-x
    if (x > cmaxX || x < cminX) return;
    if (y > cmaxY || sy <= 0) return;

    if (y < cminY)
    {
        //re-calculate sy
        sy -= (cminY - y) + 1;
        y = cminY;
    }

    //inbound check
    if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
    if (sy <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sy
        mov     ebx, texWidth
        sub     ebx, 1
        mov     al, byte ptr[color]
    next:
        stosb
        add     edi, ebx
        loop    next
    }
#else
    //calculate starting address
    uint8_t* pixels = (uint8_t*)drawBuff + intptr_t(texWidth) * y + x;
    for (int32_t i = 0; i < sy; i++)
    {
        *pixels = color;
        pixels += texWidth;
    }
#endif
}

//fast vertical line from (x,y) with sy length, and rgb color
void vertLine32(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //check for clip-x
    if (x > cmaxX || x < cminX) return;
    if (y > cmaxY || sy <= 0) return;

    if (y < cminY)
    {
        //re-calculate sy
        sy -= (cminY - y) + 1;
        y = cminY;
    }

    //inbound check
    if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
    if (sy <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sy
        mov     ebx, texWidth
        sub     ebx, 1
        shl     ebx, 2
        mov     eax, color
    next:
        stosd
        add     edi, ebx 
        loop    next
    }
#else
    //calculate starting address
    uint32_t* pixels = (uint32_t*)drawBuff + intptr_t(texWidth) * y + x;
    for (int32_t i = 0; i < sy; i++)
    {
        *pixels = color;
        pixels += texWidth;
    }
#endif
}

//fast vertical line from (x,y) with sy length, and add color
void vertLineAdd32(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //check for clip-x
    if (x > cmaxX || x < cminX) return;
    if (y > cmaxY || sy <= 0) return;

    if (y < cminY)
    {
        //re-calculate sy
        sy -= (cminY - y) + 1;
        y = cminY;
    }

    //inbound check
    if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
    if (sy <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sy
        mov     ebx, texWidth
        sub     ebx, 1
        shl     ebx, 2
    next:
        mov     eax, [edi]
        add     al, byte ptr [color]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr [color + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     edx, eax
        shr     edx, 16
        add     dl, byte ptr [color + 2]
        jnc     rstep
        mov     dl, 255
    rstep:
        shl     edx, 16
        and     eax, 00FFFFh
        or      eax, edx
        stosd 
        add     edi, ebx 
        loop    next
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    for (int32_t i = 0; i < sy; i++)
    {
        pixels[0] = min(pixels[0] + rgb[0], 255);
        pixels[1] = min(pixels[1] + rgb[1], 255);
        pixels[2] = min(pixels[2] + rgb[2], 255);
        pixels += intptr_t(texWidth) << 2;
    }
#endif
}

//fast vertical line from (x,y) with sy length, and sub color
void vertLineSub32(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //check for clip-x
    if (x > cmaxX || x < cminX) return;
    if (y > cmaxY || sy <= 0) return;

    if (y < cminY)
    {
        //re-calculate sy
        sy -= (cminY - y) + 1;
        y = cminY;
    }

    //inbound check
    if (sy > cmaxY - y) sy = (cmaxY - y) + 1;
    if (sy <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sy
        mov     ebx, texWidth
        sub     ebx, 1
        shl     ebx, 2
    next:
        mov     eax, [edi]
        sub     al, byte ptr [color]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr [color + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     edx, eax
        shr     edx, 16
        sub     dl, byte ptr [color + 2]
        jnc     rstep
        xor     dl, dl
    rstep:
        shl     edx, 16
        and     eax, 00FFFFh
        or      eax, edx
        stosd 
        add     edi, ebx 
        loop    next
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * y + x);
    for (int32_t i = 0; i < sy; i++)
    {
        pixels[0] = max(pixels[0] - rgb[0], 0);
        pixels[1] = max(pixels[1] - rgb[1], 0);
        pixels[2] = max(pixels[2] - rgb[2], 0);
        pixels += intptr_t(texWidth) << 2;
    }
#endif
}

//smooth scaling with Bresenham (internal function)
//because of several simplifications of the algorithm,
//the zoom range is restricted between 0.5 and 2. That
//is: dstwidth must be >= srcwidth/2 and <= 2*srcwidth.
//smooth is used to calculate average pixel and mid-point
void scaleLine8(uint8_t* dst, uint8_t* src, int32_t dw, int32_t sw, int32_t smooth)
{
    if (smooth)
    {
    #ifdef _USE_ASM
        uint16_t val = 0;
        _asm {
            mov     ebx, dw
            mov     eax, dw
            cmp     eax, sw
            jna     start
            dec     ebx
        start:
            mov     esi, src
            mov     edi, dst
            mov     ecx, ebx
            xor     ebx, ebx
            mov     edx, dw
            shr     edx, 1
        next:
            cmp     ebx, edx
            jnae    done
            mov     al, [esi]
            mov     val, ax
            mov     al, [esi + 1]
            add     ax, val
            shr     ax, 1
            stosb
            jmp     skip
        done:
            mov     al, [esi]
            stosb
        skip:
            add     ebx, sw
            cmp     ebx, dw
            jnae    cycle
            sub     ebx, dw
            inc     esi
        cycle:
            loop    next
            mov     eax, dw
            cmp     eax, sw
            jna     end
            movsb
        end:
        }
#else
        int32_t error = 0;
        int32_t numPixels = dw;
        const int32_t midPixel = dw >> 1;

        if (dw > sw) numPixels--;
        while (numPixels-- > 0)
        {
            if (error >= midPixel) *dst++ = (*src + *(src + 1)) >> 1;
            else *dst++ = *src;
            error += sw;
            if (error >= dw)
            {
                error -= dw;
                src++;
            }
        }
        if (dw > sw) *dst = *src;
#endif
    }
    else
    {
#ifdef _USE_ASM
        _asm {
            xor     edx, edx
            mov     esi, src
            mov     edi, dst
            mov     eax, sw
            mov     ebx, dw
            div     ebx
            xor     ebx, ebx
            mov     ecx, dw
        next1:
            movsb
            dec     esi
            add     esi, eax
            add     ebx, edx
            cmp     ebx, dw
            jnae    done1
            sub     ebx, dw
            inc     esi
        done1:
            loop    next1
        }
#else
        int32_t error = 0;
        int32_t numPixels = dw;
        const int32_t intPart = sw / dw;
        const int32_t fractPart = sw % dw;
        
        while (numPixels-- > 0)
        {
            *dst++ = *src;
            src += intPart;
            error += fractPart;
            if (error >= dw)
            {
                error -= dw;
                src++;
            }
        }
#endif
    }
}

//Bresenham scale line with rgb color
void scaleLine32(uint32_t* dst, uint32_t* src, int32_t dw, int32_t sw, int32_t smooth)
{
    if (smooth)
    {
#ifdef _USE_ASM
        uint16_t val = 0;
        _asm {
            mov     ebx, dw
            mov     eax, dw
            cmp     eax, sw
            jna     start
            dec     ebx
        start:
            mov     esi, src
            mov     edi, dst
            mov     ecx, ebx
            xor     ebx, ebx
            mov     edx, dw
            shr     edx, 1
        next:
            cmp     ebx, edx
            jnae    done
            mov     al, [esi]
            mov     val, ax
            mov     al, [esi + 4]
            add     ax, val
            shr     ax, 1
            stosb
            mov     al, [esi + 1]
            mov     val, ax
            mov     al, [esi + 5]
            add     ax, val
            shr     ax, 1
            stosb
            mov     al, [esi + 2]
            mov     val, ax
            mov     al, [esi + 6]
            add     ax, val
            shr     ax, 1
            stosb
            inc     edi
            jmp     skip
        done:
            mov     eax, [esi]
            stosd
        skip:
            add     ebx, sw
            cmp     ebx, dw
            jnae    cycle
            sub     ebx, dw
            add     esi, 4
        cycle:
            loop    next
            mov     eax, dw
            cmp     eax, sw
            jna     end
            movsd
        end:
        }
#else
        int32_t error = 0;
        int32_t numPixels = dw;
        const int32_t midPixel = dw >> 1;
        
        if (dw > sw) numPixels--;

        while (numPixels-- > 0)
        {
            //copy source pixel to destination pixel
            if (error < midPixel) *dst = *src;
            else
            {
                //calculate average pixel p = (s0 + s1) / 2
                uint8_t* pixel  = (uint8_t*)dst;
                uint8_t* pixel0 = (uint8_t*)src;
                uint8_t* pixel1 = (uint8_t*)(src + 1);
                pixel[0] = (pixel0[0] + pixel1[0]) >> 1;
                pixel[1] = (pixel0[1] + pixel1[1]) >> 1;
                pixel[2] = (pixel0[2] + pixel1[2]) >> 1;
            }
            dst++;
            error += sw;
            if (error >= dw)
            {
                error -= dw;
                src++;
            }
        }
        if (dw > sw) *dst = *src;
#endif
    }
    else
    {
#ifdef _USE_ASM
        _asm {
            xor     edx, edx
            mov     esi, src
            mov     edi, dst
            mov     eax, sw
            mov     ebx, dw
            div     ebx
            shl     eax, 2
            xor     ebx, ebx
            mov     ecx, dw
        begin:
            movsd
            sub     esi, 4
            add     esi, eax
            add     ebx, edx
            cmp     ebx, dw
            jnae    step
            sub     ebx, dw
            add     esi, 4
        step:
            loop    begin
        }        
#else
        int32_t error = 0;
        int32_t numPixels = dw;
        const int32_t intPart = sw / dw;
        const int32_t fractPart = sw % dw;
        
        while (numPixels-- > 0)
        {
            *dst++ = *src;
            src += intPart;
            error += fractPart;
            if (error >= dw)
            {
                error -= dw;
                src++;
            }
        }
#endif
    }
}

//Breshenham scale image buffer (export function)
void scaleImage8(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t smooth)
{
#ifdef _USE_ASM
    //save local value
    const uint32_t dstWidth  = dst->mWidth;
    const uint32_t srcWidth  = src->mWidth;
    const uint32_t dstHeight = dst->mHeight;
    const uint32_t srcHeight = src->mHeight;
    uint32_t intp = 0, modp = 0;

    //make pointer to call in asm mode
    void *copier = memcpy;
    void *scaler = scaleLine8;

    //save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    _asm {
        mov     edi, dstPtr
        mov     esi, srcPtr
        xor     edx, edx
        mov     eax, srcHeight
        div     dstHeight
        mov     modp, edx
        mul     srcWidth
        mov     intp, eax
        xor     ebx, ebx
        mov     ecx, dstHeight
    next:
        push    ecx
        cmp     esi, oldPtr
        jne     skip
        mov     eax, edi
        sub     eax, dstWidth
        push    dstWidth
        push    eax
        push    edi
        call    copier
        add     esp, 12
        jmp     done
    skip:
        mov     edx, smooth
        and     edx, 0Fh
        push    edx
        push    srcWidth
        push    dstWidth
        push    esi
        push    edi
        call    scaler
        add     esp, 20
        mov     oldPtr, esi
    done:
        add     edi, dstWidth
        add     esi, intp
        add     ebx, modp
        cmp     ebx, dstHeight
        jnae    cycle
        sub     ebx, dstHeight
        add     esi, srcWidth
    cycle:
        pop     ecx
        loop    next
    }
#else    
    int32_t error = 0;
    int32_t numPixels = dst->mHeight;
    const int32_t intPart = (src->mHeight / dst->mHeight) * src->mWidth;
    const int32_t fractPart = src->mHeight % dst->mHeight;
    
    uint8_t* srcPrev = NULL;
    uint8_t* srcPtr = src->mData;
    uint8_t* dstPtr = dst->mData;

    while (numPixels-- > 0)
    {
        if (srcPtr == srcPrev)
        {
            memcpy(dstPtr, dstPtr - dst->mWidth, dst->mWidth * sizeof(dstPtr[0]));
        }
        else
        {
            scaleLine8(dstPtr, srcPtr, dst->mWidth, src->mWidth, smooth);
            srcPrev = srcPtr;
        }

        dstPtr += dst->mWidth;
        srcPtr += intPart;
        error += fractPart;

        if (error >= dst->mHeight)
        {
            error -= dst->mHeight;
            srcPtr += src->mWidth;
        }
    }
#endif
}

//Breshenham scale image buffer (export function)
void scaleImage32(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t smooth)
{
#ifdef _USE_ASM
    //save local value
    const uint32_t dstWidth  = dst->mWidth;
    const uint32_t srcWidth  = src->mWidth;
    const uint32_t dstHeight = dst->mHeight;
    const uint32_t srcHeight = src->mHeight;

    uint32_t dsi = 0, ddi = 0;
    uint32_t intp = 0, modp = 0;

    //make pointer to call in asm mode
    void *copier = memcpy;
    void *scaler = scaleLine32;

    //save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    _asm {
        mov     edi, dstPtr
        mov     esi, srcPtr
        xor     edx, edx
        mov     eax, srcHeight
        div     dstHeight
        mov     modp, edx
        mul     srcWidth
        shl     eax, 2
        mov     intp, eax
        mov     eax, dstWidth
        shl     eax, 2
        mov     ddi, eax
        mov     eax, srcWidth
        shl     eax, 2
        mov     dsi, eax
        xor     ebx, ebx
        mov     ecx, dstHeight
    next:
        push    ecx
        cmp     esi, oldPtr
        jne     skip
        mov     eax, edi
        sub     eax, ddi
        push    ddi
        push    eax
        push    edi
        call    copier
        add     esp, 12
        jmp     done
    skip:
        push    smooth
        push    srcWidth
        push    dstWidth
        push    esi
        push    edi
        call    scaler
        add     esp, 20
        mov     oldPtr, esi
    done:
        add     edi, ddi
        add     esi, intp
        add     ebx, modp
        cmp     ebx, dstHeight
        jnae    cycle
        sub     ebx, dstHeight
        add     esi, dsi
    cycle:
        pop     ecx
        loop    next
    }
#else    
    int32_t error = 0;
    int32_t numPixels = dst->mHeight;
    const int32_t intPart = (src->mHeight / dst->mHeight) * src->mWidth;
    const int32_t fractPart = src->mHeight % dst->mHeight;
    
    uint32_t* srcPrev = NULL;
    uint32_t* srcPtr = (uint32_t*)src->mData;
    uint32_t* dstPtr = (uint32_t*)dst->mData;

    while (numPixels-- > 0)
    {
        if (srcPtr == srcPrev)
        {
            memcpy(dstPtr, dstPtr - dst->mWidth, dst->mWidth * sizeof(dstPtr[0]));
        }
        else
        {
            scaleLine32(dstPtr, srcPtr, dst->mWidth, src->mWidth, smooth);
            srcPrev = srcPtr;
        }

        dstPtr += dst->mWidth;
        srcPtr += intPart;
        error += fractPart;

        if (error >= dst->mHeight)
        {
            error -= dst->mHeight;
            srcPtr += src->mWidth;
        }
    }
#endif
}

//Bi-linear resize image, this only work with RGB color mode
//optimize version using integer, this is not fully optimized
void bilinearScaleImage(GFX_IMAGE* dst, GFX_IMAGE* src)
{
    //only works with 32 bits
    if (bitsPerPixel != 32) return;

    //cache local data pointer
    uint32_t* dstp = (uint32_t*)dst->mData;
    const uint32_t* srcp = (const uint32_t*)src->mData;
    
    //cache local dimension
    const int32_t swidth = src->mWidth;
    const int32_t sheight = src->mHeight;
    const int32_t dwidth = dst->mWidth;
    const int32_t dheight = dst->mHeight;

    //calculate ratio
    const int32_t ws = ((swidth - 1) << 16) / (dwidth - 1);
    const int32_t hs = ((sheight - 1) << 16) / (dheight - 1);

    int32_t hc0 = 0;
    for (int32_t h = 0; h < dheight; h++, hc0 += hs)
    {
        //calculate height color
        int32_t wc0 = 0;
        const int32_t ofsy = (hc0 >> 16) * swidth;
        const int32_t hc2 = (hc0 >> 9) & 0x7F;
        const int32_t hc1 = 128 - hc2;

        for (int32_t w = 0; w < dwidth; w++, wc0 += ws)
        {
            //calculate width color
            const int32_t ofsx = wc0 >> 16;
            const int32_t wc2 = (wc0 >> 9) & 0x7F;
            const int32_t wc1 = 128 - wc2;
            const int32_t ofs1 = ofsx + ofsy;
            const int32_t ofs2 = ofs1 + swidth;

            //load four pixels into rgb buffer
            const uint8_t* px1 = (const uint8_t*)&srcp[ofs1];
            const uint8_t* px2 = (const uint8_t*)&srcp[ofs2];
            const uint8_t* px3 = (const uint8_t*)&srcp[ofs1 + 1];
            const uint8_t* px4 = (const uint8_t*)&srcp[ofs2 + 1];

            //calculate weight of RGB
            uint8_t* pixel = (uint8_t*)dstp++;
            pixel[2] = ((px1[2] * hc1 + px2[2] * hc2) * wc1 + (px3[2] * hc1 + px4[2] * hc2) * wc2) >> 14;
            pixel[1] = ((px1[1] * hc1 + px2[1] * hc2) * wc1 + (px3[1] * hc1 + px4[1] * hc2) * wc2) >> 14;
            pixel[0] = ((px1[0] * hc1 + px2[0] * hc2) * wc1 + (px3[0] * hc1 + px4[0] * hc2) * wc2) >> 14;
        }
    }
}

//bilinear image rotation (optimize version using FIXED POINT)
void bilinearRotateImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t angle)
{
    //only works with 32 bits
    if (bitsPerPixel != 32) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;
    const uint32_t* psrc = (const uint32_t*)src->mData;

    //calculate haft dimension
    const int32_t width = src->mWidth;
    const int32_t height = src->mHeight;
    const int32_t tx = width >> 1;
    const int32_t ty = height >> 1;

    //convert to radian
    const double alpha = angle * M_PI / 180.0;
    const double sina = sin(-alpha);
    const double cosa = cos(-alpha);

    //start pixel mapmulation
    int32_t cy = -ty;
    for (int32_t y = 0; y < height; y++, cy++)
    {
        int32_t cx = -tx;
        for (int32_t x = 0; x < width; x++, cx++)
        {
            const double sx = cx * cosa - cy * sina + tx;
            const double sy = cx * sina + cy * cosa + ty;

            //calculate bilinear pixel, using FIXED-POINT (fixed value 256)
            if (sx >= 0 && sx < width - 1.0 && sy >= 0 && sy < height - 1.0)
            {
                const int32_t dx = int32_t(sx * 256); //convert to fixed point
                const int32_t dy = int32_t(sy * 256); //convert to fixed point
                const int32_t px = (dx & -256) >> 8;  //floor of x
                const int32_t py = (dy & -256) >> 8;  //floor of y

                //pointer to first pixel
                const uint32_t* p0 = &psrc[px + py * width];

                //load the four neighboring pixels
                const uint8_t* p1 = (const uint8_t*)&p0[0];
                const uint8_t* p2 = (const uint8_t*)&p0[1];
                const uint8_t* p3 = (const uint8_t*)&p0[width];
                const uint8_t* p4 = (const uint8_t*)&p0[width + 1];

                //calculate the weights for each pixel
                const int32_t fx = dx & 0xFF;
                const int32_t fy = dy & 0xFF;
                const int32_t fx1 = 256 - fx;
                const int32_t fy1 = 256 - fy;

                const int32_t w1 = (fx1 * fy1) >> 8;
                const int32_t w2 = (fx * fy1) >> 8;
                const int32_t w3 = (fx1 * fy) >> 8;
                const int32_t w4 = (fx * fy) >> 8;

                //calculate the weighted sum of pixels (for each color channel) and store to destination buffer
                uint8_t* pixel = (uint8_t*)pdst;
                pixel[2] = (p1[2] * w1 + p2[2] * w2 + p3[2] * w3 + p4[2] * w4) >> 8;
                pixel[1] = (p1[1] * w1 + p2[1] * w2 + p3[1] * w3 + p4[1] * w4) >> 8;
                pixel[0] = (p1[0] * w1 + p2[0] * w2 + p3[0] * w3 + p4[0] * w4) >> 8;
            }
            pdst++;
        }
    }
}

//Bresenham line from (x1,y1) to (x2,y2) with color
void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    //clip coordinator
    clipLine(&x1, &y1, &x2, &y2);
    const int32_t step = abs(y2 - y1) > abs(x2 - x1);

    if (step)
    {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }

    const int32_t dx = x2 - x1;
    const int32_t dy = abs(y2 - y1);
    const int32_t ystep = (y1 < y2) ? 1 : -1;

    int32_t y = y1;
    int32_t error = dx >> 1;
    
    for (int32_t x = x1; x <= x2; x++)
    {
        putPixel(step ? y : x, step ? x : y, color);
        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

//Bresenham line from (x1,y1) to (x2,y2) with add color
void drawLineAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    //clip coordinator
    clipLine(&x1, &y1, &x2, &y2);

    const int32_t step = abs(y2 - y1) > abs(x2 - x1);

    if (step)
    {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }

    const int32_t dx = x2 - x1;
    const int32_t dy = abs(y2 - y1);
    const int32_t ystep = (y1 < y2) ? 1 : -1;

    int32_t y = y1;
    int32_t error = dx >> 1;
    

    for (int32_t x = x1; x <= x2; x++)
    {
        putPixelAdd(step ? y : x, step ? x : y, color);
        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

//Bresenham line from (x1,y1) to (x2,y2) with sub color
void drawLineSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    //clip coordinator
    clipLine(&x1, &y1, &x2, &y2);

    const int32_t step = abs(y2 - y1) > abs(x2 - x1);

    if (step)
    {
        swap(x1, y1);
        swap(x2, y2);
    }

    if (x1 > x2)
    {
        swap(x1, x2);
        swap(y1, y2);
    }

    const int32_t dx = x2 - x1;
    const int32_t dy = abs(y2 - y1);
    const int32_t ystep = (y1 < y2) ? 1 : -1;
    
    int32_t y = y1;
    int32_t error = dx >> 1;
    
    for (int32_t x = x1; x <= x2; x++)
    {
        putPixelSub(step ? y : x, step ? x : y, color);
        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

//Wu's line from (x1,y1) to (x2,y2) with alpha color
void drawLineAlpha(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    //clip line
    clipLine(&x0, &y0, &x1, &y1);
    
    const int32_t dx = abs(x1 - x0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t dy = abs(y1 - y0);
    const int32_t sy = y0 < y1 ? 1 : -1;
    const int32_t ed = ((dx + dy) == 0) ? 1 : int32_t(sqrt(double(dx) * dx + double(dy) * dy));

    int32_t e2, x2;
    int32_t err = dx - dy;

    while (1)
    {
        putPixelAlpha(x0, y0, rgb, 255 * abs(err - dx + dy) / ed);
        e2 = err;
        x2 = x0;

        if (2 * e2 >= -dx)
        {
            if (x0 == x1) break;
            if (e2 + dy < ed) putPixelAlpha(x0, y0 + sy, rgb, 255 * (e2 + dy) / ed);
            err -= dy;
            x0 += sx;
        }

        if (2 * e2 <= dy)
        {
            if (y0 == y1) break;
            if (dx - e2 < ed) putPixelAlpha(x2 + sx, y0, rgb, 255 * (dx - e2) / ed);
            err += dx;
            y0 += sy;
        }
    }
}

//Bresenham line from(x1, y1) to(x2, y2) with bobbed color
void drawLineBob(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //range check
    if (x1 < 0 || x1 >= texWidth || x2 < 0 || x2 >= texWidth || y1 < 0 || y1 >= texHeight || y2 < 0 || y2 >= texHeight) return;

#ifdef _USE_ASM    
    int32_t dst = 0;
    int32_t dxInc = 0, dyInc = 0;
    int32_t sxInc = 0, syInc = 0;
    int32_t sc = 0, dc = 0;
    void* plotPixel = putPixelBob;

    _asm {
        mov     ecx, 1
        mov     edx, 1
        mov     edi, y2
        sub     edi, y1
        jge     keepy
        neg     edx
        neg     edi
    keepy:
        mov     dyInc, edx
        mov     esi, x2
        sub     esi, x1
        jge     keepx
        neg     ecx
        neg     esi
    keepx:
        mov     dxInc, ecx
        cmp     esi, edi
        jge     horiz
        xor     ecx, ecx
        xchg    esi, edi
        jmp     saves
    horiz:
        xor     edx, edx
    saves:
        mov     dst, edi
        mov     sxInc, ecx
        mov     syInc, edx
        mov     eax, dst
        shl     eax, 1
        mov     sc, eax
        sub     eax, esi
        mov     ebx, eax
        sub     eax, esi
        mov     dc, eax
        mov     ecx, x1
        mov     edx, y1
    again:
        dec     esi
        jz      done
        push    edx
        push    ecx
        call    plotPixel
        pop     ecx
        pop     edx
        cmp     ebx, 0
        jge     dline
        add     ecx, sxInc
        add     edx, syInc
        add     ebx, sc
        jmp     again
    dline:
        add     ecx, dxInc
        add     edx, dyInc
        add     ebx, dc
        jmp     again
    done:
    }
#else
    const int32_t deltaX = abs(x2 - x1);
    const int32_t deltaY = abs(y2 - y1);
    int32_t x = x1;
    int32_t y = y1;
    int32_t xinc1 = 0, xinc2 = 0;
    int32_t yinc1 = 0, yinc2 = 0;
    int32_t numpixels = 0, curpixel = 0;
    int32_t den = 0, num = 0, numadd = 0;

    if (x2 >= x1)
    {
        xinc1 = 1;
        xinc2 = 1;
    }
    else
    {
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1)
    {
        yinc1 = 1;
        yinc2 = 1;
    }
    else
    {
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltaX >= deltaY)
    {
        xinc1 = 0;
        yinc2 = 0;
        den = deltaX;
        num = deltaX / 2;
        numadd = deltaY;
        numpixels = deltaX;
    }
    else
    {
        xinc2 = 0;
        yinc1 = 0;
        den = deltaY;
        num = deltaY / 2;
        numadd = deltaX;
        numpixels = deltaY;
    }

    for (curpixel = 0; curpixel < numpixels; curpixel++)
    {
        putPixelBob(x % texWidth, y % texHeight);
        num += numadd;
        if (num >= den)
        {
            num -= den;
            x += xinc1;
            y += yinc1;
        }
        x += xinc2;
        y += yinc2;
    }
#endif
}

//Bresenham circle at (xc,yc) with radius and color
void drawCircle(int32_t xc, int32_t yc, int32_t radius, uint32_t color)
{
    if (radius <= 0) return;

    int32_t x = 0, y = radius;
    int32_t p = 1 - radius;

    while (x <= y)
    {
        putPixel(xc + x, yc + y, color);
        putPixel(xc - x, yc + y, color);
        putPixel(xc + x, yc - y, color);
        putPixel(xc - x, yc - y, color);
        putPixel(xc + y, yc + x, color);
        putPixel(xc - y, yc + x, color);
        putPixel(xc + y, yc - x, color);
        putPixel(xc - y, yc - x, color);
        if (p < 0) p += (x++ << 1) + 3;
        else p += ((x++ - y--) << 1) + 5;
    }
}

//Bresenham circle at (xc,yc) with radius and color
void drawCircleAdd(int32_t xc, int32_t yc, int32_t radius, uint32_t color)
{
    if (radius <= 0) return;

    int32_t x = 0, y = radius;
    int32_t p = 1 - radius;

    while (x <= y)
    {
        putPixelAdd(xc + x, yc + y, color);
        putPixelAdd(xc - x, yc + y, color);
        putPixelAdd(xc + x, yc - y, color);
        putPixelAdd(xc - x, yc - y, color);
        putPixelAdd(xc + y, yc + x, color);
        putPixelAdd(xc - y, yc + x, color);
        putPixelAdd(xc + y, yc - x, color);
        putPixelAdd(xc - y, yc - x, color);
        if (p < 0) p += (x++ << 1) + 3;
        else p += ((x++ - y--) << 1) + 5;
    }
}

//Bresenham circle at (xc,yc) with radius and color
void drawCircleSub(int32_t xc, int32_t yc, int32_t radius, uint32_t color)
{
    if (radius <= 0) return;

    int32_t x = 0, y = radius;
    int32_t p = 1 - radius;

    while (x <= y)
    {
        putPixelSub(xc + x, yc + y, color);
        putPixelSub(xc - x, yc + y, color);
        putPixelSub(xc + x, yc - y, color);
        putPixelSub(xc - x, yc - y, color);
        putPixelSub(xc + y, yc + x, color);
        putPixelSub(xc - y, yc + x, color);
        putPixelSub(xc + y, yc - x, color);
        putPixelSub(xc - y, yc - x, color);
        if (p < 0) p += (x++ << 1) + 3;
        else p += ((x++ - y--) << 1) + 5;
    }
}

//Wu's circle with alpha blend
void drawCircleAlpha(int32_t xm, int32_t ym, int32_t rad, uint32_t rgb)
{
    //only 32bit support alpha-blend mode
    if (bitsPerPixel != 32) return;
    if (rad <= 0) return;

    int32_t x = -rad;
    int32_t y = 0;
    int32_t err = (1 - rad) << 1;

    rad = 1 - err;

    do {
        int32_t alpha = 255 * abs(err - 2 * (x + y) - 2) / rad;
        putPixelAlpha(xm - x, ym + y, rgb, alpha);
        putPixelAlpha(xm - y, ym - x, rgb, alpha);
        putPixelAlpha(xm + x, ym - y, rgb, alpha);
        putPixelAlpha(xm + y, ym + x, rgb, alpha);

        const int32_t e2 = err;
        const int32_t x2 = x;

        if (err + y > 0)
        {
            alpha = 255 * (err - 2 * x - 1) / rad;
            if (alpha < 256)
            {
                putPixelAlpha(xm - x, ym + y + 1, rgb, alpha);
                putPixelAlpha(xm - y - 1, ym - x, rgb, alpha);
                putPixelAlpha(xm + x, ym - y - 1, rgb, alpha);
                putPixelAlpha(xm + y + 1, ym + x, rgb, alpha);
            }

            err += ++x * 2 + 1;
        }

        if (e2 + x2 <= 0)
        {
            alpha = 255 * (2 * y + 3 - e2) / rad;
            if (alpha < 256)
            {
                putPixelAlpha(xm - x2 - 1, ym + y, rgb, alpha);
                putPixelAlpha(xm - y, ym - x2 - 1, rgb, alpha);
                putPixelAlpha(xm + x2 + 1, ym - y, rgb, alpha);
                putPixelAlpha(xm + y, ym + x2 + 1, rgb, alpha);
            }
            err += ++y * 2 + 1;
        }
    } while (x < 0);
}

//Bresenham drawing an ellipses with color
void drawEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color)
{
    if (ra <= 0) return;
    if (rb <= 0) return;

    int32_t mx1 = xc - ra;
    int32_t mx2 = xc + ra;
    int32_t my1 = yc;
    int32_t my2 = yc;

    putPixel(mx2, yc, color);
    putPixel(mx1, yc, color);

    const int32_t aq = ra * ra;
    const int32_t bq = rb * rb;
    const int32_t dx = aq << 1;
    const int32_t dy = bq << 1;

    int32_t rd = ra * bq;
    int32_t rx = rd << 1;
    int32_t ry = 0;
    int32_t ax = ra;

    while (ax > 0)
    {
        if (rd > 0)
        {
            my1++;
            my2--;
            ry += dx;
            rd -= ry;
        }

        if (rd <= 0)
        {
            ax--;
            mx1++;
            mx2--;
            rx -= dy;
            rd += rx;
        }

        putPixel(mx1, my1, color);
        putPixel(mx1, my2, color);
        putPixel(mx2, my1, color);
        putPixel(mx2, my2, color);
    }
}

//Bresenham drawing an ellipses with add color
void drawEllipseAdd(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color)
{
    if (ra <= 0) return;
    if (rb <= 0) return;

    int32_t mx1 = xc - ra;
    int32_t mx2 = xc + ra;
    int32_t my1 = yc;
    int32_t my2 = yc;

    putPixel(mx2, yc, color);
    putPixel(mx1, yc, color);

    const int32_t aq = ra * ra;
    const int32_t bq = rb * rb;
    const int32_t dx = aq << 1;
    const int32_t dy = bq << 1;

    int32_t rd = ra * bq;
    int32_t rx = rd << 1;
    int32_t ry = 0;
    int32_t ax = ra;

    while (ax > 0)
    {
        if (rd > 0)
        {
            my1++;
            my2--;
            ry += dx;
            rd -= ry;
        }

        if (rd <= 0)
        {
            ax--;
            mx1++;
            mx2--;
            rx -= dy;
            rd += rx;
        }

        putPixelAdd(mx1, my1, color);
        putPixelAdd(mx1, my2, color);
        putPixelAdd(mx2, my1, color);
        putPixelAdd(mx2, my2, color);
    }
}

//Bresenham drawing an ellipses with sub color
void drawEllipseSub(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color)
{
    if (ra <= 0) return;
    if (rb <= 0) return;

    int32_t mx1 = xc - ra;
    int32_t mx2 = xc + ra;
    int32_t my1 = yc;
    int32_t my2 = yc;

    putPixel(mx2, yc, color);
    putPixel(mx1, yc, color);

    const int32_t aq = ra * ra;
    const int32_t bq = rb * rb;
    const int32_t dx = aq << 1;
    const int32_t dy = bq << 1;

    int32_t rd = ra * bq;
    int32_t rx = rd << 1;
    int32_t ry = 0;
    int32_t ax = ra;

    while (ax > 0)
    {
        if (rd > 0)
        {
            my1++;
            my2--;
            ry += dx;
            rd -= ry;
        }

        if (rd <= 0)
        {
            ax--;
            mx1++;
            mx2--;
            rx -= dy;
            rd += rx;
        }

        putPixelSub(mx1, my1, color);
        putPixelSub(mx1, my2, color);
        putPixelSub(mx2, my1, color);
        putPixelSub(mx2, my2, color);
    }
}

//Wu's ellipse with alpha blend
void drawEllipseAlpha(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t rgb)
{
    int32_t f = 0;
    double alpha = 0;
    
    int32_t a = abs(x1 - x0);
    const int32_t b = abs(y1 - y0);

    //only 32bit support alpha-blend mode
    if (bitsPerPixel != 32) return;
    if (a <= 0 || b <= 0) return;

    int32_t b1 = b & 1;
    double dx = 4.0 * (a - 1.0) * b * b;
    double dy = 4.0 * (b1 + 1.0) * a * a;
    double err = double(b1) * a * a - dx + dy;

    //check for line
    if (a == 0 || b == 0) return;

    if (x0 > x1)
    {
        x0 = x1;
        x1 += a;
    }

    if (y0 > y1) y0 = y1;

    y0 += (b + 1) / 2;
    y1 = y0 - b1;
    a = 8 * a * a;
    b1 = 8 * b * b;

    while (1)
    {
        double ed = max(dx, dy);
        alpha = min(dx, dy);

        if (y0 == y1 + 1 && err > dy && a > b1) ed = 255.0 * 4 / a;
        else ed = 255 / (ed + 2 * ed * alpha * alpha / (4 * ed * ed + alpha * alpha));

        alpha = ed * fabs(err + dx - dy);

        putPixelAlpha(x0, y0, rgb, uint8_t(alpha));
        putPixelAlpha(x0, y1, rgb, uint8_t(alpha));
        putPixelAlpha(x1, y0, rgb, uint8_t(alpha));
        putPixelAlpha(x1, y1, rgb, uint8_t(alpha));
        
        if ((f = (2 * err + dy) >= 0))
        {
            if (x0 >= x1) break;
            alpha = ed * (err + dx);
            if (alpha < 255)
            {
                putPixelAlpha(x0, y0 + 1, rgb, uint8_t(alpha));
                putPixelAlpha(x0, y1 - 1, rgb, uint8_t(alpha));
                putPixelAlpha(x1, y0 + 1, rgb, uint8_t(alpha));
                putPixelAlpha(x1, y1 - 1, rgb, uint8_t(alpha));
            }
        }

        if (2 * err <= dx)
        {
            alpha = ed * (dy - err);
            if (alpha < 255)
            {
                putPixelAlpha(x0 + 1, y0, rgb, uint8_t(alpha));
                putPixelAlpha(x1 - 1, y0, rgb, uint8_t(alpha));
                putPixelAlpha(x0 + 1, y1, rgb, uint8_t(alpha));
                putPixelAlpha(x1 - 1, y1, rgb, uint8_t(alpha));
            }

            y0++;
            y1--;
            err += dy += a;
        }

        if (f)
        {
            x0++;
            x1--;
            err -= dx -= b1;
        }
    }

    if (--x0 == x1++)
    {
        while (y0 - y1 < b)
        {
            alpha = 255.0 * 4 * fabs(err + dx) / b1;
            putPixelAlpha(x0, ++y0, rgb, uint8_t(alpha));
            putPixelAlpha(x1, y0, rgb, uint8_t(alpha));
            putPixelAlpha(x0, --y1, rgb, uint8_t(alpha));
            putPixelAlpha(x1, y1, rgb, uint8_t(alpha));
            err += dy += a;
        }
    }
}

//rectangle with corners (x1,y1) and (x2,y2) and color
void drawRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    horizLine(x1, y1, x2 - x1 + 1, color);
    vertLine(x1, y1, y2 - y1 + 1, color);
    horizLine(x1, y2, x2 - x1 + 1, color);
    vertLine(x2, y1, y2 - y1 + 1, color);
}

//draw rectangle with add color
void drawRectAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    horizLineAdd(x1, y1, x2 - x1 + 1, col);
    vertLineAdd(x1, y1, y2 - y1 + 1, col);
    horizLineAdd(x1, y2, x2 - x1 + 1, col);
    vertLineAdd(x2, y1, y2 - y1 + 1, col);
}

//draw rectangle with sub color
void drawRectSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    horizLineSub(x1, y1, x2 - x1 + 1, col);
    vertLineSub(x1, y1, y2 - y1 + 1, col);
    horizLineSub(x1, y2, x2 - x1 + 1, col);
    vertLineSub(x2, y1, y2 - y1 + 1, col);
}

//draw rectangle with rounded border and color
void drawRectEx(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, uint32_t col)
{
    int32_t x = 0, y = 0;
    int32_t point[500] = { 0 };

    const int32_t mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    const int32_t width = abs(x2 - x1);
    calcCircle(r, point);

    horizLine(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    vertLine(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    horizLine(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    vertLine(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixel(x1 + x, y1 + y, col);
            putPixel(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixel(x1 + x, y2 - y, col);
            putPixel(x2 - x, y2 - y, col);
        }
    }
}

//draw rectangle with rounded border and add color
void drawRectExAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, uint32_t col)
{
    int32_t x = 0, y = 0;
    int32_t point[500] = { 0 };

    const int32_t mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    const int32_t width = abs(x2 - x1);
    calcCircle(r, point);

    horizLineAdd(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    vertLineAdd(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    horizLineAdd(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    vertLineAdd(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y1 + y, col);
            putPixelAdd(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y2 - y, col);
            putPixelAdd(x2 - x, y2 - y, col);
        }
    }
}

//draw rectangle with rounded border
void drawRectExSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, uint32_t col)
{
    int32_t x = 0, y = 0;
    int32_t point[500] = { 0 };

    const int32_t mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    const int32_t width = abs(x2 - x1);
    calcCircle(r, point);

    horizLineSub(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    vertLineSub(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    horizLineSub(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    vertLineSub(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelSub(x1 + x, y1 + y, col);
            putPixelSub(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelSub(x1 + x, y2 - y, col);
            putPixelSub(x2 - x, y2 - y, col);
        }
    }
}

//draw boxed with color
void drawBox(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy, uint32_t col)
{
    const int32_t x11 = x1 + dx;
    const int32_t y11 = y1 - dy;
    const int32_t x22 = x2 + dx;
    const int32_t y22 = y2 - dy;

    drawRect(x1, y1, x2, y2, col);
    drawRect(x11, y11, x22, y22, col);
    drawLine(x1, y1, x11, y11, col);
    drawLine(x2, y1, x22, y11, col);
    drawLine(x2, y2, x22, y22, col);
    drawLine(x1, y2, x11, y22, col);
}

//draw boxed with add color
void drawBoxAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy, uint32_t col)
{
    const int32_t x11 = x1 + dx;
    const int32_t y11 = y1 - dy;
    const int32_t x22 = x2 + dx;
    const int32_t y22 = y2 - dy;

    drawRectAdd(x1, y1, x2, y2, col);
    drawRectAdd(x11, y11, x22, y22, col);
    drawLineAdd(x1, y1, x11, y11, col);
    drawLineAdd(x2, y1, x22, y11, col);
    drawLineAdd(x2, y2, x22, y22, col);
    drawLineAdd(x1, y2, x11, y22, col);
}

//draw boxed with sub color
void drawBoxSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t dx, int32_t dy, uint32_t col)
{
    const int32_t x11 = x1 + dx;
    const int32_t y11 = y1 - dy;
    const int32_t x22 = x2 + dx;
    const int32_t y22 = y2 - dy;

    drawRectSub(x1, y1, x2, y2, col);
    drawRectSub(x11, y11, x22, y22, col);
    drawLineSub(x1, y1, x11, y11, col);
    drawLineSub(x2, y1, x22, y11, col);
    drawLineSub(x2, y2, x22, y22, col);
    drawLineSub(x1, y2, x11, y22, col);
}

//draw boxed with rounded border
void drawBoxEx(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, uint32_t col)
{
    int32_t x = 0, y = 0;
    int32_t point[500] = { 0 };

    const int32_t mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    const int32_t width = abs(x2 - x1);
    calcCircle(r, point);

    horizLine(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    vertLine(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    horizLine(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    vertLine(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixel(x1 + x, y1 + y, col);
            putPixel(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixel(x1 + x, y2 - y, col);
            putPixel(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= r; y++) horizLine(x1 + r - point[y - 1] + 1, y1 + y, width - (r * 2 - point[y - 1] * 2) - 1, col);
    fillRect(x1 + 1, y1 + r + 1, x2 - 1, y2 - r - 1, col);
    for (y = r; y >= 1; y--) horizLine(x1 + r - point[y - 1] + 1, y2 - y, width - (r * 2 - point[y - 1] * 2) - 1, col);
}

//draw boxed with rounded border
void drawBoxExAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, uint32_t col)
{
    int32_t x = 0, y = 0;
    int32_t point[500] = { 0 };

    const int32_t mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    const int32_t width = abs(x2 - x1);
    calcCircle(r, point);

    horizLineAdd(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    vertLineAdd(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    horizLineAdd(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    vertLineAdd(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y1 + y, col);
            putPixelAdd(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelAdd(x1 + x, y2 - y, col);
            putPixelAdd(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= r; y++) horizLineAdd(x1 + r - point[y - 1] + 1, y1 + y, width - (r * 2 - point[y - 1] * 2) - 1, col);
    fillRectAdd(x1 + 1, y1 + r + 1, x2 - 1, y2 - r - 1, col);
    for (y = r; y >= 1; y--) horizLineAdd(x1 + r - point[y - 1] + 1, y2 - y, width - (r * 2 - point[y - 1] * 2) - 1, col);
}

//draw boxed with rounded border
void drawBoxExSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, uint32_t col)
{
    int32_t x = 0, y = 0;
    int32_t point[500] = { 0 };

    const int32_t mid = (y2 - y1) >> 1;
    if (r >= mid - 1) r = mid - 1;

    const int32_t width = abs(x2 - x1);
    calcCircle(r, point);

    horizLineSub(x1 + r - point[0], y1, width - (r - point[0]) * 2 + 1, col);
    vertLineSub(x1, y1 + r, y2 - y1 - r * 2 + 1, col);
    horizLineSub(x1 + r - point[0], y2, width - (r - point[0]) * 2 + 1, col);
    vertLineSub(x2, y1 + r, y2 - y1 - r * 2 + 1, col);

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelSub(x1 + x, y1 + y, col);
            putPixelSub(x2 - x, y1 + y, col);
        }
    }

    for (y = 1; y <= r; y++)
    {
        for (x = r - point[y]; x <= r - point[y - 1]; x++)
        {
            putPixelSub(x1 + x, y2 - y, col);
            putPixelSub(x2 - x, y2 - y, col);
        }
    }

    for (y = 1; y <= r; y++) horizLineSub(x1 + r - point[y - 1] + 1, y1 + y, width - (r * 2 - point[y - 1] * 2) - 1, col);
    fillRectSub(x1 + 1, y1 + r + 1, x2 - 1, y2 - r - 1, col);
    for (y = r; y >= 1; y--) horizLineSub(x1 + r - point[y - 1] + 1, y2 - y, width - (r * 2 - point[y - 1] * 2) - 1, col);
}

//draw polygon
void drawPoly(POINT2D* point, int32_t num, uint32_t col)
{
    if (num < 3) return;
    for (int32_t i = 0; i < num; i++)
    {
        drawLine(int32_t(point[i].x), int32_t(point[i].y), int32_t(point[(i + 1) % num].x), int32_t(point[(i + 1) % num].y), col);
    }
}

//draw polygon with add color
void drawPolyAdd(POINT2D* point, int32_t num, uint32_t col)
{
    if (num < 3) return;
    for (int32_t i = 0; i < num; i++)
    {
        drawLineAdd(int32_t(point[i].x), int32_t(point[i].y), int32_t(point[(i + 1) % num].x), int32_t(point[(i + 1) % num].y), col);
    }
}

//draw polygon with sub color
void drawPolySub(POINT2D* point, int32_t num, uint32_t col)
{
    if (num < 3) return;
    for (int32_t i = 0; i < num; i++)
    {
        drawLineSub(int32_t(point[i].x), int32_t(point[i].y), int32_t(point[(i + 1) % num].x), int32_t(point[(i + 1) % num].y), col);
    }
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRect8(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    if (bitsPerPixel != 8) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        push    edx
        mov     ebx, width
        shr     ebx, 2
        mov     edx, width
        and     edx, 3
        mov     eax, color
        shl     eax, 8
        or      eax, color
        shl     eax, 8
        or      eax, color
        shl     eax, 8
        or      eax, color
    next:
        mov     ecx, ebx
        rep     stosd
        mov     ecx, edx
        rep     stosb
        add     edi, [esp]
        dec     height
        jnz     next
        pop     edx
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    for (int32_t y = 0; y < height; y++)
    {
        memset(dstPixels, color, width);
        dstPixels += texWidth;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRect32(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
        mov     eax, color
    next:
        mov     ecx, width
        rep     stosd
        add     edi, edx
        dec     height
        jnz     next
    }
#else
    //calculate starting address
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    for (int32_t y = 0; y < height; y++)
    {
        memset(dstPixels, color, intptr_t(width) << 2);
        dstPixels += texWidth;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRectAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
    next:
        mov     ecx, width
    plot:
        mov     eax, [edi]
        add     al, byte ptr[color]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, byte ptr[color + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, byte ptr[color + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        loop    plot
        add     edi, edx
        dec     height
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    const uint32_t addOfs = (texWidth - width) << 2;

    for (int32_t y = 0; y < height; y++)
    {
        for (int32_t x = 0; x < width; x++)
        {
            pixels[0] = min(pixels[0] + rgb[0], 255);
            pixels[1] = min(pixels[1] + rgb[1], 255);
            pixels[2] = min(pixels[2] + rgb[2], 255);
            pixels += 4;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRectSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    if (bitsPerPixel != 32) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
    next:
        mov     ecx, width
    plot:
        mov     eax, [edi]
        sub     al, byte ptr[color]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, byte ptr[color + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, byte ptr[color + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        loop    plot
        add     edi, edx
        dec     height
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&color;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    const uint32_t addOfs = (texWidth - width) << 2;

    for (int32_t y = 0; y < height; y++)
    {
        for (int32_t x = 0; x < width; x++)
        {
            pixels[0] = max(pixels[0] - rgb[0], 0);
            pixels[1] = max(pixels[1] - rgb[1], 0);
            pixels[2] = max(pixels[2] - rgb[2], 0);
            pixels += 4;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRectPattern8(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, uint8_t* pattern)
{
    if (bitsPerPixel != 8) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        mov     esi, pattern
    plot:
        mov     ecx, lx1
        and     ecx, 7
        mov     ebx, height
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ebx, col
        mov     ecx, width
    next:
        test    al, 1
        jz      step
        mov     [edi], bl
    step:
        inc     edi
        rol     al, 1
        loop    next
        add     edi, edx
        dec     height
        jnz     plot
    }
#else
    //calculate starting address
    const uint32_t addDstOffs = texWidth - width;
    uint8_t* dstPixels = (uint8_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    for (int32_t y = 0; y < height; y++)
    {
        uint8_t al = pattern[y & 7];
        al = _rotl8(al, lx1 & 7);
        for (int32_t x = 0; x < width; x++)
        {
            if (al & 1) *dstPixels = col;
            al = _rotl8(al, 1);
            dstPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRectPattern32(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, uint8_t* pattern)
{
    if (bitsPerPixel != 32) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
        mov     esi, pattern
    plot:
        mov     ecx, lx1
        and     ecx, 7
        mov     ebx, height
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ebx, col
        mov     ecx, width
    next:
        test    al, 1
        jz      step
        mov     [edi], ebx
    step:
        add     edi, 4
        rol     al, 1
        loop    next
        add     edi, edx
        dec     height
        jnz     plot
    }
#else
    //calculate starting address
    const uint32_t addDstOffs = texWidth - width;
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    for (int32_t y = 0; y < height; y++)
    {
        uint8_t al = pattern[y & 7];
        al = _rotl8(al, lx1 & 7);
        for (int32_t x = 0; x < width; x++)
        {
            if (al & 1) *dstPixels = col;
            al = _rotl8(al, 1);
            dstPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRectPatternAdd(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, uint8_t* pattern)
{
    if (bitsPerPixel != 32) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
        mov     esi, pattern
    plot:
        push    edx
        mov     ecx, lx1
        and     ecx, 7
        mov     ebx, height
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ecx, width
    next:
        test    al, 1
        jz      step
        mov     ebx, [edi]
        add     bl, byte ptr [col]
        jnc     bstep
        mov     bl, 255
    bstep:
        add     bh, byte ptr [col + 1]
        jnc     gstep
        mov     bh, 255
    gstep:
        mov     edx, ebx
        shr     edx, 16
        add     dl, byte ptr [col + 2]
        jnc     rstep
        mov     dl, 255
    rstep:
        shl     edx, 16
        and     ebx, 00FFFFh
        or      ebx, edx
        mov     [edi], ebx
    step:
        add     edi, 4
        rol     al, 1
        loop    next
        pop     edx
        add     edi, edx
        dec     height
        jnz     plot
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&col;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    const uint32_t addOfs = (texWidth - width) << 2;

    for (int32_t y = 0; y < height; y++)
    {
        uint8_t al = pattern[y & 7];
        al = _rotl8(al, lx1 & 7);
        for (int32_t x = 0; x < width; x++)
        {
            if (al & 1)
            {
                pixels[0] = min(pixels[0] + rgb[0], 255);
                pixels[1] = min(pixels[1] + rgb[1], 255);
                pixels[2] = min(pixels[2] + rgb[2], 255);
            }
            al = _rotl8(al, 1);
            pixels += 4;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (x2,y2) and color
void fillRectPatternSub(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, uint8_t* pattern)
{
    if (bitsPerPixel != 32) return;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
        mov     esi, pattern
    plot:
        push    edx
        mov     ecx, lx1
        and     ecx, 7
        mov     ebx, height
        and     ebx, 7
        mov     al, [esi + ebx]
        rol     al, cl
        mov     ecx, width
    next:
        test    al, 1
        jz      step
        mov     ebx, [edi]
        sub     bl, byte ptr [col]
        jnc     bstep
        xor     bl, bl
    bstep:
        sub     bh, byte ptr [col + 1]
        jnc     gstep
        xor     bh, bh
    gstep:
        mov     edx, ebx
        shr     edx, 16
        sub     dl, byte ptr [col + 2]
        jnc     rstep
        xor     dl, dl
    rstep:
        shl     edx, 16
        and     ebx, 00FFFFh
        or      ebx, edx
        mov     [edi], ebx
    step:
        add     edi, 4
        rol     al, 1
        loop    next
        pop     edx
        add     edi, edx
        dec     height
        jnz     plot
    }
#else
    //calculate starting address
    uint8_t* rgb = (uint8_t*)&col;
    uint8_t* pixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    const uint32_t addOfs = (texWidth - width) << 2;

    for (int32_t y = 0; y < height; y++)
    {
        uint8_t al = pattern[y & 7];
        al = _rotl8(al, lx1 & 7);
        for (int32_t x = 0; x < width; x++)
        {
            if (al & 1)
            {
                pixels[0] = max(pixels[0] - rgb[0], 0);
                pixels[1] = max(pixels[1] - rgb[1], 0);
                pixels[2] = max(pixels[2] - rgb[2], 0);
            }
            al = _rotl8(al, 1);
            pixels += 4;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//pre-calculate lookup table for filled-circle
void calcCircle(int32_t rd, int32_t* point)
{
    //validate radius
    if (rd <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     ebx, 1
        sub     ebx, rd
        mov     edi, point
        mov     esi, edi
        mov     eax, rd
        shl     eax, 2
        add     esi, eax
        mov     eax, rd
        xor     ecx, ecx
    start:
        or      ebx, ebx
        jns     next
        mov     edx, ecx
        shl     edx, 1
        add     edx, 3
        add     ebx, edx
        inc     ecx
        sub     esi, 4
        jmp     stop
    next:
        mov     edx, ecx
        sub     edx, eax
        shl     edx, 1
        add     edx, 3
        add     ebx, edx
        inc     ecx
        dec     eax
        sub     esi, 4
        add     edi, 4
    stop:
        mov     [edi], ecx
        mov     [esi], eax
        cmp     eax, ecx
        jg      start
    }
#else
    int32_t eax = rd;
    int32_t ebx = 1 - rd;
    int32_t edi = 0, esi = rd;
    int32_t ecx = 0, edx = 0;

    while (ecx < eax)
    {
        if (ebx < 0)
        {
            edx = (ecx++ << 1) + 3;
            ebx += edx;
            esi--;
        }
        else
        {
            edx = ((ecx++ - eax--) << 1) + 3;
            ebx += edx;
            esi--;
            edi++;
        }
        point[edi] = ecx;
        point[esi] = eax;
    }
#endif
}

//pre-calculate lookup table for filled-ellipse
void calcEllipse(int32_t rx, int32_t ry, int32_t* point)
{
    int32_t ra = 0, aa = 0, bb = 0;
    int32_t xa = 0, mx = 0, my = 0;
    int32_t aq = 0, bq = 0, xd = 0, yd = 0;

    //validate radius
    if (rx <= 0 || ry <= 0) return;

#ifdef _USE_ASM
    _asm {
        mov     eax, rx
        mov     xa, eax
        neg     eax
        mov     mx, eax
        xor     edx, edx
        mov     eax, rx
        mul     eax
        mov     aq, eax
        shl     eax, 1
        mov     xd, eax
        mov     eax, ry
        mul     eax
        mov     bq, eax
        shl     eax, 1
        mov     yd, eax
        mov     eax, rx
        mul     bq
        mov     ra, eax
        shl     eax, 1
        mov     aa, eax
    next:
        cmp     ra, 0
        jle     skip
        inc     my
        mov     eax, bb
        add     eax, xd
        mov     bb, eax
        sub     ra, eax
    skip:
        cmp     ra, 0
        jg      done
        dec     xa
        inc     mx
        mov     eax, aa
        sub     eax, yd
        mov     aa, eax
        add     ra, eax
    done:
        mov     edi, point
        mov     ebx, ry
        sub     ebx, my
        shl     ebx, 2
        add     edi, ebx
        mov     eax, mx
        neg     eax
        stosd
        cmp     xa, 0
        jg      next
    }
#else
    xa = rx;
    mx = -rx;
    aq = rx * rx;
    xd = aq << 1;

    bq = ry * ry;
    yd = bq << 1;
    ra = bq * rx;
    aa = ra << 1;

    while (xa > 0)
    {
        if (ra > 0)
        {
            my++;
            ra -= (bb + xd);
            bb += xd;
        }
        if (ra <= 0)
        {
            xa--;
            mx++;
            ra += (aa - yd);
            aa -= yd;
        }
        point[ry - my] = -mx;
    }
#endif
}

//fast filled Bresenham circle at (xc,yc) with radius and color
void fillCircle(int32_t xc, int32_t yc, int32_t radius, uint32_t color)
{
    int32_t i = 0;
    int32_t point[500] = { 0 };

    //range limited
    if (radius > 499) messageBox(GFX_ERROR, "fillCircle: radius must be in [0-499]");

    int32_t mc = yc - radius;
    calcCircle(radius, point);

    for (i = 0; i <= radius - 1; i++)
    {
        horizLine(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }

    for (i = radius - 1; i >= 0; i--)
    {
        horizLine(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }
}

//fast filled Bresenham circle at (xc,yc) with radius and color
void fillCircleAdd(int32_t xc, int32_t yc, int32_t radius, uint32_t color)
{
    int32_t i = 0;
    int32_t point[500] = { 0 };

    //range limited
    if (radius > 499) messageBox(GFX_ERROR, "fillCircle: radius must be in [0-499]");

    int32_t mc = yc - radius;
    calcCircle(radius, point);

    for (i = 0; i <= radius - 1; i++)
    {
        horizLineAdd(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }

    for (i = radius - 1; i >= 0; i--)
    {
        horizLineAdd(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }
}

//fast filled Bresenham circle at (xc,yc) with radius and color
void fillCircleSub(int32_t xc, int32_t yc, int32_t radius, uint32_t color)
{
    int32_t i = 0;
    int32_t point[500] = { 0 };

    //range limited
    if (radius > 499) messageBox(GFX_ERROR, "fillCircle: radius must be in [0-499]");

    int32_t mc = yc - radius;
    calcCircle(radius, point);

    for (i = 0; i <= radius - 1; i++)
    {
        horizLineSub(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }

    for (i = radius - 1; i >= 0; i--)
    {
        horizLineSub(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }
}

//filled ellipse with color
void fillEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color)
{
    int32_t i = 0;
    int32_t point[500] = { 0 };

    //range limited
    if (ra > 499 || rb > 499) messageBox(GFX_ERROR, "fillEllipse: ra, rb must be in [0-499]");

    int32_t mc = yc - rb;

    if (ra != rb) calcEllipse(ra, rb, point);
    else calcCircle(ra, point);

    for (i = 0; i <= rb - 1; i++)
    {
        horizLine(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }

    for (i = rb - 1; i >= 0; i--)
    {
        horizLine(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }
}

//filled ellipse with add color
void fillEllipseAdd(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color)
{
    int32_t i = 0;
    int32_t point[500] = { 0 };

    //range limited
    if (ra > 499 || rb > 499) messageBox(GFX_ERROR, "fillEllipse: ra, rb must be in [0-499]");

    int32_t mc = yc - rb;

    if (ra != rb) calcEllipse(ra, rb, point);
    else calcCircle(ra, point);

    for (i = 0; i <= rb - 1; i++)
    {
        horizLineAdd(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }

    for (i = rb - 1; i >= 0; i--)
    {
        horizLineAdd(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }
}

//filled ellipse with sub color
void fillEllipseSub(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color)
{
    int32_t i = 0;
    int32_t point[500] = { 0 };

    //range limited
    if (ra > 499 || rb > 499) messageBox(GFX_ERROR, "fillEllipse: ra, rb must be in [0-499]");

    int32_t mc = yc - rb;

    if (ra != rb) calcEllipse(ra, rb, point);
    else calcCircle(ra, point);

    for (i = 0; i <= rb - 1; i++)
    {
        horizLineAdd(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }

    for (i = rb - 1; i >= 0; i--)
    {
        horizLineAdd(xc - point[i], mc, point[i] << 1, color);
        mc++;
    }
}

//fill polygon using Darel Rex Finley algorithm https://alienryderflex.com/polygon_fill/
//optimize version here https://gist.github.com/ideasman42/983738130f754ef58ffa66bcdbbab892
//test vectors (screen resolution: 800x600)
//pt1[] = {{300, 100}, {192, 209}, {407, 323}, {320, 380}, {214, 350}, {375, 209}};
//pt2[] = {{169, 164}, {169, 264}, {223, 300}, {296, 209}, {214, 255}, {223, 200}, {386, 192}, {341, 273}, {404, 300}, {431, 146}};
//pt3[] = {{97, 56}, {115, 236}, {205, 146}, {276, 146}, {151, 325}, {259, 433}, {510, 344}, {510, 218}, {242, 271}, {384, 110}};
//pt4[] = {{256, 150}, {148, 347}, {327, 329}, {311, 204}, {401, 204}, {418, 240}, {257, 222}, {293, 365}, {436, 383}, {455, 150}};
//pt5[] = {{287, 76}, {129, 110}, {42, 301}, {78, 353}, {146, 337}, {199, 162}, {391, 180}, {322, 353}, {321, 198}, {219, 370}, {391, 405}, {444, 232}, {496, 440}, {565, 214}};
void fillPolygon(POINT2D* point, int32_t num, uint32_t col)
{
    int32_t nodex[MAX_POLY_CORNERS] = { 0 };
    int32_t nodes = 0, y = 0, i = 0, j = 0, swap = 0;
    int32_t left = 0, right = 0, top = 0, bottom = 0;

    //initialize clipping
    left = right = int32_t(point[0].x);
    top = bottom = int32_t(point[0].y);

    //clipping points
    for (i = 1; i < num; i++)
    {
        if (point[i].x < left)      left    = int32_t(point[i].x);
        if (point[i].x > right)     right   = int32_t(point[i].x);
        if (point[i].y < top)       top     = int32_t(point[i].y);
        if (point[i].y > bottom)    bottom  = int32_t(point[i].y);
    }

    //loop through the rows of the image
    for (y = top; y < bottom; y++)
    {
        //build a list of polygon intercepts on the current line
        nodes = 0;
        j = num - 1;

        for (i = 0; i < num; i++)
        {
            //intercept found, record it
            if ((point[i].y < y && point[j].y >= y) || (point[j].y < y && point[i].y >= y)) nodex[nodes++] = int32_t(point[i].x + (y - point[i].y) / (point[j].y - point[i].y) * (point[j].x - point[i].x));
            if (nodes >= MAX_POLY_CORNERS) return;
            j = i;
        }

        //sort the nodes, via a simple "Bubble" sort
        i = 0;
        while (i < nodes - 1)
        {
            if (nodex[i] > nodex[i + 1])
            {
                swap = nodex[i];
                nodex[i] = nodex[i + 1];
                nodex[i + 1] = swap;
                if (i) i--;
            }
            else i++;
        }

        //fill the pixels between node pairs
        for (i = 0; i < nodes; i += 2)
        {
            if (nodex[i] >= right) break;
            if (nodex[i + 1] > left)
            {
                if (nodex[i] < left) nodex[i] = left;
                if (nodex[i + 1] > right) nodex[i + 1] = right;
                horizLine(nodex[i], y, nodex[i + 1] - nodex[i], col);
            }
        }
    }
}

//FX-effect: fade circle
void fadeCircle(int32_t dir, uint32_t col)
{
    int32_t i = 0, x = 0, y = 0;

    switch (dir)
    {
    case 0:
        for (i = 0; i < 29; i++)
        {
            for (y = 0; y <= cmaxY / 40; y++)
            {
                for (x = 0; x <= cmaxX / 40; x++) fillCircle(x * 40 + 20, y * 40 + 20, i, col);
            }
            render();
            delay(FPS_60);
        }
        break;

    case 1:
        for (i = -cmaxY / 40; i < 29; i++)
        {
            for (y = 0; y <= cmaxY / 40; y++)
            {
                for (x = 0; x <= cmaxX / 40; x++)
                {
                    if (cmaxY / 40 - y + i < 29) fillCircle(x * 40 + 20, y * 40 + 20, cmaxY / 40 - y + i, col);
                }
            }
            render();
            delay(FPS_60);
        }
        break;

    case 2:
        for (i = -cmaxX / 40; i < 29; i++)
        {
            for (y = 0; y <= cmaxY / 40; y++)
            {
                for (x = 0; x <= cmaxX / 40; x++)
                {
                    if (cmaxX / 40 - x + i < 29) fillCircle(x * 40 + 20, y * 40 + 20, cmaxX / 40 - x + i, col);
                }
            }
            render();
            delay(FPS_60);
        }
        break;

    case 3:
        for (i = -cmaxX / 40; i < 60; i++)
        {
            for (y = 0; y <= cmaxY / 40; y++)
            {
                for (x = 0; x <= cmaxX / 40; x++)
                {
                    if (cmaxX / 40 - x - y + i < 29) fillCircle(x * 40 + 20, y * 40 + 20, cmaxX / 40 - x - y + i, col);
                }
            }
            render();
            delay(FPS_60);
        }
        break;
    }
}

//FX-effect: fade rollo
void fadeRollo(int32_t dir, uint32_t col)
{
    int32_t i = 0, j = 0;

    switch (dir)
    {
    case 0:
        for (i = 0; i < 20; i++)
        {
            for (j = 0; j <= cmaxY / 10; j++) horizLine(0, j * 20 + i, cmaxX, col);
            render();
            delay(FPS_60);
        }
        break;

    case 1:
        for (i = 0; i < 20; i++)
        {
            for (j = 0; j <= cmaxX / 10; j++) vertLine(j * 20 + i, 0, cmaxY, col);
            render();
            delay(FPS_60);
        }
        break;

    case 2:
        for (i = 0; i < 20; i++)
        {
            for (j = 0; j <= cmaxX / 10; j++)
            {
                vertLine(j * 20 + i, 0, cmaxY, col);
                if (j * 10 < cmaxY) horizLine(0, j * 20 + i, cmaxX, col);
            }
            render();
            delay(FPS_60);
        }
        break;
    }
}

//simulation visual page in VGA mode
void setActivePage(GFX_IMAGE* page)
{
    setDrawBuffer(page->mData, page->mWidth, page->mHeight);
}

//set render page
void setVisualPage(GFX_IMAGE* page)
{
    setDrawBuffer(page->mData, page->mWidth, page->mHeight);
    render();
    restoreDrawBuffer();
}

//create a new GFX image
void newImage(int32_t width, int32_t height, GFX_IMAGE* img)
{
    //calcule buffer size, add to width and height
    const int32_t bytesPerPixel = bitsPerPixel >> 3;
    const uint32_t size = height * width * bytesPerPixel;

    if (!size)
    {
        messageBox(GFX_ERROR, "Cannot create image! image size = 0!");
        return;
    }

    img->mData = (uint8_t*)calloc(size, 1);
    if (!img->mData)
    {
        messageBox(GFX_ERROR, "Cannot alloc memory with size:%lu", size);
        return;
    }

    //store image width and height
    img->mWidth = width;
    img->mHeight = height;
    img->mRowBytes = width * bytesPerPixel;
    img->mSize = size;
}

//update GFX image with new width, new height
void updateImage(int32_t width, int32_t height, GFX_IMAGE* img)
{
    //no need update
    if (img->mWidth == width && img->mHeight == height) return;

    //calcule buffer size, add to width and height
    const int32_t bytesPerPixel = bitsPerPixel >> 3;
    const uint32_t size = height * width * bytesPerPixel;

    if (!size)
    {
        messageBox(GFX_ERROR, "Create image size = 0!");
        return;
    }

    uint8_t* pdata = (uint8_t*)realloc(img->mData, size);
    if (!pdata)
    {
        messageBox(GFX_ERROR, "Alloc memory with size: %lu", size);
        return;
    }

    //store image width and height
    memset(pdata, 0, size);
    img->mData = pdata;
    img->mWidth = width;
    img->mHeight = height;
    img->mRowBytes = width * bytesPerPixel;
    img->mSize = size;
}

//cleanup image buffer
void freeImage(GFX_IMAGE* img)
{
    if (img && img->mData)
    {
        free(img->mData);
        img->mData = NULL;
        img->mWidth = 0;
        img->mHeight = 0;
        img->mRowBytes = 0;
        img->mSize = 0;
    }
}

//clear image data buffer
void clearImage(GFX_IMAGE* img)
{
    if (img && img->mData) memset(img->mData, 0, img->mSize);
}

//get GFX image buffer functions
void getImage8(int32_t x1, int32_t y1, int32_t width, int32_t height, GFX_IMAGE* img)
{
    if (bitsPerPixel != 8) return;

    //calculate new position
    const int32_t x2 = (x1 + width) - 1;
    const int32_t y2 = (y1 + height) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

    //create new image
    if (!img->mData) newImage(lbWidth, lbHeight, img);
    else if (img->mWidth != lbWidth || img->mHeight != lbHeight) updateImage(lbWidth, lbHeight, img);

#ifdef _USE_ASM
    void *imgData = img->mData;
    _asm {
        mov     edi, imgData
        mov     esi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        add     esi, eax
        mov     edx, texWidth
        sub     edx, lbWidth
        mov     eax, lbWidth
        shr     eax, 2
        mov     ebx, lbWidth
        and     ebx, 3
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* imgPixels = (uint8_t*)img->mData;
    uint8_t* dstPixels = (uint8_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    if (!dstPixels || !imgPixels) return;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        memcpy(imgPixels, dstPixels, img->mWidth);
        dstPixels += texWidth;
        imgPixels += img->mWidth;
    }
#endif
}

//get GFX image buffer functions
void getImage32(int32_t x1, int32_t y1, int32_t width, int32_t height, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + width) - 1;
    const int32_t y2 = (y1 + height) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

    //create new image
    if (!img->mData) newImage(lbWidth, lbHeight, img);
    else if (img->mWidth != lbWidth || img->mHeight != lbHeight) updateImage(lbWidth, lbHeight, img);

#ifdef _USE_ASM
    void *imgData = img->mData;
    _asm {
        mov     edi, imgData
        mov     esi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     esi, eax
        mov     edx, texWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
        rep     movsd
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else    
    //calculate starting address
    uint32_t* imgPixels = (uint32_t*)img->mData;
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    if (!dstPixels || !imgPixels) return;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        memcpy(imgPixels, dstPixels, intptr_t(img->mWidth) << 2);
        dstPixels += texWidth;
        imgPixels += img->mWidth;
    }
#endif
}

//put GFX image to point (x1, y1)
void putImage8(int32_t x1, int32_t y1, GFX_IMAGE* img)
{
    if (bitsPerPixel != 8) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth  = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        push    ebx
        mov     edx, imgWidth
        sub     edx, lbWidth
        mov     eax, lbWidth
        shr     eax, 2
        mov     ebx, lbWidth
        and     ebx, 3
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     edi, [esp]
        add     esi, edx
        dec     lbHeight
        jnz     next
        pop     ebx
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    uint8_t* imgPixels = (uint8_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1);
    if (!dstPixels || !imgPixels) return;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        memcpy(dstPixels, imgPixels, lbWidth);
        dstPixels += texWidth;
        imgPixels += img->mWidth;
    }
#endif
}

//put GFX image to point (x1, y1)
void putImage32(int32_t x1, int32_t y1, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth  = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
        rep     movsd
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    uint32_t* imgPixels = (uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1);
    if (!dstPixels || !imgPixels) return;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        memcpy(dstPixels, imgPixels, intptr_t(lbWidth) << 2);
        dstPixels += texWidth;
        imgPixels += img->mWidth;
    }
#endif
}

//put GFX image with add background color
void putImageAdd(int32_t x1, int32_t y1, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth  = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, width
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, width
        shl     edx, 2
    again:
        mov     ecx, width
        push    ebx
    plot:
        lodsd
        add     al, [edi]
        jnc     bstep
        mov     al, 0FFh
    bstep:
        add     ah, [edi + 1]
        jnc     gstep
        mov     ah, 0FFh
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, [edi + 2]
        jnc     rstep
        mov     bl, 0FFh
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        loop    plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     again
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    uint8_t* imgPixels = (uint8_t*)((uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1));
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = (texWidth - width) << 2;
    const uint32_t addImgOffs = (img->mWidth - width) << 2;
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < width; j++)
        {
            dstPixels[0] = min(dstPixels[0] + imgPixels[0], 255);
            dstPixels[1] = min(dstPixels[1] + imgPixels[1], 255);
            dstPixels[2] = min(dstPixels[2] + imgPixels[2], 255);
            dstPixels += 4;
            imgPixels += 4;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//put GFX image with sub background color
void putImageSub(int32_t x1, int32_t y1, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth  = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
        push    ebx
    plot:
        mov     eax, [edi]
        sub     al, [esi]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, [esi + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, [esi + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        stosd
        add     esi, 4
        loop    plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    uint8_t* imgPixels = (uint8_t*)((uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1));
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = (texWidth - lbWidth) << 2;
    const uint32_t addImgOffs = (img->mWidth - lbWidth) << 2;
    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            dstPixels[0] = max(dstPixels[0] - imgPixels[0], 0);
            dstPixels[1] = max(dstPixels[1] - imgPixels[1], 0);
            dstPixels[2] = max(dstPixels[2] - imgPixels[2], 0);
            dstPixels += 4;
            imgPixels += 4;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//put GFX image with alpha color
void putImageAlpha(int32_t x1, int32_t y1, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t width = (lx2 - lx1) + 1;
    const int32_t height = (ly2 - ly1) + 1;

    //check for loop
    if (!width || !height) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth  = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, width
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, width
        shl     edx, 2
    again:
        mov     ecx, width
        push    ebx
        push    edx
    plot:
        mov     al, [esi]
        mul     byte ptr[esi + 3]
        mov     bx, ax
        mov     al, [edi]
        mov     dl, 255
        sub     dl, [esi + 3]
        mul     dl
        add     ax, bx
        shr     ax, 8
        stosb
        mov     al, [esi + 1]
        mul     byte ptr[esi + 3]
        mov     bx, ax
        mov     al, [edi]
        mov     dl, 255
        sub     dl, [esi + 3]
        mul     dl
        add     ax, bx
        shr     ax, 8
        stosb
        mov     al, [esi + 2]
        mul     byte ptr[esi + 3]
        mov     bx, ax
        mov     al, [edi]
        mov     dl, 255
        sub     dl, [esi + 3]
        mul     dl
        add     ax, bx
        shr     ax, 8
        stosb
        inc     edi
        add     esi, 4
        loop    plot
        pop     edx
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     again
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    uint8_t* imgPixels = (uint8_t*)((uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1));
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = (texWidth - width) << 2;
    const uint32_t addImgOffs = (img->mWidth - width) << 2;

    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < width; j++)
        {
            dstPixels[0] = (imgPixels[3] * imgPixels[0] + dstPixels[0] * (255 - imgPixels[3])) >> 8;
            dstPixels[1] = (imgPixels[3] * imgPixels[1] + dstPixels[1] * (255 - imgPixels[3])) >> 8;
            dstPixels[2] = (imgPixels[3] * imgPixels[2] + dstPixels[2] * (255 - imgPixels[3])) >> 8;
            dstPixels += 4;
            imgPixels += 4;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//put a sprite at point(x1, y1) with key color (don't render key color)
void putSprite8(int32_t x1, int32_t y1, uint32_t keyColor, GFX_IMAGE* img)
{
    if (bitsPerPixel != 8) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth  = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        mov     edx, imgWidth
        sub     edx, lbWidth
    next:
        mov     ecx, lbWidth
    plot:
        lodsb
        cmp     al, byte ptr[keyColor]
        je      skip
        mov     [edi], al
    skip:
        inc     edi
        loop    plot
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    uint8_t* imgPixels = (uint8_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1);
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = texWidth - lbWidth;
    const uint32_t addImgOffs = img->mWidth - lbWidth;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            if (*imgPixels != keyColor) *dstPixels = *imgPixels;
            dstPixels++;
            imgPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//put a sprite at point(x1, y1) with key color (don't render key color)
void putSprite32(int32_t x1, int32_t y1, uint32_t keyColor, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
    plot:
        lodsd
        and     eax, 00FFFFFFh
        cmp     eax, keyColor
        je      skip
        mov     [edi], eax
    skip:
        add     edi, 4
        loop    plot
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    uint32_t* imgPixels = (uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1);
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = texWidth - lbWidth;
    const uint32_t addImgOffs = img->mWidth - lbWidth;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            if ((*imgPixels & 0x00FFFFFF) != keyColor) *dstPixels = *imgPixels;
            dstPixels++;
            imgPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//put a sprite at point(x1, y1) with key color (don't render key color), add with background color
void putSpriteAdd(int32_t x1, int32_t y1, uint32_t keyColor, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
        push    ebx
    plot:
        lodsd
        and     eax, 00FFFFFFh
        cmp     eax, keyColor
        je      skip
        add     al, [edi]
        jnc     bstep
        mov     al, 255
    bstep:
        add     ah, [edi + 1]
        jnc     gstep
        mov     ah, 255
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        add     bl, [edi + 2]
        jnc     rstep
        mov     bl, 255
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        mov     [edi], eax
    skip:
        add     edi, 4
        loop    plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    uint8_t* imgPixels = (uint8_t*)((uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1));
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = (texWidth - lbWidth) << 2;
    const uint32_t addImgOffs = (img->mWidth - lbWidth) << 2;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            //we accepted RGB color only
            if ((*(uint32_t*)imgPixels & 0x00FFFFFF) != keyColor)
            {
                dstPixels[0] = min(dstPixels[0] + imgPixels[0], 255);
                dstPixels[1] = min(dstPixels[1] + imgPixels[1], 255);
                dstPixels[2] = min(dstPixels[2] + imgPixels[2], 255);
            }
            dstPixels += 4;
            imgPixels += 4;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//put a sprite at point(x1, y1) with key color (don't render key color), sub with background color
void putSpriteSub(int32_t x1, int32_t y1, uint32_t keyColor, GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) return;

    //calculate new position
    const int32_t x2 = (x1 + img->mWidth) - 1;
    const int32_t y2 = (y1 + img->mHeight) - 1;

    //clip image to context boundaries
    const int32_t lx1 = (x1 >= cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 >= cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 <= cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 <= cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

#ifdef _USE_ASM
    void* imgData = img->mData;
    int32_t imgWidth = img->mWidth;

    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly1
        sub     eax, y1
        mul     imgWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
        push    ebx
    plot:
        mov     eax, [esi]
        and     eax, 00FFFFFFh
        cmp     eax, keyColor
        je      skip
        mov     eax, [edi]
        sub     al, [esi]
        jnc     bstep
        xor     al, al
    bstep:
        sub     ah, [esi + 1]
        jnc     gstep
        xor     ah, ah
    gstep:
        mov     ebx, eax
        shr     ebx, 16
        sub     bl, [esi + 2]
        jnc     rstep
        xor     bl, bl
    rstep:
        shl     ebx, 16
        and     eax, 00FFFFh
        or      eax, ebx
        mov     [edi], eax
    skip:
        add     edi, 4
        add     esi, 4
        loop    plot
        pop     ebx
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* dstPixels = (uint8_t*)((uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1);
    uint8_t* imgPixels = (uint8_t*)((uint32_t*)img->mData + img->mWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1));
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = (texWidth - lbWidth) << 2;
    const uint32_t addImgOffs = (img->mWidth - lbWidth) << 2;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            //we accepted RGB color only
            if ((*(uint32_t*)imgPixels & 0x00FFFFFF) != keyColor)
            {
                dstPixels[0] = max(dstPixels[0] - imgPixels[0], 0);
                dstPixels[1] = max(dstPixels[1] - imgPixels[1], 0);
                dstPixels[2] = max(dstPixels[2] - imgPixels[2], 0);
            }
            dstPixels += 4;
            imgPixels += 4;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
}

//Cohen-Sutherland clipping algorithm
int32_t getCode(int32_t x, int32_t y)
{
    int32_t code = 0;
    if (y >= texHeight)	code |= 1; //top
    else if (y < 0)		code |= 2; //bottom
    if (x >= texWidth)	code |= 4; //right
    else if (x < 0)		code |= 8; //left
    return code;
}

//Cohen-Sutherland clipping line (xs,ys)-(xe, ye)
void clipLine(int32_t *xs, int32_t *ys, int32_t *xe, int32_t *ye)
{
    int32_t accept = 0, done = 0;
    int32_t x1 = *xs, x2 = *xe, y1 = *ys, y2 = *ye;

    //the region outcodes for the endpoints
    int32_t code1 = getCode(x1, y1);
    int32_t code2 = getCode(x2, y2);

    //in theory, this can never end up in an infinite loop, it'll always come in one of the trivial cases eventually
    do
    {
        //accept because both endpoints are in screen or on the border, trivial accept
        if (!(code1 | code2)) accept = done = 1;

        //the line isn't visible on screen, trivial reject
        else if (code1 & code2) done = 1;

        //if no trivial reject or accept, continue the loop
        else
        {
            int32_t x = 0, y = 0;
            int32_t codeout = code1 ? code1 : code2;

            if (codeout & 1)
            {
                //top
                x = x1 + (x2 - x1) * (texHeight - y1) / (y2 - y1);
                y = texHeight - 1;
            }
            else if (codeout & 2)
            {
                //bottom
                x = x1 + (x2 - x1) * -y1 / (y2 - y1);
                y = 0;
            }
            else if (codeout & 4)
            {
                //right
                y = y1 + (y2 - y1) * (texWidth - x1) / (x2 - x1);
                x = texWidth - 1;
            }
            else
            {
                //left
                y = y1 + (y2 - y1) * -x1 / (x2 - x1);
                x = 0;
            }

            if (codeout == code1)
            {
                //first endpoint was clipped
                x1 = x;
                y1 = y;
                code1 = getCode(x1, y1);
            }
            else
            {
                //second endpoint was clipped
                x2 = x;
                y2 = y;
                code2 = getCode(x2, y2);
            }
        }
    } while (!done);

    if (accept)
    {
        *xs = x1;
        *xe = x2;
        *ys = y1;
        *ye = y2;
    }
    else
    {
        *xs = 0;
        *xe = 0;
        *ys = 0;
        *ye = 0;
    }
}

//set palette color to render palette table
void setPalette(RGB* pal)
{
    SDL_SetPaletteColors(sdlSurface->format->palette, pal, 0, 256);
    render();
}

//get current palette table
void getPalette(RGB* pal)
{
    memcpy(pal, sdlSurface->format->palette->colors, sdlSurface->format->palette->ncolors * sizeof(RGB));
}

//FX-effect: palette rotation
void rotatePalette(int32_t from, int32_t to, int32_t loop, int32_t ms)
{
    RGB tmp = { 0 };
    RGB pal[256] = { 0 };
    
    getPalette(pal);

    if (loop > 0)
    {
        while (loop--)
        {
            memcpy(&tmp, &pal[from], sizeof(RGB));
            memcpy(&pal[from], &pal[from + 1], (intptr_t(to) - from) * sizeof(RGB));
            memcpy(&pal[to], &tmp, sizeof(RGB));
            setPalette(pal);
            delay(ms);
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }
    }
    else
    {
        while (1)
        {
            memcpy(&tmp, &pal[from], sizeof(RGB));
            memcpy(&pal[from], &pal[from + 1], (intptr_t(to) - from) * sizeof(RGB));
            memcpy(&pal[to], &tmp, sizeof(RGB));
            setPalette(pal);
            delay(ms);
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }
    }
}

//FX-effect: palette fade-in
void fadeIn(RGB* dest, uint32_t ms)
{
    RGB src[256] = { 0 };
    int32_t i = 0, j = 0, k = 0;
    
    getPalette(src);
    for (i = 63; i >= 0; i--)
    {
        for (j = 0; j < 256; j++)
        {
            k = i << 2;
            if (dest[j].r > k && src[j].r < 252) src[j].r += 4;
            if (dest[j].g > k && src[j].g < 252) src[j].g += 4;
            if (dest[j].b > k && src[j].b < 252) src[j].b += 4;
        }
        setPalette(src);
        delay(ms);
        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }
}

//FX-effect: palette fade-out
void fadeOut(RGB* dest, uint32_t ms)
{
    RGB src[256] = { 0 };
    int32_t i = 0, j = 0, k = 0;
    
    getPalette(src);
    for (i = 63; i >= 0; i--)
    {
        for (j = 0; j < 256; j++)
        {
            k = i << 2;
            if (dest[j].r < k && src[j].r > 4) src[j].r -= 4;
            if (dest[j].g < k && src[j].g > 4) src[j].g -= 4;
            if (dest[j].b < k && src[j].b > 4) src[j].b -= 4;
        }
        setPalette(src);
        delay(ms);
        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }
}

//FX-effect: palette fade-max
void fadeMax(uint32_t ms)
{
    RGB src[256] = { 0 };
    int32_t i = 0, j = 0;
    
    getPalette(src);
    for (i = 0; i < 64; i++)
    {
        for (j = 0; j < 256; j++)
        {
            if (src[j].r < 252) src[j].r += 4; else src[j].r = 255;
            if (src[j].g < 252) src[j].g += 4; else src[j].g = 255;
            if (src[j].b < 252) src[j].b += 4; else src[j].b = 255;
        }
        setPalette(src);
        delay(ms);
        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }
}

//FX-effect: palette fade-min
void fadeMin(uint32_t ms)
{
    RGB src[256] = { 0 };
    int32_t i = 0, j = 0;
    
    getPalette(src);
    for (i = 0; i < 64; i++)
    {
        for (j = 0; j < 256; j++)
        {
            if (src[j].r > 4) src[j].r -= 4; else src[j].r = 0;
            if (src[j].g > 4) src[j].g -= 4; else src[j].g = 0;
            if (src[j].b > 4) src[j].b -= 4; else src[j].b = 0;
        }
        setPalette(src);
        delay(ms);
        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }
}

//FX-effect: palette fade-down
void fadeDown(RGB* pal)
{
    for (int32_t i = 0; i < 256; i++)
    {
        if (pal[i].r > 4) pal[i].r -= 2; else pal[i].r = 0;
        if (pal[i].g > 4) pal[i].g -= 2; else pal[i].g = 0;
        if (pal[i].b > 4) pal[i].b -= 2; else pal[i].b = 0;
    }
    setPalette(pal);
}

//convert palette buffer to RGB buffer
void convertPalette(const uint8_t* palette, RGB* color)
{
    uint8_t* rgb = (uint8_t*)color;
    uint8_t* pal = (uint8_t*)palette;
    for (int32_t i = 0; i < 256; i++)
    {
        rgb[0] = pal[0] << 2;
        rgb[1] = pal[1] << 2;
        rgb[2] = pal[2] << 2;
        rgb += 4;
        pal += 3;
    }
}

//convert current RGB palette to RGB32
void shiftPalette(RGB* pal)
{
    uint8_t* rgb = (uint8_t*)pal;
    for (int32_t i = 0; i < 256; i++)
    {
        rgb[0] <<= 2;
        rgb[1] <<= 2;
        rgb[2] <<= 2;
        rgb += 4;
    }
}

//make a black palette
void clearPalette()
{
    RGB pal[256] = { 0 };
    setPalette(pal);
}

//make white palette
void whitePalette()
{
    RGB pal[256] = { 255 };
    setPalette(pal);
}

//build default 256 colors palette
void getBasePalette(RGB* pal)
{
    memcpy(pal, basePalette, sizeof(basePalette));
}

//make linear palette (7 circle colors)
void makeLinearPalette()
{
    int16_t i = 0, j = 0;
    RGB pal[256] = { 0 };

    for (i = 0; i < 32; i++)
    {
        j = 16 + i;
        pal[j].r = 63;
        pal[j].g = 0;
        pal[j].b = 63 - (i << 1);

        j = 48 + i;
        pal[j].r = 63;
        pal[j].g = i << 1;
        pal[j].b = 0;

        j = 80 + i;
        pal[j].r = 63 - (i << 1);
        pal[j].g = 63;
        pal[j].b = 0;

        j = 112 + i;
        pal[j].r = 0;
        pal[j].g = 63;
        pal[j].b = i << 1;

        j = 144 + i;
        pal[j].r = 0;
        pal[j].g = 63 - (i << 1);
        pal[j].b = 63;

        j = 176 + i;
        pal[j].r = i << 1;
        pal[j].g = 0;
        pal[j].b = 63;
    }

    shiftPalette(pal);
    setPalette(pal);
}

//yet another funky palette
void makeFunkyPalette()
{
    RGB pal[256] = { 0 };

    int32_t r = 0;
    int32_t g = 0;
    int32_t b = 0;

    uint8_t ry = 1;
    uint8_t gy = 1;
    uint8_t by = 1;

    int32_t rx = (rand() % 5) + 1;
    int32_t gx = (rand() % 5) + 1;
    int32_t bx = (rand() % 5) + 1;

    for (int32_t i = 0; i < 256; i++)
    {
        pal[i].r = r;
        pal[i].g = g;
        pal[i].b = b;

        if (ry) r += rx; else r -= rx;
        if (gy) g += gx; else g -= gx;
        if (by) b += bx; else b -= bx;

        if ((r + rx > 63) || (r - rx < 0))
        {
            ry = !ry;
            rx = (rand() % 5) + 1;
        }

        if ((g + gx > 63) || (g - gx < 0))
        {
            gy = !gy;
            gx = (rand() % 5) + 1;
        }

        if ((b + bx > 63) || (b - bx < 0))
        {
            by = !by;
            bx = (rand() % 5) + 1;
        }
    }

    shiftPalette(pal);
    setPalette(pal);
}

//make rainbow palette
void makeRainbowPalette()
{
    RGB pal[256] = { 0 };

    for (int32_t i = 0; i < 32; i++)
    {
        pal[i      ].r = i << 1;
        pal[63 - i ].r = i << 1;
        pal[i + 64 ].g = i << 1;
        pal[127 - i].g = i << 1;
        pal[i + 128].b = i << 1;
        pal[191 - i].b = i << 1;
        pal[i + 192].r = i << 1;
        pal[i + 192].g = i << 1;
        pal[i + 192].b = i << 1;
        pal[255 - i].r = i << 1;
        pal[255 - i].g = i << 1;
        pal[255 - i].b = i << 1;
    }

    shiftPalette(pal);
    setPalette(pal);
}

//scrolling current palette
void scrollPalette(int32_t from, int32_t to, int32_t step)
{
    RGB tmp = { 0 };
    RGB pal[256] = { 0 };

    getPalette(pal);

    while (step--)
    {
        memcpy(&tmp, &pal[from], sizeof(tmp));
        memcpy(&pal[from], &pal[from + 1], (intptr_t(to) - from) * sizeof(RGB));
        memcpy(&pal[to], &tmp, sizeof(tmp));
        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }

    setPalette(pal);
}

//move to drawing pointer
void moveTo(int32_t x, int32_t y)
{
    currX = x;
    currY = y;
}

//draw line from current to (x,y)
void lineTo(int32_t x, int32_t y, uint32_t col)
{
    drawLine(currX, currY, x, y, col);
    moveTo(x, y);
}

//draw line from current to (x,y) with add color
void lineToAdd(int32_t x, int32_t y, uint32_t col)
{
    drawLineAdd(currX, currY, x, y, col);
    moveTo(x, y);
}

//draw line from current to (x,y) with sub color
void lineToSub(int32_t x, int32_t y, uint32_t col)
{
    drawLineSub(currX, currY, x, y, col);
    moveTo(x, y);
}

//draw line from current to (x,y) with alpha color
void lineToAlpha(int32_t x, int32_t y, uint32_t col)
{
    if (bitsPerPixel != 32) return;
    drawLineAlpha(currX, currY, x, y, col);
    moveTo(x, y);
}

//initialize 3D projection params
void initProjection()
{
    const double ph = M_PI * phi / 180;
    const double th = M_PI * theta / 180;

    aux1 = sin(th);
    aux2 = sin(ph);
    aux3 = cos(th);
    aux4 = cos(ph);
    aux5 = aux3 * aux2;
    aux6 = aux1 * aux2;
    aux7 = aux3 * aux4;
    aux8 = aux1 * aux4;
}

//projection point (x,y,z)
void projette(double x, double y, double z)
{
    obsX = -x * aux1 + y * aux3;
    obsY = -x * aux5 - y * aux6 + z * aux4;

    switch (projection)
    {
    case PROJ_TYPE::PERSPECTIVE:
        obsZ = -x * aux7 - y * aux8 - z * aux2 + rho;
        projX = DE * obsX / obsZ;
        projY = DE * obsY / obsZ;
    break;

    case PROJ_TYPE::PARALLELE:
        projX = DE * obsX;
        projY = DE * obsY;
    break;

    default:
        messageBox(GFX_WARNING, "Unknown projection type!");
    break;
    }
}

//move current cursor in 3D mode
void deplaceEn(double x, double y, double z)
{
    projette(x, y, z);
    cranX = int32_t(centerX + projX * ECHE);
    cranY = int32_t(centerY - projY);
    moveTo(cranX, cranY);
}

//draw line from current cursor in 3D mode
void traceVers(double x, double y, double z, uint32_t col)
{
    projette(x, y, z);
    cranX = int32_t(centerX + projX * ECHE);
    cranY = int32_t(centerY - projY);
    lineTo(cranX, cranY, col);
}

//draw line from current cursor in 3D mode with add color
void traceVersAdd(double x, double y, double z, uint32_t col)
{
    projette(x, y, z);
    cranX = int32_t(centerX + projX * ECHE);
    cranY = int32_t(centerY - projY);
    lineToAdd(cranX, cranY, col);
}

//draw line from current cursor in 3D mode with sub color
void traceVersSub(double x, double y, double z, uint32_t col)
{
    projette(x, y, z);
    cranX = int32_t(centerX + projX * ECHE);
    cranY = int32_t(centerY - projY);
    lineToSub(cranX, cranY, col);
}

//string position
int32_t strPos(char* str, const char* sub)
{
    const char* ptr = strstr(str, sub);
    return ptr ? int32_t(ptr - str) : -1;
}

//insert character to current string
void insertChar(char* str, uint8_t chr, int32_t pos)
{
    if (pos < 0 || pos >= int32_t(strlen(str))) return;
    str[pos] = chr;
}

//delete character of string
void strDelete(char* str, int32_t i, int32_t num)
{
    if (i < 0 || i >= int32_t(strlen(str))) return;
    memmove(&str[i + 1], &str[i + num], strlen(str) - i - 1);
}

//replace current string with another string
void schRepl(char* str, const char* schr, uint8_t repl)
{
    int32_t pos = strPos(str, schr);
    while (pos >= 0)
    {
        strDelete(str, pos, int32_t(strlen(schr)));
        insertChar(str, repl, pos);
        pos = strPos(str, schr);
    }
}

//character to string convertion
void chr2Str(uint8_t chr, uint8_t num, char* str)
{
    str[0] = chr;
    str[1] = num;
    str[2] = 0;
}

//encode string to VNI string (format type VNI)
void makeFont(char* str)
{
    char buff[4] = { 0 };
    schRepl(str, "a8", 128);
    chr2Str(128, '1', buff);
    schRepl(str, buff, 129);
    chr2Str(128, '2', buff);
    schRepl(str, buff, 130);
    chr2Str(128, '3', buff);
    schRepl(str, buff, 131);
    chr2Str(128, '4', buff);
    schRepl(str, buff, 132);
    chr2Str(128, '5', buff);
    schRepl(str, buff, 133);
    schRepl(str, "a6", 134);
    chr2Str(134, '1', buff);
    schRepl(str, buff, 135);
    chr2Str(134, '2', buff);
    schRepl(str, buff, 136);
    chr2Str(134, '3', buff);
    schRepl(str, buff, 137);
    chr2Str(134, '4', buff);
    schRepl(str, buff, 138);
    chr2Str(134, '5', buff);
    schRepl(str, buff, 139);
    schRepl(str, "e6", 140);
    chr2Str(140, '1', buff);
    schRepl(str, buff, 141);
    chr2Str(140, '2', buff);
    schRepl(str, buff, 142);
    chr2Str(140, '3', buff);
    schRepl(str, buff, 143);
    chr2Str(140, '4', buff);
    schRepl(str, buff, 144);
    chr2Str(140, '5', buff);
    schRepl(str, buff, 145);
    schRepl(str, "o7", 146);
    chr2Str(146, '1', buff);
    schRepl(str, buff, 147);
    chr2Str(146, '2', buff);
    schRepl(str, buff, 148);
    chr2Str(146, '3', buff);
    schRepl(str, buff, 149);
    chr2Str(146, '4', buff);
    schRepl(str, buff, 150);
    chr2Str(146, '5', buff);
    schRepl(str, buff, 151);
    schRepl(str, "o6", 152);
    chr2Str(152, '1', buff);
    schRepl(str, buff, 153);
    chr2Str(152, '2', buff);
    schRepl(str, buff, 154);
    chr2Str(152, '3', buff);
    schRepl(str, buff, 155);
    chr2Str(152, '4', buff);
    schRepl(str, buff, 156);
    chr2Str(152, '5', buff);
    schRepl(str, buff, 157);
    schRepl(str, "u7", 158);
    chr2Str(158, '1', buff);
    schRepl(str, buff, 159);
    chr2Str(158, '2', buff);
    schRepl(str, buff, 160);
    chr2Str(158, '3', buff);
    schRepl(str, buff, 161);
    chr2Str(158, '4', buff);
    schRepl(str, buff, 162);
    chr2Str(158, '5', buff);
    schRepl(str, buff, 163);
    schRepl(str, "a1", 164);
    schRepl(str, "a2", 165);
    schRepl(str, "a3", 166);
    schRepl(str, "a4", 167);
    schRepl(str, "a5", 168);
    schRepl(str, "e1", 169);
    schRepl(str, "e2", 170);
    schRepl(str, "e3", 171);
    schRepl(str, "e4", 172);
    schRepl(str, "e5", 173);
    schRepl(str, "i1", 174);
    schRepl(str, "i2", 175);
    schRepl(str, "i3", 181);
    schRepl(str, "i4", 182);
    schRepl(str, "i5", 183);
    schRepl(str, "o1", 184);
    schRepl(str, "o2", 190);
    schRepl(str, "o3", 198);
    schRepl(str, "o4", 199);
    schRepl(str, "o5", 208);
    schRepl(str, "u1", 210);
    schRepl(str, "u2", 211);
    schRepl(str, "u3", 212);
    schRepl(str, "u4", 213);
    schRepl(str, "u5", 214);
    schRepl(str, "y1", 215);
    schRepl(str, "y2", 216);
    schRepl(str, "y3", 221);
    schRepl(str, "y4", 222);
    schRepl(str, "y5", 248);
    schRepl(str, "d9", 249);
    schRepl(str, "D9", 250);
}

//generate random word array
void randomBuffer(void* buff, int32_t count, int32_t range)
{
#ifdef _USE_ASM
    _asm {
        mov     edi, buff
        mov     ecx, count
        mov     ebx, randSeed
    next:
        mov     eax, ebx
        mul     factor
        inc     eax
        mov     ebx, eax
        shr     eax, 16
        mul     range
        shr     eax, 16
        stosw
        loop    next
        mov     randSeed, ebx
    }
#else
    //check range
    int32_t val = 0;
    uint8_t* ptrBuff = (uint8_t*)buff;
    if (!count || !randSeed || !range) return;

    for (int32_t i = 0; i < count; i++)
    {
        val = factor * randSeed + 1;
        *(uint16_t*)&ptrBuff[i] = ((val >> 16) * range) >> 16;
    }

    randSeed = val;
#endif
}

//set current selected font
void setFontType(int32_t type)
{
    //check current range
    if (type >= GFX_MAX_FONT) type = GFX_MAX_FONT - 1;
    fontType = type;
}

//set current font size
void setFontSize(uint32_t size)
{
    //have sub-fonts
    if (gfxFonts[fontType].header.subFonts > 0)
    {
        //correct sub-fonts number
        if (size > gfxFonts[fontType].header.subFonts) size = gfxFonts[fontType].header.subFonts;
        //copy sub-fonts header
        memcpy(&gfxFonts[fontType].header.subData, &gfxFonts[fontType].dataPtr[(intptr_t(gfxFonts[fontType].header.subData.endChar) - gfxFonts[fontType].header.subData.startChar + 1) * 4 * (intptr_t(gfxFonts[fontType].header.subFonts) + 1) + size * sizeof(GFX_CHAR_HEADER)], sizeof(GFX_CHAR_HEADER));
    }
    subFonts = size;
}

//get height of string with current font in pixel
int32_t getFontHeight(const char* str)
{
    uint32_t i = 0;
    uint32_t height = 0;
    uint32_t mempos = 0;
    uint32_t size = 0;
    uint32_t len = uint32_t(strlen(str));

    //check for font is loaded
    if (!gfxFonts[fontType].dataPtr) return 0;
    if (!str || !len) return 0;

    //fixed font, all characters have a same height
    if (gfxFonts[fontType].header.flags & GFX_FONT_FIXED) height = gfxFonts[fontType].header.subData.height;
    else
    {
        //vector font
        if (gfxFonts[fontType].header.flags & GFX_FONT_VECTOR)
        {
            for (i = 0; i < len; i++)
            {
                //skip invalid character
                if (str[i] < gfxFonts[fontType].header.subData.startChar || str[i] > gfxFonts[fontType].header.subData.endChar) continue;

                //position of raw data of current character
                mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((str[i] - gfxFonts[fontType].header.subData.startChar) << 2)];
                size = gfxFonts[fontType].dataPtr[mempos + 1];
                if (size > height) height = size;
            }
            height *= subFonts;
        }
        else
        {
            //BMP1 font
            if (gfxFonts[fontType].header.subData.bitsPerPixel == 1)
            {
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < gfxFonts[fontType].header.subData.startChar || str[i] > gfxFonts[fontType].header.subData.endChar) continue;

                    //position of raw data of current character
                    mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((str[i] - gfxFonts[fontType].header.subData.startChar) << 2)];
                    size = gfxFonts[fontType].dataPtr[mempos + 1];
                    if (size > height) height = size;
                }
            }
            else if (gfxFonts[fontType].header.subData.bitsPerPixel >= 2 && gfxFonts[fontType].header.subData.bitsPerPixel <= 32)
            {
                //BMP8 and RGB font
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < gfxFonts[fontType].header.subData.startChar || str[i] > gfxFonts[fontType].header.subData.endChar) continue;

                    //position of raw data of current character
                    mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((str[i] - gfxFonts[fontType].header.subData.startChar) << 2)];
                    size = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 4] + *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 12];
                    if (size > height) height = size;
                }
            }
        }
    }

    //animation font
    if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS) height += gfxFonts[fontType].header.subData.randomY;
    return height;
}

//get width of string with current font in pixel
int32_t getFontWidth(const char* str)
{
    GFX_STROKE_DATA* data = NULL;
    uint32_t i = 0;
    uint32_t mempos = 0;
    uint32_t width = 0;
    uint32_t size = 0;
    uint32_t len = uint32_t(strlen(str));

    //check for font is loaded
    if (!gfxFonts[fontType].dataPtr) return 0;
    if (!str || !len) return 0;

    //fixed font, all characters have a same width
    if (gfxFonts[fontType].header.flags & GFX_FONT_FIXED) width = (gfxFonts[fontType].header.subData.width + gfxFonts[fontType].header.subData.distance) * len;
    else
    {
        //vector font
        if (gfxFonts[fontType].header.flags & GFX_FONT_VECTOR)
        {
            for (i = 0; i < len; i++)
            {
                //skip invalid character
                if (str[i] < gfxFonts[fontType].header.subData.startChar || str[i] > gfxFonts[fontType].header.subData.endChar) size = gfxFonts[fontType].header.subData.spacer;
                else
                {
                    //position of raw data of current character
                    mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((str[i] - gfxFonts[fontType].header.subData.startChar) << 2)];
                    data = (GFX_STROKE_DATA*)&gfxFonts[fontType].dataPtr[mempos];
                    size = data->width * subFonts;
                }
                width += size + gfxFonts[fontType].header.subData.distance;
            }
        }
        else
        {
            //BMP1 font
            if (gfxFonts[fontType].header.subData.bitsPerPixel == 1)
            {
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < gfxFonts[fontType].header.subData.startChar || str[i] > gfxFonts[fontType].header.subData.endChar)
                    {
                        width += gfxFonts[fontType].header.subData.spacer + gfxFonts[fontType].header.subData.distance;
                        continue;
                    }

                    //position of raw data of current character
                    mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((str[i] - gfxFonts[fontType].header.subData.startChar) << 2)];
                    width += *(uint8_t*)&gfxFonts[fontType].dataPtr[mempos] + gfxFonts[fontType].header.subData.distance;
                }
            }
            else if (gfxFonts[fontType].header.subData.bitsPerPixel >= 2 && gfxFonts[fontType].header.subData.bitsPerPixel <= 32)
            {
                //BMP8 and RGB font
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < gfxFonts[fontType].header.subData.startChar || str[i] > gfxFonts[fontType].header.subData.endChar)
                    {
                        width += gfxFonts[fontType].header.subData.spacer + gfxFonts[fontType].header.subData.distance;
                        continue;
                    }

                    //position of raw data of current character
                    mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((str[i] - gfxFonts[fontType].header.subData.startChar) << 2)];
                    width += *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos] + *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 8] + gfxFonts[fontType].header.subData.distance;
                }
            }
        }
    }

    //animation font
    if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS) width += gfxFonts[fontType].header.subData.randomX;
    return width - gfxFonts[fontType].header.subData.distance;
}

//load font name to memory
void loadFont(const char* fname, int32_t type)
{
    //check for type range
    if (type >= GFX_MAX_FONT) messageBox(GFX_WARNING, "Error load font: %s! unknown font type!", fname);

    //read font header
    FILE* fp = fopen(fname, "rb");
    if (!fp) messageBox(GFX_WARNING, "Error load font: %s! file not found!", fname);
    fread(&gfxFonts[type].header, sizeof(GFX_FONT_HEADER), 1, fp);

    //check font signature, version number and memory size
    if (memcmp(gfxFonts[type].header.sign, "Fnt2", 4) || gfxFonts[type].header.version != 0x0101 || !gfxFonts[type].header.memSize)
    {
        fclose(fp);
        messageBox(GFX_WARNING, "Error load font: %s! wrong GFX font!", fname);
    }

    //allocate raw data buffer
    gfxFonts[type].dataPtr = (uint8_t*)calloc(gfxFonts[type].header.memSize, 1);
    if (gfxFonts[type].dataPtr)
    {
        //read raw font data
        fread(gfxFonts[type].dataPtr, gfxFonts[type].header.memSize, 1, fp);
        fclose(fp);
    }
    else
    {
        fclose(fp);
        messageBox(GFX_WARNING, "Error load font: %s! not enough memory!", fname);
    }

    //reset font header for old font
    if (gfxFonts[type].header.flags & GFX_FONT_MULTI) setFontSize(0);

    //default sub-fonts
    if (gfxFonts[type].header.flags & GFX_FONT_VECTOR) subFonts = 1;
    else subFonts = 0;

    //BMP8 font palette
    if (gfxFonts[type].header.subData.usedColors > 1)
    {
        //BMP8 use up to 128 colors (128 * 4)
        fontPalette[type] = (uint8_t*)calloc(512, 1);
        if (!fontPalette[type])
        {
            free(gfxFonts[type].dataPtr);
            messageBox(GFX_WARNING, "Error load font: %s! wrong font palette!", fname);
        }
    }
}

//release current loaded font
void freeFont(int32_t type)
{
    //check for type range
    if (type >= GFX_MAX_FONT) return;

    //free font raw data buffer
    if (gfxFonts[type].dataPtr)
    {
        free(gfxFonts[type].dataPtr);
        gfxFonts[type].dataPtr = NULL;
    }

    //free font palette
    if (fontPalette[type])
    {
        free(fontPalette[type]);
        fontPalette[type] = NULL;
    }

    //reset header
    memset(&gfxFonts[type].header, 0, sizeof(GFX_FONT_HEADER));
}

//draw a stroke of BGI font (YES we also support BGI font)
int32_t outStroke(int32_t x, int32_t y, char chr, uint32_t col, uint32_t mode)
{
    //check for font is loaded
    if (!gfxFonts[fontType].dataPtr) return 0;

    //check for non-drawable character
    if (gfxFonts[fontType].header.subData.startChar > chr || gfxFonts[fontType].header.subData.endChar < chr) return gfxFonts[fontType].header.subData.spacer;

    //memory position of character
    uint32_t mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((chr - gfxFonts[fontType].header.subData.startChar) << 2)];
    GFX_STROKE_DATA* data = (GFX_STROKE_DATA*)&gfxFonts[fontType].dataPtr[mempos];
    GFX_STROKE_INFO* stroke = (GFX_STROKE_INFO*)&gfxFonts[fontType].dataPtr[mempos + sizeof(GFX_STROKE_DATA)];

    //scan for all lines
    for (int32_t i = 0; i < data->numOfLines; i++)
    {
        const uint32_t mx = x + stroke->x * subFonts;
        const uint32_t my = y + stroke->y * subFonts;

        //check for drawable
        if (stroke->code == 1) moveTo(mx, my);
        else
        {
            //automatic antialias when in 32-bits mode
            if (bitsPerPixel == 4) lineToAlpha(mx, my, col);
            else if (mode == 2) lineToAdd(mx, my, col);
            else if (mode == 3) lineToSub(mx, my, col);
            else lineTo(mx, my, col);
        }
        stroke++;
    }

    return data->width * subFonts;
}

//draw string with current loaded font
void writeString(int32_t x, int32_t y, uint32_t col, uint32_t mode, const char* str)
{
    RGB rgb = { 0 };
    uint32_t i = 0, cx = 0, cy = 0;
    uint32_t width = 0, height = 0, addx = 0, addy = 0;
    uint32_t data = 0, datapos = 0, mempos = 0;

    //check for font is loaded
    if (!gfxFonts[fontType].dataPtr) return;

    uint32_t len = uint32_t(strlen(str));

    //check for vector font
    if (gfxFonts[fontType].header.flags & GFX_FONT_VECTOR)
    {
        for (i = 0; i < len; i++)
        {
            x += outStroke(x, y, str[i], col, mode) + gfxFonts[fontType].header.subData.distance;
            if (mode == 1) col++;
        }
        return;
    }

    //BMP1 font format
    if (gfxFonts[fontType].header.subData.bitsPerPixel == 1)
    {
        for (i = 0; i < len; i++)
        {
            //invalid character, update position
            if (uint8_t(str[i]) < gfxFonts[fontType].header.subData.startChar || uint8_t(str[i]) > gfxFonts[fontType].header.subData.endChar)
            {
                if (!(gfxFonts[fontType].header.flags & GFX_FONT_FIXED)) x += gfxFonts[fontType].header.subData.spacer + gfxFonts[fontType].header.subData.distance;
                else x += gfxFonts[fontType].header.subData.width + gfxFonts[fontType].header.subData.distance;
                continue;
            }

            //memory position for each character
            mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((uint8_t(str[i]) - gfxFonts[fontType].header.subData.startChar) << 2)];
            width = gfxFonts[fontType].dataPtr[mempos];
            height = gfxFonts[fontType].dataPtr[mempos + 1];
            mempos += 2;

            //scans for font width and height
            for (cy = 0; cy < height; cy++)
            {
                datapos = 0;
                data = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos];
                for (cx = 0; cx < width; cx++)
                {
                    if ((cx > 31) && !(cx & 31))
                    {
                        datapos += 4;
                        data = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + datapos];
                    }
                    if (data & (1 << (cx & 31)))
                    {
                        if (mode == 2) putPixelAdd(x + cx, y + cy, col);
                        else if (mode == 3) putPixelSub(x + cx, y + cy, col);
                        else putPixel(x + cx, y + cy, col);
                    }
                }
                mempos += gfxFonts[fontType].header.subData.bytesPerLine;
            }
            x += width + gfxFonts[fontType].header.subData.distance;
            if (mode == 1) col++;
        }
    }
    //BMP8 font format
    else if (gfxFonts[fontType].header.subData.bitsPerPixel >= 2 && gfxFonts[fontType].header.subData.bitsPerPixel <= 7)
    {
        //calculate font palette, use for hi-color and true-color
        if (bitsPerPixel > 8)
        {
            rgb = INT2RGB(col);
            for (i = 1; i <= gfxFonts[fontType].header.subData.usedColors; i++) *(uint32_t*)&fontPalette[fontType][i << 2] = RGB2INT(rgb.r * i / (gfxFonts[fontType].header.subData.usedColors - 1), rgb.g * i / (gfxFonts[fontType].header.subData.usedColors - 1), rgb.b * i / (gfxFonts[fontType].header.subData.usedColors - 1));
        }

        //genertate random position for animation font
        if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS)
        {
            randomBuffer(gfxBuff, len + 1, gfxFonts[fontType].header.subData.randomX);
            randomBuffer(&gfxBuff[512], len + 1, gfxFonts[fontType].header.subData.randomY);
        }

        for (i = 0; i < len; i++)
        {
            //invalid character, update character position
            if (uint8_t(str[i]) < gfxFonts[fontType].header.subData.startChar || uint8_t(str[i]) > gfxFonts[fontType].header.subData.endChar)
            {
                if (!(gfxFonts[fontType].header.flags & GFX_FONT_FIXED)) x += gfxFonts[fontType].header.subData.spacer + gfxFonts[fontType].header.subData.distance;
                else x += gfxFonts[fontType].header.subData.width + gfxFonts[fontType].header.subData.distance;
                continue;
            }

            //lookup character position
            mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((uint8_t(str[i]) - gfxFonts[fontType].header.subData.startChar) << 2)];
            addx = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos];
            addy = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 4];

            //Update position for animation font
            if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[512 + (i << 1)];
            }

            //get font width and height
            width = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 8];
            height = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            //scans font raw data
            for (cy = 0; cy < height; cy++)
            {
                for (cx = 0; cx < width; cx++)
                {
                    data = gfxFonts[fontType].dataPtr[mempos++];
                    if (!(data & 0x80))
                    {
                        if (bitsPerPixel == 8) putPixel(x + cx, y + cy + addy, data);
                        else if (mode == 2) putPixelAdd(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                        else if (mode == 3) putPixelSub(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                        else putPixel(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                    }
                }
            }

            //update next position
            if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS) x -= gfxBuff[i << 1];
            if (gfxFonts[fontType].header.flags & GFX_FONT_FIXED) x += gfxFonts[fontType].header.subData.width + gfxFonts[fontType].header.subData.distance;
            else x += width + gfxFonts[fontType].header.subData.distance;
        }
    }
    //alpha channel font
    else if (gfxFonts[fontType].header.subData.bitsPerPixel == 32)
    {
        //genertate random position for animation font
        if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS)
        {
            randomBuffer(gfxBuff, len + 1, gfxFonts[fontType].header.subData.randomX);
            randomBuffer(&gfxBuff[512], len + 1, gfxFonts[fontType].header.subData.randomY);
        }

        for (i = 0; i < len; i++)
        {
            //invalid character, update character position
            if (uint8_t(str[i]) < gfxFonts[fontType].header.subData.startChar || uint8_t(str[i]) > gfxFonts[fontType].header.subData.endChar)
            {
                if (!(gfxFonts[fontType].header.flags & GFX_FONT_FIXED)) x += gfxFonts[fontType].header.subData.spacer + gfxFonts[fontType].header.subData.distance;
                else x += gfxFonts[fontType].header.subData.width + gfxFonts[fontType].header.subData.distance;
                continue;
            }

            //lookup character position
            mempos = *(uint32_t*)&gfxFonts[fontType].dataPtr[gfxFonts[fontType].header.subData.startOffset + ((uint8_t(str[i]) - gfxFonts[fontType].header.subData.startChar) << 2)];
            addx = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos];
            addy = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 4];

            //update position for animation font
            if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[(i << 1) + 512];
            }

            //get font width and height
            width = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 8];
            height = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            //scans raw font data
            for (cy = 0; cy < height; cy++)
            {
                for (cx = 0; cx < width; cx++)
                {
                    data = *(uint32_t*)&gfxFonts[fontType].dataPtr[mempos];
                    putPixelAlpha(x + cx, y + addy + cy, data, 255);
                    mempos += 4;
                }
            }

            //update next position
            if (gfxFonts[fontType].header.flags & GFX_FONT_ANIPOS) x -= gfxBuff[i << 1];
            if (gfxFonts[fontType].header.flags & GFX_FONT_FIXED) x += gfxFonts[fontType].header.subData.width + gfxFonts[fontType].header.subData.distance;
            else x += width + gfxFonts[fontType].header.subData.distance;
        }
    }
}

//print text with format to display screen
void writeText(int32_t x, int32_t y, uint32_t txtCol, uint32_t mode, const char* format, ...)
{
    char buffer[1024] = { 0 };
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    writeString(x, y, txtCol, mode, buffer);
}

//draw muti-line string font
int32_t drawText(int32_t ypos, int32_t size, const char** str)
{
    //check for font loaded
    if (!gfxFonts[fontType].dataPtr) return 0;

    for (int32_t i = 0; i < size; i++)
    {
        if (ypos > -30) writeString(centerX - (getFontWidth(str[i]) >> 1), ypos, 62, 0, str[i]);
        ypos += getFontHeight(str[i]);
        if (ypos > cmaxY) break;
    }

    return ypos;
}

//fast show BMP image to screen
void showBMP(const char* fname)
{
    GFX_IMAGE bmp;
    loadImage(fname, &bmp);
    putImage(0, 0, &bmp);
    render();
    freeImage(&bmp);
}

//fast show PNG image to screen
void showPNG(const char* fname)
{
    GFX_IMAGE png;
    loadImage(fname, &png);

    //make caro background
    for (int32_t y = 0; y < texHeight; y++)
    {
        for (int32_t x = 0; x < texWidth; x++)
        {
            if (((x >> 4) % 2) != ((y >> 4) % 2)) putPixel(x, y, RGB_GREY191);
            else putPixel(x, y, RGB_WHITE);
        }
    }

    //render image
    putImageAlpha(0, 0, &png);
    render();
    freeImage(&png);
}

//load 8bits PNG
void loadPNG(uint8_t* raw, RGB* pal, const char* fname)
{
    if (bitsPerPixel != 8) messageBox(GFX_ERROR, "Only 8 bits screen support this function!");

    //simple image loader for all supported types
    SDL_Surface* image = IMG_Load(fname);
    if (!image)
    {
        messageBox(GFX_ERROR, "Load image error: %s", IMG_GetError());
        return;
    }

    //copy raw data and palette
    if (raw) memcpy(raw, image->pixels, intptr_t(image->pitch) * image->h);
    if (pal) memcpy(pal, image->format->palette->colors, image->format->palette->ncolors * sizeof(RGB));
    SDL_FreeSurface(image);
}

//load image as GFXLIB texture
void loadImage(const char* fname, GFX_IMAGE* im)
{
    if (bitsPerPixel != 32) messageBox(GFX_ERROR, "Only 32 bits screen support this function!");

    //simple image loader for all supported types
    SDL_Surface* image = IMG_Load(fname);
    if (!image)
    {
        messageBox(GFX_ERROR, "Load image error: %s", IMG_GetError());
        return;
    }

    //create 32bits texture to convert image format
    SDL_Surface* texture = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!texture)
    {
        messageBox(GFX_ERROR, "Cannot create surface: %s!", SDL_GetError());
        return;
    }

    //convert to target pixel format
    if (SDL_UpperBlit(image, NULL, texture, NULL)) messageBox(GFX_ERROR, "Cannot convert texture: %s!", SDL_GetError());

    //build GFX texture
    newImage(texture->w, texture->h, im);
    memcpy(im->mData, texture->pixels, im->mSize);
    SDL_FreeSurface(image);
    SDL_FreeSurface(texture);
}

//load image as 32bits texture buffer
int32_t loadTexture(uint32_t** txout, int32_t* txw, int32_t* txh, const char* fname)
{
    if (bitsPerPixel != 32)
    {
        messageBox(GFX_ERROR, "Only 32 bits screen support this function!");
        return 0;
    }

    SDL_Surface* image = IMG_Load(fname);
    if (!image)
    {
        messageBox(GFX_ERROR, "Load texture error: %s", IMG_GetError());
        return 0;
    }

    SDL_Surface* texture = SDL_CreateRGBSurface(0, image->w, image->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!texture)
    {
        messageBox(GFX_ERROR, "Cannot create surface: %s!", SDL_GetError());
        return 0;
    }

    if (SDL_UpperBlit(image, NULL, texture, NULL)) messageBox(GFX_ERROR, "Cannot convert texture: %s!", SDL_GetError());

    const uint32_t size = texture->pitch * texture->h;
    uint32_t* pixels = (uint32_t*)calloc(size, 1);

    if (!pixels)
    {
        messageBox(GFX_ERROR, "Cannot alloc memory for image data!!!");
        return 0;
    }

    memcpy(pixels, texture->pixels, size);
    *txout = pixels;
    *txw = texture->w;
    *txh = texture->h;
    SDL_FreeSurface(image);
    SDL_FreeSurface(texture);
    return 1;
}

//initalize mouse driver and bitmap mouse image
int32_t initMouseButton(GFX_MOUSE* mi)
{
    //initialize mouse image value
    mi->msPosX   = centerX;
    mi->msPosY   = centerY;
    mi->msWidth  = 0;
    mi->msHeight = 0;
    mi->msUnder  = NULL;
    mi->msBitmap = NULL;
    return 1;
}

//draw mouse cursor
void drawMouseCursor(GFX_MOUSE* mi)
{
    //check color channel
    if (bitsPerPixel != 32) return;

    int32_t mx = mi->msPosX - mi->msBitmap->mbHotX;
    int32_t my = mi->msPosY - mi->msBitmap->mbHotY;

    //check clip boundary
    if (mx < cminX) mx = cminX;
    if (mx > cmaxX) mx = cmaxX;
    if (my < cminY) my = cminY;
    if (my > cmaxY) my = cmaxY;

    const int32_t msWidth   = mi->msWidth;
    const int32_t msHeight  = mi->msHeight;
    uint32_t* msUnder = (uint32_t*)mi->msUnder;
    uint32_t* msImage = (uint32_t*)mi->msBitmap->mbData;

#ifdef _USE_ASM
    _asm {
        mov     eax, my
        mul     texWidth
        add     eax, mx
        shl     eax, 2
        mov     esi, drawBuff
        add     esi, eax
        mov     edi, msUnder
        mov     eax, texWidth
        sub     eax, msWidth
        shl     eax, 2
        push    eax
        xor     edx, edx
    next:
        xor     ecx, ecx
    step:
        //check mouse boundary
        mov     ebx, mx
        add     ebx, ecx
        cmp     ebx, cminX
        jb      skip
        cmp     ebx, cmaxX
        ja      skip
        mov     ebx, my
        add     ebx, edx
        cmp     ebx, cminY
        jb      skip
        cmp     ebx, cmaxY
        ja      skip
        //copy screen background to mouse under
        movsd
        push    esi
        push    edi
        //render mouse cursor to screen
        mov     eax, edi
        sub     eax, msUnder
        mov     edi, esi
        sub     edi, 4
        mov     esi, msImage  
        add     esi, eax
        sub     esi, 4
        //don't render color key
        lodsd
        and     eax, 00FFFFFFh
        test    eax, eax
        jz      done
        stosd
        jmp     done
    skip:
        add     esi, 4
        add     edi, 4
        jmp     cycle
    done:
        pop     edi
        pop     esi
    cycle:
        inc     ecx
        cmp     ecx, msWidth
        jb      step
        add     esi, [esp]
        inc     edx
        cmp     edx, msHeight
        jb      next
        pop     ebx
    }
#else
    //calculate starting address
    const uint32_t addOffs = texWidth - msWidth;
    uint32_t* srcPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * my + mx;

    for (int32_t i = 0; i < msHeight; i++)
    {
        for (int32_t j = 0; j < msWidth; j++)
        {
            //check for mouse boundary
            if (mx + j >= cminX && mx + j <= cmaxX && my + i >= cminY && my + i <= cmaxY)
            {
                //copy current background under cursor
                *msUnder = *srcPixels;

                //check color and copy pixel from cursor image to render buffer
                if (*msImage & 0x00FFFFFF) *srcPixels = *msImage;
            }

            //goto next pixel
            msUnder++;
            msImage++;
            srcPixels++;
        }

        //goto next render offset
        if (addOffs > 0) srcPixels += addOffs;
    }
#endif
    render();
}

//hide the mouse cursor
void clearMouseCursor(GFX_MOUSE* mi)
{
    int32_t mx = mi->msPosX - mi->msBitmap->mbHotX;
    int32_t my = mi->msPosY - mi->msBitmap->mbHotY;

    //check color channel
    if (bitsPerPixel != 32) return;

    //check clip boundary
    if (mx < cminX) mx = cminX;
    if (mx > cmaxX) mx = cmaxX;
    if (my < cminY) my = cminY;
    if (my > cmaxY) my = cmaxY;

    const int32_t msWidth = mi->msWidth;
    const int32_t msHeight = mi->msHeight;
    uint32_t* msUnder = (uint32_t*)mi->msUnder;

#ifdef _USE_ASM
    _asm {
        mov     eax, my
        mul     texWidth
        add     eax, mx
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     esi, msUnder
        mov     ebx, texWidth
        sub     ebx, msWidth
        shl     ebx, 2
        xor     edx, edx
    next:
        xor     ecx, ecx
    step:
        //check mouse boundary
        mov     eax, mx
        add     eax, ecx
        cmp     eax, cminX
        jb      skip
        cmp     eax, cmaxX
        ja      skip
        mov     eax, my
        add     eax, edx
        cmp     eax, cminY
        jb      skip
        cmp     eax, cmaxY
        ja      skip
        movsd
        jmp     done
    skip:
        add     edi, 4
        add     esi, 4
    done:
        inc     ecx
        cmp     ecx, msWidth
        jb      step
        add     edi, ebx
        inc     edx
        cmp     edx, msHeight
        jb      next
    }
#else
    //calculate starting address
    const uint32_t addOffs = texWidth - msWidth;
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * my + mx;

    for (int32_t i = 0; i < msHeight; i++)
    {
        for (int32_t j = 0; j < msWidth; j++)
        {
            //check for mouse boundary
            if (mx + j >= cminX && mx + j <= cmaxX && my + i >= cminY && my + i <= cmaxY) *dstPixels = *msUnder;
            dstPixels++;
            msUnder++;
        }

        if (addOffs > 0) dstPixels += addOffs;
    }
#endif
    render();
}

//draw bitmap button
void drawButton(GFX_BUTTON* btn)
{
    //check color channel
    if (bitsPerPixel != 32) return;

    //calculate coordinator
    const int32_t x1 = btn->btPosX;
    const int32_t y1 = btn->btPosY;
    const int32_t x2 = (x1 + btn->btWidth) - 1;
    const int32_t y2 = (y1 + btn->btHeight) - 1;

    //clip button image to context boundaries
    const int32_t lx1 = (x1 > cminX) ? x1 : cminX;
    const int32_t ly1 = (y1 > cminY) ? y1 : cminY;
    const int32_t lx2 = (x2 < cmaxX) ? x2 : cmaxX;
    const int32_t ly2 = (y2 < cmaxY) ? y2 : cmaxY;

    //validate boundaries
    if (lx1 >= lx2) return;
    if (ly1 >= ly2) return;

    //initialize loop variables
    const int32_t lbWidth = (lx2 - lx1) + 1;
    const int32_t lbHeight = (ly2 - ly1) + 1;

    //check for loop
    if (!lbWidth || !lbHeight) return;

    const int32_t btnWidth = btn->btWidth;
    void* btnData = btn->btData[btn->btState % BUTTON_STATES];

#ifdef _USE_ASM
    _asm {
        mov     edi, drawBuff
        mov     eax, ly1
        mul     texWidth
        add     eax, lx1
        shl     eax, 2
        add     edi, eax
        mov     esi, btnData
        mov     eax, ly1
        sub     eax, y1
        mul     btnWidth
        mov     ebx, lx1
        sub     ebx, x1
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, lbWidth
        shl     ebx, 2
        mov     edx, btnWidth
        sub     edx, lbWidth
        shl     edx, 2
    next:
        mov     ecx, lbWidth
    plot:
        lodsd
        and     eax, 00FFFFFFh
        test    eax, eax
        jz      skip
        stosd
        jmp     done
    skip:
        add     edi, 4
    done:
        loop    plot
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    uint32_t* dstPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * ly1 + lx1;
    uint32_t* imgPixels = (uint32_t*)btnData + btnWidth * (intptr_t(ly1) - y1) + (intptr_t(lx1) - x1);
    if (!dstPixels || !imgPixels) return;

    const uint32_t addDstOffs = texWidth - lbWidth;
    const uint32_t addImgOffs = btnWidth - lbWidth;

    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            if (*imgPixels & 0x00FFFFFF) *dstPixels = *imgPixels;
            dstPixels++;
            imgPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) imgPixels += addImgOffs;
    }
#endif
    render();
}

//load the bitmap mouse button
void loadMouseButton(const char* fname, GFX_MOUSE* mi, GFX_BITMAP* mbm, GFX_BUTTON* btn)
{
    GFX_IMAGE bmp = { 0 };
    int32_t i = 0, j = 0, y = 0;
    uint8_t *src = NULL, *dst = NULL;
    const int32_t bytesPerPixel = bitsPerPixel >> 3;

    //load mouse bitmap and animation
    loadImage(fname, &bmp);

    //allocate memory for mouse under background
    mi->msUnder = (uint8_t*)calloc(MOUSE_SIZE, bytesPerPixel);
    if (!mi->msUnder)
    {
        messageBox(GFX_ERROR, "Cannot alloc memory!");
        return;
    }

    //init mouse image width and height
    mi->msWidth = MOUSE_WIDTH;
    mi->msHeight = MOUSE_HEIGHT;

    //copy mouse cursors
    for (i = 0; i < NUM_MOUSE_BITMAPS; i++)
    {
        mbm[i].mbData = (uint8_t*)calloc(MOUSE_SIZE, bytesPerPixel);
        if (!mbm[i].mbData)
        {
            messageBox(GFX_ERROR, "Cannot alloc memory!");
            return;
        }

        mbm[i].mbHotX = 12;
        mbm[i].mbHotY = 12;
        mbm[i].mbNext = &mbm[i + 1];
        
        for (y = 0; y < MOUSE_HEIGHT; y++)
        {
            dst = &mbm[i].mbData[y * MOUSE_WIDTH * bytesPerPixel];
            src = &bmp.mData[(i * MOUSE_WIDTH + y * bmp.mWidth) * bytesPerPixel];
            memcpy(dst, src, intptr_t(bytesPerPixel) * MOUSE_WIDTH);
        }
    }

    //init current and next mouse animated
    mbm[0].mbHotX = 7;
    mbm[0].mbHotY = 2;
    mbm[0].mbNext = &mbm[0];
    mbm[8].mbNext = &mbm[1];

    //copy button bitmaps
    for (i = 0; i < NUM_BUTTONS; i++)
    {
        btn[i].btWidth = BUTTON_WIDTH;
        btn[i].btHeight = BUTTON_HEIGHT;
        for (j = 0; j < BUTTON_STATES; j++)
        {
            btn[i].btData[j] = (uint8_t*)calloc(BUTTON_SIZE, bytesPerPixel);
            if (!btn[i].btData[j])
            {
                messageBox(GFX_ERROR, "Cannot alloc memory!");
                return;
            }

            for (y = 0; y < BUTTON_HEIGHT; y++)
            {
                dst = &btn[i].btData[j][y * BUTTON_WIDTH * bytesPerPixel];
                src = &bmp.mData[(i * (bmp.mWidth >> 1) + j * BUTTON_WIDTH + (BUTTON_HEIGHT + y) * bmp.mWidth) * bytesPerPixel];
                memcpy(dst, src, intptr_t(bytesPerPixel) * BUTTON_WIDTH);
            }
        }
    }

    //init button 'click me'
    btn[0].btPosX = centerX - BUTTON_WIDTH - 20;
    btn[0].btPosY = centerY - (BUTTON_HEIGHT >> 1);
    btn[0].btState = STATE_NORM;

    //init button 'exit'
    btn[1].btPosX = centerX + BUTTON_WIDTH + 10;
    btn[1].btPosY = centerY - (BUTTON_HEIGHT >> 1);
    btn[1].btState = STATE_NORM;
}

//release mouse button
void closeMouseButton(GFX_MOUSE* mi, GFX_BITMAP* mbm, GFX_BUTTON* btn)
{
    int32_t i = 0, j = 0;

    //cleanup mouse bitmap
    for (i = 0; i < NUM_MOUSE_BITMAPS; i++)
    {
        if (mbm[i].mbData)
        {
            free(mbm[i].mbData);
            mbm[i].mbData = NULL;
        }
    }

    //cleanup button bitmap
    for (i = 0; i < NUM_BUTTONS; i++)
    {
        for (j = 0; j < BUTTON_STATES; j++)
        {
            if (btn[i].btData[j])
            {
                free(btn[i].btData[j]);
                btn[i].btData[j] = NULL;
            }
        }
    }

    //cleanup mouse underground
    clearMouseCursor(mi);
    if (mi->msUnder)
    {
        free(mi->msUnder);
        mi->msUnder = NULL;
    }
}

//automatic mouse event handler
void handleMouse(const char* fname)
{
    GFX_MOUSE mi = { 0 };
    GFX_BITMAP* msNew = NULL;
    GFX_BUTTON btn[NUM_BUTTONS] = { 0 };
    GFX_BITMAP mbm[NUM_MOUSE_BITMAPS] = { 0 };

    int32_t i = 0, done = 0;
    int32_t mcx = 0, mdx = 0, mbx = 0;
    uint32_t needDraw = 0xFFFF;
    const char* bkg[] = { "assets/1lan8.bmp", "assets/1lan16.bmp", "assets/1lan24.bmp", "assets/1lan32.bmp" };

    //init and setup bitmap mouse and button
    if (!initMouseButton(&mi)) messageBox(GFX_ERROR, "Cannot init mouse button!");
    loadMouseButton(fname, &mi, mbm, btn);
    
    //hide mouse pointer
    showMouseCursor(SDL_DISABLE);

    //install user-define mouse handler
    setMousePosition(centerX, centerY);

    //init mouse normal and wait cursor bitmap
    GFX_BITMAP* msNormal = &mbm[0];
    GFX_BITMAP* msWait = &mbm[1];

    mi.msBitmap = msNormal;

    //setup screen background
    showBMP(bkg[(bitsPerPixel >> 3) - 1]);
    drawMouseCursor(&mi);

    //update last mouse pos
    int32_t lastx = centerX;
    int32_t lasty = centerY;
    double lastTime = getTime();

    do
    {
        delay(FPS_60);

        //get current moust coordinate
        getMouseState(&mcx, &mdx, &mbx, NULL);

        //limit pointer range
        if (mcx < centerX - 100) mcx = centerX - 100;
        if (mcx > centerX + 110) mcx = centerX + 110;
        if (mdx < centerY - 100) mdx = centerY - 100;
        if (mdx > centerY + 110) mdx = centerY + 110;

        //only draw if needed
        if (needDraw)
        {
            //clear old mouse position
            clearMouseCursor(&mi);

            //draw buttons
            if (needDraw > 1)
            {
                for (i = 0; i < NUM_BUTTONS; i++)
                {
                    if (needDraw & (2 << i)) drawButton(&btn[i]);
                }
            }

            //update new button bitmap
            if (msNew) mi.msBitmap = msNew;

            //update mouse position and draw new mouse position
            mi.msPosX = mcx;
            mi.msPosY = mdx;
            drawMouseCursor(&mi);

            //update last ticks count and turn off drawable
            lastTime = getTime();
            needDraw = 0;
            msNew = NULL;
        }

        //check for draw new state button
        if (getTime() != lastTime)
        {
            if (mi.msBitmap != mi.msBitmap->mbNext)
            {
                needDraw = 1;
                mi.msBitmap = mi.msBitmap->mbNext;
            }
            else
            {
                lastTime = getTime();
            }
        }

        //update drawable when position changing
        if (lastx != mcx || lasty != mdx)
        {
            lastx = mcx;
            lasty = mdx;
            needDraw = 1;
        }

        //check for new button state
        for (i = 0; i < NUM_BUTTONS; i++)
        {
            //check if mouse inside the button region
            if (mcx >= btn[i].btPosX && mcx <= btn[i].btPosX + BUTTON_WIDTH && mdx >= btn[i].btPosY && mdx <= btn[i].btPosY + BUTTON_HEIGHT)
            {
                if (mbx == 0 && btn[i].btState == STATE_PRESSED)
                {
                    btn[i].btState = STATE_ACTIVE;
                    needDraw |= (2 << i);
                    if (i == 0)
                    {
                        if (mi.msBitmap == msNormal) msNew = msWait;
                        else msNew = msNormal;
                    }
                    else if (i == 1) done = 1;
                }
                else if (mbx == 1)
                {
                    btn[i].btState = STATE_PRESSED;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == STATE_NORM && mbx == 0)
                {
                    btn[i].btState = STATE_ACTIVE;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == STATE_WAITING)
                {
                    if (mbx == 1)
                    {
                        btn[i].btState = STATE_PRESSED;
                    }
                    else
                    {
                        btn[i].btState = STATE_ACTIVE;
                    }
                    needDraw |= (2 << i);
                }
            }
            else if (btn[i].btState == STATE_ACTIVE)
            {
                btn[i].btState = STATE_NORM;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == STATE_PRESSED && mbx == 1)
            {
                btn[i].btState = STATE_WAITING;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == STATE_WAITING && mbx == 0)
            {
                btn[i].btState = STATE_NORM;
                needDraw |= (2 << i);
            }
        }
    } while (!finished(SDL_SCANCODE_RETURN) && !done);

    //release mouse bitmap and callback handler
    closeMouseButton(&mi, mbm, btn);
    showMouseCursor(SDL_ENABLE);
}

//create texture plasma
void createPlasma(uint8_t* dx, uint8_t* dy, uint8_t* sint, uint8_t* cost, GFX_IMAGE* img)
{
#ifdef _USE_ASM
    const uint8_t lx = (*dx) += 2;
    const uint8_t ly = (*dy)--;
    
    void* data = img->mData;
    const uint32_t ofs = img->mWidth;
    const uint8_t sx = img->mWidth >> 1;
    const uint8_t sy = img->mHeight >> 1;

    _asm {
        mov     edi, data
        xor     eax, eax
        xor     dh, dh
    next0:
        xor     bh, bh
        mov     al, dh
        mov     bl, ly
        add     ax, bx
        cmp     ax, 255
        jbe     skip0
        sub     ax, 255
    skip0:
        mov     esi, sint
        add     esi, eax
        mov     cl, [esi]
        mov     al, lx
        mov     esi, sint
        add     esi, eax
        mov     ch, [esi]
        xor     dl, dl
    next1:
        xor     bh, bh
        mov     al, dl
        mov     bl, cl
        add     ax, bx
        cmp     ax, 255
        jbe     skip1
        sub     ax, 255
    skip1:
        mov     esi, sint
        add     esi, eax
        mov     bl, [esi]
        mov     al, dh
        add     al, ch
        mov     esi, cost
        add     esi, eax
        add     bl, [esi]
        shr     bl, 1
        add     bl, 128
        mov     bh, bl
        mov     esi, ofs
        mov     [edi], bx
        mov     [edi + esi], bx
        add     edi, 2
        inc     dl
        cmp     dl, sx
        jb      next1
        add     edi, esi
        inc     dh
        cmp     dh, sy
        jb      next0
    }
#else
    const uint8_t lx = (*dx) += 2;
    const uint8_t ly = (*dy)--;
    const uint16_t width = img->mWidth >> 1;
    const uint16_t height = img->mHeight >> 1;

    uint16_t ofs = 0;
    uint16_t* data = (uint16_t*)img->mData;

    for (uint16_t sy = 0; sy < height; sy++)
    {
        uint16_t val = sy + ly;
        if (val > 255) val -= 255;

        const uint8_t cl = sint[val];
        const uint8_t ch = sint[lx];

        for (uint16_t sx = 0; sx < width; sx++)
        {
            val = cl + sx;
            if (val > 255) val -= 255;
            val = (uint8_t(sint[val] + cost[uint8_t(ch + sy)]) >> 1) + 128;
            val = (val << 8) | (val & 0xFF);
            data[ofs] = val;
            data[ofs + width] = val;
            ofs++;
        }
        ofs += width;
    }
#endif
}

//initialize texture plasma
void initPlasma(uint8_t* sint, uint8_t* cost)
{
    int32_t i = 0;
    RGB pal[256] = { 0 };

    for (i = 0; i < 256; i++)
    {
        sint[i] = uint8_t(sin(2 * M_PI * i / 255.0) * 128.0 + 128.0);
        cost[i] = uint8_t(cos(2 * M_PI * i / 255.0) * 128.0 + 128.0);
    }

    for (i = 0; i < 64; i++)
    {
        pal[i      ].r = i;
        pal[i      ].g = 0;
        pal[i      ].b = i << 2;
        pal[127 - i].r = i;
        pal[127 - i].g = 0;
        pal[127 - i].b = i << 2;
        pal[127 + i].r = i << 2;
        pal[127 + i].g = i << 1;
        pal[127 + i].b = 0;
        pal[254 - i].r = i << 2;
        pal[254 - i].g = i << 1;
        pal[254 - i].b = 0;
    }

    for (i = 127; i >= 0; i--)
    {
        pal[i + 128].r = pal[i << 1].r;
        pal[i + 128].g = pal[i << 1].g;
        pal[i + 128].b = pal[i << 1].b;
    }

    setPalette(pal);
}

//round-up function
inline int32_t roundf(double x)
{
    return (x >= 0) ? int32_t(x + 0.5) : int32_t(x - 0.5);
}

//FX-effect: pre-calculate tunnel buffer
void prepareTunnel(GFX_IMAGE* dimg, uint8_t* buff1, uint8_t* buff2)
{
    const int32_t maxAng = 2048;
    const double angDec = 0.85;
    const double dstInc = 0.02;
    const double preCalc = M_PI / (maxAng >> 2);

    double z = 250.0;
    double dst = 1.0;
    double ang = maxAng - 1.0;
    
    do {
        int32_t x = roundf(z * sin(ang * preCalc)) + (dimg->mWidth >> 1);
        int32_t y = roundf(z * cos(ang * preCalc)) + (dimg->mHeight >> 1);

        ang -= angDec;
        if (ang < 0)
        {
            ang += maxAng;
            dst += dst * dstInc;
            z -= angDec;
        }

        if (x >= 0 && x < dimg->mWidth && y >= 0 && y < dimg->mHeight)
        {
            const int32_t ofs = y * dimg->mWidth + x;
            buff1[ofs] = roundf(dst);
            buff2[ofs] = roundf(dst - ang / 4);
        }
    } while (z >= 0);
}

//FX-effect: draw tunnel
void drawTunnel(GFX_IMAGE* dimg, GFX_IMAGE* simg, uint8_t* buff1, uint8_t* buff2, uint8_t* ang, uint8_t step)
{
    uint32_t nsize = dimg->mSize >> 2;
    uint32_t* dst = (uint32_t*)dimg->mData;
    uint32_t* src = (uint32_t*)simg->mData;
    
    if (bitsPerPixel != 32) return;

    *ang += step;

#ifdef _USE_ASM
    uint8_t tmp = *ang;
    _asm {
        mov     ecx, ang
        mov     edi, dst
        mov     esi, src
        mov     ebx, buff1
        mov     edx, buff2
        xor     ecx, ecx
    again:
        mov     cl, [edx]
        mov     ch, [ebx]
        add     ch, tmp
        mov     eax, [esi + ecx * 4]
        stosd
        inc     edx
        inc     ebx
        dec     nsize
        jnz     again
    }
#else
    while (nsize--)
    {
        const uint8_t val = *buff1 + *ang;
        *dst = src[(val << 8) + *buff2];
        dst++;
        buff1++;
        buff2++;
    }
#endif
}

//FX-effect: blur image buffer
void blurImageEx(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t blur)
{
    uint8_t* pdst = dst->mData;
    uint8_t* psrc = src->mData;
    const uint32_t nsize = src->mSize >> 2;

    //only support 32bit color
    if (bitsPerPixel != 32) return;

    //check for small source size
    if (blur <= 0 || nsize <= uint32_t(2 * blur)) return;

    //check for MAX blur
    if (blur > 127) blur = 127;

#ifdef _USE_ASM
    _asm {
        mov     esi, psrc
        mov     edi, pdst
        mov     ecx, nsize
        sub     ecx, blur
        sub     ecx, blur
        push    ecx
        mov     eax, 4
        xor     ebx, ebx
    nr1bx:
        xor     ecx, ecx
        push    esi
        push    edi
        push    eax
    lp1xb:
        xor     eax, eax
        mov     al, [esi]
        push    edi
        mov     edx, blur
        add     eax, edx
        mov     edi, 1
    lpi1xb:
        mov     bl, [esi + edi * 4]
        add     eax, ebx
        cmp     edi, ecx
        ja      nb1xb
        neg     edi
        mov     bl, [esi + edi * 4]
        neg     edi
        add     eax, ebx
        inc     edx
    nb1xb:
        inc     edi
        cmp     edi, blur
        jnae    lpi1xb
        pop     edi
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        inc     ecx
        cmp     ecx, blur
        jne     lp1xb
        pop     eax
        pop     edi
        pop     esi
        inc     edi
        inc     esi
        dec     eax
        jnz     nr1bx
        dec     ecx
        shl     ecx, 2
        add     esi, ecx
        add     edi, ecx
        pop     ecx
        mov     edx, blur
        add     edx, edx
        inc     edx
        mov     eax, 4
    nrxb:
        push    ecx
        push    esi
        push    edi
        push    eax
    lpxb:
        xor     eax, eax
        mov     al, [esi]
        push    ecx
        mov     ecx, blur
        add     eax, ecx
    lpixb:
        mov     bl, [esi + ecx * 4]
        neg     ecx
        add     eax, ebx
        mov     bl, [esi + ecx * 4]
        neg     ecx
        add     eax, ebx
        loop    lpixb
        pop     ecx
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        loop    lpxb
        pop     eax
        pop     edi
        pop     esi
        pop     ecx
        inc     edi
        inc     esi
        dec     eax
        jnz     nrxb
        dec     ecx
        shl     ecx, 2
        add     esi, ecx
        add     edi, ecx
        mov     eax, 4
    nr2xb:
        mov     ecx, blur
        push    esi
        push    edi
        push    eax
    lp2xb:
        xor     eax, eax
        mov     al, [esi]
        push    edi
        mov     edx, blur
        add     eax, edx
        mov     edi, 1
    lpi2xb:
        cmp     edi, ecx
        jae     nb2xb
        mov     bl, [esi + edi * 4]
        add     eax, ebx
        inc     edx
    nb2xb:
        neg     edi
        mov     bl, [esi + edi * 4]
        neg     edi
        add     eax, ebx
        inc     edi
        cmp     edi, blur
        jnae    lpi2xb
        pop     edi
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        loop    lp2xb
        pop     eax
        pop     edi
        pop     esi
        inc     edi
        inc     esi
        dec     eax
        jnz     nr2xb
    }
#else
    int32_t i = 0, j = 0, k = 0;
    int32_t ofs = 0, idx = 0;
    int32_t col1 = 0, col2 = 0;
    int32_t tsize = nsize - (blur << 1);

    for (i = 0; i < 4; i++)
    {
        ofs = idx;
        for (j = 0; j < blur; j++)
        {
            col1 = blur;
            col2 = psrc[ofs] + col1;
            for (k = 1; k < blur; k++)
            {
                col2 += psrc[ofs + (k << 2)];
                if (k <= j)
                {
                    col2 += psrc[ofs - (k << 2)];
                    col1++;
                }
            }
            pdst[ofs] = uint8_t(col2 / col1);
            ofs += 4;
        }
        idx++;
    }
    
    idx += (blur - 1) << 2;
    col1 = (blur << 1) + 1;

    for (i = 0; i < 4; i++)
    {
        ofs = idx;
        for (j = 0; j < tsize; j++)
        {
            col2 = psrc[ofs] + blur;
            for (k = blur; k > 0; k--)
            {
                col2 += psrc[ofs + (k << 2)];
                col2 += psrc[ofs - (k << 2)];
            }
            pdst[ofs] = uint8_t(col2 / col1);
            ofs += 4;
        }
        idx++;
    }
    
    tsize--;
    tsize <<= 2;
    idx += tsize;

    for (i = 0; i < 4; i++)
    {
        ofs = idx;
        for (j = blur; j > 0; j--)
        {
            col1 = blur;
            col2 = psrc[ofs] + col1;
            for (k = 1; k < blur; k++)
            {
                if (k < j)
                {
                    col2 += psrc[ofs + (k << 2)];
                    col1++;
                }
                col2 += psrc[ofs - (k << 2)];
            }
            pdst[ofs] = uint8_t(col2 / col1);
            ofs += 4;
        }
        idx++;
    }
#endif
}

//FX-effect: brightness image buffer
void brightnessImage(GFX_IMAGE* dst, GFX_IMAGE* src, uint8_t bright)
{
    uint32_t* psrc = (uint32_t*)src->mData;
    const uint32_t nsize = src->mSize >> 2;

    //only support 32bit color
    if (bitsPerPixel != 32) return;

    //check light range
    if (bright == 0 || bright == 255) return;

#ifdef _USE_ASM
    uint32_t* pdst = (uint32_t*)dst->mData;
    _asm {
        mov     ecx, nsize
        mov     edi, pdst
        mov     esi, psrc
        xor     edx, edx
        mov     dl, bright
    next:
        mov     ebx, [esi]
        mov     al, bh
        and     ebx, 00FF00FFh
        imul    ebx, edx
        shr     ebx, 8
        mul     dl
        mov     bh, ah
        mov     eax, ebx
        stosd
        add     esi, 4
        loop    next
    }
#else
    for (uint32_t i = 0; i < nsize; i++)
    {
        uint8_t* col = (uint8_t*)psrc;
        col[0] = (col[0] * bright) >> 8;
        col[1] = (col[1] * bright) >> 8;
        col[2] = (col[2] * bright) >> 8;
        psrc++;
    }
#endif
}

//FX-effect: block-out image buffer
void blockOutMid(uint32_t* dst, uint32_t* src, int32_t count, int32_t val)
{
#ifdef _USE_ASM
    _asm {
        mov     edx, count
        mov     ebx, val
        mov     edi, dst
        mov     esi, src
        mov     ecx, ebx
        shr     ecx, 17
        lea     esi, [esi + ecx * 4 + 4]
        mov     ecx, ebx
        shr     ecx, 16
        and     ebx, 00FFFFh
    next:
        mov     eax, [esi]
        sub     edx, ecx
        cmp     edx, 0
        jg      skip
        add     ecx, edx
        rep     stosd
        jmp     done
    skip:
        lea     esi, [esi + ecx * 4]
        rep     stosd
        mov     ecx, ebx
        cmp     edx, 0
        jg      next
    done:
    }
#else
    int32_t i = 0;
    int32_t mid = val >> 16;
    const int32_t blk = val & 0xFFFF;

    src += intptr_t(val >> 17) + 1;
    
    do {
        count -= mid;
        if (count <= 0)
        {
            mid += count;
            for (i = 0; i < mid; i++) *dst++ = *src;
        }
        else
        {
            for (i = 0; i < mid; i++) *dst++ = *src;
            src += mid;
            mid = blk;
        }
    } while (count > 0);
#endif
}

//FX-effect: brightness alpha buffer
void brightnessAlpha(GFX_IMAGE* img, uint8_t bright)
{
    uint32_t* data = (uint32_t*)img->mData;
    const uint32_t nsize = img->mSize >> 2;
    
    //only support 32bit color
    if (bitsPerPixel != 4) return;

    //check for minimum
    if (bright == 0 || bright == 255) return;

#ifdef _USE_ASM
    _asm {
        mov     ecx, nsize
        mov     edi, data
        xor     eax, eax
        mov     bl, bright
    next:
        mov     al, [edi + 3]
        mul     bl
        mov     [edi + 3], ah
        add     edi, 4
        loop    next
    }
#else
    for (uint32_t i = 0; i < nsize; i++)
    {
        uint8_t* pval = (uint8_t*)data;
        const uint16_t val = pval[3] * bright;
        pval[3] = val & 0xFF00;
        data++;
    }
#endif
}

//FX-effect: block-out and middle image buffer
void blockOutMidImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t xb, int32_t yb)
{
    uint32_t* pdst = (uint32_t*)dst->mData;
    uint32_t* psrc = (uint32_t*)src->mData;

    //only support 32bit color
    if (bitsPerPixel != 32) return;

    //check minimum blocking
    if (xb == 0) xb = 1;
    if (yb == 0) yb = 1;

    //nothing to do, make source and destination are the same
    if (xb == 1 && yb == 1) memcpy(pdst, psrc, src->mSize);
    else
    {
        //calculate deltax, deltay
        const int32_t cx = (((src->mWidth >> 1) % xb) << 16) | xb;
        const int32_t cy = (yb - (src->mHeight >> 1) % yb);

        //process line by line
        for (int32_t y = 0; y < src->mHeight; y++)
        {
            //blocking line by line
            if ((y + cy) % yb == 0 || y == 0)
            {
                int32_t mid = y + (cy >> 1);
                if (mid >= src->mHeight) mid = (src->mHeight + y) >> 1;
                blockOutMid(pdst, &psrc[mid * src->mWidth], src->mWidth, cx);
            }
            //already blocking, copy it
            else memcpy(pdst, pdst - dst->mWidth, dst->mRowBytes);
            pdst += dst->mWidth;
        }
    }
}

//FX-effect: fade circle image buffer
void fadeOutCircle(double pc, int32_t size, int32_t type, uint32_t col)
{
    int32_t val = 0, x = 0, y = 0;

    const int32_t dsize = size << 1;
    const int32_t smax = int32_t(size * 1.4);

    if (pc < 0.0) pc = 0.0;
    if (pc > 100.0) pc = 100.0;

    switch (type)
    {
    case 0:
        for (y = 0; y < cresY / dsize; y++)
        {
            val = int32_t(smax * pc / 100.0);
            for (x = 0; x < cresX / dsize; x++) fillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;
    case 1:
        for (y = 0; y < cresY / dsize; y++)
        {
            for (x = 0; x < cresX / dsize; x++)
            {
                val = int32_t((smax + (double(cresY) / dsize - y) * 2.0) * pc / 100.0);
                if (val > smax) val = smax;
                fillCircle(x * dsize + size, y * dsize + size, val, col);
            }
        }
        break;
    case 2:
        for (y = 0; y < cresY / dsize; y++)
        {
            for (x = 0; x < cresX / dsize; x++)
            {
                val = int32_t((smax + (double(cresX) / dsize - x) * 2.0) * pc / 100.0);
                if (val > smax) val = smax;
                fillCircle(x * dsize + size, y * dsize + size, val, col);
            }
        }
        break;
    case 3:
        for (y = 0; y < cresY / dsize; y++)
        {
            for (x = 0; x < cresX / dsize; x++)
            {
                val = int32_t((smax + (double(cresX) / size - (double(x) + y))) * pc / 100.0);
                if (val > smax) val = smax;
                fillCircle(x * dsize + size, y * dsize + size, val, col);
            }
        }
        break;
    }
}

//FX-effect: scale-up image buffer
void scaleUpLine(uint32_t* dst, uint32_t* src, int32_t* tables, int32_t count, int32_t yval)
{
#ifdef _USE_ASM
    yval <<= 2;
    _asm {
        mov     ecx, count
        mov     ebx, src
        add     ebx, yval
        mov     esi, tables
        mov     edi, dst
    next:
        lodsd
        mov     eax, [ebx + eax * 4]
        stosd
        loop    next
    }
#else
    for (int32_t i = 0; i < count; i++) *dst++ = src[tables[i] + yval];
#endif
}

//FX-effect: scale-up image buffer
void scaleUpImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t* tables, int32_t xfact, int32_t yfact)
{
    int32_t i = 0, y = 0;
    uint32_t* pdst = (uint32_t*)dst->mData;
    uint32_t* psrc = (uint32_t*)src->mData;

    //check color mode
    if (bitsPerPixel != 32) messageBox(GFX_ERROR, "Wrong pixel format!");

    //init lookup table
    for (i = 0; i < src->mWidth; i++) tables[i] = roundf(double(i) / (src->mWidth - 1.0) * ((src->mWidth - 1.0) - (intptr_t(xfact) << 1))) + xfact;

    //scaleup line by line
    for (i = 0; i < src->mHeight; i++)
    {
        y = roundf(double(i) / (src->mHeight - 1.0) * ((src->mHeight - 1.0) - (intptr_t(yfact) << 1))) + yfact;
        scaleUpLine(pdst, psrc, tables, src->mWidth, y * src->mWidth);
        pdst += dst->mWidth;
    }
}

//FX-effect: blur image buffer
void blurImage(GFX_IMAGE* img)
{
    if (bitsPerPixel != 32) messageBox(GFX_ERROR, "Wrong pixel format!");

    const uint32_t width = img->mWidth;
    const uint32_t* data = (const uint32_t*)img->mData;

#ifdef _USE_ASM
    uint32_t height = img->mHeight;
    _asm {
        mov     edi, data
    step:
        mov     edx, width
        sub     edx, 2
        push    edx
        mov     edx, width
        shl     edx, 2
        mov     ebx, [edi]
        mov     esi, [edi + 4]
        and     ebx, 00FF00FFh
        and     esi, 00FF00FFh
        add     ebx, ebx
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00FF00FFh
        add     esi, ecx
        mov     al, [edi + 5]
        mov     bl, [edi + 1]
        add     ebx, ebx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        xor     ebx, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00FF00FFh
        shl     eax, 6
        and     eax, 0000FF00h
        or      eax, esi
        stosd
    next:
        mov     esi, [edi - 4]
        mov     ecx, [edi + 4]
        and     esi, 00FF00FFh
        and     ecx, 00FF00FFh
        mov     ebx, [edi]
        add     esi, ecx
        and     ebx, 00FF00FFh
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00FF00FFh
        xor     eax, eax
        mov     al, [edi - 3]
        add     esi, ecx
        mov     cl, [edi + 5]
        mov     bl, [edi + 1]
        add     eax, ecx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00FF00FFh
        shl     eax, 6
        and     eax, 0000FF00h
        or      eax, esi
        stosd
        dec     dword ptr[esp]
        jnz     next
        mov     ebx, [edi]
        mov     esi, [edi - 4]
        and     ebx, 00FF00FFh
        and     esi, 00FF00FFh
        add     ebx, ebx
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00FF00FFh
        add     esi, ecx
        mov     al, [edi - 3]
        mov     bl, [edi - 4]
        add     ebx, ebx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        xor     ebx, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00FF00FFh
        shl     eax, 6
        and     eax, 0000FF00h
        or      eax, esi
        stosd
        pop     edx
        dec     height
        jnz     step
    }
#else
    const uint32_t offset = (img->mSize >> 2) - (width << 1);
    for (uint32_t i = width; i < offset; i++)
    {
        uint8_t* col0 = (uint8_t*)&data[i];
        uint8_t* col1 = (uint8_t*)&data[i - 1];
        uint8_t* col2 = (uint8_t*)&data[i + 1];
        uint8_t* col3 = (uint8_t*)&data[i - width];
        uint8_t* col4 = (uint8_t*)&data[i + width];
        col0[0] = (col1[0] + col2[0] + col3[0] + col4[0]) >> 2;
        col0[1] = (col1[1] + col2[1] + col3[1] + col4[1]) >> 2;
        col0[2] = (col1[2] + col2[2] + col3[2] + col4[2]) >> 2;
    }
#endif
}

//FX-effect: blending image buffer
void blendImage(GFX_IMAGE* dst, GFX_IMAGE* src1, GFX_IMAGE* src2, int32_t cover)
{
    uint8_t* psrc1 = src1->mData;
    uint8_t* psrc2 = src2->mData;
    uint8_t* pdst = dst->mData;
    const uint32_t count = src1->mSize >> 2;

    if (bitsPerPixel != 32) messageBox(GFX_ERROR, "Wrong pixel format!");

#ifdef _USE_ASM
    _asm {
        mov     edi, pdst
        mov     esi, psrc2
        mov     ecx, cover
        mov     edx, psrc1
        neg     cl
    next:
        push    edx
        mov     ebx, [esi]
        mov     edx, [edx]
        mov     al, dh
        and     edx, 00FF00FFh
        mov     ah, bh
        and     ebx, 00FF00FFh
        sub     edx, ebx
        imul    edx, ecx
        shr     edx, 8
        add     ebx, edx
        xor     edx, edx
        mov     dl, ah
        xor     ah, ah
        mov     bh, dl
        sub     ax, dx
        mul     cx
        add     bh, ah
        pop     edx
        mov     [edi], ebx
        add     esi, 4
        add     edx, 4
        add     edi, 4
        dec     count
        jnz     next
    }
#else
    for (uint32_t i = 0; i < count; i++)
    {
        pdst[0] = (cover * psrc1[0] + psrc2[0] * (255 - cover)) >> 8;
        pdst[1] = (cover * psrc1[1] + psrc2[1] * (255 - cover)) >> 8;
        pdst[2] = (cover * psrc1[2] + psrc2[2] * (255 - cover)) >> 8;
        pdst  += 4;
        psrc1 += 4;
        psrc2 += 4;
    }
#endif
}

//FX-effect: rotate image buffer, line by line
void rotateLine(uint32_t* dst, uint32_t* src, int32_t* tables, int32_t width, int32_t siny, int32_t cosy)
{
#ifdef _USE_ASM
    const int32_t pos = (width + 1) << 3;
    _asm {
        mov     ecx, width
        dec     ecx
        mov     esi, src
        mov     edi, dst
        mov     ebx, tables
    next:
        mov     eax, ecx
        shl     eax, 3
        mov     edx, eax
        add     eax, 8
        add     edx, 12
        mov     eax, [ebx + eax]
        mov     edx, [ebx + edx]
        add     eax, cosy
        sub     edx, siny
        sar     eax, 1
        js      skip
        sar     edx, 1
        js      skip
        cmp     eax, [ebx + 4]
        jnl     skip
        cmp     edx, [ebx]
        jnl     skip
        shl     eax, 2
        add     eax, pos
        mov     eax, [ebx + eax]
        shl     edx, 2
        add     edx, eax
        mov     eax, [esi + edx]
        mov     [edi], eax
    skip:
        add     edi, 4
        dec     ecx
        jns     next
    }
#else
    int32_t idx = 0;
    const int32_t mx = tables[0];
    const int32_t my = tables[1];
    const int32_t pos = (width + 1) << 1;

    for (int32_t i = width - 1; i > 0; i--)
    {
        const int32_t ofs = (i + 1) << 1;
        const int32_t y = (tables[ofs    ] + cosy) >> 1;
        const int32_t x = (tables[ofs + 1] - siny) >> 1;
        if (y > 0 && x > 0 && y < my && x < mx)
        {
            const int32_t t = tables[y + pos];
            dst[idx] = src[x + (t >> 2)];
        }
        idx++;
    }
#endif
}

//FX-effect: rotate image buffer
void rotateImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t* tables, int32_t axisx, int32_t axisy, double angle, double scale)
{
    int32_t x = 0, y = 0, lineWidth = 0;
    uint32_t* psrc = (uint32_t*)src->mData;
    uint32_t* pdst = (uint32_t*)dst->mData;

    if (bitsPerPixel != 32) messageBox(GFX_ERROR, "Wrong pixel format!");

    //recalculate axisx, axisy
    axisx = dst->mWidth - axisx;
    axisy = dst->mHeight - axisy;

    //store source image width, height
    tables[0] = src->mWidth;
    tables[1] = src->mHeight;

    //calculate rotation data
    const double th = M_PI * (180 - angle) / 180.0;
    double sint = sin(th) / scale;
    double cost = cos(th) / scale;

    double primex = (-axisx << 1) + 1;
    double sinx = primex * sint - 1;
    double cosx = primex * cost - 1 + src->mWidth;

    sint *= 2;
    cost *= 2;

    //init lookup tables
    for (x = 0; x < dst->mWidth; x++)
    {
        tables[(x << 1) + 2] = int32_t(sinx);
        tables[(x << 1) + 3] = int32_t(cosx);
        sinx += sint;
        cosx += cost;
    }

    sint /= 2;
    cost /= 2;

    for (y = 0; y < src->mHeight; y++)
    {
        tables[y + ((src->mWidth + 1) << 1)] = lineWidth;
        lineWidth += src->mRowBytes;
    }

    double primey = ((dst->mHeight - 1 - axisy) << 1) + 1;
    double siny = primey * sint;
    double cosy = primey * cost + src->mHeight;

    sint *= 2;
    cost *= 2;

    //process rotate line by line
    for (y = 0; y < dst->mHeight; y++)
    {
        rotateLine(pdst, psrc, tables, dst->mWidth, int32_t(siny), int32_t(cosy));
        pdst += dst->mWidth;
        siny -= sint;
        cosy -= cost;
    }
}

//FX-effect: 2d bumping
void bumpImage(GFX_IMAGE* dst, GFX_IMAGE* src1, GFX_IMAGE* src2, int32_t lx, int32_t ly)
{
    uint32_t* dstdata = (uint32_t*)dst->mData;
    uint32_t* src1data = (uint32_t*)src1->mData;
    uint32_t* src2data = (uint32_t*)src2->mData;

    const int32_t src1width = src1->mWidth;
    const int32_t src2width = src2->mWidth;
    const int32_t dstwidth = dst->mWidth;
    const int32_t src1len = src1->mRowBytes - 1;

    const int32_t bmax = 260;
    int32_t nx = 0, ny = 0, vlx = 0, vly = 0;
    int32_t x = 0, y = 0, osrc2 = 0, osrc1 = 0, odst = 0;
    
#ifdef _USE_ASM
    _asm {
        mov     y, 100
    starty:
        mov     ebx, y
        mov     eax, src1width
        mul     ebx
        add     eax, 99
        shl     eax, 2
        mov     osrc1, eax
        mov     eax, src2width
        mul     ebx
        add     eax, 99
        shl     eax, 2
        mov     osrc2, eax
        mov     eax, dstwidth
        mul     ebx
        add     eax, 99
        shl     eax, 2
        mov     odst, eax
        mov     x, 100
    startx:
        mov     eax, x
        sub     eax, lx
        mov     vlx, eax
        mov     eax, y
        sub     eax, ly
        mov     vly, eax
        mov     ecx, vlx
        mov     eax, vly
        mov     ebx, bmax
        neg     ebx
        cmp     ecx, bmax
        jnl     stop
        cmp     eax, bmax
        jnl     stop
        cmp     ecx, ebx
        jng     stop
        cmp     eax, ebx
        jng     stop
        xor     eax, eax
        xor     ebx, ebx
        mov     edi, src1data
        add     edi, osrc1
        mov     al, [edi + 1]
        or      al, al
        jz      stop
        mov     bl, [edi - 1]
        sub     eax, ebx
        mov     nx, eax
        mov     ecx, src1len
        mov     al, [edi + ecx]
        sub     edi, ecx
        mov     bl, [edi]
        sub     eax, ebx
        mov     ny, eax
        mov     eax, vlx
        sub     eax, nx
        jns     nsx
        neg     eax
    nsx:
        shr     eax, 1
        cmp     eax, 127
        jna     nax
        mov     eax, 127
    nax:
        mov     ebx, 127
        sub     ebx, eax
        jns     nsx2
        mov     ebx, 1
    nsx2:
        mov     eax, vly
        sub     eax, ny
        jns     nsy
        neg     eax
    nsy:
        shr     eax, 1
        cmp     eax, 127
        jna     nay
        mov     eax, 127
    nay:
        mov     ecx, 127
        sub     ecx, eax
        jns     nsy2
        mov     ecx, 1
    nsy2:
        add     ebx, ecx
        cmp     ebx, 128
        jna     stop
        sub     ebx, 128
        mov     edi, src2data
        add     edi, osrc2
        mov     ecx, [edi]
        mov     edi, dstdata
        add     edi, odst
        xor     eax, eax
        mov     al, cl
        mul     ebx
        shr     eax, 5
        cmp     eax, 255
        jna     nextb
        mov     eax, 255
    nextb:
        mov     [edi], al
        mov     al, ch
        mul     ebx
        shr     eax, 5
        cmp     eax, 255
        jna     nextg
        mov     eax, 255
    nextg:
        mov     [edi + 1], al
        shr     ecx, 16
        mov     al, cl
        mul     ebx
        shr     eax, 5
        cmp     eax, 255
        jna     nextr
        mov     eax, 255
    nextr:
        mov     [edi + 2], al
    stop:
        add     osrc2, 4
        add     osrc1, 4
        add     odst, 4
        inc     x
        mov     eax, x
        cmp     eax, 700
        jna     startx
        inc     y
        mov     eax, y
        cmp     eax, 500
        jna     starty
    }
#else
    //scan for image height
    for (y = 100; y <= 500; y++)
    {
        //calculate starting offset
        osrc1 = (src1width * y + 99);
        osrc2 = (src2width * y + 99);
        odst = (dstwidth * y + 99);

        //scan for image width
        for (x = 100; x <= 700; x++)
        {
            //calculate delta x,y
            vlx = x - lx;
            vly = y - ly;

            //range checking
            if (vlx > -bmax && vlx < bmax && vly > -bmax && vly < bmax)
            {
                nx = uint8_t(src1data[osrc1 + 1]) - uint8_t(src1data[osrc1 - 1]);
                ny = uint8_t(src1data[osrc1 + src1len]) - uint8_t(src1data[osrc1 - src1len]);
                int32_t difx = 127 - min(abs(vlx - nx) >> 1, 127);
                if (difx <= 0) difx = 1;
                int32_t dify = 127 - min(abs(vly - ny) >> 1, 127);
                if (dify <= 0) dify = 1;
                uint8_t col = difx + dify;
                if (col > 128)
                {
                    col -= 128;
                    uint8_t* pdst = (uint8_t*)&dstdata[odst];
                    uint8_t* psrc = (uint8_t*)&src2data[osrc2];
                    pdst[0] = min((col * psrc[0]) >> 5, 255);
                    pdst[1] = min((col * psrc[1]) >> 5, 255);
                    pdst[2] = min((col * psrc[2]) >> 5, 255);
                }
            }

            //next pixel
            osrc1++;
            osrc2++;
            odst++;
        }
    }
#endif
}

//FX-effect: fade-out image buffer
void fadeOutImage(GFX_IMAGE* img, uint8_t step)
{
    if (bitsPerPixel != 32) return;

    uint8_t* pixels = img->mData;
    const uint32_t size = img->mSize >> 2;

    for (uint32_t i = 0; i < size; i++)
    {
        pixels[0] = max(pixels[0] - step, 0);
        pixels[1] = max(pixels[1] - step, 0);
        pixels[2] = max(pixels[2] - step, 0);
        pixels += 4;
    }
}

//get total system momory in MB
void initMemoryInfo()
{
#ifdef __APPLE__
    size_t len = 0;
    uint64_t physMem = 0, memSize = 0;
    len = sizeof(memSize);
    if (!sysctlbyname("hw.memsize", &memSize, &len, NULL, 0)) totalMemory = uint32_t(memSize >> 20);
    len = sizeof(physMem);
    if (!sysctlbyname("hw.physmem", &physMem, &len, NULL, 0)) availableMemory = uint32_t(physMem >> 20);
#else
    MEMORYSTATUSEX statex = { 0 };
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        totalMemory = uint32_t(statex.ullTotalPhys >> 20);
        availableMemory = uint32_t(statex.ullAvailPhys >> 20);
    }
#endif
}

//get RDTSC count
uint64_t getCyclesCount()
{
#ifdef _USE_ASM
    uint64_t count = 0;
    _asm {
        cpuid
        rdtsc
        mov dword ptr[count    ], eax
        mov dword ptr[count + 4], edx
    }
    return count;
#else
    return __rdtsc();
#endif
}

//get current CPU clock rate in MHz
void calcCpuSpeed()
{
    const uint64_t start = getCyclesCount();
    delay(50);
    const uint64_t stop = getCyclesCount();
    const uint64_t speed = (stop - start) / 50000;
    cpuSpeed = uint32_t(speed);
}

//CPUID instruction wrapper
void CPUID(int32_t* cpuinfo, uint32_t funcid)
{
#ifdef _USE_ASM
    _asm {
        mov    eax, funcid
        mov    edi, cpuinfo
        cpuid
        mov[edi     ], eax
        mov[edi +  4], ebx
        mov[edi +  8], ecx
        mov[edi + 12], edx
    }
#elif defined(__APPLE__)
    __cpuid(funcid, cpuinfo[0], cpuinfo[1], cpuinfo[2], cpuinfo[3]);
#else
    __cpuid(cpuinfo, funcid);
#endif
}

//return CPU type (INTEL, AMD, ...)
void calcCpuType()
{
    int32_t i = 0;
    int32_t cpuInfo[4] = { 0 };

    memset(cpuType, 0, sizeof(cpuType));
    CPUID(cpuInfo, 0x00000000);

    cpuType[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
    cpuType[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
    cpuType[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
    cpuType[i++] = cpuInfo[1] & 0xff;
    cpuType[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
    cpuType[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
    cpuType[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
    cpuType[i++] = cpuInfo[3] & 0xff;
    cpuType[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
    cpuType[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
    cpuType[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
    cpuType[i++] = cpuInfo[2] & 0xff;
    if (!cpuType[0]) strcpy(cpuType, "Unknown");
}

//return full CPU description string (i.e: Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz)
void calcCpuName()
{
    int32_t i = 0;
    int32_t cpuInfo[4] = { 0 };
    
    memset(cpuName, 0, sizeof(cpuName));
    CPUID(cpuInfo, 0x80000000);

    if (cpuInfo[0] >= 0x80000004)
    {
        CPUID(cpuInfo, 0x80000002);
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        CPUID(cpuInfo, 0x80000003);
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        CPUID(cpuInfo, 0x80000004);
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[0] & 0xff; cpuInfo[0] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[1] & 0xff; cpuInfo[1] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[2] & 0xff; cpuInfo[2] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
        cpuName[i++] = cpuInfo[3] & 0xff; cpuInfo[3] >>= 8;
    }
    if (!cpuName[0]) strcpy(cpuName, "Unknown");
}

//AMD 3DNow! detected
int32_t have3DNow()
{
    int32_t cpuInfo[4] = { 0 };
    CPUID(cpuInfo, 0x80000000);
    if (cpuInfo[0] >= 0x80000001)
    {
        CPUID(cpuInfo, 0x80000001);
        return cpuInfo[3] & 0x80000000;
    }
    return 0;
}

//calculate some CPU features...
void calcCpuFeatures()
{
    int32_t cpuInfo[4] = { 0 };
    CPUID(cpuInfo, 0);
    if (cpuInfo[0] >= 1) CPUID(cpuInfo, 1);
    
    memset(cpuFeatures, 0, sizeof(cpuFeatures));
    if (have3DNow()) strcat(cpuFeatures, "3DNow!,");
    if (cpuInfo[3] & 0x00800000) strcat(cpuFeatures, "MMX,");
    if (cpuInfo[3] & 0x02000000) strcat(cpuFeatures, "SSE,");
    if (cpuInfo[3] & 0x04000000) strcat(cpuFeatures, "SSE2,");
    if (cpuInfo[2] & 0x00000001) strcat(cpuFeatures, "SSE3,");
    size_t len = strlen(cpuFeatures);
    if (len > 1) cpuFeatures[len - 1] = '\0';
}

//initialize CPU info
void initCpuInfo()
{
    calcCpuSpeed();
    calcCpuType();
    calcCpuName();
    calcCpuFeatures();
}

//initialize video info
void initVideoInfo()
{
    videoMemory = 0;
    strcpy(videoName, "Unknown");
    strcpy(driverVersion, "0.0.0.0");
    strcpy(renderVersion, "0.0.0.0");
    strcpy(imageVersion, "0.0.0.0");

    //retrive SDL and SDL_image version string
    SDL_version sdl;
    SDL_GetVersion(&sdl);
    const SDL_version* img = IMG_Linked_Version();
    sprintf(renderVersion, "SDL %u.%u.%u", sdl.major, sdl.minor, sdl.patch);
    sprintf(imageVersion, "SDL_image %u.%u.%u", img->major, img->minor, img->patch);
    
#ifdef __APPLE__
    io_iterator_t iterator;
    CFMutableDictionaryRef matchDict;

    //get dictionary of all the PCI Devices
    matchDict = IOServiceMatching("IOPCIDevice");
    
    //get IOService descriptor
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iterator) == kIOReturnSuccess)
    {
        //iterator for devices found
        io_registry_entry_t regEntry;
        
        while ((regEntry = IOIteratorNext(iterator)))
        {
            //put this services object into a dictionary object.
            CFMutableDictionaryRef serviceDictionary;
            if (IORegistryEntryCreateCFProperties(regEntry, &serviceDictionary, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
            {
                //service dictionary creation failed.
                IOObjectRelease(regEntry);
                continue;
            }
            
            //get graphic card model
            CFDataRef GPUModel = (CFDataRef)CFDictionaryGetValue(serviceDictionary, CFSTR("model"));
            if (GPUModel == NULL) continue;

            //get graphic card name
            CFIndex length = CFDataGetLength(GPUModel);
            memcpy(videoName, CFDataGetBytePtr(GPUModel), length);
            
            //retrived driver version???
            CFDataRef devNum = (CFDataRef)CFDictionaryGetValue(serviceDictionary, CFSTR("device-id"));
            CFDataRef revNum = (CFDataRef)CFDictionaryGetValue(serviceDictionary, CFSTR("revision-id"));
            if (revNum != NULL && devNum != NULL)
            {
                uint32_t deviceId = 0, revisionId = 0;
                length = CFDataGetLength(devNum);
                memcpy(&deviceId, CFDataGetBytePtr(devNum), length);
                length = CFDataGetLength(revNum);
                memcpy(&revisionId, CFDataGetBytePtr(revNum), length);
                sprintf(driverVersion, "%u.%u.%u.%u", HIWORD(deviceId), LOWORD(deviceId), HIWORD(revisionId), LOWORD(revisionId));
            }
            
            // Release the dictionary
            CFRelease(serviceDictionary);
            
            // Release the serviceObject
            IOObjectRelease(regEntry);
        }
        
        // Release the iterator
        IOObjectRelease(iterator);
    }
    
    //get graphic card memory
    matchDict = IOServiceMatching(kIOAcceleratorClassName);
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchDict, &iterator) == kIOReturnSuccess)
    {
        //iterator for devices found
        io_registry_entry_t regEntry;
        
        while ((regEntry = IOIteratorNext(iterator)))
        {
            //put this services object into a dictionary object.
            CFMutableDictionaryRef serviceDictionary;
            if (IORegistryEntryCreateCFProperties(regEntry, &serviceDictionary, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
            {
                //service dictionary creation failed.
                IOObjectRelease(regEntry);
                continue;
            }
            
            //catch video memory key
            CFNumberRef vram = (CFNumberRef)CFDictionaryGetValue(serviceDictionary, CFSTR("VRAM,totalMB"));
            if (vram == NULL) continue;
            
            //get total video memory
            CFNumberGetValue(vram, kCFNumberSInt32Type, &videoMemory);
            
            //release the dictionary
            CFRelease(serviceDictionary);
            
            //release the serviceObject
            IOObjectRelease(regEntry);
        }
        
        //release the iterator
        IOObjectRelease(iterator);
    }
#else
    //retrive video memory of graphic card name
    LUID luid = { 0 };
    IDXGIFactory* factory = NULL;
    if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
    {
        IDXGIAdapter* adapter = NULL;
        if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
        {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);
            adapter->Release();
            luid = desc.AdapterLuid;
            videoMemory = uint32_t(desc.DedicatedVideoMemory >> 20);
            wcstombs(videoName, desc.Description, 128);
        }
        factory->Release();
    }

    //retrive graphic card driver version by fetch registry data
    HKEY dxKeyHandle = NULL;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\DirectX"), 0, KEY_READ | KEY_WOW64_64KEY, &dxKeyHandle) != ERROR_SUCCESS) return;

    //find all subkeys
    DWORD numOfAdapters = 0;
    DWORD subKeyMaxLength = 0;
    if (RegQueryInfoKey(dxKeyHandle, NULL, NULL, NULL, &numOfAdapters, &subKeyMaxLength, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    {
        RegCloseKey(dxKeyHandle);
        return;
    }

    //must include the null character
    subKeyMaxLength += 1;

    LARGE_INTEGER driverVersionRaw = { 0 };
    bool foundVersion = false;
    TCHAR* subKeyName = (TCHAR*)calloc(subKeyMaxLength, sizeof(TCHAR));

    for (DWORD i = 0; i < numOfAdapters; ++i)
    {
        DWORD subKeyLength = subKeyMaxLength;
        if (RegEnumKeyEx(dxKeyHandle, i, subKeyName, &subKeyLength, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            LUID adapterLUID = {};
            DWORD qwordSize = sizeof(uint64_t);

            //find match the vendor ID and device ID
            if (RegGetValue(dxKeyHandle, subKeyName, _T("AdapterLuid"), RRF_RT_QWORD, NULL, &adapterLUID, &qwordSize) == ERROR_SUCCESS && adapterLUID.HighPart == luid.HighPart && adapterLUID.LowPart == luid.LowPart)
            {
                //let's get the driver version num now
                if (RegGetValue(dxKeyHandle, subKeyName, _T("DriverVersion"), RRF_RT_QWORD, NULL, &driverVersionRaw, &qwordSize) == ERROR_SUCCESS)
                {
                    foundVersion = true;
                    break;
                }
            }
        }
    }

    RegCloseKey(dxKeyHandle);
    free(subKeyName);
    if (!foundVersion) return;
    sprintf(driverVersion, "%u.%u.%u.%u", HIWORD(driverVersionRaw.HighPart), LOWORD(driverVersionRaw.HighPart), HIWORD(driverVersionRaw.LowPart), LOWORD(driverVersionRaw.LowPart));
#endif
}

//initialize some system info
void initSystemInfo()
{
    SDL_DisplayMode mode;
    SDL_GetWindowDisplayMode(sdlWindow, &mode);
    sprintf(modeInfo, "%dx%dx%ub @ %dHz", mode.w, mode.h, SDL_BYTESPERPIXEL(mode.format) << 3, mode.refresh_rate);
    initCpuInfo();
    initVideoInfo();
    initMemoryInfo();
}

//return current video memory in GB
uint32_t getVideoMemory()
{
    return videoMemory;
}

//return total system memory in GB
uint32_t getTotalMemory()
{
    return totalMemory;
}

//return free memory in GB
uint32_t getAvailableMemory()
{
    return availableMemory;
}

//return name of graphic card
const char* getVideoName()
{
    return videoName;
}

//return current CPU speed in MHZ
uint32_t getCpuSpeed()
{
    return cpuSpeed;
}

//return current CPU type (INTEL, AMD)
const char* getCpuType()
{
    return cpuType;
}

//return full string of CPU name
const char* getCpuName()
{
    return cpuName;
}

//return CPU features (MMX, 3DNow!, SSE, SSE2, SSE3, ...)
const char* getCpuFeatures()
{
    return cpuFeatures;
}

//return version of render system
const char* getRenderVersion()
{
    return renderVersion;
}

//return version of image file loading
const char* getImageVersion()
{
    return imageVersion;
}

//return render driver version
const char* getDriverVersion()
{
    return driverVersion;
}

//return current video mode info (resolution-bitsperpixels-refreshrate)
const char* getVideoModeInfo()
{
    return modeInfo;
}
