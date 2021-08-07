#include "gfxlib.h"
#include <io.h>

void showFontsDir(const char* path, const char* ext)
{
    intptr_t hfile;
    _finddata_t fileInfo;

    int32_t i, height, y = 10;
    char buff[256] = { 0 };
    
    //open font directory
    sprintf(buff, "%s/%s", path, ext);
    hfile = _findfirst(buff, &fileInfo);
    if (hfile == -1) return;

    do
    {
        sprintf(buff, "%s/%s", path, fileInfo.name);
        loadFont(buff, 0);
        sprintf(buff, "%s - The quick brown fox jumps over the lazy dog", fileInfo.name);

        //view all size of font
        for (i = 0; i <= gfxFonts[fontType].header.subFonts; i++)
        {
            if (gfxFonts[fontType].header.subFonts > 0) setFontSize(i);
            height = getFontHeight(buff);

            //have limit line
            if (y > cmaxY - height)
            {
                waitKeyPressed();
                clearScreen(0);
                y = 10;
            }

            //draw font
            writeText(10, y, RGB_WHITE, 0, buff);
            horizLine(0, y + height + 1, cmaxX, RGB_BLUE);
            y += (height + 3);
        }

        //display font contents on screen
        render();
        freeFont(0);
    } while (!_findnext(hfile, &fileInfo));

    waitKeyPressed();
    _findclose(hfile);
}

void gfxFontView()
{
    initScreen(800, 600, 32, 0, "GFXLIB Fonts");
    showFontsDir("assets", "*.xfn");
    cleanup();
}
