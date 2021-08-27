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

    if (!initScreen(800, 600, 0, 0, "Fill-Polygon")) return 1;
    POINT2D points[] = { {659, 336}, {452, 374}, {602, 128}, {509, 90}, {433, 164}, {300, 71}, {113, 166}, {205, 185}, {113, 279}, {169, 278}, {206, 334}, {263, 279}, {355, 129}, {301, 335}, {432, 204}, {433, 297}, {245, 467}, {414, 392}, {547, 523} };
    
    fillPolygon(points, 19, 120);
    drawPoly(points, 19, 255);

    render();
    waitKeyPressed();
    cleanup();
    return 0;
}
