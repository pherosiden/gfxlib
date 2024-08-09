#include "gfxlib.h"
#include <windows.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef SDL_PLATFORM_APPLE
    chdir(dirname(args[0]));
    chdir("../Resources");
#endif
    gfxEffects();
    gfxDemoMix();
    gfxDemo();
    gfxFontView();
    gfxEffectsMix();
    gfxFractals();
    return 0;
}
