/*===============================================================*/
/*                 GFXLIB Graphics Library                       */
/*               Use SDL3 for backend render                     */
/*               SDL3_image for backend image                    */
/*            Target OS: cross-platform (win32, macos)           */
/*               Author: Nguyen Ngoc Van                         */
/*               Create: 22/10/2018                              */
/*              Version: 1.4.2                                   */
/*          Last Update: 2024-08-08                              */
/*              Website: http://codedemo.net                     */
/*                Email: pherosiden@gmail.com                    */
/*           References: https://lodev.org                       */
/*                       https://permadi.com                     */
/*                       https://sources.ru                      */
/*                       http://eyecandyarchive.com              */
/*                       https://crossfire-designs.de            */
/*              License: GNU GPL                                 */
/*===============================================================*/

#include <map>
#include "gfxlib.h"
#ifdef SDL_PLATFORM_APPLE
#include <cpuid.h>
#include <sys/sysctl.h>
#else
#include <dxgi.h>
#include <tchar.h>
#pragma comment (lib, "DXGI")
#pragma comment (lib, "SDL3")
#pragma comment (lib, "SDL3_image")
#endif

//drawing buffer
void*           drawBuff = NULL;                    //current render buffer
int32_t         texWidth = 0, texHeight = 0;        //current draw buffer height

//save current buffer
void*           oldBuffer = NULL;                   //saved render buffer
int32_t         oldWidth = 0, oldHeight = 0;        //saved buffer height

//pixels attributes
int32_t         bitsPerPixel = 0;                   //bits per pixel (8/15/16/24/32)
int32_t         bytesPerPixel = 0;                  //bytes per pixel (1/2/3/4)
int32_t         bytesPerScanline = 0;               //bytes per scan line

//mid-screen coordinate
int32_t         centerX = 0, centerY = 0;           //x, y center of screen

//current screen view-port
int32_t         cminX = 0, cminY = 0;               //current left-top
int32_t         cmaxX = 0, cmaxY = 0;               //current right-bottom

//saved screen view-port
int32_t         oldMinX = 0, oldMinY = 0;           //saved left-top
int32_t         oldMaxX = 0, oldMaxY = 0;           //saved right-bottom

//current draw cursor (2D)
int32_t         currX = 0, currY = 0;               //current cursor x, y

//3D projection
double          DE = 0.0;                           //deplane end for projection

//trigonometric angle
double          RHO = 0.0;                          //RHO perspective projection

//saved transform values
double          sinth = 0.0, sinph = 0.0;           //sin(theta), sin(phi)
double          costh = 0.0, cosph = 0.0;           //cos(theta), cos(phi)
double          sincosx = 0.0, sinsinx = 0.0;       //sinsin(theta, phi)
double          coscosx = 0.0, sincosy = 0.0;       //coscos(theta, phi)

//current draw cursor (3D)
int32_t         cranX = 0, cranY = 0;               //current x, y cursor (3d mode)

//3D projection type
uint8_t         projectionType = 0;                 //current projection type

//GFX font data
GFX_FONT        gfxFonts[GFX_MAX_FONT] = { 0 };     //GFX font loadable at the same time
uint8_t*        fontPalette[GFX_MAX_FONT] = { 0 };  //GFX font palette data (BMP8 type)
uint8_t*        gfxBuff = NULL;                     //GFX buffer
uint32_t        subFonts = 0;                       //GFX sub-fonts
uint32_t        fontType = 0;                       //current selected font (use for multiple loaded font)
uint32_t        randSeed = 0;                       //global random seed
uint32_t        factor = 0x8088405;                 //global factor

//pattern filled styles
uint8_t         ptnLine[]           = { 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 };
uint8_t         ptnLiteSlash[]      = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
uint8_t         ptnSlash[]          = { 0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0, 0xC1, 0x83 };
uint8_t         ptnBackSlash[]      = { 0x07, 0x83, 0xC1, 0xE0, 0x70, 0x38, 0x1C, 0x0E };
uint8_t         ptnLiteBackSlash[]  = { 0x5A, 0x2D, 0x96, 0x4B, 0xA5, 0xD2, 0x69, 0xB4 };
uint8_t         ptnHatch[]          = { 0xff, 0x88, 0x88, 0x88, 0xff, 0x88, 0x88, 0x88 };
uint8_t         ptnHatchX[]         = { 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24, 0x18 };
uint8_t         ptnInterLeave[]     = { 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33 };
uint8_t         ptnWideDot[]        = { 0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00 };
uint8_t         ptnCloseDot[]       = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };

//CPU and video card parameters
uint32_t        cpuSpeed = 0;                       //CPU speed in MHz
char            cpuName[50] = { 0 };                //full CPU name string
char            cpuType[16] = { 0 };                //CPU type (GenuineIntel, AuthenticAMD, ...)
char            cpuFeatures[50] = { 0 };            //CPU features (MMX, 3DNow!, 3DNowExt!, SSE, SSE2, SSE3, ...)
char            videoName[128] = { 0 };             //full name of graphic card
char            driverVersion[32] = { 0 };          //graphic driver version string
char            renderVersion[32] = { 0 };          //SDL2 version string
char            imageVersion[32] = { 0 };           //SDL2_image version string
char            modeInfo[32] = { 0 };               //current display mode info string

//system memory profiles
uint32_t        totalMemory = 0;                    //total physical memory in MB
uint32_t        availableMemory = 0;                //available physical memory in MB
uint32_t        videoMemory = 0;                    //total video memory in MB

//global SDL objects
SDL_Window*     sdlWindow = NULL;                   //display windows
SDL_Surface*    sdlSurface = NULL;                  //display windows surface
SDL_Surface*    sdlScreen = NULL;                   //convert screen surface
SDL_Texture*    sdlTexture = NULL;                  //texture steaming
SDL_Renderer*   sdlRenderer = NULL;                 //render object
SDL_Event		sdlEvent = { 0 };                   //store key input event

//keyboard status
uint8_t*        keyStates = 0;                      //key input states

//the "keyPressed" status map
std::map<int32_t, int32_t> keyStatus;               //key input status

//default 8-bits palette entries for mixed mode, SDL3 initialized with black palette
SDL_Color basePalette[256] = {
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

//current input data
int32_t dataX = 0, dataY = 0;

//get current input data x
int32_t getDataX()
{
    return dataX;
}

//get current input data y
int32_t getDataY()
{
    return dataY;
}

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

//read input from user
int32_t waitUserInput(int32_t inputMask /* = INPUT_KEY_PRESSED */)
{
    SDL_Event event = { 0 };

    while (1)
    {
        if (SDL_WaitEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                quit();
                break;

            case SDL_EVENT_KEY_DOWN:
                if (inputMask & INPUT_KEY_PRESSED)
                {
                    return event.key.scancode;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (inputMask & INPUT_MOUSE_CLICK)
                {
                    float px = 0, py = 0;
                    SDL_GetMouseState(&px, &py);
                    dataX = int32_t(px);
                    dataY = int32_t(py);
                    return SDL_EVENT_MOUSE_BUTTON_DOWN;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (inputMask & INPUT_MOUSE_CLICK)
                {
					float px = 0, py = 0;
					SDL_GetMouseState(&px, &py);
					dataX = int32_t(px);
					dataY = int32_t(py);
                    return SDL_EVENT_MOUSE_BUTTON_UP;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (inputMask & INPUT_MOUSE_MOTION)
                {
					float px = 0, py = 0;
					SDL_GetMouseState(&px, &py);
					dataX = int32_t(px);
					dataY = int32_t(py);
                    return SDL_EVENT_MOUSE_MOTION;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                if (inputMask & INPUT_MOUSE_WHEEL)
                {
                    dataX = int32_t(event.wheel.x);
                    dataY = int32_t(event.wheel.y);
                    return SDL_EVENT_MOUSE_WHEEL;
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                if ((inputMask & INPUT_WIN_RESIZED) && (event.window.type == SDL_EVENT_WINDOW_RESIZED))
                {
                    dataX = event.window.data1;
                    dataY = event.window.data2;
                    return SDL_EVENT_WINDOW_RESIZED;
                }
                break;

            default:
                break;
            }
        }

        //reduce CPU time
        SDL_Delay(1);
    }
}

//returns the time in milliseconds since the program started
uint64_t getTime()
{
    return SDL_GetTicks();
}

//return passing time from start time
uint64_t getElapsedTime(uint64_t tmstart)
{
    return getTime() - tmstart;
}

//sleep CPU execution
void delay(uint32_t miliseconds)
{
    SDL_Delay(miliseconds);
}

//only return 1 when exit key scancode (not escape) is given, ESCAPE key to exit program
int32_t finished(int32_t keyCode)
{
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

//wait until time wait is passed, program exit when ESCAPE key pressed
void waitFor(uint64_t tmstart, uint64_t tmwait)
{
    while (getElapsedTime(tmstart) < tmwait)
    {
        SDL_PollEvent(&sdlEvent);
        if (sdlEvent.type == SDL_EVENT_QUIT) quit();
        keyStates = (uint8_t*)SDL_GetKeyboardState(NULL);
        if (keyStates[SDL_SCANCODE_ESCAPE]) quit();
        SDL_Delay(1);
    }
}

//sleep until time wait is passed or enter key, program exit when ESCAPE key pressed
void sleepFor(uint64_t tmwait)
{
    int32_t done = 0;
    const uint64_t tmstart = getTime();

    while (!done && getElapsedTime(tmstart) < tmwait)
    {
        SDL_PollEvent(&sdlEvent);
        if (sdlEvent.type == SDL_EVENT_QUIT) quit();
        keyStates = (uint8_t*)SDL_GetKeyboardState(NULL);
        if (keyStates[SDL_SCANCODE_ESCAPE]) quit();
        if (keyStates[SDL_SCANCODE_RETURN])
        {
            done = 1;
            keyStates[SDL_SCANCODE_RETURN] = 0;
        }
        SDL_Delay(1);
    }
}

//exit program
void quit()
{
    cleanup();
    exit(1);
}

//show the mouse cursor
void showMouseCursor()
{
    SDL_ShowCursor();
}

//hide the mouse cursor
void hideMouseCursor()
{
    SDL_HideCursor();
}

//get current mouse state
void getMouseState(int32_t* mx, int32_t* my, int32_t* lmb, int32_t* rmb)
{
    float px = 0, py = 0;
    const SDL_MouseButtonFlags mstate = SDL_GetMouseState(&px, &py);
    if (lmb) *lmb = (mstate == SDL_BUTTON_LEFT);
    if (rmb) *rmb = (mstate == SDL_BUTTON_RIGHT);
    *mx = int32_t(px);
    *my = int32_t(py);
}

//set mouse position
void setMousePosition(int32_t px, int32_t py)
{
    SDL_SetWindowMouseGrab(sdlWindow, SDL_TRUE);
    SDL_WarpMouseInWindow(sdlWindow, float(px), float(py));
}

//initialize graphic video system
int32_t initScreen(int32_t width, int32_t height, int32_t bpp, int32_t scaled, const char* title, int32_t resizeable)
{
    //initialize SDL video mode only
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        messageBox(GFX_ERROR, "Failed init SDL2: %s", SDL_GetError());
        return 0;
    }
    
    //initialize SDL2 image lib
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        messageBox(GFX_ERROR, "Failed init SDL image: %s", IMG_GetError());
        return 0;
    }

    //create screen to display contents
    sdlWindow = SDL_CreateWindow(title, scaled ? SCREEN_WIDTH : width, scaled ? SCREEN_HEIGHT : height, resizeable ? SDL_WINDOW_RESIZABLE : 0);
    if (!sdlWindow)
    {
        messageBox(GFX_ERROR, "Failed to create window: %s", SDL_GetError());
        return 0;
    }

    //set windows icon
    SDL_Surface* icon = IMG_Load("assets/gfxicon-128x.png");
    if (icon)
    {
        SDL_SetSurfaceColorKey(icon, true, SDL_MapRGB(SDL_GetPixelFormatDetails(icon->format), NULL, 0, 0, 0));
        SDL_SetWindowIcon(sdlWindow, icon);
        SDL_DestroySurface(icon);
    }

    //create render windows
    sdlRenderer = SDL_CreateRenderer(sdlWindow, NULL);
    if (!sdlRenderer)
    {
        messageBox(GFX_ERROR, "Failed to create renderer: %s", SDL_GetError());
        return 0;
    }

    //create 32bits texture for render
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!sdlTexture)
    {
        messageBox(GFX_ERROR, "Failed to create texture: %s", SDL_GetError());
        return 0;
    }

    //initialize bits per pixel
    bitsPerPixel = bpp;

    //initialize bytes per pixel
    bytesPerPixel = (bitsPerPixel + 7) / 8;

    //initialize bytes per scan line (should be 32-bytes alignment)
    bytesPerScanline = width * bytesPerPixel;

    //check for 32-bytes alignment
    if (bytesPerScanline % 32)
    {
        messageBox(GFX_ERROR, "GFXLIB required 32-bytes alignment:%d", width);
        return 0;
    }

    //use palette color for 8 bits?
    if (bpp == 8)
    {
        //create 32bits surface and use this to convert to texture before render to screen
        sdlScreen = SDL_CreateSurface(width, height, SDL_GetPixelFormatForMasks(32, 0, 0, 0, 0));
        if (!sdlScreen)
        {
            messageBox(GFX_ERROR, "Failed to create 32 bits surface: %s", SDL_GetError());
            return 0;
        }

        //create 8bits surface with palette
        sdlSurface = SDL_CreateSurface(width, height, SDL_GetPixelFormatForMasks(8, 0, 0, 0, 0));
        if (!sdlSurface)
        {
            messageBox(GFX_ERROR, "Failed to create 8 bits surface: %s", SDL_GetError());
            return 0;
        }

        //initialize drawing buffer (use current surface pixel buffer)
        drawBuff = sdlSurface->pixels;
        if (!drawBuff)
        {
            messageBox(GFX_ERROR, "Failed to create render buffer!");
            return 0;
        }

        //make default palette and retrive the surface palette
        shiftPalette(basePalette);
        SDL_Palette* palette = SDL_CreateSurfacePalette(sdlSurface);

        //set default palette to surface
        if (palette && SDL_SetPaletteColors(palette, basePalette, 0, 256))
        {
            messageBox(GFX_ERROR, "Failed to initialize palette colors!");
            return 0;
        }
    }
    else
    {
        //initialize drawing buffer for 32 bits RGBA (32-bytes alignment for AVX2 use)
        const uint32_t msize = height * bytesPerScanline;
        drawBuff = SDL_aligned_alloc(32, msize);
        if (!drawBuff)
        {
            messageBox(GFX_ERROR, "Failed to create render buffer!");
            return 0;
        }
    }

    //initialize random number generation
    randSeed = uint32_t(time(NULL));
    srand(randSeed);

    //initialize GFXLIB buffer
    gfxBuff = (uint8_t*)SDL_calloc(GFX_BUFF_SIZE, 1);
    if (!gfxBuff)
    {
        messageBox(GFX_ERROR, "Error initialize GFXLIB memory!");
        return 0;
    }

    //initialize screen buffer size
    texWidth    = width;
    texHeight   = height;
    centerX     = (texWidth >> 1) - 1;
    centerY     = (texHeight >> 1) - 1;

    //initialize view port size
    cminX       = 0;
    cminY       = 0;
    cmaxX       = texWidth - 1;
    cmaxY       = texHeight - 1;
    
    //OK, I'm fine!
    return 1;
}

//cleanup function must call after graphics operations ended
void cleanup()
{
    if (bitsPerPixel == 8)
    {
        if (sdlScreen)
        {
            SDL_DestroySurface(sdlScreen);
            sdlScreen = NULL;
        }

        if (sdlSurface)
        {
            SDL_DestroySurface(sdlSurface);
            sdlSurface = NULL;
        }
    }
    else
    {
        if (drawBuff)
        {
            SDL_aligned_free(drawBuff);
            drawBuff = NULL;
        }
    }

    if (gfxBuff)
    {
        SDL_free(gfxBuff);
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
    if (bitsPerPixel == 8)
    {
        //256 colors palette, we must convert 8 bits surface to 32 bits surface
        SDL_BlitSurface(sdlSurface, NULL, sdlScreen, NULL);
        SDL_UpdateTexture(sdlTexture, NULL, sdlScreen->pixels, sdlScreen->pitch);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderTexture(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    }
    else
    {
        //rgb mode, just render texture to video memory without any conversation
        SDL_UpdateTexture(sdlTexture, NULL, drawBuff, bytesPerScanline);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderTexture(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    }
}

//render from user-defined buffer
void renderBuffer(const void* buffer, int32_t width, int32_t height)
{
    //calculate amount of bytes transfer (should be 32-bytes alignment)
    const uint32_t rowBytes = width * bytesPerPixel;
    const uint32_t bytesCopy = height * rowBytes;

    //check for 32-bytes alignment
    if (rowBytes % 32)
    {
        messageBox(GFX_ERROR, "GFXLIB required 32-bytes alignment:%d", width);
        return;
    }

    //detect texture size has changed?
    if (texWidth != width || texHeight != height)
    {
        //create new texture with new size
        if (sdlTexture) SDL_DestroyTexture(sdlTexture);
        sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!sdlTexture)
        {
            messageBox(GFX_ERROR, "Failed to create new texture: %s", SDL_GetError());
            return;
        }

        //8 bits
        if (bytesPerPixel == 1)
        {
            //save current palette
            RGBA pal[256] = { 0 };
            getPalette(pal);

            //create new 32bits surface
            if (sdlScreen) SDL_DestroySurface(sdlScreen);
            sdlScreen = SDL_CreateSurface(width, height, SDL_GetPixelFormatForMasks(32, 0, 0, 0, 0));
            if (!sdlScreen)
            {
                messageBox(GFX_ERROR, "Failed to create new 32bits surface: %s", SDL_GetError());
                return;
            }

            //create new 8bits surface
            if (sdlSurface) SDL_DestroySurface(sdlSurface);
            sdlSurface = SDL_CreateSurface(width, height, SDL_GetPixelFormatForMasks(8, 0, 0, 0, 0));
            if (!sdlSurface)
            {
                messageBox(GFX_ERROR, "Failed to create new 8bits surface: %s", SDL_GetError());
                return;
            }

            //initialize new drawing buffer
            if (!sdlSurface->pixels)
            {
                messageBox(GFX_ERROR, "Failed to create render buffer!");
                return;
            }

            //make the draw buffer
            drawBuff = sdlSurface->pixels;

            //restore palette on new surface
            SDL_Palette* palette = SDL_GetSurfacePalette(sdlSurface);
            if (palette) SDL_SetPaletteColors(palette, pal, 0, 256);
        }
        else
        {
            //adjust render buffer (32-bytes alignment)
            if (drawBuff) SDL_aligned_free(drawBuff);
            drawBuff = SDL_aligned_alloc(32, bytesCopy);
            if (!drawBuff)
            {
                messageBox(GFX_INFO, "Error create new render buffer:%u!", bytesCopy);
                return;
            }
        }

        //update new screen buffer size
        texWidth = width;
        texHeight = height;
        centerX = (texWidth >> 1) - 1;
        centerY = (texHeight >> 1) - 1;

        //update new view port size
        cminX = 0;
        cminY = 0;
        cmaxX = texWidth - 1;
        cmaxY = texHeight - 1;

        //update bytes per scan line
        bytesPerScanline = rowBytes;
    }

    //done adjustment render buffer
    memcpy(drawBuff, buffer, bytesCopy);
    render();
}

//raise a message box
void messageBox(int32_t type, const char* fmt, ...)
{
    char buffer[1024] = { 0 };
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    switch (type)
    {
    case GFX_ERROR:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "GFX Error!", buffer, NULL);
        break;

    case GFX_WARNING:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "GFX Warning!", buffer, NULL);
        break;

    default:
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "GFX Info", buffer, NULL);
        break;
    }
}

//get current bits per pixel
must_inline int32_t getBitsPerPixel()
{
    return bitsPerPixel;
}

//get current bytes per pixel
must_inline int32_t getBytesPerPixel()
{
    return bytesPerPixel;
}

//get current bytes per line
must_inline int32_t getBytesPerScanline()
{
    return bytesPerScanline;
}

//get current draw buffer width
int32_t getDrawBufferWidth()
{
    return texWidth;
}

//get current draw buffer height
int32_t getDrawBufferHeight()
{
    return texHeight;
}

//retrieve raw pixels data buffer
void* getDrawBuffer(int32_t *width, int32_t *height)
{
    if (width) *width = texWidth;
    if (height) *height = texHeight;
    return drawBuff;
}

//set the draw buffer
//!!!changeDrawBuffer and restoreDrawBuffer must be a pair functions!!!
void changeDrawBuffer(void* newBuff, int32_t newWidth, int32_t newHeight)
{
    oldBuffer = drawBuff;
    drawBuff = newBuff;
    changeViewPort(0, 0, newWidth - 1, newHeight - 1);
}

//must call after setDrawBuffer call
//!!!changeDrawBuffer and restoreDrawBuffer must be a pair functions!!!
void restoreDrawBuffer()
{
    drawBuff = oldBuffer;
    restoreViewPort();
}

//set current screen view port for clipping
//!!!changeViewPort and restoreViewPort must be a pair functions!!!
void changeViewPort(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //save current view port
    oldMinX     = cminX;
    oldMinY     = cminY;
    oldMaxX     = cmaxX;
    oldMaxY     = cmaxY;
    oldWidth    = texWidth;
    oldHeight   = texHeight;

    //update new clip view port
    cminX = x1;
    cminY = y1;
    cmaxX = x2;
    cmaxY = y2;

    //update buffer width and height
    texWidth    = cmaxX - cminX + 1;
    texHeight   = cmaxY - cminY + 1;

    //update center x,y
    centerX = (texWidth >> 1) - 1;
    centerY = (texHeight >> 1) - 1;

    //update row bytes
    bytesPerScanline = texWidth * bytesPerPixel;
}

//must call after changeViewPort call
//!!!changeViewPort and restoreViewPort must be a pair functions!!!
void restoreViewPort()
{
    cminX       = oldMinX;
    cminY       = oldMinY;
    cmaxX       = oldMaxX;
    cmaxY       = oldMaxY;
    texWidth    = oldWidth;
    texHeight   = oldHeight;
    centerX     = (texWidth >> 1) - 1;
    centerY     = (texHeight >> 1) - 1;
    bytesPerScanline = texWidth * bytesPerPixel;
}

//get current view port
void getViewPort(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2)
{
    *x1 = cminX;
    *y1 = cminY;
    *x2 = cmaxX;
    *y2 = cmaxY;
}

//get current x center
int32_t getCenterX()
{
    return centerX;
}

//get current y center
int32_t getCenterY()
{
    return centerY;
}

//get current max x
int32_t getMaxX()
{
    return cmaxX;
}

//get current max y
int32_t getMaxY()
{
    return cmaxY;
}

//get current min x
int32_t getMinX()
{
    return cminX;
}

//get current min y
int32_t getMinY()
{
    return cminY;
}

//clear screen with color
void clearScreenMix(uint32_t color)
{
    const uint32_t msize = texHeight * texWidth;
#ifdef _USE_ASM
    __asm {
        mov     edi, drawBuff
        mov     ecx, msize
        shr     ecx, 2
        mov     ebx, msize
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
    //32-bytes alignment
    const int32_t aligned = msize >> 5;
    const __m256i ymm0 = _mm256_set1_epi8(color);
    uint8_t* pixels = (uint8_t*)drawBuff;

    //loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        _mm256_stream_si256((__m256i*)pixels, ymm0);
        pixels += 32;
    }

    //have unaligned bytes?
    const int32_t remainder = msize % 32;
    if (remainder > 0)
    {
        for (int32_t i = 0; i < remainder; i++) *pixels++ = color;
    }
#endif
    render();
}

//clear screen with color
void clearScreen(uint32_t color)
{
    //mixed mode?
    if (bitsPerPixel == 8)
    {
        clearScreenMix(color);
        return;
    }

    //height color mode
    const uint32_t msize = texHeight * texWidth;
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        movd        mm0, color
        mov         ecx, msize
        shr         ecx, 1
        jz          once
        punpckldq   mm0, mm0
    again:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         again
    once:
        test        msize, 1
        jz          end
        movd        [edi], mm0
    end:
        emms        
    }
#else
    //32-bytes alignment
    const int32_t aligned = msize >> 3;
    const __m256i ymm0 = _mm256_set1_epi32(color);
    uint32_t* pixels = (uint32_t*)drawBuff;
    
    //loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        _mm256_stream_si256((__m256i*)pixels, ymm0);
        pixels += 8;
    }

    //have unaligned bytes?
    const int32_t remainder = msize % 8;
    if (remainder > 0)
    {
        for (int32_t i = 0; i < remainder; i++) *pixels++ = color;
    }
#endif
    render();
}

//plot a pixel at (x,y) with color
must_inline void putPixelMix(int32_t x, int32_t y, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
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
must_inline void putPixelNormal(int32_t x, int32_t y, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
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

//plot a pixel at (x,y) with alpha-blending
//correct formular: (SC*SA+DC*(256-SA))>>8
//don't use (SC*SA+DC*(255-SA))>>8, you'll always get 254 as your maximum value.
//ie: (255*128+255*(255-128))>>8=254 --> WRONG!!!
//with: (255*128+255*(256-128))>>8=255 --> ACCEPTED!!!
must_inline void putPixelAlpha(int32_t x, int32_t y, uint32_t argb)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff   
        add         edi, eax
        mov         eax, argb
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
        movd        mm0, [edi]
        movd        mm1, argb
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
        emms
    }
#else
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];
    const uint32_t dst = *pixels;
    const uint8_t cover = argb >> 24;
    const uint8_t rcover = 255 - cover;
    const uint32_t rb = ((dst & 0x00ff00ff) * rcover + (argb & 0x00ff00ff) * cover);
    const uint32_t ag = (((dst & 0xff00ff00) >> 8) * rcover + ((argb & 0xff00ff00) >> 8) * cover);
    *pixels = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
#endif
}

//plot a pixel at (x,y) with anti-aliased
must_inline void putPixelAA(int32_t x, int32_t y, uint32_t argb)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff
        add         edi, eax
        mov         eax, argb
        shr         eax, 24
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
        neg         eax
        add         eax, 256
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        pxor        mm5, mm5
        movd        mm0, [edi]
        movd        mm1, argb
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
        emms
    }
#else
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];
    const uint32_t dst = *pixels;
    const uint8_t cover = argb >> 24;
    const uint8_t rcover = 255 - cover;
    const uint32_t rb = ((argb & 0x00ff00ff) * rcover + (dst & 0x00ff00ff) * cover);
    const uint32_t ag = (((argb & 0xff00ff00) >> 8) * rcover + ((dst & 0xff00ff00) >> 8) * cover);
    *pixels = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
#endif
}

//plot a pixel at (x,y) with background color
must_inline void putPixelBob(int32_t x, int32_t y)
{
    if (bitsPerPixel != 8) return;
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;
#ifdef _USE_ASM
    __asm {
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
    const uint8_t col = min(getPixel(x, y) + 1, 255);
    putPixel(x, y, col);
#endif
}

//plot a pixel at (x,y) with add color
must_inline void putPixelAdd(int32_t x, int32_t y, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        movd    mm0, [edi]
        paddusb mm0, color
        movd    [edi], mm0
        emms
    }
#else
    const ARGB* rgb = (ARGB*)&color;
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    pixels->r = min(pixels->r + rgb->r, 255);
    pixels->g = min(pixels->g + rgb->g, 255);
    pixels->b = min(pixels->b + rgb->b, 255);
#endif
}

//plot a pixel at (x,y) with sub color
must_inline void putPixelSub(int32_t x, int32_t y, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        movd    mm0, [edi]
        psubusb mm0, color
        movd    [edi], mm0
        emms
    }
#else
    const ARGB* rgb = (ARGB*)&color;
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    pixels->r = max(pixels->r - rgb->r, 0);
    pixels->g = max(pixels->g - rgb->g, 0);
    pixels->b = max(pixels->b - rgb->b, 0);
#endif
}

//put pixel at (x,y) with color and mode
void putPixel(int32_t x, int32_t y, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //range checking
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return;

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        putPixelMix(x, y, color);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        putPixelNormal(x, y, color);
        break;

    case BLEND_MODE_ADD:
        putPixelAdd(x, y, color);
        break;

    case BLEND_MODE_SUB:
        putPixelSub(x, y, color);
        break;

    case BLEND_MODE_ALPHA:
        putPixelAlpha(x, y, color);
        break;

    case BLEND_MODE_ANTIALIASED:
        putPixelAA(x, y, color);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//peek a pixel at (x,y)
must_inline uint32_t getPixelMix(int32_t x, int32_t y)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     esi, drawBuff
        add     esi, eax
        xor     eax, eax
        lodsb
    }
    //eax will auto returned
#else
    const uint8_t* pixels = (uint8_t*)drawBuff;
    return pixels[y * texWidth + x];
#endif
}

//peek a pixel at (x,y)
uint32_t getPixel(int32_t x, int32_t y)
{
    //range checking
    if (x < cminX || y < cminY || x > cmaxX || y > cmaxY) return 0;

    //mixed mode?
    if (bitsPerPixel == 8) return getPixelMix(x, y);
        
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     esi, drawBuff
        add     esi, eax
        xor     eax, eax
        lodsd
    }
    //eax will auto returned
#else
    const uint32_t* pixel = (uint32_t*)drawBuff;
    return pixel[y * texWidth + x];
#endif
}

//fast horizontal line from (x,y) with sx length, and color
must_inline void horizLineMix(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        mov     edi, drawBuff
        add     edi, eax
        mov     al, byte ptr [color]
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
    //32-bytes alignment
    const int32_t aligned = sx >> 5;
    const __m256i ymm0 = _mm256_set1_epi8(color);
    uint8_t* pdata = (uint8_t*)drawBuff;
    uint8_t* pixels = &pdata[texWidth * y + x];
    
    //loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        //_mm256_storeu_si256 is slower than _mm256_stream_si256
        //but _mm256_stream_si256 required memory is aligned by 32-bytes
        _mm256_storeu_si256((__m256i*)pixels, ymm0);
        pixels += 32;
    }

    //have unaligned bytes?
    const int32_t remainder = sx % 32;
    if (remainder > 0)
    {
        for (int32_t i = 0; i < remainder; i++) *pixels++ = color;
    }
#endif
}

//fast horizontal line from (x,y) with sx length, and color
must_inline void horizLineNormal(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff
        add         edi, eax
        movd        mm0, color
        mov         ecx, sx
        shr         ecx, 1
        jz          once
        punpckldq   mm0, mm0
    plot:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        sx, 1
        jz          end
        movd        [edi], mm0
    end:
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = sx >> 3;
    const __m256i ymm0 = _mm256_set1_epi32(color);
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];

    //loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        _mm256_storeu_si256((__m256i*)pixels, ymm0);
        pixels += 8;
    }

    //have unaligned bytes?
    const int32_t remainder = sx % 8;
    if (remainder > 0)
    {
        for (int32_t i = 0; i < remainder; i++) *pixels++ = color;
    }
#endif
}

//fast horizontal line from (x,y) with sx length, and add color
must_inline void horizLineAdd(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff
        add         edi, eax
        movd        mm1, color
        mov         ecx, sx
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        movq        mm0, [edi]
        paddusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         next
    once:
        test        sx, 1
        jz          end
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
    end:
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = sx >> 3;
    const __m256i ymm0 = _mm256_set1_epi32(color);
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    
    //loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)pixels);
        ymm1 = _mm256_adds_epu8(ymm1, ymm0);
        _mm256_stream_si256((__m256i*)pixels, ymm1);
        pixels += 8;
    }

    //have unaligned bytes?
    const int32_t remainder = sx % 8;
    if (remainder > 0)
    {
        const ARGB* rgb = (const ARGB*)&color;
        for (int32_t i = 0; i < remainder; i++)
        {
            pixels->r = min(pixels->r + rgb->r, 255);
            pixels->g = min(pixels->g + rgb->g, 255);
            pixels->b = min(pixels->b + rgb->b, 255);
            pixels++;
        }
    }
#endif
}

//fast horizontal line from (x,y) with sx length, and sub color
must_inline void horizLineSub(int32_t x, int32_t y, int32_t sx, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff
        add         edi, eax
        movd        mm1, color
        mov         ecx, sx
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         next
    once:
        test        sx, 1
        jz          end
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    end:
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = sx >> 3;
    const __m256i ymm0 = _mm256_set1_epi32(color);
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];

    //loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)pixels);
        ymm1 = _mm256_subs_epu8(ymm1, ymm0);
        _mm256_stream_si256((__m256i*)pixels, ymm1);
        pixels += 8;
    }

    //have unaligned bytes?
    const int32_t remainder = sx % 8;
    if (remainder > 0)
    {
        const ARGB* rgb = (const ARGB*)&color;
        for (int32_t i = 0; i < remainder; i++)
        {
            pixels->r = max(pixels->r - rgb->r, 0);
            pixels->g = max(pixels->g - rgb->g, 0);
            pixels->b = max(pixels->b - rgb->b, 0);
            pixels++;
        }
    }
#endif
}

//fast horizontal line from (x,y) with sx length, and blending pixel
must_inline void horizLineAlpha(int32_t x, int32_t y, int32_t sx, uint32_t argb)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff
        add         edi, eax
        mov         eax, argb
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
    next:
        movd        mm0, [edi]
        movd        mm1, argb
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
        add         edi, 4
        dec         sx
        jnz         next
        emms
    }
#else
    //calculate starting address
    const uint8_t cover = argb >> 24;
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];

    //zero all
    const __m256i ymm0 = _mm256_setzero_si256();

    //alpha and inverted alpha, 8 x 32 bits
    const __m256i alpha = _mm256_set1_epi16(cover);
    const __m256i invert = _mm256_set1_epi16(256 - cover);

    //set 4 pixels source color (8 x 32 bits data)
    __m256i losrc = _mm256_set1_epi32(argb);
    __m256i hisrc = losrc;

    //unpack to low & hi (S * A) (8 x 32 bits data)
    losrc = _mm256_unpacklo_epi8(losrc, ymm0);
    losrc = _mm256_mullo_epi16(losrc, alpha);
    hisrc = _mm256_unpackhi_epi8(hisrc, ymm0);
    hisrc = _mm256_mullo_epi16(hisrc, alpha);

    //process 32-bytes aligned
    const int32_t aligned = sx >> 3;
    for (int32_t i = 0; i < aligned; i++)
    {
        //load 8 pixes width from dest (8 x 32 bits data)
        __m256i lodst = _mm256_stream_load_si256((const __m256i*)pixels);
        __m256i hidst = lodst;

        //unpack to low & high (D * (256 - A)) (8 x 32 bits data)
        lodst = _mm256_unpacklo_epi8(lodst, ymm0);
        lodst = _mm256_mullo_epi16(lodst, invert);
        hidst = _mm256_unpackhi_epi8(hidst, ymm0);
        hidst = _mm256_mullo_epi16(hidst, invert);

        //blending low, high (S * A + D * (256 - A)) >> 8 (8 x 32 bits data)
        __m256i loret = _mm256_adds_epu16(losrc, lodst);
        loret = _mm256_srli_epi16(loret, 8);
        __m256i hiret = _mm256_adds_epu16(hisrc, hidst);
        hiret = _mm256_srli_epi16(hiret, 8);

        //destination = PACKED(low,hi) 32 x 8 bits
        const __m256i result = _mm256_packus_epi16(loret, hiret);
        _mm256_stream_si256((__m256i*)pixels, result);

        //next 8 pixels
        pixels += 8;
    }

    //have unaligned bytes
    const int32_t remainder = sx % 8;
    if (remainder > 0)
    {
        const uint8_t rcover = 255 - cover;
        for (int32_t i = 0; i < remainder; i++)
        {
            const uint32_t dst = *pixels;
            const uint32_t rb = ((dst & 0x00ff00ff) * rcover + (argb & 0x00ff00ff) * cover);
            const uint32_t ag = (((dst & 0xff00ff00) >> 8) * rcover + ((argb & 0xff00ff00) >> 8) * cover);
            *pixels++ = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
        }
    }
#endif
}

//fast horizon line from (x, y) with sx length
void horizLine(int32_t x, int32_t y, int32_t sx, uint32_t color, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
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

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        horizLineMix(x, y, sx, color);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        horizLineNormal(x, y, sx, color);
        break;

    case BLEND_MODE_ADD:
        horizLineAdd(x, y, sx, color);
        break;

    case BLEND_MODE_SUB:
        horizLineSub(x, y, sx, color);
        break;

    case BLEND_MODE_ALPHA:
        horizLineAlpha(x, y, sx, color);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//fast vertical line from (x,y) with sy length, and palette color
must_inline void vertLineMix(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
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
        dec     ecx
        jnz     next
    }
#else
    //calculate starting address
    uint8_t* pdata = (uint8_t*)drawBuff;
    uint8_t* pixels = &pdata[texWidth * y + x];
    for (int32_t i = 0; i < sy; i++)
    {
        *pixels = color;
        pixels += texWidth;
    }
#endif
}

//fast vertical line from (x,y) with sy length, and rgb color
must_inline void vertLineNormal(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
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
        dec     ecx
        jnz     next
    }
#else
    //calculate starting address
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];
    for (int32_t i = 0; i < sy; i++)
    {
        *pixels = color;
        pixels += texWidth;
    }
#endif
}

//fast vertical line from (x,y) with sy length, and add color
must_inline void vertLineAdd(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sy
        mov     ebx, texWidth
        shl     ebx, 2
    next:
        movd    mm0, [edi]
        paddusb mm0, color
        movd    [edi], mm0
        add     edi, ebx 
        dec     ecx
        jnz     next
        emms
    }
#else
    //calculate starting address
    const ARGB* rgb = (ARGB*)&color;
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    for (int32_t i = 0; i < sy; i++)
    {
        pixels->r = min(pixels->r + rgb->r, 255);
        pixels->g = min(pixels->g + rgb->g, 255);
        pixels->b = min(pixels->b + rgb->b, 255);
        pixels += intptr_t(texWidth);
    }
#endif
}

//fast vertical line from (x,y) with sy length, and sub color
must_inline void vertLineSub(int32_t x, int32_t y, int32_t sy, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        mov     edi, drawBuff
        add     edi, eax
        mov     ecx, sy
        mov     ebx, texWidth
        shl     ebx, 2
    next:
        movd    mm0, [edi]
        psubusb mm0, color
        movd    [edi], mm0
        add     edi, ebx
        dec     ecx
        jnz     next
        emms
    }
#else
    //calculate starting address
    const ARGB* rgb = (ARGB*)&color;
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    for (int32_t i = 0; i < sy; i++)
    {
        pixels->r = max(pixels->r - rgb->r, 0);
        pixels->g = max(pixels->g - rgb->g, 0);
        pixels->b = max(pixels->b - rgb->b, 0);
        pixels += intptr_t(texWidth);
    }
#endif
}

//fast vertical line from (x,y) with sy length, and blending pixels
must_inline void vertLineAlpha(int32_t x, int32_t y, int32_t sy, uint32_t argb)
{
#ifdef _USE_ASM
    __asm {
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        mov         edi, drawBuff
        add         edi, eax
        xor         eax, eax
        mov         edx, texWidth
        shl         edx, 2
        mov         eax, argb
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
    next:
        movd        mm0, [edi]
        movd        mm1, argb
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
        add         edi, edx
        dec         sy
        jnz         next
        emms
    }
#else
    //calculate starting address
    const uint8_t cover = argb >> 24;
    const uint8_t rcover = 255 - cover;
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];
    for (int32_t i = 0; i < sy; i++)
    {
        const uint32_t dst = *pixels;
        const uint32_t rb = ((dst & 0x00ff00ff) * rcover + (argb & 0x00ff00ff) * cover);
        const uint32_t ag = (((dst & 0xff00ff00) >> 8) * rcover + ((argb & 0xff00ff00) >> 8) * cover);
        *pixels = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
        pixels += intptr_t(texWidth);
    }
#endif
}

//fast vertical line from (x,y) with sy length, and color
void vertLine(int32_t x, int32_t y, int32_t sy, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
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

    if (bitsPerPixel == 8)
    {
        vertLineMix(x, y, sy, color);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        vertLineNormal(x, y, sy, color);
        break;

    case BLEND_MODE_ADD:
        vertLineAdd(x, y, sy, color);
        break;

    case BLEND_MODE_SUB:
        vertLineSub(x, y, sy, color);
        break;

    case BLEND_MODE_ALPHA:
        vertLineAlpha(x, y, sy, color);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectMix(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov     edi, drawBuff
        mov     eax, y
        mul     texWidth
        add     eax, x
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
    //align 32-bytes
    const int32_t aligned = width >> 5;
    const int32_t remainder = width % 32;
    const int32_t addOffset = texWidth - width;
    
    //calculate starting address
    uint8_t* pdata = (uint8_t*)drawBuff;
    uint8_t* dstPixels = &pdata[texWidth * y + x];
    
    //initialize vector color
    const __m256i ymm0 = _mm256_set1_epi8(color);

    //lines-by-lines
    for (int32_t i = 0; i < height; i++)
    {
        //loop for 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            _mm256_stream_si256((__m256i*)dstPixels, ymm0);
            dstPixels += 32;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            for (int32_t j = 0; j < remainder; j++) *dstPixels++ = color;
        }

        //next lines
        if (addOffset > 0) dstPixels += addOffset;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectNormal(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         edx, texWidth
        sub         edx, width
        shl         edx, 2
        movd        mm0, color
    again:
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm0, mm0
    next:
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         next
    once:
        test        width, 1
        jz          end
        movd        [edi], mm0
        add         edi, 4
    end:
        add         edi, edx
        dec         height
        jnz         again
        emms
    }
#else
    //align 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;
    const int32_t addOffset = texWidth - width;

    //calculate starting address
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* dstPixels = &pdata[texWidth * y + x];

    //initialize vector color
    const __m256i ymm0 = _mm256_set1_epi32(color);

    //lines-by-lines
    for (int32_t i = 0; i < height; i++)
    {
        //loop for 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            _mm256_stream_si256((__m256i*)dstPixels, ymm0);
            dstPixels += 8;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            for (int32_t j = 0; j < remainder; j++) *dstPixels++ = color;
        }

        //next lines
        if (addOffset > 0) dstPixels += addOffset;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectAdd(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        movd        mm1, color
    again:
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    plot:
        movq        mm0, [edi]
        paddusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          end
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
        add         edi, 4
    end:
        add         edi, ebx
        dec         height
        jnz         again
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;
    const int32_t addOfs = texWidth - width;
    
    //calculate starting address
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    
    //initialize vector color
    const __m256i ymm0 = _mm256_set1_epi32(color);

    //lines-by-lines
    for (int32_t i = 0; i < height; i++)
    {
        //loop for 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)pixels);
            ymm1 = _mm256_adds_epu8(ymm1, ymm0);
            _mm256_stream_si256((__m256i*)pixels, ymm1);
            pixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            const ARGB* pcol = (const ARGB*)&color;
            for (int32_t j = 0; j < remainder; j++)
            {
                pixels->b = min(pixels->b + pcol->b, 255);
                pixels->g = min(pixels->g + pcol->g, 255);
                pixels->r = min(pixels->r + pcol->r, 255);
                pixels++;
            }
        }

        //next line
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectSub(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        movd        mm1, color
    again:
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    plot:
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          end
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
        add         edi, 4
    end:
        add         edi, ebx
        dec         height
        jnz         again
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;
    const int32_t addOfs = texWidth - width;

    //calculate starting address
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];

    //initialize vector color
    const __m256i ymm0 = _mm256_set1_epi32(color);

    //lines-by-lines
    for (int32_t i = 0; i < height; i++)
    {
        //loop for 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)pixels);
            ymm1 = _mm256_subs_epu8(ymm1, ymm0);
            _mm256_stream_si256((__m256i*)pixels, ymm1);
            pixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            const ARGB* pcol = (const ARGB*)&color;
            for (int32_t j = 0; j < remainder; j++)
            {
                pixels->b = max(pixels->b - pcol->b, 0);
                pixels->g = max(pixels->g - pcol->g, 0);
                pixels->r = max(pixels->r - pcol->r, 0);
                pixels++;
            }
        }

        //next line
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectAnd(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        movd        mm1, color
    again:
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    plot:
        movq        mm0, [edi]
        pand        mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          end
        movd        mm0, [edi]
        pand        mm0, mm1
        movd        [edi], mm0
        add         edi, 4
    end:
        add         edi, ebx
        dec         height
        jnz         again
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;
    const int32_t addOfs = texWidth - width;

    //calculate starting address
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];

    //initialize vector color
    const __m256i ymm0 = _mm256_set1_epi32(color);

    //lines-by-lines
    for (int32_t i = 0; i < height; i++)
    {
        //loop for 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)pixels);
            ymm1 = _mm256_and_si256(ymm1, ymm0);
            _mm256_stream_si256((__m256i*)pixels, ymm1);
            pixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            const ARGB* pcol = (const ARGB*)&color;
            for (int32_t j = 0; j < remainder; j++)
            {
                pixels->b = pixels->b & pcol->b;
                pixels->g = pixels->g & pcol->g;
                pixels->r = pixels->r & pcol->r;
                pixels++;
            }
        }

        //next line
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectXor(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        movd        mm1, color
    again:
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    plot:
        movq        mm0, [edi]
        pxor        mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          end
        movd        mm0, [edi]
        pxor        mm0, mm1
        movd        [edi], mm0
        add         edi, 4
    end:
        add         edi, ebx
        dec         height
        jnz         again
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;
    const int32_t addOfs = texWidth - width;

    //calculate starting address
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];

    //initialize vector color
    const __m256i ymm0 = _mm256_set1_epi32(color);

    //lines-by-lines
    for (int32_t i = 0; i < height; i++)
    {
        //loop for 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)pixels);
            ymm1 = _mm256_xor_si256(ymm1, ymm0);
            _mm256_stream_si256((__m256i*)pixels, ymm1);
            pixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            const ARGB* pcol = (const ARGB*)&color;
            for (int32_t j = 0; j < remainder; j++)
            {
                pixels->b = pixels->b ^ pcol->b;
                pixels->g = pixels->g ^ pcol->g;
                pixels->r = pixels->r ^ pcol->r;
                pixels++;
            }
        }

        //next line
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and blending pixel
void fillRectAlpha(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t argb)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         edx, texWidth
        sub         edx, width
        shl         edx, 2
        mov         eax, argb
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
    next:
        mov         ecx, width
    plot:
        movd        mm0, [edi]
        movd        mm1, argb
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
        add         edi, 4
        dec         ecx
        jnz         plot
        add         edi, edx
        dec         height
        jnz         next
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;
    const int32_t addOfs = texWidth - width;
    const uint8_t cover = argb >> 24;

    //calculate starting address
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];

    //zero all
    const __m256i ymm0 = _mm256_setzero_si256();

    //alpha and inverted alpha, 8 x 32 bits
    const __m256i alpha = _mm256_set1_epi16(cover);
    const __m256i invert = _mm256_set1_epi16(256 - cover);

    //set 8 pixels source color (8 x 32 bits data)
    __m256i losrc = _mm256_set1_epi32(argb);
    __m256i hisrc = losrc;

    //unpack to low & hi (S * A) (8 x 32 bits data)
    losrc = _mm256_unpacklo_epi8(losrc, ymm0);
    losrc = _mm256_mullo_epi16(losrc, alpha);
    hisrc = _mm256_unpackhi_epi8(hisrc, ymm0);
    hisrc = _mm256_mullo_epi16(hisrc, alpha);

    //process 32-bytes aligned
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            //load 8 pixes width from dest (8 x 32 bits data)
            __m256i lodst = _mm256_stream_load_si256((const __m256i*)pixels);
            __m256i hidst = lodst;

            //unpack to low & high (D * (256 - A)) (8 x 32 bits data)
            lodst = _mm256_unpacklo_epi8(lodst, ymm0);
            lodst = _mm256_mullo_epi16(lodst, invert);
            hidst = _mm256_unpackhi_epi8(hidst, ymm0);
            hidst = _mm256_mullo_epi16(hidst, invert);

            //blending low, high (S * A + D * (256 - A)) >> 8 (8 x 32 bits data)
            __m256i loret = _mm256_adds_epu16(losrc, lodst);
            loret = _mm256_srli_epi16(loret, 8);
            __m256i hiret = _mm256_adds_epu16(hisrc, hidst);
            hiret = _mm256_srli_epi16(hiret, 8);

            //destination = PACKED(low,hi) 32 x 8 bits
            const __m256i result = _mm256_packus_epi16(loret, hiret);
            _mm256_stream_si256((__m256i*)pixels, result);

            //next 8 pixels
            pixels += 8;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            const uint8_t rcover = 255 - cover;
            for (int32_t k = 0; k < remainder; k++)
            {
                const uint32_t dst = *pixels;
                const uint32_t rb = ((dst & 0x00ff00ff) * rcover + (argb & 0x00ff00ff) * cover);
                const uint32_t ag = (((dst & 0xff00ff00) >> 8) * rcover + ((argb & 0xff00ff00) >> 8) * cover);
                *pixels++ = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
            }
        }

        //next line
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //calculate new position
    const int32_t x1 = x + (width - 1);
    const int32_t y1 = y + (height - 1);

    //clip image to context boundaries
    const int32_t lx = max(x, cminX);
    const int32_t ly = max(y, cminY);
    const int32_t lx1 = min(x1, cmaxX);
    const int32_t ly1 = min(y1, cmaxY);

    //initialize loop variables
    const int32_t lwidth = (lx1 - lx) + 1;
    const int32_t lheight = (ly1 - ly) + 1;

    //check for loop
    if (lwidth <= 0 || lheight <= 0) return;

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        fillRectMix(lx, ly, lwidth, lheight, color);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        fillRectNormal(lx, ly, lwidth, lheight, color);
        break;

    case BLEND_MODE_ADD:
        fillRectAdd(lx, ly, lwidth, lheight, color);
        break;

    case BLEND_MODE_SUB:
        fillRectSub(lx, ly, lwidth, lheight, color);
        break;

    case BLEND_MODE_AND:
        fillRectAnd(lx, ly, lwidth, lheight, color);
        break;

    case BLEND_MODE_XOR:
        fillRectXor(lx, ly, lwidth, lheight, color);
        break;

    case BLEND_MODE_ALPHA:
        fillRectAlpha(lx, ly, lwidth, lheight, color);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectPatternMix(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, const uint8_t* pattern)
{
#ifdef _USE_ASM
    __asm {
        mov     edi, drawBuff
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        add     edi, eax
        mov     edx, texWidth
        sub     edx, width
        mov     esi, pattern
    plot:
        mov     ecx, x
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
        dec     ecx
        jnz     next
        add     edi, edx
        dec     height
        jnz     plot
    }
#else
    //calculate starting address
    const int32_t addDstOffs = texWidth - width;
    uint8_t* pdata = (uint8_t*)drawBuff;
    uint8_t* dstPixels = &pdata[texWidth * y + x];

    for (int32_t i = 0; i < height; i++)
    {
        uint8_t pat = pattern[i & 7];
        pat = _rotl8(pat, 7);
        for (int32_t j = 0; j < width; j++)
        {
            if (pat & 1) *dstPixels = pat;
            pat = _rotl8(pat, 1);
            dstPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectPatternNormal(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, const uint8_t* pattern)
{
#ifdef _USE_ASM
    __asm {
        mov         esi, pattern
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         edx, texWidth
        sub         edx, width
        shl         edx, 2
        movd        mm0, col
    plot:
        mov         ecx, x
        and         ecx, 7
        mov         ebx, height
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm0, mm0
    next:
        test        al, 1
        jz          skip1
        test        al, 2
        jz          skip2
        movq        [edi], mm0
        jmp         skip0
    skip2:
        movd        [edi + 4], mm0
        jmp         skip0
    skip1:
        test        al, 2
        jz          skip0
        movd        [edi], mm0
    skip0:
        add         edi, 8
        rol         al, 2
        dec         ecx
        jnz         next
    once:
        test        width, 1
        jz          end1
        test        al, 2
        jz          end0
        movd        [edi], mm0
    end0:
        add         edi, 4
    end1:
        add         edi, edx
        dec         height
        jnz         plot
        emms
    }
#else
    //calculate starting address
    const int32_t addDstOffs = texWidth - width;
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* dstPixels = &pdata[texWidth * y + x];

    for (int32_t i = 0; i < height; i++)
    {
        uint8_t pat = pattern[i & 7];
        pat = _rotl8(pat, 7);
        for (int32_t j = 0; j < width; j++)
        {
            if (pat & 1) *dstPixels = pat;
            pat = _rotl8(pat, 1);
            dstPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectPatternAdd(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, const uint8_t* pattern)
{
#ifdef _USE_ASM
    __asm {
        mov         esi, pattern
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         edx, texWidth
        sub         edx, width
        shl         edx, 2
        movd        mm1, col
    plot:
        mov         ecx, x
        and         ecx, 7
        mov         ebx, height
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        test        al, 1
        jz          skip1
        test        al, 2
        jz          skip2
        movq        mm0, [edi]
        paddusb     mm0, mm1
        movq        [edi], mm0
        jmp         skip0
    skip2:
        movd        mm0, [edi + 4]
        paddusb     mm0, mm1
        movd        [edi + 4], mm0
        jmp         skip0
    skip1:
        test        al, 2
        jz          skip0
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
    skip0:
        add         edi, 8
        rol         al, 2
        dec         ecx
        jnz         next
    once:
        test        width, 1
        jz          end1
        test        al, 2
        jz          end0
        movd        mm0, [edi]
        paddusb     mm0, mm1
        movd        [edi], mm0
    end0:
        add         edi, 4
    end1:
        add         edi, edx
        dec         height
        jnz         plot
        emms
    }
#else
    //convert to byte color
    const ARGB* pcol = (const ARGB*)&col;
    const int32_t addOfs = texWidth - width;

    //calculate starting address
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];
    
    //start scan line
    for (int32_t i = 0; i < height; i++)
    {
        uint8_t pat = pattern[i & 7];
        pat = _rotl8(pat, 7);
        for (int32_t j = 0; j < width; j++)
        {
            if (pat & 1)
            {
                pixels->r = min(pixels->r + pcol->r, 255);
                pixels->g = min(pixels->g + pcol->g, 255);
                pixels->b = min(pixels->b + pcol->b, 255);
            }
            pat = _rotl8(pat, 1);
            pixels++;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectPatternSub(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, const uint8_t* pattern)
{
#ifdef _USE_ASM
    __asm {
        mov         esi, pattern
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         edx, texWidth
        sub         edx, width
        shl         edx, 2
        movd        mm1, col
    plot:
        mov         ecx, x
        and         ecx, 7
        mov         ebx, height
        and         ebx, 7
        mov         al, [esi + ebx]
        rol         al, cl
        mov         ecx, width
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    next:
        test        al, 1
        jz          skip1
        test        al, 2
        jz          skip2
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        jmp         skip0
    skip2:
        movd        mm0, [edi + 4]
        psubusb     mm0, mm1
        movd        [edi + 4], mm0
        jmp         skip0
    skip1:
        test        al, 2
        jz          skip0
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    skip0:
        add         edi, 8
        rol         al, 2
        dec         ecx
        jnz         next
    once:
        test        width, 1
        jz          end1
        test        al, 2
        jz          end0
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    end0:
        add         edi, 4
    end1:
        add         edi, edx
        dec         height
        jnz         plot
        emms
    }
#else
    //convert to byte color
    const ARGB* pcol = (const ARGB*)&col;
    const int32_t addOfs = texWidth - width;

    //calculate starting address
    ARGB* pdata = (ARGB*)drawBuff;
    ARGB* pixels = &pdata[texWidth * y + x];

    //start scan line
    for (int32_t i = 0; i < height; i++)
    {
        uint8_t pat = pattern[i & 7];
        pat = _rotl8(pat, 7);
        for (int32_t j = 0; j < width; j++)
        {
            if (pat & 1)
            {
                pixels->r = max(pixels->r - pcol->r, 0);
                pixels->g = max(pixels->g - pcol->g, 0);
                pixels->b = max(pixels->b - pcol->b, 0);
            }
            pat = _rotl8(pat, 1);
            pixels++;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and blending pixel
void fillRectPatternAlpha(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t argb, const uint8_t* pattern)
{
#ifdef _USE_ASM
    __asm {
        mov         edi, drawBuff
        mov         eax, y
        mul         texWidth
        add         eax, x
        shl         eax, 2
        add         edi, eax
        mov         edx, texWidth
        sub         edx, width
        shl         edx, 2
        mov         esi, pattern
        mov         eax, argb
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
    plot:
        push        edx
        mov         ecx, x
        and         ecx, 7
        mov         ebx, height
        and         ebx, 7
        mov         dl, [esi + ebx]
        rol         dl, cl
        mov         ecx, width
    next:
        test        dl, 1
        jz          step
        movd        mm0, [edi]
        movd        mm1, argb
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
    step:
        add         edi, 4
        rol         dl, 1
        dec         ecx
        jnz         next
        pop         edx
        add         edi, edx
        dec         height
        jnz         plot
        emms
    }
#else
    //calculate alpha and adding offset for the next line
    const uint8_t cover = argb >> 24;
    const uint8_t rcover = 255 - cover;
    const int32_t addOfs = texWidth - width;

    //calculate starting address
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* pixels = &pdata[texWidth * y + x];

    for (int32_t i = 0; i < height; i++)
    {
        uint8_t pat = pattern[i & 7];
        pat = _rotl8(pat, 7);
        for (int32_t j = 0; j < width; j++)
        {
            if (pat & 1)
            {
                const uint32_t dst = *pixels;
                const uint32_t rb = ((dst & 0x00ff00ff) * rcover + (argb & 0x00ff00ff) * cover);
                const uint32_t ag = (((dst & 0xff00ff00) >> 8) * rcover + ((argb & 0xff00ff00) >> 8) * cover);
                *pixels = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
            }
            pat = _rotl8(pat, 1);
            pixels++;
        }
        if (addOfs > 0) pixels += addOfs;
    }
#endif
}

//fill rectangle with corners (x1,y1) and (width,height) and color
void fillRectPattern(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t col, const uint8_t* pattern, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //calculate new position
    const int32_t x1 = x + (width - 1);
    const int32_t y1 = y + (height - 1);

    //clip image to context boundaries
    const int32_t lx = max(x, cminX);
    const int32_t ly = max(y, cminY);
    const int32_t lx1 = min(x1, cmaxX);
    const int32_t ly1 = min(y1, cmaxY);

    //initialize loop variables
    const int32_t lwidth = (lx1 - lx) + 1;
    const int32_t lheight = (ly1 - ly) + 1;

    //check for loop
    if (lwidth <= 0 || lheight <= 0) return;

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        fillRectPatternMix(lx, ly, lwidth, lheight, col, pattern);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        fillRectPatternNormal(lx, ly, lwidth, lheight, col, pattern);
        break;

    case BLEND_MODE_ADD:
        fillRectPatternAdd(lx, ly, lwidth, lheight, col, pattern);
        break;

    case BLEND_MODE_SUB:
        fillRectPatternSub(lx, ly, lwidth, lheight, col, pattern);
        break;

    case BLEND_MODE_ALPHA:
        fillRectPatternAlpha(lx, ly, lwidth, lheight, col, pattern);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//get current pattern type
uint8_t* getPattern(int32_t type)
{
    switch (type)
    {
    case PATTERN_TYPE_LINE:
        return ptnLine;

    case PATTERN_TYPE_LITE_SLASH:
        return ptnLiteSlash;

    case PATTERN_TYPE_SLASH:
        return ptnSlash;

    case PATTERN_TYPE_BACK_SLASH:
        return ptnBackSlash;

    case PATTERN_TYPE_LITE_BACK_SLASH:
        return ptnLiteBackSlash;

    case PATTERN_TYPE_HATCH:
        return ptnHatch;

    case PATTERN_TYPE_HATCH_X:
        return ptnHatchX;

    case PATTERN_TYPE_INTER_LEAVE:
        return ptnInterLeave;

    case PATTERN_TYPE_WIDE_DOT:
        return ptnWideDot;

    case PATTERN_TYPE_CLOSE_DOT:
        return ptnCloseDot;

    default:
        messageBox(GFX_ERROR, "Unknown pattern type:%d", type);
        return NULL;
        break;
    }
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
void clipLine(int32_t* xs, int32_t* ys, int32_t* xe, int32_t* ye)
{
    int32_t accept = 0, done = 0;
    int32_t x1 = *xs, x2 = *xe, y1 = *ys, y2 = *ye;

    //the region out codes for the endpoints
    int32_t code1 = getCode(x1, y1);
    int32_t code2 = getCode(x2, y2);

    //in theory, this can never end up in an infinite loop, it'll always come in one of the trivial cases eventually
    do {
        //accept because both endpoints are in screen or on the border, trivial accept
        if (!(code1 | code2)) accept = done = 1;

        //the line isn't visible on screen, trivial reject
        else if (code1 & code2) done = 1;

        //if no trivial reject or accept, continue the loop
        else
        {
            int32_t x = 0, y = 0;
            const int32_t codeout = code1 ? code1 : code2;

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

//Wu's line from (x1,y1) to (x2,y2) with anti-aliased
void drawLineAA(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t argb)
{
    const int32_t dx = abs(x1 - x0);
    const int32_t sx = x0 < x1 ? 1 : -1;
    const int32_t dy = abs(y1 - y0);
    const int32_t sy = y0 < y1 ? 1 : -1;
    const int32_t ed = ((dx + dy) == 0) ? 1 : int32_t(sqrt(sqr(double(dx)) + sqr(double(dy))));

    int32_t err = dx - dy;

    while (1)
    {
        putPixel(x0, y0, rgba(argb, 255 * abs(err - dx + dy) / ed), BLEND_MODE_ANTIALIASED);
        const int32_t e2 = err;
        const int32_t x2 = x0;

        if (2 * e2 >= -dx)
        {
            if (x0 == x1) break;
            if (e2 + dy < ed) putPixel(x0, y0 + sy, rgba(argb, 255 * (e2 + dy) / ed), BLEND_MODE_ANTIALIASED);
            err -= dy;
            x0 += sx;
        }

        if (2 * e2 <= dy)
        {
            if (y0 == y1) break;
            if (dx - e2 < ed) putPixel(x2 + sx, y0, rgba(argb, 255 * (dx - e2) / ed), BLEND_MODE_ANTIALIASED);
            err += dx;
            y0 += sy;
        }
    }
}

//Bresenham line from (x1,y1) to (x2,y2) with color and mode
void drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //alpha mode
    if (mode == BLEND_MODE_ANTIALIASED)
    {
        drawLineAA(x1, y1, x2, y2, color);
        return;
    }

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
        putPixel(step ? y : x, step ? x : y, color, mode);
        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

//Bresenham diagonal line from(x1, y1) to (x2, y2) with added background color
void drawLineBob(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    //this function only support mixed mode
    if (bitsPerPixel != 8) return;

    //range check
    if (x1 < 0 || x1 > cmaxX || x2 < 0 || x2 > cmaxX || y1 < 0 || y1 > cmaxY || y2 < 0 || y2 > cmaxY) return;

#ifdef _USE_ASM    
    int32_t dst = 0, sc = 0, dc = 0;
    int32_t adddx = 0, adddy = 0;
    int32_t addsx = 0, addsy = 0;
    void* plotpixel = putPixelBob;

    __asm {
        mov     ecx, 1
        mov     edx, 1
        mov     edi, y2
        sub     edi, y1
        jge     keepy
        neg     edx
        neg     edi
    keepy:
        mov     adddy, edx
        mov     esi, x2
        sub     esi, x1
        jge     keepx
        neg     ecx
        neg     esi
    keepx:
        mov     adddx, ecx
        cmp     esi, edi
        jge     horiz
        xor     ecx, ecx
        xchg    esi, edi
        jmp     saves
    horiz:
        xor     edx, edx
    saves:
        mov     dst, edi
        mov     addsx, ecx
        mov     addsy, edx
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
        call    plotpixel
        pop     ecx
        pop     edx
        cmp     ebx, 0
        jge     dline
        add     ecx, addsx
        add     edx, addsy
        add     ebx, sc
        jmp     again
    dline:
        add     ecx, adddx
        add     edx, adddy
        add     ebx, dc
        jmp     again
    done:
    }
#else
    const int32_t dx = abs(x2 - x1);
    const int32_t dy = abs(y2 - y1);

    int32_t x = x1;
    int32_t y = y1;
    int32_t addx1 = 0, addx2 = 0;
    int32_t addy1 = 0, addy2 = 0;
    int32_t numpixels = 0, curpixel = 0;
    int32_t den = 0, num = 0, numadd = 0;

    if (x2 >= x1)
    {
        addx1 = 1;
        addx2 = 1;
    }
    else
    {
        addx1 = -1;
        addx2 = -1;
    }

    if (y2 >= y1)
    {
        addy1 = 1;
        addy2 = 1;
    }
    else
    {
        addy1 = -1;
        addy2 = -1;
    }

    if (dx >= dy)
    {
        addx1 = 0;
        addy2 = 0;
        den = dx;
        num = dx >> 1;
        numadd = dy;
        numpixels = dx;
    }
    else
    {
        addx2 = 0;
        addy1 = 0;
        den = dy;
        num = dy >> 1;
        numadd = dx;
        numpixels = dy;
    }

    for (curpixel = 0; curpixel < numpixels; curpixel++)
    {
        putPixelBob(x, y);
        num += numadd;
        if (num >= den)
        {
            num -= den;
            x += addx1;
            y += addy1;
        }
        x += addx2;
        y += addy2;
    }
#endif
}

//pre-calculate lookup table for filled-circle
void calcCircle(int32_t rd, int32_t* points)
{
    //validate radius
    if (rd <= 0) return;

#ifdef _USE_ASM
    __asm {
        mov     ebx, 1
        sub     ebx, rd
        mov     edi, points
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
        points[edi] = ecx;
        points[esi] = eax;
    }
#endif
}

//pre-calculate lookup table for filled-ellipse
void calcEllipse(int32_t rx, int32_t ry, int32_t* points)
{
    int32_t ra = 0, aa = 0, bb = 0;
    int32_t xa = 0, mx = 0, my = 0;
    int32_t aq = 0, bq = 0, xd = 0, yd = 0;

    //validate radius
    if (rx <= 0 || ry <= 0) return;

#ifdef _USE_ASM
    __asm {
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
        mov     edi, points
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
        points[ry - my] = -mx;
    }
#endif
}

//Wu's circle with anti-aliased
void drawCircleAA(int32_t xm, int32_t ym, int32_t rad, uint32_t argb)
{
    int32_t y = 0;
    int32_t x = -rad;
    int32_t err = (1 - rad) << 1;

    if (rad <= 0) return;

    rad = 1 - err;

    do {
        int32_t alpha = 255 * abs(err - 2 * (x + y) - 2) / rad;
        uint32_t col = rgba(argb, alpha);
        putPixel(xm - x, ym + y, col, BLEND_MODE_ANTIALIASED);
        putPixel(xm - y, ym - x, col, BLEND_MODE_ANTIALIASED);
        putPixel(xm + x, ym - y, col, BLEND_MODE_ANTIALIASED);
        putPixel(xm + y, ym + x, col, BLEND_MODE_ANTIALIASED);

        const int32_t x2 = x;
        const int32_t e2 = err;
        
        if (err + y > 0)
        {
            alpha = 255 * (err - 2 * x - 1) / rad;
            if (alpha < 256)
            {
                col = rgba(argb, alpha);
                putPixel(xm - x, ym + y + 1, col, BLEND_MODE_ANTIALIASED);
                putPixel(xm - y - 1, ym - x, col, BLEND_MODE_ANTIALIASED);
                putPixel(xm + x, ym - y - 1, col, BLEND_MODE_ANTIALIASED);
                putPixel(xm + y + 1, ym + x, col, BLEND_MODE_ANTIALIASED);
            }

            err += ++x * 2 + 1;
        }

        if (e2 + x2 <= 0)
        {
            alpha = 255 * (2 * y + 3 - e2) / rad;
            if (alpha < 256)
            {
                col = rgba(argb, alpha);
                putPixel(xm - x2 - 1, ym + y, col, BLEND_MODE_ANTIALIASED);
                putPixel(xm - y, ym - x2 - 1, col, BLEND_MODE_ANTIALIASED);
                putPixel(xm + x2 + 1, ym - y, col, BLEND_MODE_ANTIALIASED);
                putPixel(xm + y, ym + x2 + 1, col, BLEND_MODE_ANTIALIASED);
            }
            err += ++y * 2 + 1;
        }
    } while (x < 0);
}

//Bresenham circle at (xc,yc) with radius and color
void drawCircle(int32_t xc, int32_t yc, int32_t rad, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //range checking
    if (rad <= 0) return;

    //alpha mode
    if (mode == BLEND_MODE_ANTIALIASED)
    {
        drawCircleAA(xc, yc, rad, color);
        return;
    }

    int32_t x = 0, y = rad;
    int32_t p = 1 - rad;

    while (x <= y)
    {
        putPixel(xc + x, yc + y, color, mode);
        putPixel(xc - x, yc + y, color, mode);
        putPixel(xc + x, yc - y, color, mode);
        putPixel(xc - x, yc - y, color, mode);
        putPixel(xc + y, yc + x, color, mode);
        putPixel(xc - y, yc + x, color, mode);
        putPixel(xc + y, yc - x, color, mode);
        putPixel(xc - y, yc - x, color, mode);
        if (p < 0) p += (x++ << 1) + 3;
        else p += ((x++ - y--) << 1) + 5;
    }
}

//Wu's ellipse with anti-aliased
void drawEllipseAA(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t argb)
{
    int32_t f = 0;
    uint32_t col = 0;
    double alpha = 0;

    //only 32bit support alpha-blend mode
    if (bitsPerPixel <= 8) return;
    if (x0 <= 0 || y0 <= 0 || x1 <= 0 || y1 <= 0) return;

    int32_t a = abs(x1 - x0);
    const int32_t b = abs(y1 - y0);
    
    //check for line
    if (a <= 0 || b <= 0) return;

    int32_t b1 = b & 1;
    double dx = 4.0 * (a - 1.0) * b * b;
    double dy = 4.0 * (b1 + 1.0) * a * a;
    double err = double(b1) * a * a - dx + dy;

    if (x0 > x1)
    {
        x0 = x1;
        x1 += a;
    }

    if (y0 > y1) y0 = y1;

    y0 += ((b + 1) >> 1);
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
        col = rgba(argb, uint8_t(alpha));
        putPixel(x0, y0, col, BLEND_MODE_ANTIALIASED);
        putPixel(x0, y1, col, BLEND_MODE_ANTIALIASED);
        putPixel(x1, y0, col, BLEND_MODE_ANTIALIASED);
        putPixel(x1, y1, col, BLEND_MODE_ANTIALIASED);

        if ((f = (2 * err + dy) >= 0))
        {
            if (x0 >= x1) break;
            alpha = ed * (err + dx);
            if (alpha < 255)
            {
                col = rgba(argb, uint8_t(alpha));
                putPixel(x0, y0 + 1, col, BLEND_MODE_ANTIALIASED);
                putPixel(x0, y1 - 1, col, BLEND_MODE_ANTIALIASED);
                putPixel(x1, y0 + 1, col, BLEND_MODE_ANTIALIASED);
                putPixel(x1, y1 - 1, col, BLEND_MODE_ANTIALIASED);
            }
        }

        if (2 * err <= dx)
        {
            alpha = ed * (dy - err);
            if (alpha < 255)
            {
                col = rgba(argb, uint8_t(alpha));
                putPixel(x0 + 1, y0, col, BLEND_MODE_ANTIALIASED);
                putPixel(x1 - 1, y0, col, BLEND_MODE_ANTIALIASED);
                putPixel(x0 + 1, y1, col, BLEND_MODE_ANTIALIASED);
                putPixel(x1 - 1, y1, col, BLEND_MODE_ANTIALIASED);
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
            col = rgba(argb, uint8_t(alpha));
            putPixel(x0, ++y0, col, BLEND_MODE_ANTIALIASED);
            putPixel(x1, y0, col, BLEND_MODE_ANTIALIASED);
            putPixel(x0, --y1, col, BLEND_MODE_ANTIALIASED);
            putPixel(x1, y1, col, BLEND_MODE_ANTIALIASED);
            err += dy += a;
        }
    }
}

//Bresenham drawing an ellipses with sub color
void drawEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //range checking
    if (ra <= 0 || rb <= 0) return;

    //alpha mode?
    if (mode == BLEND_MODE_ANTIALIASED)
    {
        drawEllipseAA(xc, yc, ra, rb, color);
        return;
    }

    int32_t mx1 = xc - ra;
    int32_t mx2 = xc + ra;
    int32_t my1 = yc;
    int32_t my2 = yc;

    putPixel(mx2, yc, color, mode);
    putPixel(mx1, yc, color, mode);

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

        putPixel(mx1, my1, color, mode);
        putPixel(mx1, my2, color, mode);
        putPixel(mx2, my1, color, mode);
        putPixel(mx2, my2, color, mode);
    }
}

//rectangle with corners (x1,y1) and (width,height) and color
void drawRect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    horizLine(x, y, width, color, mode);
    vertLine(x, y, height, color, mode);
    horizLine(x, y + height - 1, width, color, mode);
    vertLine(x + width - 1, y, height, color, mode);
}

//draw rectangle with rounded border and color
void drawRoundRect(int32_t x, int32_t y, int32_t width, int32_t height, int32_t rad, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    int32_t i = 0, j = 0;
    int32_t points[500] = { 0 };

    const int32_t x1 = x + width - 1;
    const int32_t y1 = y + height - 1;
    const int32_t mid = height >> 1;

    if (rad >= mid - 1) rad = mid - 1;

    calcCircle(rad, points);

    horizLine(x + rad - points[0], y + 1, width - ((rad - points[0]) << 1), col, mode);
    vertLine(x, y + rad, height - (rad << 1), col, mode);
    horizLine(x + rad - points[0], y1 - 1, width - ((rad - points[0]) << 1), col, mode);
    vertLine(x1, y + rad, height - (rad << 1), col, mode);

    for (i = 1; i <= rad; i++)
    {
        for (j = rad - points[i]; j <= rad - points[i - 1]; j++)
        {
            putPixel(x + j, y + i, col, mode);
            putPixel(x1 - j, y + i, col, mode);
        }
    }

    for (i = 1; i <= rad; i++)
    {
        for (j = rad - points[i]; j <= rad - points[i - 1]; j++)
        {
            putPixel(x + j, y1 - i, col, mode);
            putPixel(x1 - j, y1 - i, col, mode);
        }
    }
}

//draw boxed with color
void drawBox(int32_t x, int32_t y, int32_t width, int32_t height, int32_t dx, int32_t dy, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    const int32_t x1 = x + width - 1;
    const int32_t y1 = y + height - 1;
    const int32_t x0 = x + dx;
    const int32_t y0 = y - dy;
    const int32_t x2 = x1 + dx;
    const int32_t y2 = y1 - dy;

    drawRect(x, y, width, height, col, mode);
    drawRect(x0, y0, width, height, col, mode);
    drawLine(x, y, x0, y0, col, mode);
    drawLine(x1, y, x2, y0, col, mode);
    drawLine(x1, y1, x2, y2, col, mode);
    drawLine(x, y1, x0, y2, col, mode);
}

//draw anti-aliased line with width
void drawLineWidthAA(int32_t x0, int32_t y0, int32_t x1, int32_t y1, double wd, uint32_t col)
{
    const int32_t dx = abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
    const int32_t dy = abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx - dy, e2 = 0, x2 = 0, y2 = 0;

    const double ed = (dx + dy == 0) ? 1 : sqrt(sqr(double(dx)) + sqr(double(dy)));

    wd = (wd + 1) / 2;

    while (true)
    {
        putPixel(x0, y0, rgba(col, uint8_t(max(0, 255 * (abs(err - dx + dy) / ed - wd + 1)))), BLEND_MODE_ALPHA);

        e2 = err;
        x2 = x0;

        if (2 * e2 >= -dx)
        {
            for (e2 += dy, y2 = y0; e2 < ed * wd && (y1 != y2 || dx > dy); e2 += dx) putPixel(x0, y2 += sy, rgba(col, uint8_t(max(0, 255 * (abs(e2) / ed - wd + 1)))), BLEND_MODE_ALPHA);
            if (x0 == x1) break;
            e2 = err;
            err -= dy;
            x0 += sx;
        }

        if (2 * e2 <= dy)
        {
            for (e2 = dx - e2; e2 < ed * wd && (x1 != x2 || dx < dy); e2 += dy) putPixel(x2 += sx, y0, rgba(col, uint8_t(max(0, 255 * (abs(e2) / ed - wd + 1)))), BLEND_MODE_ALPHA);
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
}

//draw anti-aliased full quad bezier segment
void drawQuadBezierSegAA(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    int32_t sx = x2 - x1, sy = y2 - y1;
    int32_t xx = x0 - x1, yy = y0 - y1, xy = 0;
    double dx = 0.0, dy = 0.0, err = 0.0, ed = 0.0;
    double cur = double(xx) * sy - double(yy) * sx;

    if (sqr(sx) + sqr(sy) > sqr(xx) + sqr(yy))
    {
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur;
    }

    if (cur != 0)
    {
        xx += sx;
        xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy;
        yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;

        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * sy * (intmax_t(x1) - x0) * cur + xx - xy;
        dy = 4.0 * sx * (intmax_t(y0) - y1) * cur + yy - xy;
        xx += xx;
        yy += yy;
        err = dx + dy + xy;

        do {
            cur = min(dx + xy, -xy - dy);
            ed = max(dx + xy, -xy - dy);
            ed += 2 * ed * sqr(cur) / (4 * sqr(ed) + sqr(cur));
            putPixel(x0, y0, rgba(col, uint8_t(255 * fabs(err - dx - dy - xy) / ed)), BLEND_MODE_ANTIALIASED);

            if (x0 == x2 || y0 == y2) break;

            x1 = x0;
            cur = dx - err;
            y1 = 2 * err + dy < 0;

            if (2 * err + dx > 0)
            {
                if (err - dy < ed) putPixel(x0, y0 + sy, rgba(col, uint8_t(255 * fabs(err - dy) / ed)), BLEND_MODE_ANTIALIASED);
                x0 += sx;
                dx -= xy;
                err += dy += yy;
            }

            if (y1)
            {
                if (cur < ed) putPixel(x1 + sx, y0, rgba(col, uint8_t(255 * fabs(cur) / ed)), BLEND_MODE_ANTIALIASED);
                y0 += sy;
                dy -= xy;
                err += dx += xx;
            }
        } while (dy < dx);
    }

    drawLine(x0, y0, x2, y2, col, BLEND_MODE_ANTIALIASED);
}

//draw a quad bezier segment
void drawQuadBezierSeg(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    //anti-aliased mode?
    if (mode == BLEND_MODE_ANTIALIASED)
    {
        drawQuadBezierSegAA(x0, y0, x1, y1, x2, y2, col);
        return;
    }

    int32_t sx = x2 - x1, sy = y2 - y1;
    int32_t xx = x0 - x1, yy = y0 - y1, xy = 0;
    double dx = 0.0, dy = 0.0, err = 0.0;
    double cur = double(xx) * sy - double(yy) * sx;

    if (sqr(sx) + sqr(sy) > sqr(xx) + sqr(yy))
    {
        x2 = x0;
        x0 = sx + x1;
        y2 = y0;
        y0 = sy + y1;
        cur = -cur;
    }

    if (cur != 0)
    {
        xx += sx;
        xx *= sx = x0 < x2 ? 1 : -1;
        yy += sy;
        yy *= sy = y0 < y2 ? 1 : -1;
        xy = 2 * xx * yy;
        xx *= xx;
        yy *= yy;

        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * sy * cur * (intmax_t(x1) - x0) + xx - xy;
        dy = 4.0 * sx * cur * (intmax_t(y0) - y1) + yy - xy;
        xx += xx;
        yy += yy;
        err = dx + dy + xy;

        do {
            putPixel(x0, y0, col, mode);

            if (x0 == x2 && y0 == y2) return;
            
            y1 = 2 * err < dx;
            
            if (2 * err > dy)
            {
                x0 += sx;
                dx -= xy;
                err += dy += yy;
            }
            
            if (y1)
            {
                y0 += sy;
                dy -= xy;
                err += dx += xx;
            }
        } while (dy < 0 && dx > 0);
    }

    drawLine(x0, y0, x2, y2, col, mode);
}

//draw anti-aliased cubic bezier segment
void drawCubicBezierSegAA(int32_t x0, int32_t y0, double x1, double y1, double x2, double y2, int32_t x3, int32_t y3, uint32_t col)
{
    int32_t f = 0, fx = 0, fy = 0, leg = 1;
    int32_t sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;

    const double EP = 0.01;
    const double xc = -fabs(x0 + x1 - x2 - x3);
    const double xa = xc - 4.0 * sx * (x1 - x2);
    const double yc = -fabs(y0 + y1 - y2 - y3);
    const double ya = yc - 4.0 * sy * (y1 - y2);

    double xb = sx * (x0 - x1 - x2 + x3);
    double yb = sy * (y0 - y1 - y2 + y3);

    double ip = 0.0;
    double ab = 0.0, ac = 0.0, bc = 0.0, ba = 0.0;
    double xx = 0.0, xy = 0.0, yy = 0.0;
    double dx = 0.0, dy = 0.0, ex = 0.0;
    double px = 0.0, py = 0.0, ed = 0.0;

    if (xa == 0 && ya == 0)
    {
        sx = int32_t(floor((3 * x1 - x0 + 1) / 2));
        sy = int32_t(floor((3 * y1 - y0 + 1) / 2));
        drawQuadBezierSeg(x0, y0, sx, sy, x3, y3, col, BLEND_MODE_ANTIALIASED);
        return;
    }

    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;

    do {
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ip = 4 * ab * bc - sqr(ac);
        ex = ab * (ab + ac - 3 * bc) + sqr(ac);
        f = (ex > 0) ? 1 : int32_t(sqrt(1 + 1024 / x1));
        ab *= f;
        ac *= f;
        bc *= f;
        ex *= sqr(intmax_t(f));
        xy = 9 * (ab + ac + bc) / 8;
        ba = 8 * (xa - ya);
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * ba)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * ba)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + ba);
        ac = ya * ya;
        ba = xa * xa;
        xy = 3 * (xy + 9.0 * f * (ba * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0)
        {
            dx = -dx;
            dy = -dy;
            xx = -xx;
            yy = -yy;
            xy = -xy;
            ac = -ac;
            ba = -ba;
        }

        ab = 6 * ya * ac;
        ac = -6 * xa * ac;
        bc = 6 * ya * ba;
        ba = -6 * xa * ba;
        
        dx += xy;
        ex = dx + dy;
        dy += xy;

        for (fx = fy = f; x0 != x3 && y0 != y3;)
        {
            y1 = min(fabs(xy - dx), fabs(dy - xy));
            ed = max(fabs(xy - dx), fabs(dy - xy));
            ed = f * (ed + 2 * ed * y1 * y1 / (4 * ed * ed + y1 * y1));

            y1 = 255 * fabs(ex - (intmax_t(f) - fx + 1) * dx - (intmax_t(f) - fy + 1) * dy + f * xy) / ed;
            if (y1 < 256) putPixel(x0, y0, rgba(col, uint8_t(y1)), BLEND_MODE_ANTIALIASED);

            px = fabs(ex - (intmax_t(f) - fx + 1) * dx + (intmax_t(fy) - 1) * dy);
            py = fabs(ex + (intmax_t(fx) - 1) * dx - (intmax_t(f) - fy + 1) * dy);
            y2 = y0;

            do {
                if ((ip >= -EP) && (dx + xx > xy || dy + yy < xy)) goto exit;
                y1 = 2 * ex + dx;
                if (2 * ex + dy > 0)
                {
                    fx--;
                    ex += dx += xx;
                    dy += xy += ac;
                    yy += bc;
                    xx += ab;
                }
                else if (y1 > 0) goto exit;

                if (y1 <= 0)
                {
                    fy--;
                    ex += dy += yy;
                    dx += xy += bc;
                    xx += ac;
                    yy += ba;
                }
            } while (fx > 0 && fy > 0);

            if (2 * fy <= f)
            {
                if (py < ed) putPixel(x0 + sx, y0, rgba(col, uint8_t(255 * py / ed)), BLEND_MODE_ANTIALIASED);
                
                y0 += sy;
                fy += f;
            }

            if (2 * fx <= f)
            {
                if (px < ed) putPixel(x0, int32_t(y2) + sy, rgba(col, uint8_t(255 * px / ed)), BLEND_MODE_ANTIALIASED);
                
                x0 += sx;
                fx += f;
            }
        }

    exit:
        if (2 * ex < dy && 2 * fy <= f + 2)
        {
            if (py < ed) putPixel(x0 + sx, y0, rgba(col, uint8_t(255 * py / ed)), BLEND_MODE_ANTIALIASED);
            y0 += sy;
        }

        if (2 * ex > dx && 2 * fx <= f + 2)
        {
            if (px < ed) putPixel(x0, int32_t(y2) + sy, rgba(col, uint8_t(255 * px / ed)), BLEND_MODE_ANTIALIASED);
            x0 += sx;
        }

        xx = x0;
        x0 = x3;
        x3 = int32_t(xx);
        
        sx = -sx;
        xb = -xb;
     
        yy = y0;
        y0 = y3;
        y3 = int32_t(yy);

        sy = -sy;
        yb = -yb;
        x1 = x2;
    } while (leg--);

    drawLine(x0, y0, x3, y3, col, BLEND_MODE_ANTIALIASED);
}

//draw full quad bezier (export function)
void drawQuadBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    int32_t x = x0 - x1, y = y0 - y1;
    double t = x0 - 2.0 * x1 + x2, r = 0.0;

    if (x * (x2 - x1) > 0)
    {
        if ((y * (y2 - y1) > 0) && fabs((y0 - 2.0 * y1 + y2) / t * x) > abs(y))
        {
            x0 = x2;
            x2 = x + x1;
            y0 = y2;
            y2 = y + y1;
        }

        t = (intmax_t(x0) - x1) / t;
        r = (1 - t) * ((1 - t) * y0 + 2.0 * t * y1) + t * t * y2;
        t = (intmax_t(x0) * x2 - intmax_t(x1) * x1) * t / (intmax_t(x0) - x1);
        x = int32_t(floor(t + 0.5));
        y = int32_t(floor(r + 0.5));
        r = (intmax_t(y1) - y0) * (t - x0) / (intmax_t(x1) - x0) + y0;
        
        drawQuadBezierSeg(x0, y0, x, int32_t(floor(r + 0.5)), x, y, col, mode);
        
        r = (intmax_t(y1) - y2) * (t - x2) / (intmax_t(x1) - x2) + y2;
        x0 = x1 = x;
        y0 = y;
        y1 = int32_t(floor(r + 0.5));
    }

    if ((y0 - y1) * (y2 - y1) > 0)
    {
        t = y0 - 2.0 * y1 + y2;
        t = (intmax_t(y0) - y1) / t;
        r = (1 - t) * ((1 - t) * x0 + 2.0 * t * x1) + t * t * x2;
        t = (intmax_t(y0) * y2 - intmax_t(y1) * y1) * t / (intmax_t(y0) - y1);
        x = int32_t(floor(r + 0.5));
        y = int32_t(floor(t + 0.5));
        r = (intmax_t(x1) - x0) * (t - y0) / (intmax_t(y1) - y0) + x0;

        drawQuadBezierSeg(x0, y0, int32_t(floor(r + 0.5)), y, x, y, col, mode);
        
        r = (intmax_t(x1) - x2) * (t - y2) / (intmax_t(y1) - y2) + x2;
        x0 = x;
        x1 = int32_t(floor(r + 0.5));
        y0 = y1 = y;
    }

    drawQuadBezierSeg(x0, y0, x1, y1, x2, y2, col, mode);
}

//draw anti-aliased rotation quad bezier segment
void drawQuadRationalBezierSegAA(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, uint32_t col)
{
    int32_t f = 0;
    int32_t sx = x2 - x1, sy = y2 - y1;

    double err = 0.0, ed = 0.0;
    double dx = double(x0) - x2, dy = double(y0) - y2;
    double xx = double(x0) - x1, yy = double(y0) - y1;
    double xy = xx * sy + yy * sx, cur = xx * sy - yy * sx;
    
    if (cur != 0.0 && w > 0.0)
    {
        if (sqr(double(sx)) + sqr(double(sy)) > sqr(xx) + sqr(yy))
        {
            x2 = x0;
            x0 -= int32_t(dx);
            y2 = y0;
            y0 -= int32_t(dy);
            cur = -cur;
        }

        xx = 2.0 * (4.0 * w * sx * xx + sqr(dx));
        yy = 2.0 * (4.0 * w * sy * yy + sqr(dy));
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0 * sx * sy * (2.0 * w * xy + dx * dy);

        if (cur * sx * sy < 0)
        {
            xx = -xx;
            yy = -yy;
            cur = -cur;
            xy = -xy;
        }

        dx = 4.0 * w * (intmax_t(x1) - x0) * sy * cur + xx / 2.0 + xy;
        dy = 4.0 * w * (intmax_t(y0) - y1) * sx * cur + yy / 2.0 + xy;

        if (w < 0.5 && dy > dx)
        {
            cur = (w + 1.0) / 2.0;
            w = sqrt(w);
            xy = 1.0 / (w + 1.0);
            
            sx = int32_t(floor((x0 + 2.0 * w * x1 + x2) * xy / 2.0 + 0.5));
            sy = int32_t(floor((y0 + 2.0 * w * y1 + y2) * xy / 2.0 + 0.5));
            
            dx = floor((w * x1 + x0) * xy + 0.5);
            dy = floor((y1 * w + y0) * xy + 0.5);
            drawQuadRationalBezierSegAA(x0, y0, int32_t(dx), int32_t(dy), sx, sy, cur, col);
            
            dx = floor((w * x1 + x2) * xy + 0.5);
            dy = floor((y1 * w + y2) * xy + 0.5);
            drawQuadRationalBezierSegAA(sx, sy, int32_t(dx), int32_t(dy), x2, y2, cur, col);
            
            return;
        }

        err = dx + dy - xy;

        do {
            cur = min(dx - xy, xy - dy);
            ed = max(dx - xy, xy - dy);
            ed += 2 * ed * sqr(cur) / (4.0 * sqr(ed) + sqr(cur));

            x1 = int32_t(255 * fabs(err - dx - dy + xy) / ed);
            if (x1 < 256) putPixel(x0, y0, rgba(col, x1), BLEND_MODE_ANTIALIASED);

            if ((f = 2 * err + dy < 0))
            {
                if (y0 == y2) return;
                if (dx - err < ed) putPixel(x0 + sx, y0, rgba(col, uint8_t(255 * fabs(dx - err) / ed)), BLEND_MODE_ANTIALIASED);
            }

            if (2 * err + dx > 0)
            {
                if (x0 == x2) return;
                if (err - dy < ed) putPixel(x0, y0 + sy, rgba(col, uint8_t(255 * fabs(err - dy) / ed)), BLEND_MODE_ANTIALIASED);
                
                x0 += sx;
                dx += xy;
                err += dy += yy;
            }

            if (f)
            {
                y0 += sy;
                dy += xy;
                err += dx += xx;
            }
        } while (dy < dx);
    }

    drawLine(x0, y0, x2, y2, col, BLEND_MODE_ANTIALIASED);
}

//draw rotation quad bezier segment
void drawQuadRationalBezierSeg(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    //anti-aliased mode
    if (mode == BLEND_MODE_ANTIALIASED)
    {
        drawQuadRationalBezierSegAA(x0, y0, x1, y1, x2, y2, w, col);
        return;
    }

    int32_t sx = x2 - x1, sy = y2 - y1;

    double dx = double(x0) - x2, dy = double(y0) - y2;
    double xx = double(x0) - x1, yy = double(y0) - y1;

    double err = 0.0;
    double xy = xx * sy + yy * sx, cur = xx * sy - yy * sx;

    if (cur != 0.0 && w > 0.0)
    {
        if (sqr(double(sx)) + sqr(double(sy)) > sqr(xx) + sqr(yy))
        {
            x2 = x0;
            x0 -= int32_t(dx);
            y2 = y0;
            y0 -= int32_t(dy);
            cur = -cur;
        }

        xx = 2.0 * (4.0 * w * sx * xx + sqr(dx));
        yy = 2.0 * (4.0 * w * sy * yy + sqr(dy));
        sx = x0 < x2 ? 1 : -1;
        sy = y0 < y2 ? 1 : -1;
        xy = -2.0 * sx * sy * (2.0 * w * xy + dx * dy);

        if (cur * sx * sy < 0.0)
        {
            xx = -xx;
            yy = -yy;
            xy = -xy;
            cur = -cur;
        }

        dx = 4.0 * w * (intmax_t(x1) - x0) * sy * cur + xx / 2.0 + xy;
        dy = 4.0 * w * (intmax_t(y0) - y1) * sx * cur + yy / 2.0 + xy;

        if (w < 0.5 && (dy > xy || dx < xy))
        {
            cur = (w + 1.0) / 2.0;
            w = sqrt(w);
            xy = 1.0 / (w + 1.0);
            
            sx = int32_t(floor((x0 + 2.0 * w * x1 + x2) * xy / 2.0 + 0.5));
            sy = int32_t(floor((y0 + 2.0 * w * y1 + y2) * xy / 2.0 + 0.5));
            
            dx = floor((w * x1 + x0) * xy + 0.5);
            dy = floor((y1 * w + y0) * xy + 0.5);
            drawQuadRationalBezierSeg(x0, y0, int32_t(dx), int32_t(dy), sx, sy, cur, col, mode);
            
            dx = floor((w * x1 + x2) * xy + 0.5);
            dy = floor((y1 * w + y2) * xy + 0.5);
            drawQuadRationalBezierSeg(sx, sy, int32_t(dx), int32_t(dy), x2, y2, cur, col, mode);
            
            return;
        }

        err = dx + dy - xy;

        do {
            putPixel(x0, y0, col, mode);

            if (x0 == x2 && y0 == y2) return;

            x1 = 2 * err > dy;
            y1 = 2 * (err + yy) < -dy;
            
            if (2 * err < dx || y1)
            {
                y0 += sy;
                dy += xy;
                err += dx += xx;
            }

            if (2 * err > dx || x1)
            {
                x0 += sx;
                dx += xy;
                err += dy += yy;
            }
        } while (dy <= xy && dx >= xy);
    }

    drawLine(x0, y0, x2, y2, col, mode);
}

//draw rotation full quad bezier segment (export function)
void drawQuadRationalBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, double w, int32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    int32_t x = x0 - 2 * x1 + x2;
    int32_t y = y0 - 2 * y1 + y2;

    double xx = double(x0) - x1;
    double yy = double(y0) - y1;
    double ww = 0.0, t = 0.0, q = 0.0;

    if (xx * (intmax_t(x2) - x1) > 0)
    {
        if (yy * (intmax_t(y2) - y1) > 0 && fabs(xx * y) > fabs(yy * x))
        {
            x0 = x2;
            x2 = int32_t(xx) + x1;
            y0 = y2;
            y2 = int32_t(yy) + y1;
        }

        if (x0 == x2 || w == 1.0) t = (intmax_t(x0) - x1) / double(x);
        else
        {
            q = sqrt(4.0 * w * w * (intmax_t(x0) - x1) * (intmax_t(x2) - x1) + (intmax_t(x2) - x0) * (intmax_t(x2) - x0));
            if (x1 < x0) q = -q;
            t = (2.0 * w * (intmax_t(x0) - x1) - x0 + x2 + q) / (2.0 * (1.0 - w) * (intmax_t(x2) - x0));
        }

        q = 1.0 / (2.0 * t * (1.0 - t) * (w - 1.0) + 1.0);
        xx = (t * t * (x0 - 2.0 * w * x1 + x2) + 2.0 * t * (w * x1 - x0) + x0) * q;
        yy = (t * t * (y0 - 2.0 * w * y1 + y2) + 2.0 * t * (w * y1 - y0) + y0) * q;
        ww = t * (w - 1.0) + 1.0;
        ww *= ww * q;
        w = ((1.0 - t) * (w - 1.0) + 1.0) * sqrt(q);
        x = int32_t(floor(xx + 0.5));
        y = int32_t(floor(yy + 0.5));
        yy = (xx - x0) * (intmax_t(y1) - y0) / (intmax_t(x1) - x0) + y0;
        
        drawQuadRationalBezierSeg(x0, y0, x, int32_t(floor(yy + 0.5)), x, y, ww, col, mode);
        
        yy = (xx - x2) * (intmax_t(y1) - y2) / (intmax_t(x1) - x2) + y2;
        y1 = int32_t(floor(yy + 0.5));
        x0 = x1 = x;
        y0 = y;
    }

    if ((y0 - y1) * (y2 - y1) > 0)
    {
        if (y0 == y2 || w == 1.0) t = (intmax_t(y0) - y1) / (y0 - 2.0 * y1 + y2);
        else
        {
            q = sqrt(4.0 * w * w * (intmax_t(y0) - y1) * (intmax_t(y2) - y1) + (intmax_t(y2) - y0) * (intmax_t(y2) - y0));
            if (y1 < y0) q = -q;
            t = (2.0 * w * (intmax_t(y0) - y1) - y0 + y2 + q) / (2.0 * (1.0 - w) * (intmax_t(y2) - y0));
        }

        q = 1.0 / (2.0 * t * (1.0 - t) * (w - 1.0) + 1.0);
        xx = (t * t * (x0 - 2.0 * w * x1 + x2) + 2.0 * t * (w * x1 - x0) + x0) * q;
        yy = (t * t * (y0 - 2.0 * w * y1 + y2) + 2.0 * t * (w * y1 - y0) + y0) * q;
        ww = t * (w - 1.0) + 1.0;
        ww *= ww * q;
        w = ((1.0 - t) * (w - 1.0) + 1.0) * sqrt(q);
        x = int32_t(floor(xx + 0.5));
        y = int32_t(floor(yy + 0.5));
        xx = (intmax_t(x1) - x0) * (yy - y0) / (intmax_t(y1) - y0) + x0;
        
        drawQuadRationalBezierSeg(x0, y0, int32_t(floor(xx + 0.5)), y, x, y, ww, col, mode);
        
        xx = (intmax_t(x1) - x2) * (yy - y2) / (intmax_t(y1) - y2) + x2;
        x1 = int32_t(floor(xx + 0.5));
        x0 = x;
        y0 = y1 = y;
    }

    drawQuadRationalBezierSeg(x0, y0, x1, y1, x2, y2, w * w, col, mode);
}

//draw rotation of ellipse with rect
void drawRotatedEllipseRect(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t zd, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    int32_t xd = x1 - x0, yd = y1 - y0;
    double w = double(xd) * yd;

    if (zd == 0)
    {
        drawEllipse(x0, y0, xd + 1, yd + 1, col, mode);
        return;
    }

    if (w != 0.0) w = (w - zd) / (w + w);

    xd = int32_t(floor(xd * w + 0.5));
    yd = int32_t(floor(yd * w + 0.5));

    drawQuadRationalBezierSeg(x0, y0 + yd, x0, y0, x0 + xd, y0, 1.0 - w, col, mode);
    drawQuadRationalBezierSeg(x0, y0 + yd, x0, y1, x1 - xd, y1, w, col, mode);
    drawQuadRationalBezierSeg(x1, y1 - yd, x1, y1, x1 - xd, y1, 1.0 - w, col, mode);
    drawQuadRationalBezierSeg(x1, y1 - yd, x1, y0, x0 + xd, y0, w, col, mode);
}

//draw rotation of ellipse (export function)
void drawRotatedEllipse(int32_t x, int32_t y, int32_t ra, int32_t rb, double angle, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    const double s = sin(angle);
    double xd = sqr(double(ra));
    double yd = sqr(double(rb));
    double zd = (xd - yd) * s;

    xd = sqrt(xd - zd * s);
    yd = sqrt(yd + zd * s);
    ra = int32_t(xd + 0.5);
    rb = int32_t(yd + 0.5);
    zd = zd * ra * rb / (xd * yd);

    drawRotatedEllipseRect(x - ra, y - rb, x + ra, y + rb, int32_t(4 * zd * cos(angle)), col, mode);
}

//draw cubic bezier segment
void drawCubicBezierSeg(int32_t x0, int32_t y0, double x1, double y1, double x2, double y2, int32_t x3, int32_t y3, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    //anti-aliased mode?
    if (mode == BLEND_MODE_ANTIALIASED)
    {
        drawCubicBezierSegAA(x0, y0, x1, y1, x2, y2, x3, y3, col);
        return;
    }

    int32_t leg = 1;
    int32_t f = 0, fx = 0, fy = 0;
    int32_t sx = x0 < x3 ? 1 : -1;
    int32_t sy = y0 < y3 ? 1 : -1;
    
    const double xc = -fabs(x0 + x1 - x2 - x3), xa = xc - 4.0 * sx * (x1 - x2);
    const double yc = -fabs(y0 + y1 - y2 - y3), ya = yc - 4.0 * sy * (y1 - y2);
    
    double* pxy = NULL, EP = 0.01;
    double xb = sx * (x0 - x1 - x2 + x3);
    double yb = sy * (y0 - y1 - y2 + y3);
    double ab = 0.0, ac = 0.0, bc = 0.0, cb = 0.0;
    double xx = 0.0, xy = 0.0, yy = 0.0;
    double dx = 0.0, dy = 0.0, ex = 0.0;
        
    if (xa == 0 && ya == 0)
    {
        sx = int32_t(floor((3 * x1 - x0 + 1) / 2));
        sy = int32_t(floor((3 * y1 - y0 + 1) / 2));
        drawQuadBezierSeg(x0, y0, sx, sy, x3, y3, col, mode);
        return;
    }

    x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
    x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;

    do {
        ab = xa * yb - xb * ya;
        ac = xa * yc - xc * ya;
        bc = xb * yc - xc * yb;
        ex = ab * (ab + ac - 3 * bc) + sqr(ac);
        f = (ex > 0) ? 1 : int32_t(sqrt(1 + 1024 / x1));

        ab *= f;
        ac *= f;
        bc *= f;
        ex *= intmax_t(f) * f;

        xy = 9 * (ab + ac + bc) / 8;
        cb = 8 * (xa - ya);
        dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
        dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

        xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * cb)) / 4;
        yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * cb)) / 4;
        xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + cb);
        
        ac = ya * ya;
        cb = xa * xa;
        xy = 3 * (xy + 9.0 * f * (cb * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

        if (ex < 0)
        {
            dx = -dx;
            dy = -dy;
            xx = -xx;
            yy = -yy;
            xy = -xy;
            ac = -ac;
            cb = -cb;
        }

        ab = 6 * ya * ac;
        ac = -6 * xa * ac;
        bc = 6 * ya * cb;
        cb = -6 * xa * cb;

        dx += xy;
        ex = dx + dy;
        dy += xy;

        for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3; )
        {
            putPixel(x0, y0, col, mode);

            do {
                if (dx > *pxy || dy < *pxy) goto exit;

                y1 = 2 * ex - dy;
                if (2 * ex >= dx)
                {
                    fx--;
                    ex += dx += xx;
                    dy += xy += ac;
                    yy += bc;
                    xx += ab;
                }

                if (y1 <= 0)
                {
                    fy--;
                    ex += dy += yy;
                    dx += xy += bc;
                    xx += ac;
                    yy += cb;
                }
            } while (fx > 0 && fy > 0);

            if (2 * fx <= f)
            {
                x0 += sx;
                fx += f;
            }

            if (2 * fy <= f)
            {
                y0 += sy;
                fy += f;
            }

            if (pxy == &xy && dx < 0 && dy > 0) pxy = &EP;
        }

    exit:
        xx = x0;
        x0 = x3;
        x3 = int32_t(xx);

        sx = -sx;
        xb = -xb;
        yy = y0;
        y0 = y3;
        y3 = int32_t(yy);
        sy = -sy;
        yb = -yb;
        x1 = x2;
    } while (leg--);

    drawLine(x0, y0, x3, y3, col, mode);
}

//draw cubic bezier (export function)
void drawCubicBezier(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint32_t col, int32_t mode /*= BLEND_MODE_NORMAL*/)
{
    int32_t n = 0, i = 0;
    const int32_t xc = x0 + x1 - x2 - x3, xa = xc - 4 * (x1 - x2);
    const int32_t xb = x0 - x1 - x2 + x3, xd = xb + 4 * (x1 + x2);
    const int32_t yc = y0 + y1 - y2 - y3, ya = yc - 4 * (y1 - y2);
    const int32_t yb = y0 - y1 - y2 + y3, yd = yb + 4 * (y1 + y2);

    double t2 = 0, t[10] = { 0 };
    double fx0 = x0, fx1 = 0.0, fx2 = 0.0, fx3 = 0.0;
    double fy0 = y0, fy1 = 0.0, fy2 = 0.0, fy3 = 0.0;
    double t1 = double(xb) * xb - double(xa) * xc;
    
    if (xa == 0)
    {
        if (abs(xc) < 2 * abs(xb)) t[n++] = xc / (2.0 * xb);
    }
    else if (t1 > 0.0)
    {
        t2 = sqrt(t1);
        
        t1 = (xb - t2) / xa;
        if (fabs(t1) < 1.0) t[n++] = t1;

        t1 = (xb + t2) / xa;
        if (fabs(t1) < 1.0) t[n++] = t1;
    }

    t1 = sqr(double(yb)) - double(ya) * yc;

    if (ya == 0)
    {
        if (abs(yc) < 2 * abs(yb)) t[n++] = yc / (2.0 * yb);
    }
    else if (t1 > 0.0)
    {
        t2 = sqrt(t1);
        t1 = (yb - t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
        t1 = (yb + t2) / ya; if (fabs(t1) < 1.0) t[n++] = t1;
    }

    for (i = 1; i != n; i++)
    {
        if ((t1 = t[i - 1]) > t[i])
        {
            t[i - 1] = t[i];
            t[i] = t1;
            i = 0;
        }
    }

    t1 = -1.0;
    t[n] = 1.0;

    for (i = 0; i <= n; i++)
    {
        t2 = t[i];
        fx1 = (t1 * (t1 * xb - 2.0 * xc) - t2 * (t1 * (t1 * xa - 2.0 * xb) + xc) + xd) / 8 - fx0;
        fy1 = (t1 * (t1 * yb - 2.0 * yc) - t2 * (t1 * (t1 * ya - 2.0 * yb) + yc) + yd) / 8 - fy0;
        fx2 = (t2 * (t2 * xb - 2.0 * xc) - t1 * (t2 * (t2 * xa - 2.0 * xb) + xc) + xd) / 8 - fx0;
        fy2 = (t2 * (t2 * yb - 2.0 * yc) - t1 * (t2 * (t2 * ya - 2.0 * yb) + yc) + yd) / 8 - fy0;
        fx0 -= fx3 = (t2 * (t2 * (3.0 * xb - t2 * xa) - 3.0 * xc) + xd) / 8;
        fy0 -= fy3 = (t2 * (t2 * (3.0 * yb - t2 * ya) - 3.0 * yc) + yd) / 8;
        x3 = int32_t(floor(fx3 + 0.5));
        y3 = int32_t(floor(fy3 + 0.5));
        
        if (fx0 != 0.0)
        {
            fx1 *= fx0 = (intmax_t(x0) - x3) / fx0;
            fx2 *= fx0;
        }

        if (fy0 != 0.0)
        {
            fy1 *= fy0 = (intmax_t(y0) - y3) / fy0;
            fy2 *= fy0;
        }
        
        if (x0 != x3 || y0 != y3) drawCubicBezierSeg(x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3, col, mode);
        
        x0 = x3;
        y0 = y3;
        fx0 = fx3;
        fy0 = fy3;
        t1 = t2;
    }
}

//draw boxed with rounded border
void drawRoundBox(int32_t x, int32_t y, int32_t width, int32_t height, int32_t rad, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    int32_t i = 0, j = 0;
    int32_t a = 0, b = 0;
    int32_t points[500] = { 0 };

    const int32_t x1 = x + width - 1;
    const int32_t y1 = y + height - 1;
    const int32_t mid = height >> 1;

    if (rad >= mid - 1) rad = mid - 1;

    calcCircle(rad, points);

    horizLine(x + rad - points[0], y + 1, width - ((rad - points[0]) << 1), col, mode);
    vertLine(x, y + rad, height - (rad << 1), col, mode);
    horizLine(x + rad - points[0], y1 - 1, width - ((rad - points[0]) << 1), col, mode);
    vertLine(x1, y + rad, height - (rad << 1), col, mode);

    for (i = 1; i <= rad; i++)
    {
        a = rad - points[i];
        b = rad - points[i - 1];
        for (j = a; j <= b; j++)
        {
            putPixel(x + j, y + i, col, mode);
            putPixel(x1 - j, y + i, col, mode);
        }
    }

    for (i = 1; i <= rad; i++)
    {
        a = rad - points[i];
        b = rad - points[i - 1];
        for (j = a; j <= b; j++)
        {
            putPixel(x + j, y1 - i, col, mode);
            putPixel(x1 - j, y1 - i, col, mode);
        }
    }

    for (i = 1; i <= rad; i++) horizLine(x + rad - points[i - 1] + 1, y + i, width - ((rad << 1) - (points[i - 1] << 1)) - 1, col, mode);
    fillRect(x + 1, y + rad + 1, width - 2, height - (rad << 1) - 2, col, mode);
    for (i = rad; i >= 1; i--) horizLine(x + rad - points[i - 1] + 1, y1 - i, width - ((rad << 1) - (points[i - 1] << 1)) - 1, col, mode);
}

//draw polygon
void drawPolygon(const POINT2D* points, int32_t num, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    if (num < 3) return;
    for (int32_t i = 0; i < num - 1; i++) drawLine(int32_t(points[i].x), int32_t(points[i].y), int32_t(points[i + 1].x), int32_t(points[i + 1].y), col, mode);
    drawLine(int32_t(points[num - 1].x), int32_t(points[num - 1].y), int32_t(points[0].x), int32_t(points[0].y), col, mode);
}

//fast filled Bresenham circle at (xc,yc) with radius and color
void fillCircle(int32_t xc, int32_t yc, int32_t radius, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    int32_t i = 0;
    int32_t points[500] = { 0 };

    //range limited
    if (radius <= 0) return;

    //out of range
    if (radius > 499)
    {
        messageBox(GFX_ERROR, "fillCircle: radius must be in [0-499] pixels");
        return;
    }

    int32_t mc = yc - radius;
    calcCircle(radius, points);

    for (i = 0; i <= radius - 1; i++, mc++) horizLine(xc - points[i], mc, (points[i] << 1), color, mode);
    for (i = radius - 1; i >= 0; i--, mc++) horizLine(xc - points[i], mc, (points[i] << 1), color, mode);
}

//filled ellipse with color
void fillEllipse(int32_t xc, int32_t yc, int32_t ra, int32_t rb, uint32_t color, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    int32_t i = 0;
    int32_t points[500] = { 0 };

    //range limited
    if (ra <= 0 || rb <= 0) return;

    //out of range
    if (ra > 499 || rb > 499)
    {
        messageBox(GFX_ERROR, "fillEllipse: ra, rb must be in [0-499] pixels");
        return;
    }

    int32_t mc = yc - rb;

    if (ra != rb) calcEllipse(ra, rb, points);
    else calcCircle(ra, points);

    for (i = 0; i <= rb - 1; i++, mc++) horizLine(xc - points[i], mc, points[i] << 1, color, mode);
    for (i = rb - 1; i >= 0; i--, mc++) horizLine(xc - points[i], mc, points[i] << 1, color, mode);
}

//fill polygon using Darel Rex Finley algorithm https://alienryderflex.com/polygon_fill/
//optimize version here https://gist.github.com/ideasman42/983738130f754ef58ffa66bcdbbab892
//test vectors (screen resolution: 800x600)
//pt1[] = {{300, 100}, {192, 209}, {407, 323}, {320, 380}, {214, 350}, {375, 209}};
//pt2[] = {{169, 164}, {169, 264}, {223, 300}, {296, 209}, {214, 255}, {223, 200}, {386, 192}, {341, 273}, {404, 300}, {431, 146}};
//pt3[] = {{97, 56}, {115, 236}, {205, 146}, {276, 146}, {151, 325}, {259, 433}, {510, 344}, {510, 218}, {242, 271}, {384, 110}};
//pt4[] = {{256, 150}, {148, 347}, {327, 329}, {311, 204}, {401, 204}, {418, 240}, {257, 222}, {293, 365}, {436, 383}, {455, 150}};
//pt5[] = {{287, 76}, {129, 110}, {42, 301}, {78, 353}, {146, 337}, {199, 162}, {391, 180}, {322, 353}, {321, 198}, {219, 370}, {391, 405}, {444, 232}, {496, 440}, {565, 214}};
//pt6[] = {{659, 336}, {452, 374}, {602, 128}, {509, 90}, {433, 164}, {300, 71}, {113, 166}, {205, 185}, {113, 279}, {169, 278}, {206, 334}, {263, 279}, {355, 129}, {301, 335}, {432, 204}, {433, 297}, {245, 467}, {414, 392}, {547, 523}};
void fillPolygon(const POINT2D* points, int32_t num, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL*/)
{
    int32_t nodex[MAX_POLY_CORNERS] = { 0 };
    int32_t nodes = 0, y = 0, i = 0, j = 0, swap = 0;
    int32_t left = 0, right = 0, top = 0, bottom = 0;

    //initialize clipping
    left = right = int32_t(points[0].x);
    top = bottom = int32_t(points[0].y);

    //clipping points
    for (i = 1; i < num; i++)
    {
        if (points[i].x < left)      left = int32_t(points[i].x);
        if (points[i].x > right)     right = int32_t(points[i].x);
        if (points[i].y < top)       top = int32_t(points[i].y);
        if (points[i].y > bottom)    bottom = int32_t(points[i].y);
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
            if ((points[i].y < y && points[j].y >= y) || (points[j].y < y && points[i].y >= y)) nodex[nodes++] = int32_t(points[i].x + (y - points[i].y) / (points[j].y - points[i].y) * (points[j].x - points[i].x));
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
                horizLine(nodex[i], y, nodex[i + 1] - nodex[i], col, mode);
            }
        }
    }
}

//generate random polygon
void randomPolygon(const int32_t cx, const int32_t cy, const int32_t avgRadius, double irregularity, double spikeyness, const int32_t numVerts, POINT2D* points)
{
    //valid params
    spikeyness = clamp(spikeyness, 0, 1) * avgRadius;
    irregularity = clamp(irregularity, 0, 1) * 2 * M_PI / numVerts;
    
    //generate n angle steps
    double angleSteps[1024] = { 0 };
    const double lower = (2 * M_PI / numVerts) - irregularity;
    const double upper = (2 * M_PI / numVerts) + irregularity;
    double sum = 0;

    for (int32_t i = 0; i < numVerts; i++)
    {
        const double tmp = uniformRand(lower, upper);
        angleSteps[i] = tmp;
        sum += tmp;
    }

    //normalize the steps so that points 0 and points n+1 are the same
    const double koef = sum / (2 * M_PI);
    for (int32_t i = 0; i < numVerts; i++) angleSteps[i] /= koef;

    //now generate the points
    double angle = uniformRand(0, 2 * M_PI);
    const double radius = 2.0 * avgRadius;

    for (int32_t i = 0; i < numVerts; i++)
    {
        const double gaussValue = gaussianRand(avgRadius, spikeyness);
        const double rad = clamp(gaussValue, 0, radius);
        points[i].x = cx + rad * cos(angle);
        points[i].y = cy + rad * sin(angle);
        angle += angleSteps[i];
    }
}

//FX-effect: fade circle
void fadeCircle(int32_t dir, uint32_t col, uint32_t mswait)
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
            delay(mswait);
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
            delay(mswait);
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
            delay(mswait);
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
            delay(mswait);
        }
        break;

    default:
        break;
    }
}

//FX-effect: fade Rollo
void fadeRollo(int32_t dir, uint32_t col, uint32_t mswait)
{
    int32_t i = 0, j = 0;

    switch (dir)
    {
    case 0:
        for (i = 0; i < 20; i++)
        {
            for (j = 0; j <= cmaxY / 10; j++) horizLine(0, j * 20 + i, cmaxX, col);
            render();
            delay(mswait);
        }
        break;

    case 1:
        for (i = 0; i < 20; i++)
        {
            for (j = 0; j <= cmaxX / 10; j++) vertLine(j * 20 + i, 0, cmaxY, col);
            render();
            delay(mswait);
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
            delay(mswait);
        }
        break;

    default:
        break;
    }
}

//simulation visual page in VGA mode for compatible code
//setActivePage and setVisualPage must be a paired function
void setActivePage(GFX_IMAGE* page)
{
    changeDrawBuffer(page->mData, page->mWidth, page->mHeight);
}

//set render page to current page
//setActivePage and setVisualPage must be a paired function
void setVisualPage(GFX_IMAGE* page)
{
    changeDrawBuffer(page->mData, page->mWidth, page->mHeight);
    render();
    restoreDrawBuffer();
}

//create a new GFX image
int32_t newImage(int32_t width, int32_t height, GFX_IMAGE* img)
{
    //calculate buffer size (should be 32-bytes alignment)
    const uint32_t rowBytes = width * bytesPerPixel;
    
    //check for 32-bytes alignment
    if (rowBytes % 32)
    {
        messageBox(GFX_ERROR, "GFXLIB required 32-bytes alignment:%d", width);
        return 0;
    }

    //check size
    const uint32_t memSize = height * rowBytes;
    if (!memSize)
    {
        messageBox(GFX_ERROR, "Error create image, size = 0!");
        return 0;
    }

    //32-bytes alignment for SIMD optimize
    img->mData = SDL_aligned_alloc(32, memSize);
    if (!img->mData)
    {
        messageBox(GFX_ERROR, "Error alloc memory, size:%lu", memSize);
        return 0;
    }

    //store image info
    img->mWidth    = width;
    img->mHeight   = height;
    img->mSize     = memSize;
    img->mRowBytes = rowBytes;
    memset(img->mData, 0, memSize);
    return 1;
}

//update GFX image with new width, new height
int32_t updateImage(int32_t width, int32_t height, GFX_IMAGE* img)
{
    //no need update
    if (img->mWidth == width && img->mHeight == height) return 1;

    //calculate new buffer size (should be 32-bytes alignment)
    const uint32_t rowBytes = width * bytesPerPixel;

    //check for 32-bytes alignment
    if (rowBytes % 32)
    {
        messageBox(GFX_ERROR, "GFXLIB required 32-bytes alignment:%d", width);
        return 0;
    }

    //check size
    const uint32_t msize = height * rowBytes;
    if (!msize)
    {
        messageBox(GFX_ERROR, "Error update image size = 0!");
        return 0;
    }

    //reallocate new memory with new size (32-bytes alignment)
    if (img->mData) SDL_aligned_free(img->mData);
    img->mData = SDL_aligned_alloc(32, msize);
    if (!img->mData)
    {
        messageBox(GFX_ERROR, "Error alloc memory with size: %lu", msize);
        return 0;
    }

    //store image width and height
    img->mWidth    = width;
    img->mHeight   = height;
    img->mSize     = msize;
    img->mRowBytes = rowBytes;
    memset(img->mData, 0, msize);
    return 1;
}

//cleanup image buffer
void freeImage(GFX_IMAGE* img)
{
    if (img && img->mData)
    {
        SDL_aligned_free(img->mData);
        img->mData     = NULL;
        img->mWidth    = 0;
        img->mHeight   = 0;
        img->mSize     = 0;
        img->mRowBytes = 0;
    }
}

//clear image data buffer
void clearImage(GFX_IMAGE* img)
{
    if (img && img->mData) memset(img->mData, 0, img->mSize);
}

//get GFX image buffer functions
void getImageMix(int32_t x, int32_t y, int32_t width, int32_t height, GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void *imgData = img->mData;
    __asm {
        mov     edi, imgData
        mov     esi, drawBuff
        mov     eax, y
        mul     texWidth
        add     eax, x
        add     esi, eax
        mov     edx, texWidth
        sub     edx, width
        mov     eax, width
        shr     eax, 2
        mov     ebx, width
        and     ebx, 3
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     esi, edx
        dec     height
        jnz     next
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 5;
    const int32_t remainder = width % 32;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint8_t* pdata = (uint8_t*)drawBuff;
    uint8_t* srcPixels = (uint8_t*)img->mData;
    uint8_t* dstPixels = &pdata[texWidth * y + x];

    //use AVX2 for faster copy alignment memory
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            const __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)dstPixels);
            _mm256_stream_si256((__m256i*)srcPixels, ymm0);
            srcPixels += 32;
            dstPixels += 32;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++) *srcPixels++ = *dstPixels++;
        }

        //next offsets
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//get GFX image buffer functions
void getImageNormal(int32_t x, int32_t y, int32_t width, int32_t height, GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void *imgData = img->mData;
    __asm {
        mov     edi, imgData
        mov     esi, drawBuff
        mov     eax, y
        mul     texWidth
        add     eax, x
        shl     eax, 2
        add     esi, eax
        mov     edx, texWidth
        sub     edx, width
        shl     edx, 2
    next:
        mov     ecx, width
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        test    width, 1
        jz      end
        movd    mm0, [esi]
        movd    [edi], mm0
        add     edi, 4
        add     esi, 4
    end:
        add     esi, edx
        dec     height
        jnz     next
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* srcPixels = (uint32_t*)img->mData;
    uint32_t* dstPixels = &pdata[texWidth * y + x];

    //use AVX2 for faster copy alignment memory
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            const __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)dstPixels);
            _mm256_stream_si256((__m256i*)srcPixels, ymm0);
            srcPixels += 8;
            dstPixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++) *srcPixels++ = *dstPixels++;
        }

        //next offsets
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//get GFX image buffer (export function)
void getImage(int32_t x, int32_t y, int32_t width, int32_t height, GFX_IMAGE* img)
{
    //calculate new position
    const int32_t x1 = x + (width - 1);
    const int32_t y1 = y + (height - 1);

    //clip image to context boundaries
    const int32_t lx = max(x, cminX);
    const int32_t ly = max(y, cminY);
    const int32_t lx1 = min(x1, cmaxX);
    const int32_t ly1 = min(y1, cmaxY);

    //initialize loop variables
    const int32_t lwidth = (lx1 - lx) + 1;
    const int32_t lheight = (ly1 - ly) + 1;

    //check for loop
    if (lwidth <= 0|| lheight <= 0) return;

    //none image pointer?
    if (!img->mData)
    {
        //create new image
        if (!newImage(lwidth, lheight, img)) return;
    }
    else
    {
        //update an existing image
        if (!updateImage(lwidth, lheight, img)) return;
    }

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        getImageMix(lx, ly, lwidth, lheight, img);
        return;
    }

    //height color mode
    getImageNormal(lx, ly, lwidth, lheight, img);
}

//put GFX image to points (x1, y1)
void putImageMix(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
        add     eax, ebx
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, width
        push    ebx
        mov     edx, imgWidth
        sub     edx, width
        mov     eax, width
        shr     eax, 2
        mov     ebx, width
        and     ebx, 3
    next:
        mov     ecx, eax
        rep     movsd
        mov     ecx, ebx
        rep     movsb
        add     edi, [esp]
        add     esi, edx
        dec     height
        jnz     next
        pop     ebx
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 5;
    const int32_t remainder = width % 32;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint8_t* dstData = (uint8_t*)drawBuff;
    uint8_t* srcData = (uint8_t*)img->mData;
    uint8_t* dstPixels = &dstData[texWidth * ly + lx];
    uint8_t* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //use AVX2 for faster copy alignment memory
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            const __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)srcPixels);
            _mm256_stream_si256((__m256i*)dstPixels, ymm0);
            dstPixels += 32;
            srcPixels += 32;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++) *dstPixels++ = *srcPixels++;
        }

        //next offsets
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image to points (x1, y1)
void putImageNormal(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
        add     eax, ebx
        shl     eax, 2
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, width
        shl     ebx, 2
        mov     edx, imgWidth
        sub     edx, width
        shl     edx, 2
    next:
        mov     ecx, width
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        movq    [edi], mm0
        add     edi, 8
        add     esi, 8
        dec     ecx
        jnz     plot
    once:
        test    width, 1
        jz      end
        movd    mm0, [esi]
        movd    [edi], mm0
        add     edi, 4
        add     esi, 4
    end:
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     next
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint32_t* dstData = (uint32_t*)drawBuff;
    uint32_t* srcData = (uint32_t*)img->mData;
    uint32_t* dstPixels = &dstData[texWidth * ly + lx];
    uint32_t* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //use AVX2 for faster copy alignment memory
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            const __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)srcPixels);
            _mm256_stream_si256((__m256i*)dstPixels, ymm0);
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++) *dstPixels++ = *srcPixels++;
        }

        //next offsets
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image with add background color
void putImageAdd(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
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
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        paddusb mm0, [edi]
        movq    [edi], mm0
        add     esi, 8
        add     edi, 8
        dec     ecx
        jnz     plot
    once:
        test    width, 1
        jz      end
        movd    mm0, [esi]
        paddusb mm0, [edi]
        movd    [edi], mm0
        add     esi, 4
        add     edi, 4
    end:
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     again
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    ARGB* dstData = (ARGB*)drawBuff;
    ARGB* srcData = (ARGB*)img->mData;
    ARGB* dstPixels = &dstData[texWidth * ly + lx];
    ARGB* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            //load source and destination
            const __m256i src = _mm256_stream_load_si256((const __m256i*)srcPixels);
            const __m256i dst = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //add 32-bytes data with saturation and store
            const __m256i res = _mm256_adds_epu8(src, dst);
            _mm256_stream_si256((__m256i*)dstPixels, res);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }
        
        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                dstPixels->r = min(srcPixels->r + dstPixels->r, 255);
                dstPixels->g = min(srcPixels->g + dstPixels->g, 255);
                dstPixels->b = min(srcPixels->b + dstPixels->b, 255);
                dstPixels++;
                srcPixels++;
            }
        }
        
        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image with sub background color
void putImageSub(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
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
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        psubusb mm0, [edi]
        movq    [edi], mm0
        add     esi, 8
        add     edi, 8
        dec     ecx
        jnz     plot
    once:
        test    width, 1
        jz      end
        movd    mm0, [esi]
        psubusb mm0, [edi]
        movd    [edi], mm0
        add     esi, 4
        add     edi, 4
    end:
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     again
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    ARGB* dstData = (ARGB*)drawBuff;
    ARGB* srcData = (ARGB*)img->mData;
    ARGB* dstPixels = &dstData[texWidth * ly + lx];
    ARGB* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            //load source and destination
            const __m256i src = _mm256_stream_load_si256((const __m256i*)srcPixels);
            const __m256i dst = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //sub 32-bytes data with saturation and store
            const __m256i res = _mm256_subs_epu8(src, dst);
            _mm256_stream_si256((__m256i*)dstPixels, res);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                dstPixels->r = max(srcPixels->r - dstPixels->r, 0);
                dstPixels->g = max(srcPixels->g - dstPixels->g, 0);
                dstPixels->b = max(srcPixels->b - dstPixels->b, 0);
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image with logical and background color
void putImageAnd(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
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
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        pand    mm0, [edi]
        movq    [edi], mm0
        add     esi, 8
        add     edi, 8
        dec     ecx
        jnz     plot
    once:
        test    width, 1
        jz      end
        movd    mm0, [esi]
        pand    mm0, [edi]
        movd    [edi], mm0
        add     esi, 4
        add     edi, 4
    end:
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     again
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    ARGB* dstData = (ARGB*)drawBuff;
    ARGB* srcData = (ARGB*)img->mData;
    ARGB* dstPixels = &dstData[texWidth * ly + lx];
    ARGB* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            //load source and destination
            const __m256i src = _mm256_stream_load_si256((const __m256i*)srcPixels);
            const __m256i dst = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //sub 32-bytes data with saturation and store
            const __m256i res = _mm256_and_si256(src, dst);
            _mm256_stream_si256((__m256i*)dstPixels, res);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                dstPixels->r = srcPixels->r & dstPixels->r;
                dstPixels->g = srcPixels->g & dstPixels->g;
                dstPixels->b = srcPixels->b & dstPixels->b;
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image with logical x-or background color
void putImageXor(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        shl     eax, 2
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
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
        shr     ecx, 1
        jz      once
    plot:
        movq    mm0, [esi]
        pxor    mm0, [edi]
        movq    [edi], mm0
        add     esi, 8
        add     edi, 8
        dec     ecx
        jnz     plot
    once:
        test    width, 1
        jz      end
        movd    mm0, [esi]
        pxor    mm0, [edi]
        movd    [edi], mm0
        add     esi, 4
        add     edi, 4
    end:
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     again
        emms
    }
#else
    //32-bytes alignment
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    //calculate next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    ARGB* dstData = (ARGB*)drawBuff;
    ARGB* srcData = (ARGB*)img->mData;
    ARGB* dstPixels = &dstData[texWidth * ly + lx];
    ARGB* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < aligned; j++)
        {
            //load source and destination
            const __m256i src = _mm256_stream_load_si256((const __m256i*)srcPixels);
            const __m256i dst = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //sub 32-bytes data with saturation and store
            const __m256i res = _mm256_xor_si256(src, dst);
            _mm256_stream_si256((__m256i*)dstPixels, res);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                dstPixels->r = srcPixels->r ^ dstPixels->r;
                dstPixels->g = srcPixels->g ^ dstPixels->g;
                dstPixels->b = srcPixels->b ^ dstPixels->b;
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image with transparent color (must be RGBA format)
void putImageAlpha(const int32_t x, const int32_t y, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov         edi, drawBuff
        mov         eax, ly
        mul         texWidth
        add         eax, lx
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, ly
        sub         eax, y
        mul         imgWidth
        mov         ebx, lx
        sub         ebx, x
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, width
        shl         edx, 2
    again:
        mov         ecx, width
    plot:
        mov         eax, [esi]
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
        movd        mm0, [edi]
        movd        mm1, [esi]
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
        add         edi, 4
        add         esi, 4
        dec         ecx
        jnz         plot
        add         edi, ebx
        add         esi, edx
        dec         height
        jnz         again
        emms
    }
#else
    //align 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    const __m256i zero = _mm256_setzero_si256();
    const __m256i unity = _mm256_set1_epi16(256);

    //next offset
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint32_t* dstData = (uint32_t*)drawBuff;
    uint32_t* srcData = (uint32_t*)img->mData;
    uint32_t* dstPixels = &dstData[texWidth * ly + lx];
    uint32_t* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //scan height
    for (int32_t i = 0; i < height; i++)
    {
        //process 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            //load source and destination
            const __m256i src = _mm256_stream_load_si256((const __m256i*)srcPixels);
            const __m256i dst = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //high source (S * A)
            __m256i src16 = _mm256_unpackhi_epi8(src, zero);
            __m256i alpha = src16;
            alpha = _mm256_shufflehi_epi16(alpha, 0xff);
            alpha = _mm256_shufflelo_epi16(alpha, 0xff);
            src16 = _mm256_mullo_epi16(src16, alpha);

            //high dest (D * (256 - A))
            __m256i dst16 = _mm256_unpackhi_epi8(dst, zero);
            alpha = _mm256_sub_epi16(unity, alpha);
            dst16 = _mm256_mullo_epi16(dst16, alpha);

            //high result (S * A + D * (256 - A)) >> 8
            __m256i rlt16h = _mm256_add_epi16(src16, dst16);
            rlt16h = _mm256_srli_epi16(rlt16h, 8);

            //low (S * A)
            src16 = _mm256_unpacklo_epi8(src, zero);
            alpha = src16;
            alpha = _mm256_shufflehi_epi16(alpha, 0xff);
            alpha = _mm256_shufflelo_epi16(alpha, 0xff);
            src16 = _mm256_mullo_epi16(src16, alpha);

            //low (D * (256 - A))
            dst16 = _mm256_unpacklo_epi8(dst, zero);
            alpha = _mm256_sub_epi16(unity, alpha);
            dst16 = _mm256_mullo_epi16(dst16, alpha);

            //low result (S * A + D * (256 - A)) >> 8
            __m256i rlt16l = _mm256_add_epi16(src16, dst16);
            rlt16l = _mm256_srli_epi16(rlt16l, 8);

            //final merge result (hi,low) and store
            const __m256i result = _mm256_packus_epi16(rlt16l, rlt16h);
            _mm256_stream_si256((__m256i*)dstPixels, result);

            //next 8 pixels width (32 bytes)
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                const uint32_t dstCol = *dstPixels;
                const uint32_t srcCol = *srcPixels;
                const uint8_t cover = srcCol >> 24;
                const uint8_t rcover = 255 - cover;
                const uint32_t rb = ((dstCol & 0x00ff00ff) * rcover + (srcCol & 0x00ff00ff) * cover);
                const uint32_t ag = (((dstCol & 0xff00ff00) >> 8) * rcover + ((srcCol & 0xff00ff00) >> 8) * cover);
                *dstPixels++ = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put GFX image to draw buffer (export function)
void putImage(int32_t x, int32_t y, const GFX_IMAGE* img, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //calculate new position
    const int32_t x1 = x + (img->mWidth - 1);
    const int32_t y1 = y + (img->mHeight - 1);

    //clip image to context boundaries
    const int32_t lx = max(x, cminX);
    const int32_t ly = max(y, cminY);
    const int32_t lx1 = min(x1, cmaxX);
    const int32_t ly1 = min(y1, cmaxY);

    //initialize loop variables
    const int32_t width = (lx1 - lx) + 1;
    const int32_t height = (ly1 - ly) + 1;

    //check for loop
    if (width <= 0 || height <= 0) return;

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        putImageMix(x, y, lx, ly, width, height, img);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        putImageNormal(x, y, lx, ly, width, height, img);
        break;

    case BLEND_MODE_ADD:
        putImageAdd(x, y, lx, ly, width, height, img);
        break;

    case BLEND_MODE_SUB:
        putImageSub(x, y, lx, ly, width, height, img);
        break;

    case BLEND_MODE_AND:
        putImageAnd(x, y, lx, ly, width, height, img);
        break;

    case BLEND_MODE_XOR:
        putImageXor(x, y, lx, ly, width, height, img);
        break;

    case BLEND_MODE_ALPHA:
        putImageAlpha(x, y, lx, ly, width, height, img);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//put a sprite at points(x1, y1) with key color (don't render key color)
void putSpriteMix(const int32_t x, const int32_t y, const uint32_t keyColor, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth  = img->mWidth;

    __asm {
        mov     edi, drawBuff
        mov     eax, ly
        mul     texWidth
        add     eax, lx
        add     edi, eax
        mov     esi, imgData
        mov     eax, ly
        sub     eax, y
        mul     imgWidth
        mov     ebx, lx
        sub     ebx, x
        add     eax, ebx
        add     esi, eax
        mov     ebx, texWidth
        sub     ebx, width
        mov     edx, imgWidth
        sub     edx, width
    next:
        mov     ecx, width
    plot:
        lodsb
        cmp     al, byte ptr[keyColor]
        je      skip
        mov     [edi], al
    skip:
        inc     edi
        dec     ecx
        jnz     plot
        add     edi, ebx
        add     esi, edx
        dec     height
        jnz     next
    }
#else
    //calculate next offsets
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint8_t* dstData = (uint8_t*)drawBuff;
    uint8_t* srcData = (uint8_t*)img->mData;
    uint8_t* dstPixels = &dstData[texWidth * ly + lx];
    uint8_t* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    for (int32_t i = 0; i < height; i++)
    {
        for (int32_t j = 0; j < width; j++)
        {
            //source color diff with key color, plot it
            if (!(*srcPixels ^ keyColor)) *dstPixels = *srcPixels;

            //next pixels
            dstPixels++;
            srcPixels++;
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put a sprite at points(x1, y1) with key color (don't render key color)
void putSpriteNormal(const int32_t x, const int32_t y, const uint32_t keyColor, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov         edi, drawBuff
        mov         eax, ly
        mul         texWidth
        add         eax, lx
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, ly
        sub         eax, y
        mul         imgWidth
        mov         ebx, lx
        sub         ebx, x
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, width
        shl         edx, 2
        movd        mm4, keyColor
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        mov         eax, 0ffffffh
        movd        mm6, eax
        punpckldq   mm6, mm6
    next:
        mov         ecx, width
        shr         ecx, 1
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        movq        mm1, [edi]
        pand        mm1, mm2
        pxor        mm2, mm5
        pand        mm0, mm2
        por         mm1, mm0
        movq        [edi], mm1
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          skip
        add         esi, 4
        add         edi, 4
        mov         eax, [esi]
        cmp         eax, keyColor
        je          skip
        mov         [edi - 4], eax
    skip:
        add         edi, ebx
        add         esi, edx
        dec         height
        jnz         next
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    const __m256i ymm4 = _mm256_set1_epi32(keyColor);
    const __m256i ymm5 = _mm256_set1_epi32(0xffffffff);
    const __m256i ymm6 = _mm256_set1_epi32(0x00ffffff);

    //calculate next offsets
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint32_t* dstData = (uint32_t*)drawBuff;
    uint32_t* srcData = (uint32_t*)img->mData;
    uint32_t* dstPixels = &dstData[texWidth * ly + lx];
    uint32_t* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        //process 32-bytes align
        for (int32_t j = 0; j < aligned; j++)
        {
            //off alpha channel from source pixels
            __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)srcPixels);
            ymm0 = _mm256_and_si256(ymm0, ymm6);

            //get mask with key color
            __m256i ymm2 = _mm256_cmpeq_epi32(ymm0, ymm4);
            
            //replace background color by key color
            __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)dstPixels);
            ymm1 = _mm256_and_si256(ymm1, ymm2);
            
            //inverted mask
            ymm2 = _mm256_xor_si256(ymm2, ymm5);
            
            //turn off key color channel from source
            ymm0 = _mm256_and_si256(ymm0, ymm2);

            //add with background
            ymm1 = _mm256_or_si256(ymm1, ymm0);
            
            //store color
            _mm256_stream_si256((__m256i*)dstPixels, ymm1);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                //turn of alpha channel of source color
                const uint32_t col = *srcPixels & 0x00ffffffl;
                if (!(col ^ keyColor)) *dstPixels = *srcPixels;

                //next pixels
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put a sprite at points(x1, y1) with key color (don't render key color), add with background color
void putSpriteAdd(const int32_t x, const int32_t y, const uint32_t keyColor, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov         edi, drawBuff
        mov         eax, ly
        mul         texWidth
        add         eax, lx
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, ly
        sub         eax, y
        mul         imgWidth
        mov         ebx, lx
        sub         ebx, x
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, width
        shl         edx, 2
        movd        mm4, keyColor
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        mov         eax, 0ffffffh
        movd        mm6, eax
        punpckldq   mm6, mm6
    next:
        mov         ecx, width
        shr         ecx, 1
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        pxor        mm2, mm5
        pand        mm0, mm2
        paddusb     mm0, [edi]
        movq        [edi], mm0
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          skip
        add         esi, 4
        add         edi, 4
        mov         eax, [esi]
        cmp         eax, keyColor
        je          skip
        movd        mm3, eax
        paddusb     mm3, [edi - 4]
        movd        [edi - 4], mm3
    skip:
        add         edi, ebx
        add         esi, edx
        dec         height
        jnz         next
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    const __m256i ymm4 = _mm256_set1_epi32(keyColor);
    const __m256i ymm5 = _mm256_set1_epi32(0xffffffff);
    const __m256i ymm6 = _mm256_set1_epi32(0x00ffffff);

    //calculate next offsets
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    ARGB* dstData = (ARGB*)drawBuff;
    ARGB* srcData = (ARGB*)img->mData;
    ARGB* dstPixels = &dstData[texWidth * ly + lx];
    ARGB* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        //process 32-bytes align
        for (int32_t j = 0; j < aligned; j++)
        {
            //off alpha channel from 8 source pixels (ARGB -> 0RGB)
            __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)srcPixels);
            ymm0 = _mm256_and_si256(ymm0, ymm6);

            //load 8 pixels from background color
            __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //get mask with key color (key color is 0xff and render is 0x00)
            __m256i ymm2 = _mm256_cmpeq_epi32(ymm0, ymm4);

            //inverted mask (key color is 0x00 and render is 0xff)
            ymm2 = _mm256_xor_si256(ymm2, ymm5);

            //make source with off key color (0x00XX)
            ymm0 = _mm256_and_si256(ymm0, ymm2);

            //add with background color (only add render color)
            ymm1 = _mm256_adds_epu8(ymm0, ymm1);

            //store back to background
            _mm256_stream_si256((__m256i*)dstPixels, ymm1);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                //turn off alpha channel of source color
                const uint32_t col = (*(uint32_t*)srcPixels) & 0x00ffffff;
                if (!(col ^ keyColor))
                {
                    dstPixels->r = min(srcPixels->r + dstPixels->r, 255);
                    dstPixels->g = min(srcPixels->g + dstPixels->g, 255);
                    dstPixels->b = min(srcPixels->b + dstPixels->b, 255);
                }

                //next pixels
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put a sprite at points(x1, y1) with key color (don't render key color), sub with background color
void putSpriteSub(const int32_t x, const int32_t y, const uint32_t keyColor, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov         edi, drawBuff
        mov         eax, ly
        mul         texWidth
        add         eax, lx
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, ly
        sub         eax, y
        mul         imgWidth
        mov         ebx, lx
        sub         ebx, x
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, width
        shl         edx, 2
        movd        mm4, keyColor
        punpckldq   mm4, mm4
        mov         eax, 0ffffffffh
        movd        mm5, eax
        punpckldq   mm5, mm5
        mov         eax, 0ffffffh
        movd        mm6, eax
        punpckldq   mm6, mm6
    next:
        mov         ecx, width
        shr         ecx, 1
        jz          once
    plot:
        movq        mm0, [esi]
        movq        mm2, mm4
        pand        mm0, mm6
        pcmpeqd     mm2, mm0
        movq        mm1, [edi]
        pand        mm1, mm2
        pxor        mm2, mm5
        pand        mm0, mm2
        psubusb     mm0, [edi]
        por         mm0, mm1
        movq        [edi], mm0
        add         esi, 8
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        width, 1
        jz          skip
        add         esi, 4
        add         edi, 4
        mov         eax, [esi]
        cmp         eax, keyColor
        je          skip
        movd        mm3, eax
        psubusb     mm3, [edi - 4]
        movd        [edi - 4], mm3
    skip:
        add         edi, ebx
        add         esi, edx
        dec         height
        jnz         next
        emms
    }
#else
    //aligned 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    const __m256i ymm4 = _mm256_set1_epi32(keyColor);
    const __m256i ymm5 = _mm256_set1_epi32(0xffffffff);
    const __m256i ymm6 = _mm256_set1_epi32(0x00ffffff);

    //calculate next offsets
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    ARGB* dstData = (ARGB*)drawBuff;
    ARGB* srcData = (ARGB*)img->mData;
    ARGB* dstPixels = &dstData[texWidth * ly + lx];
    ARGB* srcPixels = &srcData[img->mWidth * (ly - y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        //process 32-bytes align
        for (int32_t j = 0; j < aligned; j++)
        {
            //off alpha channel from 8 source pixels (ARGB -> 0RGB)
            __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)srcPixels);
            ymm0 = _mm256_and_si256(ymm0, ymm6);

            //load 8 pixels from background color
            const __m256i ymm1 = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //get mask with key color (key color is 0xff and render is 0x00)
            __m256i ymm2 = _mm256_cmpeq_epi32(ymm0, ymm4);

            //save revert background
            __m256i ymm3 = _mm256_and_si256(ymm1, ymm2);

            //inverted mask (key color is 0x00 and render is 0xff)
            ymm2 = _mm256_xor_si256(ymm2, ymm5);

            //make source with off key color (0x00XX)
            ymm0 = _mm256_and_si256(ymm0, ymm2);

            //sub source to background color (only sub render color)
            ymm0 = _mm256_subs_epu8(ymm0, ymm1);

            //replace key color with background color
            ymm3 = _mm256_or_si256(ymm0, ymm3);

            //store back to background
            _mm256_stream_si256((__m256i*)dstPixels, ymm3);

            //next 8 pixels
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                //turn off alpha channel of source color
                const uint32_t col = (*(uint32_t*)srcPixels) & 0x00ffffff;
                if (!(col ^ keyColor))
                {
                    dstPixels->r = max(srcPixels->r - dstPixels->r, 0);
                    dstPixels->g = max(srcPixels->g - dstPixels->g, 0);
                    dstPixels->b = max(srcPixels->b - dstPixels->b, 0);
                }

                //next pixels
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put a sprite at points(x1, y1) with key color (don't render key color) and blending color
void putSpriteAlpha(const int32_t x, const int32_t y, const uint32_t keyColor, const int32_t lx, const int32_t ly, const int32_t width, const int32_t height, const GFX_IMAGE* img)
{
#ifdef _USE_ASM
    void* imgData = img->mData;
    const int32_t imgWidth = img->mWidth;

    __asm {
        mov         edi, drawBuff
        mov         eax, ly
        mul         texWidth
        add         eax, lx
        shl         eax, 2
        add         edi, eax
        mov         esi, imgData
        mov         eax, ly
        sub         eax, y
        mul         imgWidth
        mov         ebx, lx
        sub         ebx, x
        add         eax, ebx
        shl         eax, 2
        add         esi, eax
        mov         ebx, texWidth
        sub         ebx, width
        shl         ebx, 2
        mov         edx, imgWidth
        sub         edx, width
        shl         edx, 2
    next:
        mov         ecx, width
    plot:
        mov         eax, [esi]
        and         eax, 00ffffffh
        cmp         eax, keyColor
        je          skip
        mov         eax, [esi]
        shr         eax, 24
        pxor        mm5, mm5
        movd        mm7, eax
        pshufw	    mm7, mm7, 0
        neg	        eax
        add	        eax, 256
        movd	    mm6, eax
        pshufw	    mm6, mm6, 0
        movd        mm0, [edi]
        movd        mm1, [esi]
        punpcklbw   mm0, mm5
        punpcklbw   mm1, mm5
        pmullw      mm0, mm6
        pmullw      mm1, mm7
        paddusw     mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm0
        movd        [edi], mm0
    skip:
        add         edi, 4
        add         esi, 4
        dec         ecx
        jnz         plot
        add         edi, ebx
        add         esi, edx
        dec         height
        jnz         next
        emms
    }
#else
    //align 32-bytes
    const int32_t aligned = width >> 3;
    const int32_t remainder = width % 8;

    const __m256i zero = _mm256_setzero_si256();
    const __m256i unity = _mm256_set1_epi16(256);

    const __m256i amask = _mm256_set1_epi32(keyColor);
    const __m256i bmask = _mm256_set1_epi32(0xffffffff);

    //calculate adding offsets for next line
    const int32_t addDstOffs = texWidth - width;
    const int32_t addImgOffs = img->mWidth - width;

    //calculate starting address
    uint32_t* dstData = (uint32_t*)drawBuff;
    uint32_t* srcData = (uint32_t*)img->mData;
    uint32_t* dstPixels = &dstData[texWidth * ly + lx];
    uint32_t* srcPixels = &srcData[img->mWidth * (ly- y) + (lx - x)];

    //line-by-line
    for (int32_t i = 0; i < height; i++)
    {
        //process 32-bytes aligned
        for (int32_t j = 0; j < aligned; j++)
        {
            //load 8 pixels from source and dest
            __m256i src = _mm256_stream_load_si256((const __m256i*)srcPixels);
            const __m256i dst = _mm256_stream_load_si256((const __m256i*)dstPixels);

            //get mask with key color (key color is 0xff and render is 0x00)
            __m256i mask = _mm256_cmpeq_epi32(src, amask);

            //inverted mask (key color is 0x00 and render is 0xff)
            mask = _mm256_xor_si256(mask, bmask);

            //make source with off key color (0x00XX)
            src = _mm256_and_si256(src, mask);

            //high source (S * A)
            __m256i src16 = _mm256_unpackhi_epi8(src, zero);
            __m256i alpha = src16;
            alpha = _mm256_shufflehi_epi16(alpha, 0xff);
            alpha = _mm256_shufflelo_epi16(alpha, 0xff);
            src16 = _mm256_mullo_epi16(src16, alpha);

            //high dest (D * (256 - A))
            __m256i dst16 = _mm256_unpackhi_epi8(dst, zero);
            alpha = _mm256_sub_epi16(unity, alpha);
            dst16 = _mm256_mullo_epi16(dst16, alpha);

            //high result (S * A + D * (256 - A)) >> 8
            __m256i rlt16h = _mm256_add_epi16(src16, dst16);
            rlt16h = _mm256_srli_epi16(rlt16h, 8);

            //low (S * A)
            src16 = _mm256_unpacklo_epi8(src, zero);
            alpha = src16;
            alpha = _mm256_shufflehi_epi16(alpha, 0xff);
            alpha = _mm256_shufflelo_epi16(alpha, 0xff);
            src16 = _mm256_mullo_epi16(src16, alpha);

            //low (D * (256 - A))
            dst16 = _mm256_unpacklo_epi8(dst, zero);
            alpha = _mm256_sub_epi16(unity, alpha);
            dst16 = _mm256_mullo_epi16(dst16, alpha);

            //low result (S * A + D * (256 - A)) >> 8
            __m256i rlt16l = _mm256_add_epi16(src16, dst16);
            rlt16l = _mm256_srli_epi16(rlt16l, 8);

            //final merge result (hi,low) and store
            const __m256i result = _mm256_packus_epi16(rlt16l, rlt16h);
            _mm256_stream_si256((__m256i*)dstPixels, result);

            //next 8 pixels width
            dstPixels += 8;
            srcPixels += 8;
        }

        //have unaligned bytes?
        if (remainder > 0)
        {
            for (int32_t k = 0; k < remainder; k++)
            {
                //we accepted RGB color only
                const uint32_t col = (*srcPixels) & 0x00ffffff;
                if (!(col ^ keyColor))
                {
                    const uint32_t dstCol = *dstPixels;
                    const uint32_t srcCol = *srcPixels;
                    const uint8_t cover = srcCol >> 24;
                    const uint8_t rcover = 255 - cover;
                    const uint32_t rb = ((dstCol & 0x00ff00ff) * rcover + (srcCol & 0x00ff00ff) * cover);
                    const uint32_t ag = (((dstCol & 0xff00ff00) >> 8) * rcover + ((srcCol & 0xff00ff00) >> 8) * cover);
                    *dstPixels = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
                }

                //next pixels
                dstPixels++;
                srcPixels++;
            }
        }

        //next line
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//put a sprite at points(x1, y1) with key color (don't render key color), sub with background color
void putSprite(int32_t x, int32_t y, uint32_t keyColor, const GFX_IMAGE* img, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    //calculate new position
    const int32_t x1 = x + (img->mWidth - 1);
    const int32_t y1 = y + (img->mHeight - 1);

    //clip image to context boundaries
    const int32_t lx = max(x, cminX);
    const int32_t ly = max(y, cminY);
    const int32_t lx1 = min(x1, cmaxX);
    const int32_t ly1 = min(y1, cmaxY);

    //initialize loop variables
    const int32_t width = (lx1 - lx) + 1;
    const int32_t height = (ly1 - ly) + 1;

    //check for loop
    if (width <= 0 || height <= 0) return;

    //mixed mode?
    if (bitsPerPixel == 8)
    {
        putSpriteMix(x, y, keyColor, lx, ly, width, height, img);
        return;
    }

    //height color mode
    switch (mode)
    {
    case BLEND_MODE_NORMAL:
        putSpriteNormal(x, y, keyColor, lx, ly, width, height, img);
        break;

    case BLEND_MODE_ADD:
        putSpriteAdd(x, y, keyColor, lx, ly, width, height, img);
        break;

    case BLEND_MODE_SUB:
        putSpriteSub(x, y, keyColor, lx, ly, width, height, img);
        break;

    case BLEND_MODE_ALPHA:
        putSpriteAlpha(x, y, keyColor, lx, ly, width, height, img);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown blend mode:%d", mode);
        break;
    }
}

//boundary clip points at (x,y)
must_inline int32_t clampPoint(const int32_t width, const int32_t height, int32_t* x, int32_t* y)
{
    int32_t ret = 1;

    if (*x < 0)
    { 
        *x = 0;
        ret = 0;
    }
    else if (*x >= width)
    { 
        *x = width - 1;
        ret = 0;
    }

    if (*y < 0)
    {
        *y = 0;
        ret = 0;
    }
    else if (*y >= height)
    {
        *y = height - 1;
        ret = 0;
    }

    return ret;
}

//get source pixel
must_inline uint32_t clampOffset(const int32_t width, const int32_t height, const int32_t x, const int32_t y)
{
    //x-range check
    const int32_t xx = clamp(x, 0, width - 1);
    const int32_t yy = clamp(y, 0, height - 1);

    //return offset at (x,y)
    return yy * width + xx;
}

//clamp pixels at offset (x,y)
must_inline uint32_t clampPixels(const GFX_IMAGE* img, int32_t x, int32_t y)
{
    const uint32_t* psrc = (const uint32_t*)img->mData;
    const int32_t insrc = clampPoint(img->mWidth, img->mHeight, &x, &y);
    uint32_t result = psrc[y * img->mWidth + x];
    if (!insrc)
    {
        ARGB* pcol = (ARGB*)&result;
        pcol->a = 0;
    }
    return result;
}

//alpha-blending pixel
must_inline uint32_t alphaBlend(const uint32_t dstCol, const uint32_t srcCol)
{
#ifdef _USE_ASM
    __asm {
        pxor        mm7, mm7
        movd        mm0, srcCol
        movd        mm2, dstCol
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
    const uint8_t cover = srcCol >> 24;
    const uint8_t rcover = 255 - cover;
    const uint32_t rb = ((dstCol & 0x00ff00ff) * rcover + (srcCol & 0x00ff00ff) * cover);
    const uint32_t ag = (((dstCol & 0xff00ff00) >> 8) * rcover + ((srcCol & 0xff00ff00) >> 8) * cover);
    return ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
#endif
}

//smooth get pixel (average pixel calculation)
must_inline uint32_t smoothGetPixel(const GFX_IMAGE* img, const int32_t sx, const int32_t sy)
{
    const int32_t lx = sx >> 16;
    const int32_t ly = sy >> 16;
    const ARGB* psrc = (const ARGB*)img->mData;
    const ARGB* p0 = (const ARGB*)&psrc[clampOffset(img->mWidth, img->mHeight, lx, ly)];
    const ARGB* p1 = (const ARGB*)&psrc[clampOffset(img->mWidth, img->mHeight, lx + 1, ly)];

    uint32_t col = 0;
    ARGB* pcol = (ARGB*)&col;
    pcol->a = (p0->a + p1->a) >> 1;
    pcol->r = (p0->r + p1->r) >> 1;
    pcol->g = (p0->g + p1->g) >> 1;
    pcol->b = (p0->b + p1->b) >> 1;
    return col;
}

//bilinear get pixel with FIXED-POINT (signed 16.16)
must_inline uint32_t bilinearGetPixelCenter(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    const int32_t width = psrc->mWidth;
    const uint32_t* pixel = (const uint32_t*)psrc->mData;

#ifdef _USE_ASM
    __asm {
        mov         eax, sx
        mov         edx, sy
        pxor        mm7, mm7
        shl         edx, 16
        shl         eax, 16
        shr         edx, 24
        shr         eax, 24
        movd        mm6, edx
        movd        mm5, eax
        mov         edx, sy
        mov         eax, width
        shl         eax, 2
        sar         edx, 16
        imul        edx, eax
        add         edx, pixel
        add         eax, edx
        mov         ecx, sx
        sar         ecx, 16
        movd        mm2, dword ptr[eax + ecx * 4]
        movd        mm0, dword ptr[eax + ecx * 4 + 4]
        punpcklwd   mm5, mm5
        punpcklwd   mm6, mm6
        movd        mm3, dword ptr[edx + ecx * 4]
        movd        mm1, dword ptr[edx + ecx * 4 + 4]
        punpckldq   mm5, mm5
        punpcklbw   mm0, mm7
        punpcklbw   mm1, mm7
        punpcklbw   mm2, mm7
        punpcklbw   mm3, mm7
        psubw       mm0, mm2
        psubw       mm1, mm3
        psllw       mm2, 8
        psllw       mm3, 8
        pmullw      mm0, mm5
        pmullw      mm1, mm5
        punpckldq   mm6, mm6
        paddw       mm0, mm2
        paddw       mm1, mm3
        psrlw       mm0, 8
        psrlw       mm1, 8
        psubw       mm0, mm1
        psllw       mm1, 8
        pmullw      mm0, mm6
        paddw       mm0, mm1
        psrlw       mm0, 8
        packuswb    mm0, mm7
        movd        eax, mm0
        emms
    }
#else
    const uint32_t* pixel0 = &pixel[(sy >> 16) * width + (sx >> 16)];
    const uint32_t* pixel1 = &pixel0[width];

    const uint32_t pu = uint8_t(sx >> 8);
    const uint32_t pv = uint8_t(sy >> 8);
    const uint32_t w3 = (pu * pv) >> 8;
    const uint32_t w2 = pu - w3;
    const uint32_t w1 = pv - w3;
    const uint32_t w0 = 256 - w1 - w2 - w3;

    //zero all
    const __m128i zero = _mm_setzero_si128();

    //load 4 pixels [(x, y),(x + 1, y),(x, y + 1),(x + 1, y + 1)]
    __m128i p12 = _mm_loadl_epi64((const __m128i*)pixel0);
    __m128i p34 = _mm_loadl_epi64((const __m128i*)pixel1);

    //convert RGBA RGBA RGBA RGAB to RRRR GGGG BBBB AAAA
    p12 = _mm_unpacklo_epi8(p12, p34);
    p34 = _mm_unpackhi_epi64(p12, zero);
    p12 = _mm_unpacklo_epi8(p12, p34);

    //extend to 16bits
    __m128i rg = _mm_unpacklo_epi8(p12, zero);
    __m128i ba = _mm_unpackhi_epi8(p12, zero);

    //initialize pixel weights to 16bits integer w4 w3 w2 w1
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
    weight = _mm_packus_epi32(weight, zero);
    weight = _mm_packus_epi16(weight, zero);
    return _mm_cvtsi128_si32(weight);
#endif
}

//bilinear get pixel with FIXED-POINT (signed 16.16)
must_inline uint32_t bilinearGetPixelBorder(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    //convert to fixed points
    const int32_t lx = sx >> 16;
    const int32_t ly = sy >> 16;

    //load the 4 neighboring pixels
    uint32_t pixels[4] = { 0 };
    pixels[0] = clampPixels(psrc, lx    , ly    );
    pixels[1] = clampPixels(psrc, lx + 1, ly    );
    pixels[2] = clampPixels(psrc, lx    , ly + 1);
    pixels[3] = clampPixels(psrc, lx + 1, ly + 1);

    GFX_IMAGE img = { 0 };
    img.mData = pixels;
    img.mWidth = 2;
    img.mHeight = 2;
    img.mRowBytes = 8;
    return bilinearGetPixelCenter(&img, sx & 0xffff, sy & 0xffff);
}

//bilinear get pixel with FIXED-POINT (signed 16.16)
//general optimize version, fast speed
must_inline uint32_t bilinearGetPixelFixed(const GFX_IMAGE* psrc, const int32_t sx, const int32_t sy)
{
    //convert to fixed points
    const int32_t lx = sx >> 16;
    const int32_t ly = sy >> 16;
    const uint32_t u = uint8_t((sx & 0xffff) >> 8);
    const uint32_t v = uint8_t((sy & 0xffff) >> 8);

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

    uint32_t rb = ( p0 & 0x00ff00ff) * w0;
    uint32_t ag = ((p0 & 0xff00ff00) >> 8) * w0;
    rb += ( p1 & 0x00ff00ff) * w2;
    ag += ((p1 & 0xff00ff00) >> 8) * w2;
    rb += ( p2 & 0x00ff00ff) * w1;
    ag += ((p2 & 0xff00ff00) >> 8) * w1;
    rb += ( p3 & 0x00ff00ff) * w3;
    ag += ((p3 & 0xff00ff00) >> 8) * w3;
    return (ag & 0xff00ff00) | ((rb & 0xff00ff00) >> 8);
}

//constant values that will be needed
static const __m256d CONST_1 = _mm256_set1_pd(1);
static const __m256d CONST_256 = _mm256_set1_pd(256);

//AVX2 calculate the weights of pixel
must_inline __m256d calcWeights(const double x, const double y)
{
    __m256d ymm0 = _mm256_set1_pd(x);
    __m256d ymm1 = _mm256_set1_pd(y);
    __m256d ymm2 = _mm256_unpacklo_pd(ymm0, ymm1);

    ymm0 = _mm256_floor_pd(ymm2);
    ymm1 = _mm256_sub_pd(ymm2, ymm0);
    ymm2 = _mm256_sub_pd(CONST_1, ymm1);

    __m256d ymm3 = _mm256_unpacklo_pd(ymm2, ymm1);
    ymm3 = _mm256_permute4x64_pd(ymm3, _MM_SHUFFLE(1, 0, 1, 0));

    __m256d ymm4 = _mm256_permute2f128_pd(ymm2, ymm1, _MM_SHUFFLE2(16, 1));
    ymm4 = _mm256_mul_pd(ymm3, ymm4);

    return _mm256_mul_pd(ymm4, CONST_256);
}

//get pixels bilinear with AVX2
must_inline uint32_t bilinearGetPixelAVX2(const GFX_IMAGE* psrc, const double x, const double y)
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

    //convert RGBA RGBA RGBA RGBA to RRRR GGGG BBBB AAAA
    p12 = _mm_unpacklo_epi8(p12, p34);
    p34 = _mm_unpackhi_epi64(p12, _mm_setzero_si128());
    p12 = _mm_unpacklo_epi8(p12, p34);

    //extend to 16bits
    __m128i rg = _mm_unpacklo_epi8(p12, _mm_setzero_si128());
    __m128i ba = _mm_unpackhi_epi8(p12, _mm_setzero_si128());

    //convert floating points weights to 16bits integer w4 w3 w2 w1
    __m128i weight = _mm256_cvtpd_epi32(calcWeights(x, y));

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

//calculate sin-cos of an angle (merge sin/cos functions)
must_inline void sincos(const double angle, double* sina, double* cosa)
{
#ifdef _USE_ASM
    __asm {
        fld     angle
        mov     eax, sina
        mov     edx, cosa
        fsincos
        fstp    qword ptr[edx]
        fstp    qword ptr[eax]
    }
#else
    *sina = sin(angle);
    *cosa = cos(angle);
#endif
}

//bi-cubic helper (this function will be replaced by sin(x)/x for optimize
must_inline double cubicHermite(const double a, const double b, const double c, const double d, const double fract)
{
    const double aa = -a / 2.0 + 1.5 * b - 1.5 * c + d / 2.0;
    const double bb = a - 2.5 * b + 2.0 * c - d / 2.0;
    const double cc = -a / 2.0 + c / 2.0;
    return aa * fract * fract * fract + bb * fract * fract + cc * fract + b;
}

//calculate function sin(x)/x replacement for cubicHermite
//so this will add to lookup table for speedup improvement
must_inline double sinXDivX(const double b)
{
    //control constant, i.e: -2,-1,-0.75,-0.5
    const double ctl = -1;
    const double x = (b < 0) ? -b : b;
    const double x2 = x * x, x3 = x2 * x;

    if (x <= 1) return (ctl + 2) * x3 - (ctl + 3) * x2 + 1;
    else if (x <= 2) return ctl * x3 - (5 * ctl) * x2 + (8 * ctl) * x - (4 * ctl);
    return 0;
}

//4 signed 32bits sum of bits of data (simulation for _mm_madd_epi32)
must_inline int32_t _mm_hsum_epi32(const __m128i val)
{
    //_mm_extract_epi32 is slower
    __m128i result = _mm_add_epi32(val, _mm_srli_si128(val, 8));
    result = _mm_add_epi32(result, _mm_srli_si128(result, 4));
    return _mm_cvtsi128_si32(result);
}

//calculate pixel by bi-cubic interpolation (original version)
//this is show how to bi-cubic interpolation works, don't use for production
must_inline uint32_t bicubicGetPixel(const GFX_IMAGE* img, const double sx, const double sy)
{
    const int32_t px = int32_t(sx);
    const double fx = sx - int32_t(sx);

    const int32_t py = int32_t(sy);
    const double fy = sy - int32_t(sy);

    const uint32_t width = img->mWidth;
    const uint32_t height = img->mHeight;
    const uint32_t* psrc = (const uint32_t*)img->mData;

    //clamp 16 pixels at center and border
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

    //start interpolate bi-cubically for each pixel channel
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

//this calculate pixel with boundary so quite slowly (using fixed points)
must_inline uint32_t bicubicGetPixelFixed(const GFX_IMAGE* img, const int16_t *sintab, const int32_t sx, const int32_t sy)
{
    //peek offset at (px,py)
    const int32_t px = sx >> 16, py = sy >> 16;

    const uint32_t width = img->mWidth;
    const uint32_t height = img->mHeight;
    const uint32_t* psrc = (const uint32_t*)img->mData;

    //calculate 16 around pixels
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

    //4 pixels weights
    const int32_t u = uint8_t(sx >> 8), v = uint8_t(sy >> 8);
    const int32_t u0 = sintab[256 + u], u1 = sintab[u];
    const int32_t u2 = sintab[256 - u], u3 = sintab[512 - u];
    const int32_t v0 = sintab[256 + v], v1 = sintab[v];
    const int32_t v2 = sintab[256 - v], v3 = sintab[512 - v];

    uint32_t color = 0;
    uint8_t* pcol = (uint8_t*)&color;

    //a,r,g,b channel
    for (int32_t i = 0; i < 4; i++)
    {
        const int32_t s1 = (p00[i] * u0 + p01[i] * u1 + p02[i] * u2 + p03[i] * u3) * v0;
        const int32_t s2 = (p10[i] * u0 + p11[i] * u1 + p12[i] * u2 + p13[i] * u3) * v1;
        const int32_t s3 = (p20[i] * u0 + p21[i] * u1 + p22[i] * u2 + p23[i] * u3) * v2;
        const int32_t s4 = (p30[i] * u0 + p31[i] * u1 + p32[i] * u2 + p33[i] * u3) * v3;
        pcol[i] = clamp((s1 + s2 + s3 + s4) >> 16, 0, 255);
    }

    return color;
}

//fast calculate pixel at center, don't care boundary
must_inline uint32_t bicubicGetPixelCenter(const GFX_IMAGE* img, const int16_t* stable, const int32_t sx, const int32_t sy)
{
    const int32_t pu = uint8_t(sx >> 8), pv = uint8_t(sy >> 8);
    const int16_t u0 = stable[256 + pu], u1 = stable[pu];
    const int16_t u2 = stable[256 - pu], u3 = stable[512 - pu];

    const __m128i xpart = _mm_setr_epi16(u0, u1, u2, u3, u0, u1, u2, u3); //U0 U1 U2 U3 U0 U1 U2 U3
    const __m128i ypart = _mm_setr_epi32(stable[256 + pv], stable[pv], stable[256 - pv], stable[512 - pv]);

    const uint32_t* psrc = (const uint32_t*)img->mData;
    const uint32_t *pixel0 = (const uint32_t*)&psrc[((sy >> 16) - 1) * img->mWidth + ((sx >> 16) - 1)];
    const uint32_t *pixel1 = &pixel0[img->mWidth];
    const uint32_t *pixel2 = &pixel1[img->mWidth];
    const uint32_t *pixel3 = &pixel2[img->mWidth];

    //load 16 pixels for calculation
    __m128i p0 = _mm_lddqu_si128((const __m128i*)pixel0); //P00 P01 P02 P03
    __m128i p1 = _mm_lddqu_si128((const __m128i*)pixel1); //P10 P11 P12 P13
    __m128i p2 = _mm_lddqu_si128((const __m128i*)pixel2); //P20 P21 P22 P23
    __m128i p3 = _mm_lddqu_si128((const __m128i*)pixel3); //P30 P31 P32 P33

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
must_inline uint32_t bicubicGetPixelBorder(const GFX_IMAGE* img, const int16_t *sintab, const int32_t sx, const int32_t sy)
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
    GFX_IMAGE mpic = { 0 };
    mpic.mData = pixels;
    mpic.mWidth = 4;
    mpic.mHeight = 4;
    mpic.mRowBytes = 16;

    //optimize function
    return bicubicGetPixelCenter(&mpic, sintab, (sx & 0xffff) + 0x10000, (sy & 0xffff) + 0x10000);
}

//smooth scaling with Bresenham (internal function)
//because of several simplifications of the algorithm,
//the zoom range is restricted between 0.5 and 2. That
//is: dstwidth must be >= srcwidth/2 and <= 2*srcwidth.
//smooth is used to calculate average pixel and mid-points
void scaleLineMix(uint8_t* dst, uint8_t* src, int32_t dw, int32_t sw, int32_t type)
{
    if (type == INTERPOLATION_TYPE_SMOOTH)
    {
#ifdef _USE_ASM
        uint16_t val = 0;
        __asm {
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
            dec     ecx
            jnz     next
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
    else if (type == INTERPOLATION_TYPE_NEAREST)
    {
#ifdef _USE_ASM
        __asm {
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
            dec     ecx
            jnz     next1
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
    else
    {
        messageBox(GFX_ERROR, "Unknown interpolation type!");
        return;
    }
}

//Bresenham scale line with rgb color
void scaleLineNormal(uint32_t* dst, uint32_t* src, int32_t dw, int32_t sw, int32_t type)
{
    if (type == INTERPOLATION_TYPE_SMOOTH)
    {
#ifdef _USE_ASM
        uint16_t val = 0;
        __asm {
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
            dec     ecx
            jnz     next
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
                //calculate average pixel pd = (p0 + p1) / 2
                ARGB* pd = (ARGB*)dst;
                const ARGB* p0 = (ARGB*)src;
                const ARGB* p1 = (ARGB*)&src[1];
                pd->r = (p0->r + p1->r) >> 1;
                pd->g = (p0->g + p1->g) >> 1;
                pd->b = (p0->b + p1->b) >> 1;
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
    else if (type == INTERPOLATION_TYPE_NEAREST)
    {
#ifdef _USE_ASM
        __asm {
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
            dec     ecx
            jnz     begin
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
    else
    {
        messageBox(GFX_ERROR, "Unknown interpolation type!");
        return;
    }
}

//Bresenham scale image buffer
void scaleImageMix(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t mode)
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
    void *scaler = scaleLineMix;

    //save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    __asm {
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
        mov     edx, mode
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
        dec     ecx
        jnz     next
    }
#else    
    int32_t error = 0;
    int32_t numPixels = dst->mHeight;
    const int32_t intPart = (src->mHeight / dst->mHeight) * src->mWidth;
    const int32_t fractPart = src->mHeight % dst->mHeight;
    
    uint8_t* srcPrev = NULL;
    uint8_t* srcPtr = (uint8_t*)src->mData;
    uint8_t* dstPtr = (uint8_t*)dst->mData;

    while (numPixels-- > 0)
    {
        if (srcPtr == srcPrev)
        {
            memcpy(dstPtr, dstPtr - dst->mWidth, dst->mWidth * sizeof(dstPtr[0]));
        }
        else
        {
            scaleLineMix(dstPtr, srcPtr, dst->mWidth, src->mWidth, mode);
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

//Bresenham scale line with rgb color
void scaleLine(void* dst, void* src, int32_t dw, int32_t sw, int32_t type)
{
    //mixed mode
    if (bitsPerPixel == 8)
    {
        scaleLineMix((uint8_t*)dst, (uint8_t*)src, dw, sw, type);
        return;
    }

    //height color mode
    scaleLineNormal((uint32_t*)dst, (uint32_t*)src, dw, sw, type);
}

//Bresenham scale image buffer
void scaleImageNormal(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t mode)
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
    void *scaler = scaleLine;

    //save local address
    void *oldPtr = NULL;
    void *dstPtr = dst->mData;
    void *srcPtr = src->mData;

    __asm {
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
        push    mode
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
        dec     ecx
        jnz     next
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
            scaleLine(dstPtr, srcPtr, dst->mWidth, src->mWidth, mode);
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

//nearest neighbor image scaling (using fixed-points)
void nearestScaleImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //mapping pointer
    uint32_t* pdst = (uint32_t*)dst->mData;
    const uint32_t* psrc = (const uint32_t*)src->mData;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;

    //calculate scale ratio
    const int32_t scalex = (srcw << 16) / dstw + 1;
    const int32_t scaley = (srch << 16) / dsth + 1;
    
    //very slow loop
    int32_t sy = 0;
    for (int32_t y = 0; y < dsth; y++, sy += scaley)
    {
        int32_t sx = 0;
        const uint32_t* msrc = &psrc[(sy >> 16) * srcw];
        for (int32_t x = 0; x < dstw; x++, sx += scalex) *pdst++ = msrc[sx >> 16];
    }
}

//average pixels (smooth) image scaling (using fixed-points)
void smoothScaleImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //mapping pointer
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;

    //calculate scale ratio
    const int32_t scalex = (srcw << 16) / dstw + 1;
    const int32_t scaley = (srch << 16) / dsth + 1;
    const int32_t errorx = (scalex >> 1) - 32768;
    const int32_t errory = (scaley >> 1) - 32768;

    //very slow loop
    for (int32_t y = 0, sy = errory; y < dsth; y++, sy += scaley)
    {
        for (int32_t x = 0, sx = errorx; x < dstw; x++, sx += scalex) *pdst++ = smoothGetPixel(src, sx, sy);
    }
}

//bilinear scale image with fixed-points
void bilinearScaleImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //mapping pointer
    uint32_t* pdst = (uint32_t*)dst->mData;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;

    //calculate scale ratio
    const int32_t scalex = (srcw << 16) / dstw + 1;
    const int32_t scaley = (srch << 16) / dsth + 1;
    const int32_t errorx = (scalex >> 1) - 32768;
    const int32_t errory = (scaley >> 1) - 32768;

    //very slow loop
    for (int32_t y = 0, sy = errory; y < dsth; y++, sy += scaley)
    {
        for (int32_t x = 0, sx = errorx; x < dstw; x++, sx += scalex) *pdst++ = bilinearGetPixelFixed(src, sx, sy);
    }
}

//maximize optimize version (extremely fast)
//using sticks:
//1. FIXED-POINT
//2. separate inbound and outbound pixels calculation
//3. SSE2 instructions
void bilinearScaleImageMax(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cache local data pointer
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    const int32_t scalex = (srcw << 16) / dstw + 1;
    const int32_t scaley = (srch << 16) / dsth + 1;
    const int32_t errorx = (scalex >> 1) - 32768;
    const int32_t errory = (scaley >> 1) - 32768;

    int32_t startx = (65536 - errorx) / scalex + 1;
    if (startx >= dstw) startx = dstw;

    int32_t starty = (65536 - errory) / scaley + 1;
    if (starty >= dsth) starty = dsth;

    int32_t endx = (((srcw - 2) << 16) - errorx) / scalex + 1;
    if (endx < startx) endx = startx;

    int32_t endy = (((srch - 2) << 16) - errory) / scaley + 1;
    if (endy < starty) endy = starty;

    int32_t srcy = errory;
    for (int32_t y = 0; y < starty; y++, srcy += scaley)
    {
        for (int32_t x = 0, srcx = errorx; x < dstw; x++, srcx += scalex) *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
    }

    for (int32_t y = starty; y < endy; y++, srcy += scaley)
    {
        int32_t srcx = errorx;
        for (int32_t x = 0; x < startx; x++, srcx += scalex)    *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
        for (int32_t x = startx; x < endx; x++, srcx += scalex) *pdst++ = bilinearGetPixelCenter(src, srcx, srcy);
        for (int32_t x = endx; x < dstw; x++, srcx += scalex)   *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
    }

    for (int32_t y = endy; y < dsth; y++, srcy += scaley)
    {
        for (int32_t x = 0, srcx = errorx; x < dstw; x++, srcx += scalex) *pdst++ = bilinearGetPixelBorder(src, srcx, srcy);
    }
}

//normal optimize version (very fast)
//use hardware acceleration with AVX2, seem no faster than FIXED-POINT
//benchmark for 5000 iterations
//CPU: Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz
//RAM: 32GB
//OS: Windows 10 Pro x64
//Compiler: VC++ 2019
//x64 release build
//FIXED-POINT: about 8.649s
//AVX2: about 8.617s --> no much faster than FIXED-POINT
//x32 release build
//FIXED-POINT: about 14.658s
//AVX2: about 9.043s --> seem faster than FIXED-POINT
//use hardware acceleration will get constantly speed
//in modern system (64bits) integer will be operated fastest
void bilinearScaleImageAVX2(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cache local data pointer
    uint32_t* pdst = (uint32_t*)dst->mData;

    //cache local dimension
    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;

    //calculate ratio
    const double xratio = double(intmax_t(srcw) - 1) / dstw;
    const double yratio = double(intmax_t(srch) - 1) / dsth;

    //very slow loop
    for (int32_t y = 0; y < dsth; y++)
    {
        const double sy = y * yratio;
        for (int32_t x = 0; x < dstw; x++)
        {
            const double sx = x * xratio;
            *pdst++ = bilinearGetPixelAVX2(src, sx, sy);
        }
    }
}

//bi-cubic scale image
//original version, very slow
//this is show bi-cubic interpolation only, don't use in production
void bicubicScaleImage(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cache local data pointer
    uint32_t* pdst = (uint32_t*)dst->mData;

    //cache local dimension
    const int32_t swidth = src->mWidth;
    const int32_t sheight = src->mHeight;
    const int32_t dwidth = dst->mWidth;
    const int32_t dheight = dst->mHeight;

    //calculate ratio
    const double xratio = double(intmax_t(swidth) - 1) / dwidth;
    const double yratio = double(intmax_t(sheight) - 1) / dheight;

    //very slow loop
    for (int32_t y = 0; y < dheight; y++)
    {
        const double sy = y * yratio;
        for (int32_t x = 0; x < dwidth; x++)
        {
            const double sx = x * xratio;
            *pdst++ = bicubicGetPixel(src, sx, sy);
        }
    }
}

//maximize optimized version (extremely fast)
//the tricks we're used:
//1. lookup table
//2. fixed-points
//3. SSE2 instructions
//4. separate calculate pixels (inbound and outbound)
void bicubicScaleImageMax(const GFX_IMAGE* dst, const GFX_IMAGE* src)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    uint32_t* pdst = (uint32_t*)dst->mData;
    
    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;
    
    const int32_t addx = (srcw << 16) / dstw + 1;
    const int32_t addy = (srch << 16) / dsth + 1;
    const int32_t errorx = (addx >> 1) - 32768;
    const int32_t errory = (addy >> 1) - 32768;

    int16_t stable[513] = { 0 };
    for (int32_t i = 0; i < 513; i++) stable[i] = fround(256.0 * sinXDivX(i / 256.0));

    int32_t sx = (65536 - errorx) / addx + 1;
    if (sx >= dstw) sx = dstw;
    int32_t sy = (65536 - errory) / addy + 1;
    if (sy >= dsth) sy = dsth;
    int32_t ex = (((srcw - 3) << 16) - errorx) / addx + 1;
    if (ex < sx) ex = sx;
    int32_t ey = (((srch - 3) << 16) - errory) / addy + 1;
    if (ey < sy) ey = sy;

    int32_t srcy = errory;
    for (int32_t y = 0; y < sy; y++, srcy += addy)
    {
        for (int32_t x = 0, srcx = errorx; x < dstw; x++, srcx += addx) *pdst++ = bicubicGetPixelBorder(src, stable, srcx, srcy);
    }

    for (int32_t y = sy; y < ey; y++, srcy += addy)
    {
        int32_t srcx = errorx;
        for (int32_t x = 0; x < sx; x++, srcx += addx)      *pdst++ = bicubicGetPixelBorder(src, stable, srcx, srcy);
        for (int32_t x = sx; x < ex; x++, srcx += addx)     *pdst++ = bicubicGetPixelCenter(src, stable, srcx, srcy);
        for (int32_t x = ex; x < dstw; x++, srcx += addx)   *pdst++ = bicubicGetPixelBorder(src, stable, srcx, srcy);
    }

    for (int32_t y = ey; y < dsth; y++, srcy += addy)
    {
        for (int32_t x = 0, srcx = errorx; x < dstw; x++, srcx += addx) *pdst++ = bicubicGetPixelBorder(src, stable, srcx, srcy);
    }
}

//nearest neighbor image rotation for mixed mode (optimize version using FIXED-POINT)
void rotateImageMix(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel != 8) return;

    //cast to image data
    uint8_t* pdst = (uint8_t*)dst->mData;
    const uint8_t* psrc = (const uint8_t*)src->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dcx = dstw >> 1;
    const int32_t dsth = dst->mHeight;
    const int32_t dcy = dsth >> 1;

    const int32_t srcw = src->mWidth;
    const int32_t scx = srcw << 15;
    const int32_t srch = src->mHeight;
    const int32_t scy = srch << 15;
    const int32_t scw = (srcw - 1) << 16;
    const int32_t sch = (srch - 1) << 16;

    const double alpha = (angle * M_PI) / 180;
    const int32_t dx = fround((sin(alpha) / scalex) * 65536);
    const int32_t dy = fround((cos(alpha) / scaley) * 65536);

    int32_t xs = scx - (dcx * dy + dcy * dx);
    int32_t ys = scy - (dcy * dy - dcx * dx);

    for (int32_t y = 0; y < dsth; y++, xs += dx, ys += dy, pdst += dstw)
    {
        uint8_t* pline = pdst;
        int32_t sx = xs, sy = ys;
        for (int32_t x = 0; x < dstw; x++, sx += dy, sy -= dx)
        {
            if (sx >= 0 && sy >= 0 && sx <= scw && sy <= sch) *pline = psrc[(sy >> 16) * srcw + (sx >> 16)];
            pline++;
        }
    }
}

//nearest neighbor pixel image rotation (optimize version using FIXED-POINT)
void nearestRotateImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;
    const uint32_t* psrc = (const uint32_t*)src->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dcx = dstw >> 1;
    const int32_t dsth = dst->mHeight;
    const int32_t dcy = dsth >> 1;

    const int32_t srcw = src->mWidth;
    const int32_t scx = srcw << 15;
    const int32_t srch = src->mHeight;
    const int32_t scy = srch << 15;
    const int32_t scw = (srcw - 1) << 16;
    const int32_t sch = (srch - 1) << 16;

    const double alpha = (angle * M_PI) / 180;
    const int32_t dx = fround((sin(alpha) / scalex) * 65536);
    const int32_t dy = fround((cos(alpha) / scaley) * 65536);

    int32_t xs = scx - (dcx * dy + dcy * dx);
    int32_t ys = scy - (dcy * dy - dcx * dx);

    for (int32_t y = 0; y < dsth; y++, xs += dx, ys += dy, pdst += dstw)
    {
        int32_t sx = xs, sy = ys;
        uint32_t* pline = pdst;
        for (int32_t x = 0; x < dstw; x++, sx += dy, sy -= dx)
        {
            if (sx >= 0 && sy >= 0 && sx <= scw && sy <= sch) *pline = psrc[(sy >> 16) * srcw + (sx >> 16)];
            pline++;
        }
    }
}

//average (smooth) pixel image rotation (optimize version using FIXED-POINT)
void smoothRotateImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dcx = dstw >> 1;
    const int32_t dsth = dst->mHeight;
    const int32_t dcy = dsth >> 1;

    const int32_t srcw = src->mWidth;
    const int32_t scx = srcw << 15;
    const int32_t srch = src->mHeight;
    const int32_t scy = srch << 15;
    const int32_t scw = (srcw - 1) << 16;
    const int32_t sch = (srch - 1) << 16;

    const double alpha = (angle * M_PI) / 180;
    const int32_t dx = fround((sin(alpha) / scalex) * 65536);
    const int32_t dy = fround((cos(alpha) / scaley) * 65536);

    int32_t xs = scx - (dcx * dy + dcy * dx);
    int32_t ys = scy - (dcy * dy - dcx * dx);

    for (int32_t y = 0; y < dsth; y++, xs += dx, ys += dy, pdst += dstw)
    {
        int32_t sx = xs, sy = ys;
        uint32_t* pline = pdst;
        for (int32_t x = 0; x < dstw; x++, sx += dy, sy -= dx)
        {
            if (sx >= 0 && sy >= 0 && sx <= scw && sy <= sch) *pline = smoothGetPixel(src, sx, sy);
            pline++;
        }
    }
}

//use hardware acceleration with AVX2, seem no faster than FIXED-POINT
//benchmark for 5000 iterations
//CPU: Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz
//RAM: 32GB
//OS: Windows 10 Pro x64
//Compiler: VC++ 2019
//x64 release build
//FIXED-POINT: about 9.486s
//AVX2: about 10.250s --> no faster than FIXED-POINT
//x32 release build
//FIXED-POINT: about 14.768s
//AVX2: about 10.422s --> seem faster than FIXED-POINT
//use hardware acceleration will get constantly speed
//in modern system (64bits) integer will be operated fastest
void bilinearRotateImageAVX2(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dcx = dstw >> 1;
    const int32_t dsth = dst->mHeight;
    const int32_t dcy = dsth >> 1;

    const int32_t srcw = src->mWidth;
    const int32_t scx = srcw >> 1;
    const int32_t srch = src->mHeight;
    const int32_t scy = srch >> 1;

    const double alpha = (angle * M_PI) / 180;
    const double dx = sin(alpha) / scalex;
    const double dy = cos(alpha) / scaley;

    double xs = scx - (dcx * dy + dcy * dx);
    double ys = scy - (dcy * dy - dcx * dx);

    for (int32_t y = 0; y < dsth; y++, xs += dx, ys += dy, pdst += dstw)
    {
        double sx = xs, sy = ys;
        uint32_t* pline = pdst;
        for (int32_t x = 0; x < dstw; x++, sx += dy, sy -= dx)
        {
            if (sx >= 0 && sy >= 0 && sx < srcw && sy < srch) *pline = bilinearGetPixelAVX2(src, sx, sy);
            pline++;
        }
    }
}

//bilinear image rotation (single optimize version)
//this version will faster than SSE2 version, in modern CPU, operating
//on integer will always give lower cost because it aligned memory
void bilinearRotateImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dcx = dstw >> 1;
    const int32_t dsth = dst->mHeight;
    const int32_t dcy = dsth >> 1;

    const int32_t srcw = src->mWidth;
    const int32_t scx = srcw << 15;
    const int32_t srch = src->mHeight;
    const int32_t scy = srch << 15;

    const int32_t scw = (srcw - 1) << 16;
    const int32_t sch = (srch - 1) << 16;

    const double alpha = (angle * M_PI) / 180;
    const int32_t dx = fround((sin(alpha) / scalex) * 65536);
    const int32_t dy = fround((cos(alpha) / scaley) * 65536);

    int32_t xs = scx - (dcx * dy + dcy * dx);
    int32_t ys = scy - (dcy * dy - dcx * dx);

    for (int32_t y = 0; y < dsth; y++, xs += dx, ys += dy, pdst += dstw)
    {
        int32_t sx = xs, sy = ys;
        uint32_t* pline = pdst;
        for (int32_t x = 0; x < dstw; x++, sx += dy, sy -= dx)
        {
            if (sx >= 0 && sy >= 0 && sx <= scw && sy <= sch) *pline = bilinearGetPixelFixed(src, sx, sy);
            pline++;
        }
    }
}

//bilinear rotate scan line (sub-routine of full optimize version)
//improve smooth border when rotating will make image look better
void bilinearRotateLine(uint32_t* pdst, const int32_t boundx0, const int32_t inx0, const int32_t inx1, const int32_t boundx1, const GFX_IMAGE* psrc, int32_t sx, int32_t sy, const int32_t addx, const int32_t addy)
{
    int32_t x = 0;
    for (x = boundx0; x < inx0; x++, sx += addx, sy += addy)    pdst[x] = alphaBlend(pdst[x], bilinearGetPixelBorder(psrc, sx, sy));
    for (x = inx0; x < inx1; x++, sx += addx, sy += addy)       pdst[x] = bilinearGetPixelCenter(psrc, sx, sy);
    for (x = inx1; x < boundx1; x++, sx += addx, sy += addy)    pdst[x] = alphaBlend(pdst[x], bilinearGetPixelBorder(psrc, sx, sy));
}

//maximize optimize version (extremely fast)
//use sticks:
//1. fixed-points
//2. separate inbound and outbound pixel calculation
//3. SSE2 instructions
//4. clipping data
void bilinearRotateImageMax(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    const double scalexy = 1.0 / (scalex * scaley);  
    const double rscalex = scalexy * scaley;
    const double rscaley = scalexy * scalex;

    double sina = 0, cosa = 0;
    sincos(-(angle * M_PI) / 180, &sina, &cosa);
    const int32_t sini = fround(sina * 65536); //convert to fixed points (no truncated)
    const int32_t cosi = fround(cosa * 65536); //convert to fixed points (no truncated)

    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;

    uint32_t* pdst = (uint32_t*)dst->mData;
    
    const int32_t ax = int32_t(rscalex * cosi);
    const int32_t ay = int32_t(rscalex * sini);
    const int32_t bx = int32_t(-rscaley * sini);
    const int32_t by = int32_t(rscaley * cosi);

    const int32_t dcx = dstw >> 1;
    const int32_t dcy = dsth >> 1;
    const int32_t scx = srcw << 15; //(srcw >> 1) << 16 convert to fixed points
    const int32_t scy = srch << 15; //(srch >> 1) << 16 convert to fixed points

    //rotation points
    const int32_t cx = scx - int32_t(dcx * rscalex * cosi - dcy * rscaley * sini);
    const int32_t cy = scy - int32_t(dcx * rscalex * sini + dcy * rscaley * cosi); 

    ROTATE_CLIP clip;
    clip.ax = ax;
    clip.bx = bx;
    clip.ay = ay;
    clip.by = by;
    clip.cx = cx;
    clip.cy = cy;
    clip.dstw = dstw;
    clip.dsth = dsth;
    clip.srcw = srcw;
    clip.srch = srch;
    
    //clipping data
    if (!initClip(&clip, dcx, dcy, 1)) return;

    uint32_t* yline = &pdst[dstw * clip.yDown];
    while (true)
    {
        if (clip.yDown >= dsth) break;
        if (clip.yDown >= 0) bilinearRotateLine(yline, clip.outBound0, clip.inBound0, clip.inBound1, clip.outBound1, src, clip.srcx, clip.srcy, ax, ay);
        if (!nextLineDown(&clip)) break;
        yline += dstw;
    }

    yline = &pdst[dstw * clip.yUp];
    while (nextLineUp(&clip))
    {
        if (clip.yUp < 0) break;
        yline -= dstw;
        if (clip.yUp < dsth) bilinearRotateLine(yline, clip.outBound0, clip.inBound0, clip.inBound1, clip.outBound1, src, clip.srcx, clip.srcy, ax, ay);
    }
}

//bi-cubic rotate image (original version, very slow)
//use maximize optimize version below
void bicubicRotateImage(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;

    //calculate haft dimension
    const int32_t width = src->mWidth;
    const int32_t height = src->mHeight;
    const int32_t cx = (width >> 1) - 1;
    const int32_t cy = (height >> 1) - 1;

    //convert to radian
    const double alpha = double(angle * M_PI) / 180;
    const double sina = sin(-alpha) / scalex;
    const double cosa = cos(-alpha) / scaley;
    const double mx = -cx * cosa + cx;
    const double my = -cx * sina + cy;
    double px = -cy * sina;
    double py = -cy * cosa;
    
    //start pixel manipulation
    for (int32_t y = 0; y < height; y++, px += sina, py += cosa)
    {
        double sx = mx - px;
        double sy = my + py;
        for (int32_t x = 0; x < width; x++, sx += cosa, sy += sina)
        {
            if (sx >= 0.0 && sx <= width - 1.0 && sy >= 0 && sy <= height - 1.0) *pdst = bicubicGetPixel(src, sx, sy);
            pdst++;
        }
    }
}

//single optimize version
//sticks used:
//1. fixed-points
//2. lookup table
void bicubicRotateImageFixed(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    //only works with rgb mode
    if (bitsPerPixel <= 8) return;

    //cast to image data
    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t dstw = dst->mWidth;
    const int32_t dcx = dstw >> 1;
    const int32_t dsth = dst->mHeight;
    const int32_t dcy = dsth >> 1;

    const int32_t srcw = src->mWidth;
    const int32_t scx = srcw << 15;
    const int32_t srch = src->mHeight;
    const int32_t scy = srch << 15;
    const int32_t scw = (srcw - 1) << 16;
    const int32_t sch = (srch - 1) << 16;

    const double alpha = (angle * M_PI) / 180;
    const int32_t dx = fround((sin(alpha) / scalex) * 65536);
    const int32_t dy = fround((cos(alpha) / scaley) * 65536);

    int16_t stable[513] = { 0 };
    for (int32_t i = 0; i < 513; i++) stable[i] = fround(256.0 * sinXDivX(i / 256.0));

    int32_t xs = scx - (dcx * dy + dcy * dx);
    int32_t ys = scy - (dcy * dy - dcx * dx);

    for (int32_t y = 0; y < dsth; y++, xs += dx, ys += dy, pdst += dstw)
    {
        int32_t sx = xs, sy = ys;
        uint32_t* pline = pdst;
        for (int32_t x = 0; x < dstw; x++, sx += dy, sy -= dx)
        {
            if (sx >= 0 && sy >= 0 && sx <= scw && sy <= sch) *pline = bicubicGetPixelFixed(src, stable, sx, sy);
            pline++;
        }
    }
}

//bi-cubic rotate scan line (sub-routine for maximize optimize version)
//this use smooth border when rotating to keep image looked
void bicubicRotateLine(uint32_t* pdst, const int32_t boundx0, const int32_t inx0, const int32_t inx1, const int32_t boundx1, const GFX_IMAGE* psrc, int32_t sx, int32_t sy, const int32_t addx, const int32_t addy, const int16_t* stable)
{
    int32_t x = 0;

    for (x = boundx0; x < inx0; x++, sx += addx, sy += addy)    pdst[x] = alphaBlend(pdst[x], bicubicGetPixelBorder(psrc, stable, sx, sy));
    for (x = inx0; x < inx1; x++, sx += addx, sy += addy)       pdst[x] = bicubicGetPixelCenter(psrc, stable, sx, sy);
    for (x = inx1; x < boundx1; x++, sx += addx, sy += addy)    pdst[x] = alphaBlend(pdst[x], bicubicGetPixelBorder(psrc, stable, sx, sy));
}

//maximize optimize version (extremely fast)
//use sticks:
//1. fixed-points
//2. separate get pixel inbound and outbound of source image
//3. SSE2 instructions
//4. lookup table
//5. clipping data
void bicubicRotateImageMax(const GFX_IMAGE* dst, const GFX_IMAGE* src, const double angle, const double scalex, const double scaley)
{
    const double scalexy = 1.0 / (scalex * scaley);  
    const double rscalex = scalexy * scaley;
    const double rscaley = scalexy * scalex;

    double sina = 0, cosa = 0;
    sincos(-(angle * M_PI) / 180, &sina, &cosa);
    const int32_t sini = fround(sina * 65536); //convert to fixed points (no truncated)
    const int32_t cosi = fround(cosa * 65536); //convert to fixed points (no truncated)

    const int32_t srcw = src->mWidth;
    const int32_t srch = src->mHeight;
    const int32_t dstw = dst->mWidth;
    const int32_t dsth = dst->mHeight;

    uint32_t* pdst = (uint32_t*)dst->mData;

    const int32_t ax = int32_t(rscalex * cosi);
    const int32_t ay = int32_t(rscalex * sini);
    const int32_t bx = -int32_t(rscaley * sini);
    const int32_t by = int32_t(rscaley * cosi);

    const int32_t dcx = dstw >> 1;
    const int32_t dcy = dsth >> 1;
    const int32_t scx = srcw << 15; //(srcw >> 1) << 16 convert to fixed points
    const int32_t scy = srch << 15; //(srch >> 1) << 16 convert to fixed points

    //rotation points
    const int32_t cx = scx - int32_t(dcx * rscalex * cosi - dcy * rscaley * sini);
    const int32_t cy = scy - int32_t(dcx * rscalex * sini + dcy * rscaley * cosi); 

    ROTATE_CLIP clip;
    clip.ax = ax;
    clip.bx = bx;
    clip.ay = ay;
    clip.by = by;
    clip.cx = cx;
    clip.cy = cy;
    clip.dstw = dstw;
    clip.dsth = dsth;
    clip.srcw = srcw;
    clip.srch = srch;

    //clipping data
    if (!initClip(&clip, dcx, dcy, 2)) return;

    int16_t stable[513] = { 0 };
    for (int32_t i = 0; i < 513; i++) stable[i] = fround(256.0 * sinXDivX(i / 256.0));

    uint32_t* yline = &pdst[dstw * clip.yDown];
    while (true)
    {
        if (clip.yDown >= dsth) break;
        if (clip.yDown >= 0) bicubicRotateLine(yline, clip.outBound0, clip.inBound0, clip.inBound1, clip.outBound1, src, clip.srcx, clip.srcy, ax, ay, stable);
        if (!nextLineDown(&clip)) break;
        yline += dstw;
    }

    yline = &pdst[dstw * clip.yUp];
    while (nextLineUp(&clip))
    {
        if (clip.yUp < 0) break;
        yline -= dstw;
        if (clip.yUp < dsth) bicubicRotateLine(yline, clip.outBound0, clip.inBound0, clip.inBound1, clip.outBound1, src, clip.srcx, clip.srcy, ax, ay, stable);
    }
}

//scale image buffer (export function)
void scaleImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t type /* = INTERPOLATION_TYPE_SMOOTH */)
{
    //mixed mode
    if (bitsPerPixel == 8)
    {
        scaleImageMix(dst, src, type);
        return;
    }

    //which type?
    switch (type)
    {
    case INTERPOLATION_TYPE_NORMAL:
        scaleImageNormal(dst, src, 1);
        break;

    case INTERPOLATION_TYPE_NEAREST:
        nearestScaleImageFixed(dst, src);
        break;

    case INTERPOLATION_TYPE_SMOOTH:
        smoothScaleImageFixed(dst, src);
        break;

    case INTERPOLATION_TYPE_BILINEAR:
        bilinearScaleImageMax(dst, src);
        break;

    case INTERPOLATION_TYPE_BICUBIC:
        bicubicScaleImageMax(dst, src);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown scale type:%d", type);
        break;
    }
}

//rotate image buffer (export function)
void rotateImage(const GFX_IMAGE* dst, const GFX_IMAGE* src, double degree, int32_t type /* = INTERPOLATION_TYPE_SMOOTH */)
{
    //mixed mode
    if (bitsPerPixel == 8)
    {
        rotateImageMix(dst, src, degree, 1, 1);
        return;
    }

    //which type?
    switch (type)
    {
    case INTERPOLATION_TYPE_NEAREST:
        nearestRotateImageFixed(dst, src, degree, 1, 1);
        break;

    case INTERPOLATION_TYPE_SMOOTH:
        smoothRotateImageFixed(dst, src, degree, 1, 1);
        break;

    case INTERPOLATION_TYPE_BILINEAR:
        bilinearRotateImageMax(dst, src, degree, 1, 1);
        break;

    case INTERPOLATION_TYPE_BICUBIC:
        bicubicRotateImageMax(dst, src, degree, 1, 1);
        break;

    default:
        messageBox(GFX_WARNING, "Unknown rotate type:%d", type);
        break;
    }
}

//initialize 3D projection params
void initProjection(double theta, double phi, double de, double rho /* = 0 */)
{
    const double ph = (M_PI * phi) / 180;
    const double th = (M_PI * theta) / 180;
    
    sinth = sin(th);
    sinph = sin(ph);
    costh = cos(th);
    cosph = cos(ph);

    sincosx = sinph * costh;
    sinsinx = sinth * sinph;
    coscosx = costh * cosph;
    sincosy = sinth * cosph;

    DE = de;
    RHO = rho;
}

//projection points (x,y,z)
void projette(double x, double y, double z, double *px, double *py)
{
    const double obsX = -x * sinth + y * costh;
    const double obsY = -x * sincosx - y * sinsinx + z * cosph;

    if (projectionType == PROJECTION_TYPE_PERSPECTIVE)
    {
        double obsZ = -x * coscosx - y * sincosy - z * sinph + RHO;
        if (obsZ == 0.0) obsZ = DBL_MIN;
        if (px) *px = (DE * obsX) / obsZ;
        if (py) *py = (DE * obsY) / obsZ;
    }
    else if (projectionType == PROJECTION_TYPE_PARALLELE)
    {
        if (px) *px = DE * obsX;
        if (py) *py = DE * obsY;
    }
    else
    {
        if (px) *px = 0;
        if (py) *py = 0;
        messageBox(GFX_WARNING, "Unknown projection type!");
    }
}

//reset projection parameters
void resetProjection()
{
    RHO = DE = 0;
    sinth = sinph = costh = cosph = 0;
    sincosx = sinsinx = coscosx = sincosy = 0;
}

//set current projection type
void setProjection(PROJECTION_TYPE type)
{
    projectionType = type;
}

//move current cursor in 3D mode
void deplaceEn(double x, double y, double z)
{
    double px = 0, py = 0;
    projette(x, y, z, &px, &py);
    cranX = int32_t(centerX + px * ECHE);
    cranY = int32_t(centerY - py);
    moveTo(cranX, cranY);
}

//draw line from current cursor in 3D mode
void traceVers(double x, double y, double z, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    double px = 0, py = 0;
    projette(x, y, z, &px, &py);
    cranX = int32_t(centerX + px * ECHE);
    cranY = int32_t(centerY - py);
    lineTo(cranX, cranY, col, mode);
}

//move to drawing pointer
void moveTo(int32_t x, int32_t y)
{
    currX = x;
    currY = y;
}

//draw line from current to (x,y)
void lineTo(int32_t x, int32_t y, uint32_t col, int32_t mode /* = BLEND_MODE_NORMAL */)
{
    drawLine(currX, currY, x, y, col, mode);
    moveTo(x, y);
}

//set palette color to render palette table
void setPalette(const RGBA* pal)
{
    SDL_Palette* palette = SDL_GetSurfacePalette(sdlSurface);
    if (palette) SDL_SetPaletteColors(palette, pal, 0, 256);
    render();
}

//get current palette table
void getPalette(RGBA* pal)
{
    const SDL_Palette* palette = SDL_GetSurfacePalette(sdlSurface);
    if (palette) memcpy(pal, palette->colors, palette->ncolors * sizeof(RGBA));
}

//FX-effect: palette rotation
void rotatePalette(int32_t from, int32_t to, int32_t loop, int32_t ms)
{
    RGBA tmp = { 0 };

    RGBA pal[256] = { 0 };
    getPalette(pal);

    //how many steps to rotated
    const uint32_t steps = uint32_t((intptr_t(to) - from) * sizeof(RGBA));

    if (loop > 0)
    {
        while (loop--)
        {
            memcpy(&tmp, &pal[from], sizeof(RGBA));
            memcpy(&pal[from], &pal[from + 1], steps);
            memcpy(&pal[to], &tmp, sizeof(RGBA));
            setPalette(pal);
            delay(ms);
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }
    }
    else
    {
        while (true)
        {
            memcpy(&tmp, &pal[from], sizeof(RGBA));
            memcpy(&pal[from], &pal[from + 1], steps);
            memcpy(&pal[to], &tmp, sizeof(RGBA));
            setPalette(pal);
            delay(ms);
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }
    }
}

//FX-effect: palette fade-in
void fadeIn(const RGBA* dest, uint32_t ms)
{
    RGBA src[256] = { 0 };
    getPalette(src);

    for (int32_t i = 63; i >= 0; i--)
    {
        for (int32_t j = 0; j < 256; j++)
        {
            const int32_t k = i << 2;
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
void fadeOut(const RGBA* dest, uint32_t ms)
{
    RGBA src[256] = { 0 };
    getPalette(src);

    for (int32_t i = 63; i >= 0; i--)
    {
        for (int32_t j = 0; j < 256; j++)
        {
            const int32_t k = i << 2;
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
    RGBA src[256] = { 0 };
    getPalette(src);

    for (int32_t i = 0; i < 64; i++)
    {
        for (int32_t j = 0; j < 256; j++)
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
    RGBA src[256] = { 0 };
    getPalette(src);

    for (int32_t i = 0; i < 64; i++)
    {
        for (int32_t j = 0; j < 256; j++)
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
void fadeDown(RGBA* pal)
{
    for (int32_t i = 0; i < 256; i++)
    {
        if (pal[i].r > 4) pal[i].r -= 2; else pal[i].r = 0;
        if (pal[i].g > 4) pal[i].g -= 2; else pal[i].g = 0;
        if (pal[i].b > 4) pal[i].b -= 2; else pal[i].b = 0;
    }
    setPalette(pal);
}

//convert mixed palette buffer to RGB buffer
void convertPalette(const uint8_t* palette, RGBA* color)
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
void shiftPalette(RGBA* pal)
{
    RGBA* rgb = (RGBA*)pal;
    for (int32_t i = 0; i < 256; i++)
    {
        rgb->r <<= 2;
        rgb->g <<= 2;
        rgb->b <<= 2;
        rgb++;
    }
}

//make a black palette
void clearPalette()
{
    RGBA pal[256] = { 0 };
    memset(pal, 0, sizeof(pal));
    setPalette(pal);
}

//make white palette
void whitePalette()
{
    RGBA pal[256] = { 0 };
    memset(pal, 255, sizeof(pal));
    setPalette(pal);
}

//build default 256 colors palette
void getBasePalette(RGBA* pal)
{
    memcpy(pal, basePalette, sizeof(basePalette));
}

//make linear palette (7 circles color)
void makeLinearPalette()
{
    RGBA pal[256] = { 0 };
    memset(pal, 0, sizeof(pal));

    for (int16_t i = 0; i < 32; i++)
    {
        int16_t j = 16 + i;
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
    RGBA pal[256] = { 0 };

    int32_t r = 0, g = 0, b = 0;
    bool ry = true, gy = true, by = true;

    srand(uint32_t(time(NULL)));

    int32_t rx = (rand() % 5) + 1;
    int32_t gx = (rand() % 5) + 1;
    int32_t bx = (rand() % 5) + 1;
    
    memset(pal, 0, sizeof(pal));

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
    RGBA pal[256] = { 0 };
    memset(pal, 0, sizeof(pal));

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
    RGBA tmp = { 0 };
    RGBA pal[256] = { 0 };

    memset(pal, 0, sizeof(pal));
    getPalette(pal);

    //how many steps to rotated
    const uint32_t steps = (intptr_t(to) - from) * sizeof(RGBA);

    while (step--)
    {
        memcpy(&tmp, &pal[from], sizeof(tmp));
        memcpy(&pal[from], &pal[from + 1], steps);
        memcpy(&pal[to], &tmp, sizeof(tmp));
        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }

    setPalette(pal);
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
void strDelete(char* str, int32_t i, uint32_t num)
{
    if (i < 0 || i >= int32_t(strlen(str))) return;
    memmove(&str[i + 1], &str[i + num], strlen(str) - i - 1);
}

//replace current string with another string
void schRepl(char* str, const char* schr, uint8_t repl)
{
    int32_t pos = strPos(str, schr);
    const uint32_t length = uint32_t(strlen(schr));

    while (pos >= 0)
    {
        strDelete(str, pos, length);
        insertChar(str, repl, pos);
        pos = strPos(str, schr);
    }
}

//character to string conversion
void chr2Str(uint8_t chr, uint8_t num, char* str)
{
    str[0] = chr;
    str[1] = num;
    str[2] = '\0';
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
    __asm {
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
        dec     ecx
        jnz     next
        mov     randSeed, ebx
    }
#else
    //check range
    uint16_t* ptrBuff = (uint16_t*)buff;
    if (!count || !randSeed || !range || !ptrBuff) return;

    const uint32_t val = (((randSeed * factor + 1) >> 16) * range) >> 16;
    for (int32_t i = 0; i < count; i++) *ptrBuff++ = val;

    randSeed = val;
#endif
}

//get current loaded GFX font
GFX_FONT* getFont(int32_t type /* = 0 */)
{
    if (type >= GFX_MAX_FONT) type = GFX_MAX_FONT - 1;
    if (type < 0) type = 0;
    return &gfxFonts[type ? type : fontType];
}

//get current loaded font type
int32_t getFontType()
{
    return fontType;
}

//selected loaded GFX font
void setFontType(int32_t type)
{
    //check current range
    if (type >= GFX_MAX_FONT) type = GFX_MAX_FONT - 1;
    if (type < 0) type = 0;
    fontType = type;
}

//set current font size
void setFontSize(uint32_t size)
{
    GFX_FONT* font = getFont(fontType);

    //have sub-fonts
    if (font->hdr.subFonts > 0)
    {
        //correct sub-fonts number
        if (size > font->hdr.subFonts) size = font->hdr.subFonts;
        //copy sub-fonts hdr
        memcpy(&font->hdr.subData, &font->dataPtr[(intptr_t(font->hdr.subData.endChar) - font->hdr.subData.startChar + 1) * 4 * (intptr_t(font->hdr.subFonts) + 1) + size * sizeof(GFX_CHAR_HEADER)], sizeof(GFX_CHAR_HEADER));
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
    const uint32_t len = uint32_t(strlen(str));
    const GFX_FONT* font = getFont(fontType);

    //check for font is loaded
    if (!font->dataPtr) return 0;
    if (!str || !len) return 0;

    //fixed font, all characters have a same height
    if (font->hdr.flags & GFX_FONT_FIXED) height = font->hdr.subData.height;
    else
    {
        //vector font
        if (font->hdr.flags & GFX_FONT_VECTOR)
        {
            for (i = 0; i < len; i++)
            {
                //skip invalid character
                if (str[i] < font->hdr.subData.startChar || str[i] > font->hdr.subData.endChar) continue;

                //position of raw data of current character
                mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((str[i] - font->hdr.subData.startChar) << 2)];
                size = font->dataPtr[mempos + 1];
                if (size > height) height = size;
            }
            height *= subFonts;
        }
        else
        {
            //BMP1 font
            if (font->hdr.subData.bitsPerPixel == 1)
            {
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < font->hdr.subData.startChar || str[i] > font->hdr.subData.endChar) continue;

                    //position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((str[i] - font->hdr.subData.startChar) << 2)];
                    size = font->dataPtr[mempos + 1];
                    if (size > height) height = size;
                }
            }
            else if (font->hdr.subData.bitsPerPixel >= 2 && font->hdr.subData.bitsPerPixel <= 32)
            {
                //BMP8 and RGB font
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < font->hdr.subData.startChar || str[i] > font->hdr.subData.endChar) continue;

                    //position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((str[i] - font->hdr.subData.startChar) << 2)];
                    size = *(uint32_t*)&font->dataPtr[mempos + 4] + *(uint32_t*)&font->dataPtr[mempos + 12];
                    if (size > height) height = size;
                }
            }
        }
    }

    //animation font
    if (font->hdr.flags & GFX_FONT_ANIPOS) height += font->hdr.subData.randomY;
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
    const uint32_t len = uint32_t(strlen(str));
    const GFX_FONT* font = getFont(fontType);

    //check for font is loaded
    if (!font->dataPtr) return 0;
    if (!str || !len) return 0;

    //fixed font, all characters have a same width
    if (font->hdr.flags & GFX_FONT_FIXED) width = (font->hdr.subData.width + font->hdr.subData.distance) * len;
    else
    {
        //vector font
        if (font->hdr.flags & GFX_FONT_VECTOR)
        {
            for (i = 0; i < len; i++)
            {
                //skip invalid character
                if (str[i] < font->hdr.subData.startChar || str[i] > font->hdr.subData.endChar) size = font->hdr.subData.spacer;
                else
                {
                    //position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((str[i] - font->hdr.subData.startChar) << 2)];
                    data = (GFX_STROKE_DATA*)&font->dataPtr[mempos];
                    size = data->width * subFonts;
                }
                width += size + font->hdr.subData.distance;
            }
        }
        else
        {
            //BMP1 font
            if (font->hdr.subData.bitsPerPixel == 1)
            {
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < font->hdr.subData.startChar || str[i] > font->hdr.subData.endChar)
                    {
                        width += font->hdr.subData.spacer + font->hdr.subData.distance;
                        continue;
                    }

                    //position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((str[i] - font->hdr.subData.startChar) << 2)];
                    width += *(uint8_t*)&font->dataPtr[mempos] + font->hdr.subData.distance;
                }
            }
            else if (font->hdr.subData.bitsPerPixel >= 2 && font->hdr.subData.bitsPerPixel <= 32)
            {
                //BMP8 and RGB font
                for (i = 0; i < len; i++)
                {
                    //skip invalid character
                    if (str[i] < font->hdr.subData.startChar || str[i] > font->hdr.subData.endChar)
                    {
                        width += font->hdr.subData.spacer + font->hdr.subData.distance;
                        continue;
                    }

                    //position of raw data of current character
                    mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((str[i] - font->hdr.subData.startChar) << 2)];
                    width += *(uint32_t*)&font->dataPtr[mempos] + *(uint32_t*)&font->dataPtr[mempos + 8] + font->hdr.subData.distance;
                }
            }
        }
    }

    //animation font
    if (font->hdr.flags & GFX_FONT_ANIPOS) width += font->hdr.subData.randomX;
    return width - font->hdr.subData.distance;
}

//load font name to memory
int32_t loadFont(const char* fname, int32_t type)
{
    //check for type range
    if (type >= GFX_MAX_FONT)
    {
        messageBox(GFX_WARNING, "Error load font: %s! unknown font type!", fname);
        return 0;
    }

    //open font file
    FILE* fp = fopen(fname, "rb");
    if (!fp)
    {
        messageBox(GFX_WARNING, "Error load font: %s! file not found!", fname);
        return 0;
    }

    //read font hdr
    fread(&gfxFonts[type].hdr, sizeof(GFX_FONT_HEADER), 1, fp);

    //check font signature, version number and memory size
    if (memcmp(gfxFonts[type].hdr.signature, "Fnt2", 4) || gfxFonts[type].hdr.version != 0x0101 || !gfxFonts[type].hdr.memSize)
    {
        fclose(fp);
        messageBox(GFX_WARNING, "Error load font: %s! wrong GFX font!", fname);
        return 0;
    }

    //allocate raw data buffer
    gfxFonts[type].dataPtr = (uint8_t*)calloc(gfxFonts[type].hdr.memSize, 1);
    if (!gfxFonts[type].dataPtr)
    {
        fclose(fp);
        messageBox(GFX_WARNING, "Error load font: %s! not enough memory!", fname);
        return 0;
    }

    //read raw font data
    fread(gfxFonts[type].dataPtr, gfxFonts[type].hdr.memSize, 1, fp);
    fclose(fp);

    //reset font header for old font
    if (gfxFonts[type].hdr.flags & GFX_FONT_MULTI) setFontSize(0);

    //default sub-fonts
    if (gfxFonts[type].hdr.flags & GFX_FONT_VECTOR) subFonts = 1;
    else subFonts = 0;

    //BMP8 font palette
    if (gfxFonts[type].hdr.subData.usedColors > 1)
    {
        //BMP8 use up to 128 colors (128 * 4)
        fontPalette[type] = (uint8_t*)calloc(512, 1);
        if (!fontPalette[type])
        {
            free(gfxFonts[type].dataPtr);
            messageBox(GFX_WARNING, "Error load font: %s! wrong font palette!", fname);
            return 0;
        }
    }

    return 1;
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
    memset(&gfxFonts[type].hdr, 0, sizeof(GFX_FONT_HEADER));
}

//draw a stroke of BGI font (YES we also support BGI font)
int32_t outStroke(int32_t x, int32_t y, char chr, uint32_t col, uint32_t mode)
{
    const GFX_FONT* font = getFont(fontType);

    //check for font is loaded
    if (!font->dataPtr) return 0;

    //check for non-drawable character
    if (font->hdr.subData.startChar > chr || font->hdr.subData.endChar < chr) return font->hdr.subData.spacer;

    //memory position of character
    uint32_t mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((chr - font->hdr.subData.startChar) << 2)];
    const GFX_STROKE_DATA* data = (GFX_STROKE_DATA*)&font->dataPtr[mempos];
    GFX_STROKE_INFO* stroke = (GFX_STROKE_INFO*)&font->dataPtr[mempos + sizeof(GFX_STROKE_DATA)];

    //scan for all lines
    for (int32_t i = 0; i < data->numOfLines; i++)
    {
        const uint32_t mx = x + stroke->x * subFonts;
        const uint32_t my = y + stroke->y * subFonts;

        //check for drawable
        if (stroke->code == 1) moveTo(mx, my);
        else
        {
            if (mode == 2) lineTo(mx, my, col, BLEND_MODE_ADD);
            else if (mode == 3) lineTo(mx, my, col, BLEND_MODE_SUB);
            else lineTo(mx, my, col);
        }
        stroke++;
    }

    return data->width * subFonts;
}

//draw string with current loaded font
void writeString(int32_t x, int32_t y, uint32_t col, uint32_t mode, const char* str)
{
    uint32_t i = 0, cx = 0, cy = 0;
    uint32_t data = 0, datapos = 0, mempos = 0;
    uint32_t width = 0, height = 0, addx = 0, addy = 0;
    const GFX_FONT* font = getFont(fontType);

    //check for font is loaded
    if (!font->dataPtr) return;

    const uint32_t len = uint32_t(strlen(str));

    //check for vector font
    if (font->hdr.flags & GFX_FONT_VECTOR)
    {
        for (i = 0; i < len; i++)
        {
            x += outStroke(x, y, str[i], col, mode) + font->hdr.subData.distance;
            if (mode == 1) col++;
        }
        return;
    }

    //BMP1 font format
    if (font->hdr.subData.bitsPerPixel == 1)
    {
        for (i = 0; i < len; i++)
        {
            //invalid character, update position
            if (uint8_t(str[i]) < font->hdr.subData.startChar || uint8_t(str[i]) > font->hdr.subData.endChar)
            {
                if (!(font->hdr.flags & GFX_FONT_FIXED)) x += font->hdr.subData.spacer + font->hdr.subData.distance;
                else x += font->hdr.subData.width + font->hdr.subData.distance;
                continue;
            }

            //memory position for each character
            mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((uint8_t(str[i]) - font->hdr.subData.startChar) << 2)];
            width = font->dataPtr[mempos];
            height = font->dataPtr[mempos + 1];
            mempos += 2;

            //scans for font width and height
            for (cy = 0; cy < height; cy++)
            {
                datapos = 0;
                data = *(uint32_t*)&font->dataPtr[mempos];
                for (cx = 0; cx < width; cx++)
                {
                    if ((cx > 31) && !(cx & 31))
                    {
                        datapos += 4;
                        data = *(uint32_t*)&font->dataPtr[mempos + datapos];
                    }
                    if (data & (1 << (cx & 31)))
                    {
                        if (mode == 2) putPixel(x + cx, y + cy, col, BLEND_MODE_ADD);
                        else if (mode == 3) putPixel(x + cx, y + cy, col, BLEND_MODE_SUB);
                        else putPixel(x + cx, y + cy, col);
                    }
                }
                mempos += font->hdr.subData.bytesPerLine;
            }
            x += width + font->hdr.subData.distance;
            if (mode == 1) col++;
        }
    }
    //BMP8 font format
    else if (font->hdr.subData.bitsPerPixel > 1 && font->hdr.subData.bitsPerPixel < 8)
    {
        //calculate font palette, use for hi-color and true-color
        if (bitsPerPixel > 8)
        {
            const ARGB* pcol = (const ARGB*)&col;
            for (i = 0; i < font->hdr.subData.usedColors; i++)
            {
                ARGB* pixels = (ARGB*)&fontPalette[fontType][i << 2];
                pixels->r = pcol->r * (i + 1) / font->hdr.subData.usedColors;
                pixels->g = pcol->g * (i + 1) / font->hdr.subData.usedColors;
                pixels->b = pcol->b * (i + 1) / font->hdr.subData.usedColors;
            }
        }

        //generate random position for animation font
        if (font->hdr.flags & GFX_FONT_ANIPOS)
        {
            randomBuffer(gfxBuff, len + 1, font->hdr.subData.randomX);
            randomBuffer(&gfxBuff[512], len + 1, font->hdr.subData.randomY);
        }

        for (i = 0; i < len; i++)
        {
            //invalid character, update character position
            if (uint8_t(str[i]) < font->hdr.subData.startChar || uint8_t(str[i]) > font->hdr.subData.endChar)
            {
                if (!(font->hdr.flags & GFX_FONT_FIXED)) x += font->hdr.subData.spacer + font->hdr.subData.distance;
                else x += font->hdr.subData.width + font->hdr.subData.distance;
                continue;
            }

            //lookup character position
            mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((uint8_t(str[i]) - font->hdr.subData.startChar) << 2)];
            addx = *(uint32_t*)&font->dataPtr[mempos];
            addy = *(uint32_t*)&font->dataPtr[mempos + 4];

            //update position for animation font
            if (font->hdr.flags & GFX_FONT_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[(i << 1) + 512];
            }

            //get font width and height
            width = *(uint32_t*)&font->dataPtr[mempos + 8];
            height = *(uint32_t*)&font->dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            //scans font raw data
            for (cy = 0; cy < height; cy++)
            {
                for (cx = 0; cx < width; cx++)
                {
                    data = font->dataPtr[mempos++];
                    if (!(data & 0x80))
                    {
                        if (bitsPerPixel == 8) putPixel(x + cx, y + cy + addy, data);
                        else if (mode == 2) putPixel(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2], BLEND_MODE_ADD);
                        else if (mode == 3) putPixel(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2], BLEND_MODE_SUB);
                        else putPixel(x + cx, y + cy + addy, *(uint32_t*)&fontPalette[fontType][data << 2]);
                    }
                }
            }

            //update next position
            if (font->hdr.flags & GFX_FONT_ANIPOS) x -= gfxBuff[i << 1];
            if (font->hdr.flags & GFX_FONT_FIXED) x += font->hdr.subData.width + font->hdr.subData.distance;
            else x += width + font->hdr.subData.distance;
        }
    }
    //alpha channel font
    else if (font->hdr.subData.bitsPerPixel == 32)
    {
        //generate random position for animation font
        if (font->hdr.flags & GFX_FONT_ANIPOS)
        {
            randomBuffer(gfxBuff, len + 1, font->hdr.subData.randomX);
            randomBuffer(&gfxBuff[512], len + 1, font->hdr.subData.randomY);
        }

        for (i = 0; i < len; i++)
        {
            //invalid character, update character position
            if (uint8_t(str[i]) < font->hdr.subData.startChar || uint8_t(str[i]) > font->hdr.subData.endChar)
            {
                if (!(font->hdr.flags & GFX_FONT_FIXED)) x += font->hdr.subData.spacer + font->hdr.subData.distance;
                else x += font->hdr.subData.width + font->hdr.subData.distance;
                continue;
            }

            //lookup character position
            mempos = *(uint32_t*)&font->dataPtr[font->hdr.subData.startOffset + ((uint8_t(str[i]) - font->hdr.subData.startChar) << 2)];
            addx = *(uint32_t*)&font->dataPtr[mempos];
            addy = *(uint32_t*)&font->dataPtr[mempos + 4];

            //update position for animation font
            if (font->hdr.flags & GFX_FONT_ANIPOS)
            {
                addx += gfxBuff[i << 1];
                addy += gfxBuff[(i << 1) + 512];
            }

            //get font width and height
            width = *(uint32_t*)&font->dataPtr[mempos + 8];
            height = *(uint32_t*)&font->dataPtr[mempos + 12];
            mempos += 16;
            x += addx;

            //scans raw font data
            for (cy = 0; cy < height; cy++)
            {
                for (cx = 0; cx < width; cx++)
                {
                    data = *(uint32_t*)&font->dataPtr[mempos];
                    putPixel(x + cx, y + addy + cy, rgba(data, 255), BLEND_MODE_ALPHA);
                    mempos += 4;
                }
            }

            //update next position
            if (font->hdr.flags & GFX_FONT_ANIPOS) x -= gfxBuff[i << 1];
            if (font->hdr.flags & GFX_FONT_FIXED) x += font->hdr.subData.width + font->hdr.subData.distance;
            else x += width + font->hdr.subData.distance;
        }
    }
}

//print text with format to display screen
void writeText(int32_t x, int32_t y, uint32_t color, uint32_t mode, const char* format, ...)
{
    char buffer[1024] = { 0 };
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    writeString(x, y, color, mode, buffer);
}

//draw multi-line string font
int32_t drawText(const char* const str[], uint32_t count, int32_t ypos)
{
    //check for font loaded
    if (!gfxFonts[fontType].dataPtr) return 0;
    for (uint32_t i = 0; i < count; i++)
    {
        if (ypos > -30) writeString(centerX - (getFontWidth(str[i]) >> 1), ypos, 62, 0, str[i]);
        ypos += getFontHeight(str[i]);
        if (ypos > cmaxY) break;
    }

    return ypos;
}

//fast show BMP image to screen (demo version)
void showBMP(const char* fname)
{
    GFX_IMAGE bmp;
    loadImage(fname, &bmp);
    putImage(0, 0, &bmp);
    render();
    freeImage(&bmp);
}

//fast show 32 bits PNG image to screen (demo version)
void showPNG(const char* fname)
{
    GFX_IMAGE png;
    loadImage(fname, &png);

    //background color
    const uint32_t cols[2] = { RGB_GREY191, RGB_WHITE };

    //make background
    for (int32_t y = 0; y < texHeight; y++)
    {
        for (int32_t x = 0; x < texWidth; x++) fillRect(alignedSize(x), y, 8, 8, cols[((x ^ y) >> 3) & 1]);
    }

    //render image
    putImage(0, 0, &png, BLEND_MODE_ALPHA);
    render();
    freeImage(&png);
}

//fast show JPG image to screen (demo version)
void showJPG(const char* fname)
{
    //load jpeg image with SIMD support
    GFX_IMAGE jpg;
    loadImage(fname, &jpg);

    //render image
    putImage(0, 0, &jpg, BLEND_MODE_NORMAL);
    render();
    freeImage(&jpg);
}

//load image as 8bits texture to output buffer
int32_t loadPNG(uint8_t* raw, RGBA* pal, const char* fname)
{
    //simple image loader for all supported types
    SDL_Surface* image = IMG_Load(fname);
    if (!image)
    {
        messageBox(GFX_ERROR, "Load image error: %s", IMG_GetError());
        return 0;
    }

    //check image format type (must 256 palette colors)
    if (image->format != SDL_PIXELFORMAT_INDEX8)
    {
        messageBox(GFX_ERROR, "Only 8 bits image format is supported! %s", fname);
        return 0;
    }

    //copy raw data and palette
    if (raw) memcpy(raw, image->pixels, intptr_t(image->pitch) * image->h);
    
    //copy palette color
    if (pal)
    {
        const SDL_Palette* palette = SDL_GetSurfacePalette(image);
        if (palette) memcpy(pal, palette->colors, palette->ncolors * sizeof(RGBA));
    }

    SDL_DestroySurface(image);
    return 1;
}

//load image as GFXLIB texture format
int32_t loadImage(const char* fname, GFX_IMAGE* im)
{
    //simple image loader for all supported types
    SDL_Surface* image = IMG_Load(fname);
    if (!image)
    {
        messageBox(GFX_ERROR, "Load image error: %s", IMG_GetError());
        return 0;
    }

    //create 32bits texture to convert image format
    SDL_Surface* texture = SDL_CreateSurface(image->w, image->h, SDL_GetPixelFormatForMasks(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000));
    if (!texture)
    {
        SDL_DestroySurface(image);
        messageBox(GFX_ERROR, "Error create surface: %s!", SDL_GetError());
        return 0;
    }

    //convert to target pixel format
    if (SDL_BlitSurface(image, NULL, texture, NULL))
    {
        SDL_DestroySurface(image);
        SDL_DestroySurface(texture);
        messageBox(GFX_ERROR, "Error convert texture: %s!", SDL_GetError());
        return 0;
    }

    //build GFX texture
    if (!newImage(texture->w, texture->h, im))
    {
        messageBox(GFX_ERROR, "Error create new image!");
        SDL_DestroySurface(image);
        SDL_DestroySurface(texture);
        return 0;
    }

    //copy data to image buffer
    memcpy(im->mData, texture->pixels, im->mSize);
    SDL_DestroySurface(image);
    SDL_DestroySurface(texture);
    return 1;
}

//load image as 32bits texture to output buffer
int32_t loadTexture(uint32_t** txout, int32_t* txw, int32_t* txh, const char* fname)
{
    //simple load image from file
    SDL_Surface* image = IMG_Load(fname);
    if (!image)
    {
        messageBox(GFX_ERROR, "Error load texture: %s", IMG_GetError());
        return 0;
    }

    //create 32bits texture
    SDL_Surface* texture = SDL_CreateSurface(image->w, image->h, SDL_GetPixelFormatForMasks(32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000));
    if (!texture)
    {
        SDL_DestroySurface(image);
        messageBox(GFX_ERROR, "Error create surface: %s!", SDL_GetError());
        return 0;
    }

    //convert raw data to texture format
    if (SDL_BlitSurface(image, NULL, texture, NULL))
    {
        SDL_DestroySurface(image);
        SDL_DestroySurface(texture);
        messageBox(GFX_ERROR, "Error convert texture: %s!", SDL_GetError());
        return 0;
    }

    //create output data buffer
    const uint32_t size = texture->pitch * texture->h;
    txout[0] = (uint32_t*)calloc(size, 1);
    if (!txout[0])
    {
        SDL_DestroySurface(image);
        SDL_DestroySurface(texture);
        messageBox(GFX_ERROR, "Error alloc memory!");
        return 0;
    }

    //copy raw data after converted
    memcpy(txout[0], texture->pixels, size);
    if (txw) *txw = texture->w;
    if (txh) *txh = texture->h;
    SDL_DestroySurface(image);
    SDL_DestroySurface(texture);
    return 1;
}

//initialize mouse driver and bitmap mouse image
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
    //only rgb mode
    if (bitsPerPixel <= 8) return;

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
    __asm {
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
        and     eax, 00ffffffh
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
    const int32_t addOffs = texWidth - msWidth;
    uint32_t* srcPixels = (uint32_t*)drawBuff + intptr_t(texWidth) * my + mx;

    //scan bitmap data
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
                if (*msImage & 0x00ffffff) *srcPixels = *msImage;
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
}

//hide the mouse cursor
void clearMouseCursor(GFX_MOUSE* mi)
{
    int32_t mx = mi->msPosX - mi->msBitmap->mbHotX;
    int32_t my = mi->msPosY - mi->msBitmap->mbHotY;

    //only rgb mode
    if (bitsPerPixel <= 8) return;

    //check clip boundary
    if (mx < cminX) mx = cminX;
    if (mx > cmaxX) mx = cmaxX;
    if (my < cminY) my = cminY;
    if (my > cmaxY) my = cmaxY;

    const int32_t msWidth = mi->msWidth;
    const int32_t msHeight = mi->msHeight;
    uint32_t* msUnder = (uint32_t*)mi->msUnder;

#ifdef _USE_ASM
    __asm {
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
    const int32_t addOffs = texWidth - msWidth;
    uint32_t* pdata = (uint32_t*)drawBuff;
    uint32_t* dstPixels = &pdata[texWidth * my + mx];

    //scan bitmap data
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
 }

//draw bitmap button
void drawButton(GFX_BUTTON* btn)
{
    //only rgb mode
    if (bitsPerPixel <= 8) return;

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
    void* btnData = btn->btData[btn->btState % BUTTON_STATE_COUNT];

#ifdef _USE_ASM
    __asm {
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
        and     eax, 00ffffffh
        test    eax, eax
        jz      skip
        stosd
        jmp     done
    skip:
        add     edi, 4
    done:
        dec     ecx
        jnz     plot
        add     edi, ebx
        add     esi, edx
        dec     lbHeight
        jnz     next
    }
#else
    //calculate starting address
    const int32_t addDstOffs = texWidth - lbWidth;
    const int32_t addImgOffs = btnWidth - lbWidth;

    uint32_t* dstData = (uint32_t*)drawBuff;
    uint32_t* srcData = (uint32_t*)btnData;
    uint32_t* dstPixels = &dstData[texWidth * ly1 + lx1];
    uint32_t* srcPixels = &srcData[btnWidth * (ly1 - y1) + (lx1 - x1)];

    //scan button image
    for (int32_t i = 0; i < lbHeight; i++)
    {
        for (int32_t j = 0; j < lbWidth; j++)
        {
            if (*srcPixels & 0x00ffffff) *dstPixels = *srcPixels;
            dstPixels++;
            srcPixels++;
        }
        if (addDstOffs > 0) dstPixels += addDstOffs;
        if (addImgOffs > 0) srcPixels += addImgOffs;
    }
#endif
}

//load mouse sprite pointer
void loadMouse(const char* fname, GFX_MOUSE* mi, GFX_BITMAP* mbm)
{
    //load mouse pointers
    GFX_IMAGE msPointer = { 0 };
    if (!loadImage(fname, &msPointer))
    {
        messageBox(GFX_ERROR, "Cannot load mouse image:%s", fname);
        return;
    }

    const int32_t msHeight = msPointer.mHeight;
    const int32_t msWidth = msPointer.mWidth / 9;
    const uint32_t msize = msWidth * msHeight;
    const uint32_t bytesLine = msWidth * bytesPerPixel;

    //allocate memory for mouse under background
    mi->msUnder = (uint8_t*)calloc(msize, bytesPerPixel);
    if (!mi->msUnder)
    {
        messageBox(GFX_ERROR, "Error alloc memory!");
        return;
    }

    //init mouse image width and height
    mi->msWidth = msWidth;
    mi->msHeight = msHeight;

    //copy mouse cursors
    for (int32_t i = 0; i < MOUSE_SPRITE_COUNT; i++)
    {
        mbm[i].mbData = (uint8_t*)calloc(msize, bytesPerPixel);
        if (!mbm[i].mbData)
        {
            messageBox(GFX_ERROR, "Error create mouse sprite:%d!", i);
            return;
        }

        //initialize mouse state
        mbm[i].mbHotX = 12;
        mbm[i].mbHotY = 12;
        mbm[i].mbNext = &mbm[i + 1];

        //copy data from mouse image to gfx mouse struct
        const int32_t mwidth = i * msWidth;
        for (int32_t y = 0; y < msHeight; y++)
        {
            uint8_t* dst = &mbm[i].mbData[y * bytesLine];
            const uint8_t* psrc = (const uint8_t*)msPointer.mData;
            const uint8_t* src = &psrc[(mwidth + y * msPointer.mWidth) * bytesPerPixel];
            memcpy(dst, src, bytesLine);
        }
    }

    //init current and next mouse animated
    mbm[0].mbHotX = 7;
    mbm[0].mbHotY = 2;
    mbm[0].mbNext = &mbm[0];
    mbm[8].mbNext = &mbm[1];
    freeImage(&msPointer);
}

//load GFX button
void loadButton(const char* fname, GFX_BUTTON* btn)
{
    //load button
    GFX_IMAGE img = { 0 };
    if (!loadImage(fname, &img))
    {
        messageBox(GFX_ERROR, "Cannot load image button:%s", fname);
        return;
    }

    const int32_t btnHeight = img.mHeight;
    const int32_t btnWidth = img.mWidth / BUTTON_STATE_COUNT;
    const uint32_t msize = btnWidth * btnHeight;
    const uint32_t bytesLine = btnWidth * bytesPerPixel;

    btn->btWidth = btnWidth;
    btn->btHeight = btnHeight;

    //create button
    for (int32_t i = 0; i < BUTTON_STATE_COUNT; i++)
    {
        btn->btData[i] = (uint8_t*)calloc(msize, bytesPerPixel);
        if (!btn->btData[i])
        {
            messageBox(GFX_ERROR, "Error create button:%d!", i);
            return;
        }

        //copy button data
        const int32_t bwidth = i * btnWidth;
        for (int32_t y = 0; y < btnHeight; y++)
        {
            uint8_t* dst = &btn->btData[i][y * bytesLine];
            const uint8_t* psrc = (const uint8_t*)img.mData;
            const uint8_t* src = &psrc[(bwidth + y * img.mWidth) * bytesPerPixel];
            memcpy(dst, src, bytesLine);
        }
    }

    freeImage(&img);
}

//release mouse sprite
void freeMouse(GFX_MOUSE* mi, GFX_BITMAP* mbm)
{
    //cleanup mouse bitmap
    for (int32_t i = 0; i < MOUSE_SPRITE_COUNT; i++)
    {
        if (mbm[i].mbData)
        {
            free(mbm[i].mbData);
            mbm[i].mbData = NULL;
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

//free button
void freeButton(GFX_BUTTON* btn)
{
    for (int32_t i = 0; i < BUTTON_STATE_COUNT; i++)
    {
        if (btn->btData[i])
        {
            free(btn->btData[i]);
            btn->btData[i] = NULL;
        }
    }
}

//simulation mouse event handler
void handleMouseButton()
{
    const char* const bkg[] = { "assets/1lan8.bmp", "assets/1lan16.bmp", "assets/1lan24.bmp", "assets/1lan32.bmp" };

    //init and setup bitmap mouse and button
    GFX_MOUSE mi = { 0 };
    if (!initMouseButton(&mi))
    {
        messageBox(GFX_ERROR, "Error initialize mouse button!");
        return;
    }
    
    GFX_BITMAP mbm[MOUSE_SPRITE_COUNT] = { 0 };
    loadMouse("assets/mpointer24.png", &mi, mbm);

    GFX_BUTTON btn[BUTTON_HANDLE_COUNT] = { 0 };
    loadButton("assets/clickbtn24.png", &btn[0]);
    loadButton("assets/exitbtn24.png", &btn[1]);

    //init button 'click me'
    btn[0].btPosX = centerX - btn[0].btWidth - 20;
    btn[0].btPosY = centerY - (btn[0].btHeight >> 1);
    btn[0].btState = BUTTON_STATE_NORMAL;

    //init button 'exit'
    btn[1].btPosX = centerX + btn[1].btWidth + 10;
    btn[1].btPosY = centerY - (btn[1].btHeight >> 1);
    btn[1].btState = BUTTON_STATE_NORMAL;

    //hide mouse pointer
    hideMouseCursor();

    //install user-define mouse handler
    setMousePosition(centerX, centerY);

    //init mouse normal and wait cursor bitmap
    GFX_BITMAP* msNormal = &mbm[0];
    GFX_BITMAP* msWait = &mbm[1];

    mi.msBitmap = msNormal;

    //setup screen background
    showBMP(bkg[bytesPerPixel - 1]);
    drawMouseCursor(&mi);

    //update last mouse pos
    int32_t lastx = centerX;
    int32_t lasty = centerY;
    uint64_t lastTime = getTime();
    
    //current mouse pointer
    GFX_BITMAP* msNew = NULL;

    int32_t i = 0, done = 0;
    int32_t mcx = 0, mdx = 0, mbx = 0;
    uint32_t needDraw = 0xffff;

    do
    {
        //get current mouse coordinate
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
                for (i = 0; i < BUTTON_HANDLE_COUNT; i++)
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
        for (i = 0; i < BUTTON_HANDLE_COUNT; i++)
        {
            //check if mouse inside the button region
            if (mcx >= btn[i].btPosX && mcx <= btn[i].btPosX + btn[i].btWidth && mdx >= btn[i].btPosY && mdx <= btn[i].btPosY + btn[i].btHeight)
            {
                if (mbx == 0 && btn[i].btState == BUTTON_STATE_PRESSED)
                {
                    btn[i].btState = BUTTON_STATE_ACTIVE;
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
                    btn[i].btState = BUTTON_STATE_PRESSED;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == BUTTON_STATE_NORMAL && mbx == 0)
                {
                    btn[i].btState = BUTTON_STATE_ACTIVE;
                    needDraw |= (2 << i);
                }
                else if (btn[i].btState == BUTTON_STATE_WAITING)
                {
                    if (mbx == 1)
                    {
                        btn[i].btState = BUTTON_STATE_PRESSED;
                    }
                    else
                    {
                        btn[i].btState = BUTTON_STATE_ACTIVE;
                    }
                    needDraw |= (2 << i);
                }
            }
            else if (btn[i].btState == BUTTON_STATE_ACTIVE)
            {
                btn[i].btState = BUTTON_STATE_NORMAL;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == BUTTON_STATE_PRESSED && mbx == 1)
            {
                btn[i].btState = BUTTON_STATE_WAITING;
                needDraw |= (2 << i);
            }
            else if (btn[i].btState == BUTTON_STATE_WAITING && mbx == 0)
            {
                btn[i].btState = BUTTON_STATE_NORMAL;
                needDraw |= (2 << i);
            }
        }

        render();
        delay(FPS_60);
    } while (!finished(SDL_SCANCODE_RETURN) && !done);

    //release mouse bitmap and callback handler
    freeMouse(&mi, mbm);
    freeButton(&btn[0]);
    freeButton(&btn[1]);
    showMouseCursor();
}

//create texture plasma
void createPlasma(uint8_t* dx, uint8_t* dy, const uint8_t* sint, const uint8_t* cost, GFX_IMAGE* img)
{
#ifdef _USE_ASM
    const uint8_t lx = (*dx) += 2;
    const uint8_t ly = (*dy)--;
    
    void* data = img->mData;
    const uint32_t ofs = img->mWidth;
    const uint8_t sx = img->mWidth >> 1;
    const uint8_t sy = img->mHeight >> 1;

    __asm {
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
    const uint16_t cwidth = img->mWidth >> 1;
    const uint16_t cheight = img->mHeight >> 1;

    uint16_t ofs = 0;
    uint16_t* data = (uint16_t*)img->mData;

    for (uint16_t sy = 0; sy < cheight; sy++)
    {
        uint16_t val = sy + ly;
        if (val > 255) val -= 255;

        const uint8_t cl = sint[val];
        const uint8_t ch = sint[lx];

        for (uint16_t sx = 0; sx < cwidth; sx++)
        {
            val = cl + sx;
            if (val > 255) val -= 255;
            val = (sint[val] + cost[(ch + sy) & 0xff]) & 0xff;
            val = (val >> 1) + 128;
            val = (val << 8) | (val & 0xff);
            data[ofs] = val;
            data[ofs + cwidth] = val;
            ofs++;
        }
        ofs += cwidth;
    }
#endif
}

//initialize texture plasma
void initPlasma(uint8_t* sint, uint8_t* cost)
{
    int32_t i = 0;
    RGBA pal[256] = { 0 };
    memset(pal, 0, sizeof(pal));

    for (i = 0; i < 256; i++)
    {
        sint[i] = uint8_t(sin(2 * M_PI * i / 255) * 128 + 128);
        cost[i] = uint8_t(cos(2 * M_PI * i / 255) * 128 + 128);
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

//FX-effect: calculate tunnel buffer
void prepareTunnel(const GFX_IMAGE* dimg, uint8_t* buff1, uint8_t* buff2)
{
    const int32_t maxAng = 2048;
    const double angDec = 0.85;
    const double dstInc = 0.02;
    const double preCalc = M_PI / (maxAng >> 2);

    const int32_t dcx = dimg->mWidth >> 1;
    const int32_t dcy = dimg->mHeight >> 1;

    double tz = 250.0;
    double dst = 1.0;
    double ang = maxAng - 1.0;
    
    do {
        const int32_t tx = fround(tz * sin(ang * preCalc)) + dcx;
        const int32_t ty = fround(tz * cos(ang * preCalc)) + dcy;

        ang -= angDec;
        if (ang < 0)
        {
            ang += maxAng;
            dst += dst * dstInc;
            tz -= angDec;
        }

        if (tx >= 0 && tx < dimg->mWidth && ty >= 0 && ty < dimg->mHeight)
        {
            const int32_t ofs = ty * dimg->mWidth + tx;
            buff1[ofs] = fround(dst);
            buff2[ofs] = fround(dst - ang / 4);
        }
    } while (tz >= 0);
}

//FX-effect: draw tunnel
void drawTunnel(GFX_IMAGE* dimg, const GFX_IMAGE* simg, uint8_t* buff1, uint8_t* buff2, uint8_t* ang, uint8_t step)
{
    uint32_t nsize = dimg->mSize >> 2;
    uint32_t* pdst = (uint32_t*)dimg->mData;
    const uint32_t* psrc = (const uint32_t*)simg->mData;
    
    //only rgb mode
    if (bitsPerPixel <= 8) return;

    *ang += step;

#ifdef _USE_ASM
    uint8_t tmp = *ang;
    __asm {
        mov     ecx, ang
        mov     edi, pdst
        mov     esi, psrc
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
        *pdst++ = psrc[(val << 8) + *buff2];
        buff1++;
        buff2++;
    }
#endif
}

//FX-effect: blur image buffer
void blurImageEx(GFX_IMAGE* dst, const GFX_IMAGE* src, int32_t blur)
{
    uint8_t* pdst = (uint8_t*)dst->mData;
    const uint8_t* psrc = (const uint8_t*)src->mData;
    const uint32_t nsize = src->mSize >> 2;

    //only support for rgb mode
    if (bitsPerPixel <= 8) return;

    //check for small source size
    if (blur <= 0 || nsize <= uint32_t(2 * blur)) return;

    //check for MAX blur
    if (blur > 127) blur = 127;

#ifdef _USE_ASM
    __asm {
        mov     esi, psrc
        mov     edi, pdst
        mov     eax, blur
        shl     eax, 1
        mov     ecx, nsize
        sub     ecx, eax
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
        dec     ecx
        jnz     lpixb
        pop     ecx
        mov     ebx, edx
        xor     edx, edx
        div     ebx
        mov     edx, ebx
        mov     [edi], al
        add     esi, 4
        add     edi, 4
        dec     ecx
        jnz     lpxb
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
        dec     ecx
        jnz     lp2xb
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

    //compute each a,r,g,b channel
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
            pdst[ofs] = (col2 / col1) & 0xff;
            ofs += 4;
        }
        idx++;
    }
    
    idx += (blur - 1) << 2;
    col1 = (blur << 1) + 1;

    //compute each a,r,g,b channel
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
            pdst[ofs] = (col2 / col1) & 0xff;
            ofs += 4;
        }
        idx++;
    }
    
    tsize--;
    tsize <<= 2;
    idx += tsize;

    //compute each a,r,g,b channel
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
            pdst[ofs] = (col2 / col1) & 0xff;
            ofs += 4;
        }
        idx++;
    }
#endif
}

//FX-effect: brightness image buffer
void brightnessImage(GFX_IMAGE* dst, GFX_IMAGE* src, uint8_t bright)
{
    ARGB* psrc = (ARGB*)src->mData;
    const uint32_t nsize = src->mSize >> 2;

    //only support fro rgb mode
    if (bitsPerPixel <= 8) return;

    //check light range
    if (bright == 0 || bright == 255) return;

#ifdef _USE_ASM
    uint32_t* pdst = (uint32_t*)dst->mData;
    __asm {
        mov     ecx, nsize
        mov     edi, pdst
        mov     esi, psrc
        xor     edx, edx
        mov     dl, bright
    next:
        mov     ebx, [esi]
        mov     al, bh
        and     ebx, 00ff00ffh
        imul    ebx, edx
        shr     ebx, 8
        mul     dl
        mov     bh, ah
        mov     eax, ebx
        stosd
        add     esi, 4
        dec     ecx
        jnz     next
    }
#else
    for (uint32_t i = 0; i < nsize; i++)
    {
        psrc->r = (psrc->r * bright) >> 8;
        psrc->g = (psrc->g * bright) >> 8;
        psrc->b = (psrc->b * bright) >> 8;
        psrc++;
    }
#endif
}

//FX-effect: block-out image buffer
void blockOutMid(uint32_t* dst, uint32_t* src, int32_t count, int32_t val)
{
#ifdef _USE_ASM
    __asm {
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
    const int32_t block = val & 0xffff;
    uint32_t* psrc = &src[(val >> 17) + 1];
    
    do {
        count -= mid;
        if (count <= 0)
        {
            mid += count;
            for (i = 0; i < mid; i++) *dst++ = *psrc;
        }
        else
        {
            for (i = 0; i < mid; i++) *dst++ = *psrc;
            psrc += mid;
            mid = block;
        }
    } while (count > 0);
#endif
}

//FX-effect: brightness alpha buffer
void brightnessAlpha(GFX_IMAGE* img, uint8_t bright)
{
    ARGB* data = (ARGB*)img->mData;
    const uint32_t nsize = img->mSize >> 2;
    
    //only support 32bit color
    if (bitsPerPixel != 32) return;

#ifdef _USE_ASM
    __asm {
        mov     ecx, nsize
        mov     edi, data
        xor     eax, eax
        mov     bl, bright
    next:
        mov     al, [edi + 3]
        mul     bl
        mov     [edi + 3], ah
        add     edi, 4
        dec     ecx
        jnz     next
    }
#else
    for (uint32_t i = 0; i < nsize; i++)
    {
        data->a = uint16_t(data->a * bright) >> 8;
        data++;
    }
#endif
}

//FX-effect: block-out and middle image buffer
void blockOutMidImage(GFX_IMAGE* dst, GFX_IMAGE* src, int32_t xb, int32_t yb)
{
    uint32_t* pdst = (uint32_t*)dst->mData;
    uint32_t* psrc = (uint32_t*)src->mData;

    //only support for rgb mode
    if (bitsPerPixel <= 8) return;

    //check minimum blocking
    if (xb == 0) xb = 1;
    if (yb == 0) yb = 1;

    //nothing to do, make source and destination are the same
    if (xb == 1 && yb == 1) memcpy(pdst, psrc, src->mSize);
    else
    {
        //calculate delta x, delta y
        const int32_t cx = (((src->mWidth >> 1) % xb) << 16) | xb;
        const int32_t cy = (yb - ((src->mHeight >> 1) % yb));

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
        for (y = 0; y < texHeight / dsize; y++)
        {
            val = int32_t(smax * pc / 100.0);
            for (x = 0; x < texWidth / dsize; x++) fillCircle(x * dsize + size, y * dsize + size, val, col);
        }
        break;

    case 1:
        for (y = 0; y < texHeight / dsize; y++)
        {
            for (x = 0; x < texWidth / dsize; x++)
            {
                val = int32_t((smax + (double(texHeight) / dsize - y) * 2.0) * pc / 100.0);
                if (val > smax) val = smax;
                fillCircle(x * dsize + size, y * dsize + size, val, col);
            }
        }
        break;

    case 2:
        for (y = 0; y < texHeight / dsize; y++)
        {
            for (x = 0; x < texWidth / dsize; x++)
            {
                val = int32_t((smax + (double(texWidth) / dsize - x) * 2.0) * pc / 100.0);
                if (val > smax) val = smax;
                fillCircle(x * dsize + size, y * dsize + size, val, col);
            }
        }
        break;

    case 3:
        for (y = 0; y < texHeight / dsize; y++)
        {
            for (x = 0; x < texWidth / dsize; x++)
            {
                val = int32_t((smax + (double(texWidth) / size - (double(x) + y))) * pc / 100.0);
                if (val > smax) val = smax;
                fillCircle(x * dsize + size, y * dsize + size, val, col);
            }
        }
        break;

    default:
        break;
    }
}

//FX-effect: scale-up image buffer
void scaleUpLine(uint32_t* dst, const uint32_t* src, int32_t* tables, int32_t count, int32_t yval)
{
#ifdef _USE_ASM
    yval <<= 2;
    __asm {
        mov     ecx, count
        mov     ebx, src
        add     ebx, yval
        mov     esi, tables
        mov     edi, dst
    next:
        lodsd
        mov     eax, [ebx + eax * 4]
        stosd
        dec     ecx
        jnz     next
    }
#else
    for (int32_t i = 0; i < count; i++) *dst++ = src[tables[i] + yval];
#endif
}

//FX-effect: scale-up image buffer
void scaleUpImage(GFX_IMAGE* dst, const GFX_IMAGE* src, int32_t* tables, int32_t xfact, int32_t yfact)
{
    int32_t i = 0, y = 0;
    uint32_t* pdst = (uint32_t*)dst->mData;
    const uint32_t* psrc = (const uint32_t*)src->mData;

    //check color mode
    if (bitsPerPixel <= 8)
    {
        messageBox(GFX_ERROR, "Wrong pixel format!");
        return;
    }

    //init lookup table
    for (i = 0; i < src->mWidth; i++) tables[i] = fround(double(i) / (intmax_t(src->mWidth) - 1) * ((intmax_t(src->mWidth) - 1) - (intmax_t(xfact) << 1))) + xfact;

    //scale up line by line
    for (i = 0; i < src->mHeight; i++)
    {
        y = fround(double(i) / (intmax_t(src->mHeight) - 1) * ((intmax_t(src->mHeight) - 1) - (intmax_t(yfact) << 1)) + yfact);
        scaleUpLine(pdst, psrc, tables, src->mWidth, y * src->mWidth);
        pdst += dst->mWidth;
    }
}

//FX-effect: blur image buffer
void blurImage(const GFX_IMAGE* img)
{
    if (bitsPerPixel <= 8)
    {
        messageBox(GFX_ERROR, "Wrong pixel format!");
        return;
    }

    const uint32_t width = img->mWidth;
    const uint32_t* data = (const uint32_t*)img->mData;

#ifdef _USE_ASM
    uint32_t height = img->mHeight;
    __asm {
        mov     edi, data
    step:
        mov     edx, width
        sub     edx, 2
        push    edx
        mov     edx, width
        shl     edx, 2
        mov     ebx, [edi]
        mov     esi, [edi + 4]
        and     ebx, 00ff00ffh
        and     esi, 00ff00ffh
        add     ebx, ebx
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00ff00ffh
        add     esi, ecx
        mov     al, [edi + 5]
        mov     bl, [edi + 1]
        add     ebx, ebx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        xor     ebx, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00ff00ffh
        shl     eax, 6
        and     eax, 0000ff00h
        or      eax, esi
        stosd
    next:
        mov     esi, [edi - 4]
        mov     ecx, [edi + 4]
        and     esi, 00ff00ffh
        and     ecx, 00ff00ffh
        mov     ebx, [edi]
        add     esi, ecx
        and     ebx, 00ff00ffh
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00ff00ffh
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
        and     esi, 00ff00ffh
        shl     eax, 6
        and     eax, 0000ff00h
        or      eax, esi
        stosd
        dec     dword ptr[esp]
        jnz     next
        mov     ebx, [edi]
        mov     esi, [edi - 4]
        and     ebx, 00ff00ffh
        and     esi, 00ff00ffh
        add     ebx, ebx
        mov     ecx, [edi + edx]
        add     esi, ebx
        and     ecx, 00ff00ffh
        add     esi, ecx
        mov     al, [edi - 3]
        mov     bl, [edi - 4]
        add     ebx, ebx
        mov     cl, [edi + edx + 1]
        add     eax, ebx
        xor     ebx, ebx
        shr     esi, 2
        add     eax, ecx
        and     esi, 00ff00ffh
        shl     eax, 6
        and     eax, 0000ff00h
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
        ARGB* col0 = (ARGB*)&data[i];
        const ARGB* col1 = (const ARGB*)&data[i - 1];
        const ARGB* col2 = (const ARGB*)&data[i + 1];
        const ARGB* col3 = (const ARGB*)&data[i - width];
        const ARGB* col4 = (const ARGB*)&data[i + width];
        col0->r = (col1->r + col2->r + col3->r + col4->r) >> 2;
        col0->g = (col1->g + col2->g + col3->g + col4->g) >> 2;
        col0->b = (col1->b + col2->b + col3->b + col4->b) >> 2;
    }
#endif
}

//FX-effect: alpha-blending image buffer
void blendImage(GFX_IMAGE* dst, GFX_IMAGE* src1, GFX_IMAGE* src2, int32_t cover)
{
    const uint32_t pixels = src1->mSize >> 2;
    uint32_t* pdst  = (uint32_t*)dst->mData;
    uint32_t* psrc1 = (uint32_t*)src1->mData;
    uint32_t* psrc2 = (uint32_t*)src2->mData;
    
    if (bitsPerPixel <= 8)
    {
        messageBox(GFX_ERROR, "Wrong pixel format!");
        return;
    }

#ifdef _USE_ASM
    __asm {
        mov         ecx, pixels
        shr         ecx, 1
        jz          end
        mov         edi, pdst
        mov         edx, psrc1
        mov         esi, psrc2
        movzx       eax, cover
        xor         eax, 0FFh
        movd        mm3, eax
        punpcklwd   mm3, mm3
        pxor        mm2, mm2
        punpckldq   mm3, mm3
    again:
        movq        mm0, [edx]
        movq        mm1, [esi]
        movq        mm5, mm0
        punpcklbw   mm0, mm2
        movq        mm4, mm1
        punpcklbw   mm1, mm2
        psubw       mm0, mm1
        pmullw      mm0, mm3
        movq        mm6, mm4
        psrlw       mm0, 8
        psrlq       mm5, 32
        psrlq       mm4, 32
        punpcklbw   mm5, mm2
        punpcklbw   mm4, mm2
        psubw       mm5, mm4
        pmullw      mm5, mm3
        psrlw       mm5, 8
        packuswb    mm0, mm5
        paddb       mm0, mm6
        movq        [edi], mm0
        add         edx, 8
        add         edi, 8
        add         esi, 8
        dec         ecx
        jnz         again
        emms
    end:
    }
#else
    //32-bytes aligned
    const int32_t aligned = pixels >> 3;
    const int32_t remainder = pixels % 8;

    //all zero
    const __m256i ymm0 = _mm256_setzero_si256();

    //alpha and inverted alpha, 8 x 32 bits
    const __m256i alpha = _mm256_set1_epi16(cover);
    const __m256i invert = _mm256_set1_epi16(256 - cover);

    //process 32-bytes
    for (int32_t i = 0; i < aligned; i++)
    {
        //load 8 pixes width from src1 (8 x 32 bits data)
        __m256i los1 = _mm256_stream_load_si256((const __m256i*)psrc1);
        __m256i his1 = los1;

        //unpack to low & hi
        los1 = _mm256_unpacklo_epi8(los1, ymm0);
        his1 = _mm256_unpackhi_epi8(his1, ymm0);

        //load 8 pixes width from src2 (8 x 32 bits data)
        __m256i los2 = _mm256_stream_load_si256((const __m256i*)psrc2);
        __m256i his2 = los2;

        //unpack to low & high
        los2 = _mm256_unpacklo_epi8(los2, ymm0);
        his2 = _mm256_unpackhi_epi8(his2, ymm0);

        //blending low = (A * SRC + B * DST) >> 8, (8 x 32 bits data)
        los1 = _mm256_mullo_epi16(los1, alpha);
        los2 = _mm256_mullo_epi16(los2, invert);
        __m256i los = _mm256_adds_epu16(los1, los2);
        los = _mm256_srli_epi16(los, 8);

        //blending high = (A * SRC + B * DST) >> 8, (8 x 32 bits data)
        his1 = _mm256_mullo_epi16(his1, alpha);
        his2 = _mm256_mullo_epi16(his2, invert);
        __m256i his = _mm256_adds_epu16(his1, his2);
        his = _mm256_srli_epi16(his, 8);

        //destination = PACKED(low,hi) 32 x 8 bits
        const __m256i res = _mm256_packus_epi16(los, his);
        _mm256_stream_si256((__m256i*)pdst, res);

        //next 8 pixels
        pdst += 8;
        psrc1 += 8;
        psrc2 += 8;
    }

    //have unaligned bytes
    if (remainder > 0)
    {
        const uint8_t rcover = 255 - cover;
        for (int32_t i = 0; i < remainder; i++)
        {
            const uint32_t lsrc = *psrc1;
            const uint32_t ldst = *psrc2;
            const uint32_t rb = ((ldst & 0x00ff00ff) * rcover + (lsrc & 0x00ff00ff) * cover);
            const uint32_t ag = (((ldst & 0xff00ff00) >> 8) * rcover + ((lsrc & 0xff00ff00) >> 8) * cover);
            *pdst++ = ((rb & 0xff00ff00) >> 8) | (ag & 0xff00ff00);
            psrc1++;
            psrc2++;
        }
    }
#endif
}

//FX-effect: rotate image buffer, line by line
void rotateLine(uint32_t* dst, const uint32_t* src, const int32_t* tables, int32_t width, int32_t siny, int32_t cosy)
{
#ifdef _USE_ASM
    const int32_t pos = (width + 1) << 3;
    __asm {
        mov     ecx, width
        dec     ecx
        mov     esi, src
        mov     edi, dst
        mov     ebx, tables
    next:
        mov     eax, [ebx + ecx * 8 + 8]
        mov     edx, [ebx + ecx * 8 + 12]
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
        add     eax, edx
        mov     eax, [esi + eax]
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
    uint32_t* pdst = (uint32_t*)dst->mData;
    const uint32_t* psrc = (const uint32_t*)src->mData;

    if (bitsPerPixel <= 8)
    {
        messageBox(GFX_ERROR, "Wrong pixel format!");
        return;
    }

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

    const double primex = -(2.0 * axisx + 1);
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

    const double primey = ((intmax_t(dst->mHeight) - 1 - axisy) * 2.0) + 1;
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
void bumpImage(const GFX_IMAGE* dst, const GFX_IMAGE* src1, const GFX_IMAGE* src2, int32_t lx, int32_t ly)
{
    const uint32_t* dstdata = (const uint32_t*)dst->mData;
    const uint32_t* src1data = (const uint32_t*)src1->mData;
    const uint32_t* src2data = (const uint32_t*)src2->mData;

    const int32_t src1width = src1->mWidth;
    const int32_t src2width = src2->mWidth;
    const int32_t dstwidth = dst->mWidth;
    const int32_t src1len = src1->mRowBytes - 1;

    const int32_t bmax = 400;
    const int32_t xstart = 100, ystart = 100;
    const int32_t endx = getDrawBufferWidth() - xstart;
    const int32_t endy = getDrawBufferHeight() - ystart;

    int32_t nx = 0, ny = 0, vlx = 0, vly = 0;
    int32_t x = 0, y = 0, osrc2 = 0, osrc1 = 0, odst = 0;
    
#ifdef _USE_ASM
    __asm {
        mov     eax, ystart
        mov     y, eax
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
        mov     eax, xstart
        mov     x, eax
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
        mov     eax, endx
        cmp     x, eax
        jna     startx
        inc     y
        mov     eax, endy
        cmp     y, eax
        jna     starty
    }
#else
    //scan for image height
    for (y = ystart; y <= endy; y++)
    {
        //calculate starting offset
        odst = dstwidth * y + 99;
        osrc1 = src1width * y + 99;
        osrc2 = src2width * y + 99;
        
        //scan for image width
        for (x = xstart; x <= endx; x++)
        {
            //calculate delta x,y
            vlx = x - lx;
            vly = y - ly;

            //range checking
            if (vlx > -bmax && vlx < bmax && vly > -bmax && vly < bmax)
            {
                nx = (src1data[osrc1 + 1] & 0xff) - (src1data[osrc1 - 1] & 0xff);
                ny = (src1data[osrc1 + src1len] & 0xff) - (src1data[osrc1 - src1len] & 0xff);
                uint8_t difx = 127 - min(abs(vlx - nx) >> 1, 127);
                if (difx <= 0) difx = 1;
                uint8_t dify = 127 - min(abs(vly - ny) >> 1, 127);
                if (dify <= 0) dify = 1;
                uint8_t col = difx + dify;
                if (col > 128)
                {
                    col -= 128;
                    ARGB* pdst = (ARGB*)&dstdata[odst];
                    const ARGB* psrc = (const ARGB*)&src2data[osrc2];
                    pdst->r = min((col * psrc->r) >> 5, 255);
                    pdst->g = min((col * psrc->g) >> 5, 255);
                    pdst->b = min((col * psrc->b) >> 5, 255);
                }
            }

            //next pixel
            odst++;
            osrc1++;
            osrc2++;
        }
    }
#endif
}

//FX-effect: fade-out image buffer
void fadeOutImage(GFX_IMAGE* img, uint8_t step)
{
    if (bitsPerPixel <= 8) return;

    ARGB* pixels = (ARGB*)img->mData;
    const uint32_t msize = img->mSize >> 2;

#ifdef _USE_ASM
    __asm {
        mov         edi, pixels
        xor         eax, eax
        mov         al, step
        movd        mm1, eax
        punpcklbw   mm1, mm1
        punpcklwd   mm1, mm1
        mov         ecx, msize
        shr         ecx, 1
        jz          once
        punpckldq   mm1, mm1
    plot:
        movq        mm0, [edi]
        psubusb     mm0, mm1
        movq        [edi], mm0
        add         edi, 8
        dec         ecx
        jnz         plot
    once:
        test        msize, 1
        jz          end
        movd        mm0, [edi]
        psubusb     mm0, mm1
        movd        [edi], mm0
    end:
        emms
    }
#else
    //make 32-bytes alignment (8 pixels)
    const int32_t aligned = msize >> 3;
    const int32_t remainder = msize % 8;

    //make 32-bytes step
    const __m256i mstep = _mm256_set1_epi8(step);

    //start loop for 32-bytes aligned
    for (int32_t i = 0; i < aligned; i++)
    {
        //stream load 8 pixels (256-bits data)
        const __m256i ymm0 = _mm256_stream_load_si256((const __m256i*)pixels);

        //sub 32-bytes pixels with saturating
        const __m256i ymm1 = _mm256_subs_epu8(ymm0, mstep);
        
        //store data 32-bytes
        _mm256_stream_si256((__m256i*)pixels, ymm1);
        
        //next-to 32-bytes align (8 pixels)
        pixels += 8;
    }

    //have unaligned bytes?
    if (remainder > 0)
    {
        //process remainder bytes
        for (int32_t i = 0; i < remainder; i++)
        {
            pixels->r = max(pixels->r - step, 0);
            pixels->g = max(pixels->g - step, 0);
            pixels->b = max(pixels->b - step, 0);
            pixels++;
        }
    }
#endif
}

//get total system momory in MB
void initMemoryInfo()
{
#ifdef SDL_PLATFORM_APPLE
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
    __asm {
        cpuid
        rdtsc
    }
#else
    return __rdtsc();
#endif
}

//get current CPU clock rate in MHz
void calcCpuSpeed()
{
    const uint64_t start = getCyclesCount();
    SDL_Delay(200);
    const uint64_t stop = getCyclesCount();
    const uint64_t speed = (stop - start) / 200000;
    cpuSpeed = uint32_t(speed);
}

//CPUID instruction wrapper
void CPUID(int32_t* cpuinfo, uint32_t funcid)
{
#ifdef _USE_ASM
    __asm {
        mov     eax, funcid
        mov     edi, cpuinfo
        cpuid
        mov     [edi     ], eax
        mov     [edi +  4], ebx
        mov     [edi +  8], ecx
        mov     [edi + 12], edx
    }
#elif defined(SDL_PLATFORM_APPLE)
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
    if (!cpuType[0]) strncpy(cpuType, "<unknown>", sizeof(cpuType));
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
    if (!cpuName[0]) strncpy(cpuName, "<unknown>", sizeof(cpuName));
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

    if (have3DNow()) strncat(cpuFeatures, "3DNow!,", sizeof(cpuFeatures) - 1);
    if (cpuInfo[3] & 0x00800000) strncat(cpuFeatures, "MMX,", sizeof(cpuFeatures) - 1);
    if (cpuInfo[3] & 0x02000000) strncat(cpuFeatures, "SSE,", sizeof(cpuFeatures) - 1);
    if (cpuInfo[3] & 0x04000000) strncat(cpuFeatures, "SSE2,", sizeof(cpuFeatures) - 1);
    if (cpuInfo[2] & 0x10000000) strncat(cpuFeatures, "AVX,", sizeof(cpuFeatures) - 1);

#ifdef SDL_PLATFORM_APPLE
    __cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#else
    __cpuidex(cpuInfo, 7, 0);
#endif

    if (cpuInfo[1] & 0x00000020) strncat(cpuFeatures, "AVX2,", sizeof(cpuFeatures) - 1);
    if (cpuInfo[1] & 0x00010000) strncat(cpuFeatures, "AVX512,", sizeof(cpuFeatures) - 1);

    size_t len = strlen(cpuFeatures);
    if (len > 1) cpuFeatures[len - 1] = '\0';
    else strncpy(cpuFeatures, "<none>", sizeof(cpuFeatures));
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
    strncpy(videoName, "<unknown>", sizeof(videoName));
    strncpy(modeInfo, "<unknown>", sizeof(modeInfo));
    strncpy(driverVersion, "0.0.0.0", sizeof(driverVersion));
    strncpy(renderVersion, "0.0.0.0", sizeof(renderVersion));
    strncpy(imageVersion, "0.0.0.0", sizeof(imageVersion));
    
    //retrive SDL and SDL_image version string
    const int32_t sdlver = SDL_GetVersion();
    const int32_t imgver = IMG_Version();

    snprintf(renderVersion, sizeof(renderVersion), "SDL %d.%d.%d", SDL_VERSIONNUM_MAJOR(sdlver), SDL_VERSIONNUM_MINOR(sdlver), SDL_VERSIONNUM_MICRO(sdlver));
    snprintf(imageVersion, sizeof(imageVersion), "SDL_image %d.%d.%d", SDL_VERSIONNUM_MAJOR(imgver), SDL_VERSIONNUM_MINOR(imgver), SDL_VERSIONNUM_MICRO(imgver));
    
    //retrive current video mode info string
    int width = 0, height = 0;
	SDL_GetCurrentRenderOutputSize(sdlRenderer, &width, &height);
	const SDL_DisplayMode* mode = (const SDL_DisplayMode*)SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(sdlWindow));
	if (mode) snprintf(modeInfo, sizeof(modeInfo), "%dx%dx%db @ %.2fHz", width, height, SDL_BYTESPERPIXEL(mode->format) << 3, mode->refresh_rate);

#ifdef SDL_PLATFORM_APPLE
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
                snprintf(driverVersion, sizeof(driverVersion), "%u.%u.%u.%u", HIWORD(deviceId), LOWORD(deviceId), HIWORD(revisionId), LOWORD(revisionId));
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
    snprintf(driverVersion, sizeof(driverVersion), "%u.%u.%u.%u", HIWORD(driverVersionRaw.HighPart), LOWORD(driverVersionRaw.HighPart), HIWORD(driverVersionRaw.LowPart), LOWORD(driverVersionRaw.LowPart));
#endif
}

//initialize some system info
bool initSystemInfo()
{
    initCpuInfo();
    initVideoInfo();
    initMemoryInfo();

    //check CPU extension
    if (!strstr(cpuFeatures, "AVX2"))
    {
        messageBox(GFX_ERROR, "GFXLIB require modern CPU with MMX, SSE2 and AVX2 extension!");
        return false;
    }

    //check CPU speed
    if (cpuSpeed < 2000)
    {
        messageBox(GFX_ERROR, "GFXLIB require CPU speed more than 2000 MHz!");
        return false;
    }

    //check free memory
    if (availableMemory < 500)
    {
        messageBox(GFX_ERROR, "GFXLIB require system RAM more than 500 MB!");
        return false;
    }

    //check for video RAM
    if (videoMemory < 64)
    {
        messageBox(GFX_ERROR, "GFXLIB require video RAM more than 64 MB!");
        return false;
    }

    return true;
}

//set windows title text
void setWindowTitle(const char* title)
{
    SDL_SetWindowTitle(sdlWindow, title);
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

/*============================ END OF GFXLIB.CPP ========================*/
