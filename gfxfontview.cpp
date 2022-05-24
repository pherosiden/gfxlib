#include "gfxlib.h"
#ifdef __APPLE__
#include <dirent.h>
#else
#include <io.h>
#endif

void showFontsDir(const char* path, const char* ext)
{
    char buff[256] = { 0 };
    int32_t i = 0, height = 0, y = 10;

    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();

#ifdef __APPLE__
    DIR* dir;
    struct dirent* ent;

    //try to open dir first
    if ((dir = opendir(path)))
    {
        //scan font dir
        while ((ent = readdir(dir)))
        {
            //skip if not regular files, not GFX font files
            if ((ent->d_type != DT_REG) || !strstr(ent->d_name, ext)) continue;

            //format related path
            sprintf(buff, "%s/%s", path, ent->d_name);

            //try to load this font file
            loadFont(buff, 0);
            sprintf(buff, "%s - The quick brown fox jumps over the lazy dog", ent->d_name);
            
            const GFX_FONT* font = getFont();

            //view all size of font
            for (i = 0; i <= font->hdr.subFonts; i++)
            {
                if (font->hdr.subFonts > 0) setFontSize(i);
                height = getFontHeight(buff);

                //have limit line
                if (y > cmy - height)
                {
                    waitUserInput();
                    clearScreen();
                    y = 10;
                }

                //draw font
                writeText(10, y, RGB_WHITE, 0, buff);
                horizLine(0, y + height + 1, cmx, RGB_BLUE);
                y += (height + 3);
            }

            //display font contents on screen
            render();
            freeFont(0);
        }

        closedir(dir);
        waitUserInput();
    }
#else
    //build file path
    sprintf(buff, "%s/*%s", path, ext);

    //search font directory
    _finddata_t fileInfo;
    intptr_t hfile = _findfirst(buff, &fileInfo);
    if (hfile == -1) return;

    do
    {
        sprintf(buff, "%s/%s", path, fileInfo.name);
        loadFont(buff, 0);
        sprintf(buff, "%s - The quick brown fox jumps over the lazy dog", fileInfo.name);
        
        const GFX_FONT* font = getFont();

        //view all size of font
        for (i = 0; i <= font->hdr.subFonts; i++)
        {
            if (font->hdr.subFonts > 0) setFontSize(i);
            height = getFontHeight(buff);

            //have limit line
            if (y > cmy - height)
            {
                waitUserInput();
                clearScreen();
                y = 10;
            }

            //draw font
            writeText(10, y, RGB_WHITE, 0, buff);
            horizLine(0, y + height + 1, cmx, RGB_BLUE);
            y += (height + 3);
        }

        //display font contents on screen
        render();
        freeFont(0);
    } while (!_findnext(hfile, &fileInfo));

    _findclose(hfile);
    waitUserInput();
#endif
}

void gfxFontView()
{
    if (!initScreen(800, 600, 32, 0, "GFXLIB Fonts")) return;
    showFontsDir("assets", ".xfn");
    cleanup();
}
