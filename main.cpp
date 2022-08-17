#include "gfxlib.h"

int main(int argc, char* args[])
{
#ifdef __APPLE__
    chdir(dirname(args[0]));
    chdir("../Resources");
#endif
    /*gfxEffects();
    gfxDemoMix();
    gfxDemo();
    gfxFontView();*/
    gfxEffectsMix();
    gfxFractals();
    return 0;
}
