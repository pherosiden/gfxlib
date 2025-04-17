#include "gfxlib.h"
#include <windows.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef SDL_PLATFORM_APPLE
    chdir(dirname(args[0]));
    chdir("../Resources");
#endif
    /*gfxEffects();
    gfxDemoMix();
    gfxDemo();
    gfxFontView();
    gfxEffectsMix();
    gfxFractals();*/
    initScreen(800, 600, 32);
    drawLine(0, 0, getMaxX(), getMaxY(), 0x5f00FF00, BLEND_MODE_ALPHA);
    render();
    waitKeyPressed(SDL_SCANCODE_SPACE);
    quit();
    return 0;
}
