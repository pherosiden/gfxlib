#include "gfxlib.h"

int main(int argc, char* args[])
{
#ifdef __APPLE__
    chdir(dirname(args[0]));
    chdir("../Resources");
#endif
    //gfxEffects32();
    //gfxDemo8();
    //gfxDemo32();
    //gfxFontView();
    //gfxEffects8();

    POINT2D points[] = { {659, 336}, {452, 374}, {602, 128}, {509, 90}, {433, 164}, {300, 71}, {113, 166}, {205, 185}, {113, 279}, {169, 278}, {206, 334}, {263, 279}, {355, 129}, {301, 335}, {432, 204}, {433, 297}, {245, 467}, {414, 392}, {547, 523} };

    if (!initScreen(800, 600, 8, 0, "Fill-Polygon")) return 1;
    
    makeLinearPalette();
    fillPolygon(points, 19, 50);

    for (int32_t j = 50; j < cmaxY - 50; j++)
    {
        for (int32_t i = 50; i < cmaxX - 50; i++)
        {
            if (getPixel(i, j) == 50) putPixel(i, j, 16 + ((i + j) / 4) % 192);
        }
    }
    
    rotatePalette(16, 207, 192, FPS_60);
    waitKeyPressed();
    makeRainbowPalette();
    for (int32_t y = 0; y < cmaxY; y++) horizLine(0, y, cmaxX, 1 + uint32_t(y / 2.35) % 255);
    rotatePalette(16, 207, 192, FPS_60);
    waitKeyPressed();
    cleanup();
    return 0;
}
