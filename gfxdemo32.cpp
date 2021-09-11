#include "gfxlib.h"

//max message lines
#define MAX_TEXT_LINE   23

//max message lenght
#define MAX_MSG_LEN     80

//string buffer
typedef char STRBUFF[MAX_MSG_LEN];

//show text intro message
int32_t     fullSpeed = 0;

//greets scrolling text
STRBUFF     texts[MAX_TEXT_LINE] = {0};

//global cached image
GFX_IMAGE   flare = { 0 };
GFX_IMAGE   flares[16] = { 0 };
GFX_IMAGE   fade1 = { 0 }, fade2 = { 0 };
GFX_IMAGE   bumpchn = { 0 }, bumpimg = { 0 };
GFX_IMAGE   gfxlogo = { 0 }, gfxsky = { 0 };

//check and exit program
void runExit()
{
    //capture current screen buffer
    GFX_IMAGE img = { 0 };
    getImage(0, 0, texWidth, texHeight, &img);

    //decrease rgb and push back to screen
    for (int32_t i = 0; i < 32; i++)
    {
        fadeOutImage(&img, 8);
        putImage(0, 0, &img);
        render();
        delay(10);
    }

    //cleanup...
    freeImage(&img);
    freeImage(&fade1);
    freeImage(&fade2);
    freeImage(&flare);
    freeImage(&bumpchn);
    freeImage(&bumpimg);
    freeImage(&gfxlogo);
    freeImage(&gfxsky);
    for (int32_t i = 0; i < 16; i++) freeImage(&flares[i]);
}

//Show intro message text string
void showText(int32_t sx, int32_t sy, GFX_IMAGE *img, const char *str)
{
    if (!str) return;
    if (strlen(str) >= MAX_MSG_LEN) return;

    char msg[2] = { 0 };
    int32_t x, y, i, len;
        
    //make scrolling text
    memcpy(&texts[0][0], &texts[1][0], sizeof(texts) - sizeof(texts[0]));
    strcpy(texts[MAX_TEXT_LINE - 1], str);

    //don't delay each character
    if (fullSpeed)
    {
        delay(10);
        putImage(sx, sy, img);
        for (i = 0; i < MAX_TEXT_LINE; i++) writeText(sx + 10, sy + 10 + i * 10, RGB_GREY127, 2, texts[i]);
        render();
    }
    else
    {
        //show previous text
        for (y = 9; y >= 0; y--)
        {
            //fill original background
            putImage(sx, sy, img);
            for (i = 0; i < MAX_TEXT_LINE - 1; i++) writeText(sx + 10, sy + 10 + i * 10 + y, RGB_GREY127, 2, texts[i]);
            render();
            delay(10);
        }

        x = 0;
        len = int32_t(strlen(str));

        //show current text with delay each character
        for (i = 0; i < len; i++)
        {
            msg[0] = str[i];
            writeText(sx + 10 + x, sy + 10 + 220, RGB_WHITE, 2, msg);
            render();
            x += getFontWidth(msg);

            //check for delay and skip
            if (!fullSpeed) delay(45);
            if (finished(SDL_SCANCODE_RETURN)) fullSpeed = 1;
        }
    }
}

void runIntro()
{
    const int32_t tw = 3, tg = tw + 5, tu = tg + 3, to = tu + 5, ts = to + 3;
    
    //load image
    GFX_IMAGE ult = { 0 }, gfx = { 0 }, wcb = { 0 }, map = { 0 };
    if (!loadImage("assets/gfxtheultimate.png", &ult)) return;
    if (!loadImage("assets/gfxlogo.png", &gfx)) return;
    if (!loadImage("assets/gfxwelcome.png", &wcb)) return;
    if (!loadImage("assets/map03.png", &map)) return;

    //initialize buffer
    GFX_IMAGE scr = { 0 }, wci = { 0 };
    GFX_IMAGE gxb = { 0 }, utb = { 0 }, trn = { 0 };
    if (!newImage(texWidth, texHeight, &scr)) return;
    if (!newImage(texWidth >> 1, texHeight >> 1, &trn)) return;
    if (!newImage(wcb.mWidth, wcb.mHeight, &wci)) return;
    if (!newImage(gfx.mWidth, gfx.mHeight, &gxb)) return;
    if (!newImage(ult.mWidth, ult.mHeight, &utb)) return;

    //initialize tunnel buffer
    const int32_t tsize = trn.mWidth * trn.mHeight;
    uint8_t* buff1 = (uint8_t*)calloc(tsize, 1);
    uint8_t* buff2 = (uint8_t*)calloc(tsize, 1);

    if (!buff1 || !buff2)
    {
        messageBox(GFX_ERROR, "RunIntro: cannot alloc memory!");
        return;
    }

    //calculate tunnel buffer
    prepareTunnel(&trn, buff1, buff2);

    //redirect draw buffer to image buffer
    changeDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);

    int32_t i0 = 30;
    int32_t i1 = 25;
    int32_t i2 = 0;
    uint8_t mov = 0;

    //start record time
    uint32_t startTime = getTime();
    do {
        //draw and scale buffer
        uint32_t waitTime = getTime();
        drawTunnel(&trn, &map, buff1, buff2, &mov, 1);
        scaleImage(&scr, &trn, 1);
        
        //welcome message
        if ((getElapsedTime(startTime) / 1000 >= tw) && (i0 >= 0))
        {
            if (i0 > 15)
            {
                blurImageEx(&wci, &wcb, i0 & 15);
                brightnessImage(&wci, &wci, (16 - (i0 & 15)) * 15 + 15);
                i0--;
            }
            else if ((getElapsedTime(startTime) / 1000 >= tw + 3.0) && (i0 >= 0))
            {
                blurImageEx(&wci, &wcb, 15 - (i0 & 15));
                brightnessImage(&wci, &wci, ((i0 & 15) + 1) * 15 + 15);
                i0--;
            }

            if (i0 >= -1) putImage(centerX - (wci.mWidth >> 1), centerY - (wci.mHeight >> 1), &wci, BLEND_MODE_ADD);
            if (getElapsedTime(startTime) / 1000 >= tg) i0 = -1;
        }
        //logo GFXLIB
        else if ((getElapsedTime(startTime) / 1000 >= tg) && (i1 > 0))
        {
            blockOutMidImage(&gxb, &gfx, i1, i1);
            brightnessAlpha(&gxb, uint8_t(255 - i1 / 30.0 * 255.0));
            putImage(centerX - (gxb.mWidth >> 1), centerY - (gxb.mHeight >> 1), &gxb, BLEND_MODE_ALPHA);

            i1--;
            if (getElapsedTime(startTime) / 1000 >= tu) i1 = 0;
        }
        //the ultimate message
        else if (i1 == 0)
        {
            putImage(centerX - (gfx.mWidth >> 1), centerY - (gfx.mHeight >> 1), &gfx, BLEND_MODE_ALPHA);
            if ((getElapsedTime(startTime) / 1000 >= tu) && (i2 <= 15))
            {
                blurImageEx(&utb, &ult, 15 - (i2 & 15));
                brightnessImage(&utb, &utb, ((i2 & 15) + 1) * 15 + 15);
                putImage(centerX - (ult.mWidth >> 1), centerY + (gfx.mHeight >> 1) + 30, &utb, BLEND_MODE_ADD);
                i2++;
            }
            else
            {
                putImage(centerX - (ult.mWidth >> 1), centerY + (gfx.mHeight >> 1) + 30, &utb, BLEND_MODE_ADD);
                if (getElapsedTime(startTime) / 1000 >= to) fadeOutCircle(((getElapsedTime(startTime) / 1000.0 - to) / 3.0) * 100.0, 20, 3, 0);
            }
        }
        render();
        waitFor(waitTime, 50);
    } while (!finished(SDL_SCANCODE_RETURN) && getElapsedTime(startTime) / 1000 < ts);

    //restore draw buffer
    restoreDrawBuffer();

    //cleanup...
    freeImage(&map);
    freeImage(&trn);
    freeImage(&scr);
    freeImage(&wcb);
    freeImage(&wci);
    freeImage(&ult);
    freeImage(&gfx);
    freeImage(&utb);
    free(buff1);
    free(buff2);
}

void runBlocking(int32_t sx, int32_t sy)
{
    //initialize buffer
    GFX_IMAGE img2 = { 0 };
    if (!newImage(fade1.mWidth, fade1.mHeight, &img2)) return;

    //blocking background
    int32_t dec = fade1.mWidth >> 2;
    while (dec > 0 && !finished(SDL_SCANCODE_RETURN))
    {
        dec--;
        blockOutMidImage(&img2, &fade1, dec << 1, dec << 1);
        brightnessImage(&img2, &img2, uint8_t(255.0 - double(dec) / (fade1.mWidth >> 2) * 255.0));
        putImage(sx, sy, &img2);
        render();
        delay(FPS_60);
    };

    //save current background
    const int32_t width  = fade1.mWidth;
    const int32_t height = fade1.mHeight;

    putImage(sx, sy, &fade1);
    render();
    freeImage(&img2);

    //load next step
    GFX_IMAGE img1 = { 0 };
    if (!loadImage("assets/gfxtext.png", &img1)) return;
    if (!newImage(img1.mWidth, img1.mHeight, &img2)) return;

    //calculate current position and save current buffer
    GFX_IMAGE img3 = { 0 };
    const int32_t posx = (width - img2.mWidth) >> 1;
    const int32_t posy = (height - img2.mHeight) >> 1;
    getImage(sx + posx, sy + posy, img1.mWidth, img1.mHeight, &img3);

    //blocking next step
    dec = img1.mWidth >> 3;
    while (dec > 0 && !finished(SDL_SCANCODE_RETURN)) 
    {
        dec--;
        blockOutMidImage(&img2, &img1, dec << 1, dec << 1);
        brightnessAlpha(&img2, uint8_t(255 - dec / (img1.mWidth >> 3) * 255.0));
        putImage(sx + posx, sy + posy, &img3);
        putImage(sx + posx, sy + posy, &img2, BLEND_MODE_ALPHA);
        render();
        delay(FPS_60);
    }

    //cleanup...
    freeImage(&img1);
    freeImage(&img2);
    freeImage(&img3);
}

void runScaleUpImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE img1 = { 0 }, img2 = { 0 }, img3 = { 0 };

    //initialize buffer
    if (!loadImage("assets/gfxspr.png", &img3)) return;
    if (!newImage(texWidth >> 1, texHeight >> 1, &img1)) return;
    if (!newImage(texWidth >> 1, texHeight >> 1, &img2)) return;
       
    //setup lookup table
    int32_t* tables = (int32_t*)calloc(img2.mWidth, sizeof(int32_t));
    if (!tables)
    {
        messageBox(GFX_ERROR, "ScaleUpImage: not enough memory for lookup tables.");
        return;
    }

    //loop until enter key pressed
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //redirect render buffer to image buffer
        changeDrawBuffer(img1.mData, img1.mWidth, img1.mHeight);

        //put some random pixel and GFX message
        for (int32_t i = 0; i < 400; i++) putPixel(random(img1.mWidth - 4) + 2, random(img1.mHeight - 4) + 2, rgb(0, 255, 200));
        if (random(64) == 32) putImage((img1.mWidth - img3.mWidth) >> 1, (img1.mHeight - img3.mHeight) >> 1, &img3, BLEND_MODE_ALPHA);

        //blur & scale buffer
        blurImage(&img1);
        scaleUpImage(&img2, &img1, tables, 7, 7);

        //restore to screen buffer to draw
        restoreDrawBuffer();
        putImage(sx, sy, &img2);
        render();
        delay(FPS_60);

        //save current buffer for next step
        memcpy(img1.mData, img2.mData, img2.mSize);
    }

    //cleanup...
    freeImage(&img1);
    freeImage(&img2);
    freeImage(&img3);
    free(tables);
}

void runCrossFade(int32_t sx, int32_t sy)
{
    int32_t i = 0, up = 1, val = 1;

    //initialize render buffer
    GFX_IMAGE img = { 0 };
    if (!newImage(fade1.mWidth, fade1.mHeight, &img)) return;

    //loop until key enter pressed
    while (!finished(SDL_SCANCODE_RETURN)) 
    {
        //check blending value
        if (i == 0) val = 1;
        else val = (i << 2) - 1;

        //blend image buffer
        blendImage(&img, &fade1, &fade2, val);
        putImage(sx, sy, &img);
        render();
        delay(FPS_60);

        //check for change direction
        if (up) i++; else i--;
        if (i == 0 || i == 64) up = !up;
    }

    //cleanup...
    freeImage(&img);
}

void runAddImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE img = { 0 };
    if (!newImage(fade1.mWidth, fade1.mHeight, &img)) return;

    int32_t step = 320;
    while (step > 4 && !finished(SDL_SCANCODE_RETURN))
    {
        //put lens image with adding background pixel
        step -= 4;
        changeDrawBuffer(img.mData, img.mWidth, img.mHeight);
        putImage(0, 0, &fade1);
        putImage(int32_t((320 - cos(step / 160.0) * 320)), 0, &flare, BLEND_MODE_ADD);
        restoreDrawBuffer();
        putImage(sx, sy, &img);
        render();
        delay(FPS_60);
        clearImage(&img);
    }
}

void runRotateImage(int32_t sx, int32_t sy)
{
    //initialize render buffer
    GFX_IMAGE img = { 0 };
    if (!newImage(fade2.mWidth, fade2.mHeight, &img)) return;
    if (!img.mData) return;

    //pre-calculate lookup table
    int32_t* tables = (int32_t*)calloc(intptr_t(fade2.mWidth) * 2 + fade2.mHeight + 2, sizeof(int32_t));
    if (!tables)
    {
        messageBox(GFX_ERROR, "RotateImage: cannot alloc lookup tables.");
        return;
    }

    //loop step
    uint32_t deg = 0;

    //loop until return
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //first step
        deg++;

        //copy background
        memcpy(img.mData, fade1.mData, fade1.mSize);

        //rotate buffer
        rotateImage(&img, &fade2, tables, fade2.mWidth >> 1, fade2.mHeight >> 1, deg % 360, 1);
        putImage(sx, sy, &img);
        render();
        delay(FPS_60);
    }

    //cleanup...
    freeImage(&img);
    free(tables);
}

void runBilinearRotateImage(int32_t sx, int32_t sy)
{
    //initialize render buffer
    GFX_IMAGE img = { 0 };
    if (!newImage(fade2.mWidth, fade2.mHeight, &img)) return;
    if (!img.mData) return;

    //start angle
    int32_t angle = 0;

    //loop until return
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //first step
        angle++;

        //copy background
        memcpy(img.mData, fade1.mData, fade1.mSize);

        //rotate buffer
        bilinearRotateImage(&img, &fade2, angle % 360);
        putImage(sx, sy, &img);
        render();
        delay(FPS_60);
    }

    //cleanup...
    freeImage(&img);
}

void runAntiAliased(int32_t sx, int32_t sy)
{
    //save current midx, midy
    const int32_t midx = texWidth >> 1;
    const int32_t midy = texHeight >> 1;

    //initialize image buffer
    GFX_IMAGE img = { 0 }, dst = { 0 };
    if (!newImage(midx, midy, &img)) return;
    if (!newImage(midx, midy, &dst)) return;

    //loop until return
    while (!finished(SDL_SCANCODE_RETURN)) 
    {
        //redirect drawing to image buffer
        changeDrawBuffer(dst.mData, dst.mWidth, dst.mHeight);

        //draw anti-alias (smooth pixel) circle, line and ellipse
        for (int32_t i = 0; i < 3; i++)
        {
            //choose random color
            const uint32_t col = rgb(random(255) + 1, random(255) + 1, random(255) + 1);

            //which shape to be draw
            switch (random(3))
            {
            case 0: drawLine(random(midx), random(midy), random(midx), random(midy), col, BLEND_MODE_ANTIALIASED); break;
            case 1: drawCircle(random(midx), random(midy), random(midx) >> 2, col, BLEND_MODE_ANTIALIASED); break;
            case 2: drawEllipse(random(midx), random(midy), random(midx), random(midy), col, BLEND_MODE_ANTIALIASED); break;
            default: break;
            }
        }

        //restore draw buffer
        restoreDrawBuffer();

        //fade-out current buffer
        putImage(sx, sy, &dst);
        fadeOutImage(&dst, 4);
        render();
        delay(FPS_60);
    }

    //cleanup...
    freeImage(&img);
    freeImage(&dst);
}

void runLensFlare(GFX_IMAGE* outImg)
{
    const int32_t flareput[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    const int32_t flarepos[16] = { -1110, -666, 0, 1087, 1221, 1309, 1776, 2197, 2819, 3130, 3220, 3263, 3663, 3707, 4440, 5125 };
    const char *str = "Drag your mouse to see details and left click to exit!";

    //load source image
    GFX_IMAGE scr = { 0 };
    if (!newImage(texWidth, texHeight, &scr)) return;

    //current mouse pos and left button
    int32_t lmb = 0;
    int32_t mcx = centerX + 70;
    int32_t mdx = centerY - 80;

    //set mouse pointer limitation
    setMousePosition(mcx, mdx);
    showMouseCursor(SDL_DISABLE);
    
    //pre-calculate text position
    const int32_t tx = (scr.mWidth - getFontWidth(str)) >> 1;
    const int32_t ty = scr.mHeight - getFontHeight(str) - 4;

    //redirect to image buffer
    changeDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);

    //time for record FPS
    uint32_t time = 0, oldTime = 0;

    do {
        getMouseState(&mcx, &mdx, &lmb, NULL);
        putImage(0, 0, &gfxsky);
        fillRect(0, 0, cmaxX, cmaxY, rgb(0, uint8_t((double(mdx) / cmaxY) * 64), uint8_t((double(mdx) / cmaxY) * 64)), BLEND_MODE_SUB);

        //put all flare image to render buffer
        for (int32_t i = 0; i < 16; i++)
        {
            //is show?
            if (flareput[i])
            {
                //merge current image buffer to background
                int32_t x = (centerX + ((centerX - mcx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mWidth >> 1);
                int32_t y = (centerY + ((centerY - mdx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mHeight >> 1);
                putImage(x, y, &flares[i], BLEND_MODE_ADD);
            }
        }

        //put logo and draw text message
        putImage(texWidth - gfxlogo.mWidth + 1, 0, &gfxlogo, BLEND_MODE_ALPHA);
        writeText(tx, ty, rgb(255, 255, 255), 2, str);

        //timing for input and FPS counter
        oldTime = time;
        time = getTime();

        //report FPS counter
        writeText(1, 1, RGB_WHITE, 0, "FPS:%.f", 1.0 / ((time - oldTime) / 1000.0));

        render();
        delay(FPS_60);
    } while (!finished(SDL_SCANCODE_RETURN) && !lmb);

    //capture current screen
    getImage(0, 0, scr.mWidth, scr.mHeight, outImg);

    //restore draw buffer
    restoreDrawBuffer();

    //cleanup...
    freeImage(&scr);
    showMouseCursor(SDL_ENABLE);
}

void runBumpImage()
{
    //loading source image
    GFX_IMAGE dst = { 0 };
    if (!newImage(texWidth, texHeight, &dst)) return;

    //loop until return
    int32_t cnt = 0;
    while (!finished(SDL_SCANCODE_RETURN)) 
    {
        //first step
        cnt++;

        //calculate position
        const int32_t lx = int32_t(cos(cnt / 13.0) * 133.0 + centerX);
        const int32_t ly = int32_t(sin(cnt / 23.0) * 133.0 + centerY);

        //start bumping buffer
        bumpImage(&dst, &bumpchn, &bumpimg, lx, ly);
        putImage(0, 0, &dst);
        render();
        delay(FPS_60);
        clearImage(&dst);
    }

    //cleanup...
    freeImage(&dst);
}

void runPlasmaScale(int32_t sx, int32_t sy)
{
    uint8_t sinx[256] = { 0 };

    //initialized lookup table and preload image
    for (int32_t y = 0; y < 256; y++) sinx[y] = uint8_t(sin(y * M_PI / 128) * 127 + 128);

    GFX_IMAGE plasma = { 0 }, screen = { 0 };
    if (!newImage(texWidth >> 2, texHeight >> 2, &plasma)) return;
    if (!newImage(texWidth >> 1, texHeight >> 1, &screen)) return;

    uint32_t frames = 0;
    uint32_t* data = (uint32_t*)plasma.mData;
    const uint16_t endx = plasma.mWidth >> 1;

    //loop until return
    while (!finished(SDL_SCANCODE_RETURN))
    {
        uint32_t ofs = 0;
        const uint32_t tectr = frames * 10;
        const uint16_t x1 = sinx[(tectr / 12) & 0xFF];
        const uint16_t x2 = sinx[(tectr / 11) & 0xFF];
        const uint16_t x3 = sinx[frames & 0xFF];
        const uint16_t y1 = sinx[((tectr >> 3) + 64) & 0xFF];
        const uint16_t y2 = sinx[(tectr / 7 + 64) & 0xFF];
        const uint16_t y3 = sinx[(tectr / 12 + 64) & 0xFF];

        //calculate plasma buffer
        for (int32_t y = 0; y < plasma.mHeight; y++)
        {
            uint16_t a = sqr(y - y1) + sqr(x1);
            uint16_t b = sqr(y - y2) + sqr(x2);
            uint16_t c = sqr(y - y3) + sqr(x3);
            uint16_t cr = sinx[(a >> 6) & 0xFF];
            uint16_t cg = sinx[(b >> 6) & 0xFF];
            uint16_t cb = sinx[(c >> 6) & 0xFF];
#ifdef _USE_ASM
            _asm {
                xor     ax, ax
                mov     edi, data
                add     edi, ofs
                xor     dx, dx
            next:
                xor     ebx, ebx
                mov     cl, 6
                mov     bx, ax
                push    ax
                sub     bx, x3
                add     bx, c
                mov     c, bx
                shr     bx, cl
                and     bx, 0xFF
                mov     bl, byte ptr sinx[ebx]
                mov     si, bx
                mov     bx, ax
                sub     bx, x2
                add     bx, b
                mov     b, bx
                shr     bx, cl
                and     bx, 0xFF
                mov     dl, byte ptr sinx[ebx]
                mov     bx, ax
                sub     bx, x1
                add     bx, a
                mov     a, bx
                shr     bx, cl
                and     bx, 0xFF
                mov     bl, byte ptr sinx[ebx]
                mov     ax, bx
                add     ax, cr
                mov     cr, bx
                shl     ebx, 16
                shl     eax, 15
                mov     ax, dx
                add     ax, cg
                mov     cg, dx
                shl     ax, 7
                mov     cx, si
                add     cx, cb
                mov     cb, si
                shr     cx, 1
                mov     al, cl
                mov     [edi], eax
                mov     bx, si
                mov     bh, byte ptr cg
                mov     [edi + 4], ebx
                add     edi, 8
                pop     ax
                inc     ax
                cmp     ax, endx
                jnae    next
            }
            ofs += (plasma.mWidth << 2);
#else
            uint32_t idx = ofs;
            for (int32_t x = 0; x < endx; x++)
            {
                c = x - x3 + c;
                const uint8_t sc = sinx[(c >> 6) & 0xFF];
                b = x - x2 + b;
                const uint8_t sb = sinx[(b >> 6) & 0xFF];
                a = x - x1 + a;
                const uint8_t sa = sinx[(a >> 6) & 0xFF];
                const uint32_t col2 = ((sa + cr) << 15) & 0xFFFF0000;
                const uint16_t col1 = (((sb + cg) << 7) & 0xFF00) + ((sc + cb) >> 1);
                cr = sa;
                cg = sb;
                cb = sc;
                data[idx] = col2 + col1;
                data[idx + 1] = (cr << 16) | (cg << 8) | cb;
                idx += 2;
            }
            ofs += plasma.mWidth;
#endif
        }

        //bilinear scale plasma buffer
        bilinearScaleImage(&screen, &plasma);
        putImage(sx, sy, &screen);
        render();
        delay(FPS_60);
        frames++;
    }

    //clean up...
    freeImage(&plasma);
    freeImage(&screen);
}

void gfxDemo()
{
    char sbuff[128] = { 0 };
    const char* initMsg = "Please wait while initialize GFXLIB...";

    if (!loadFont("assets/sysfont.xfn", 0)) return;
    if (!initScreen(800, 600, 32, 0, "GFXLIB-Demo32")) return;
    writeText(centerX - 8 * (int32_t(strlen(initMsg)) >> 1), centerY, RGB_GREY127, 2, initMsg);
    render();

    initSystemInfo();

    GFX_IMAGE bg = { 0 };
    if (!loadImage("assets/gfxbg5.png", &bg)) return;
    if (!loadImage("assets/gfxbumpchn.png", &bumpchn)) return;
    if (!loadImage("assets/gfxbumpimg.png", &bumpimg)) return;
    if (!loadImage("assets/gfxlogosm.png", &gfxlogo)) return;
    if (!loadImage("assets/gfxsky.png", &gfxsky)) return;
    
    for (int32_t i = 0; i < 16; i++)
    {
        sprintf(sbuff, "assets/flare-%dx.png", i + 1);
        if (!loadImage(sbuff, &flares[i])) return;
    }

    runIntro();
    putImage(0, 0, &bg);
    putImage(texWidth - gfxlogo.mWidth + 1, texHeight - gfxlogo.mHeight + 1, &gfxlogo, BLEND_MODE_ALPHA);

    GFX_IMAGE txt = { 0 };
    const int32_t xc = centerX + 40;
    const int32_t yc = centerY + 40;
    
    fillRectPattern(10, 10, xc - 20, yc - 20, RGB_GREY32, ptnHatchX, BLEND_MODE_ADD);
    fillRect(10, yc, xc - 20, cmaxY - yc - 10, RGB_GREY32, BLEND_MODE_SUB);
    fillRect(20, 20, xc - 40, yc - 40, 0);
    getImage(10, yc, xc - 20, cmaxY - yc - 10, &txt);

    writeText(xc + 10,  70, RGB_GREY127, 2, "GFXLIB %s", GFX_VERSION);
    writeText(xc + 10, 90, RGB_GREY127, 2, "A short show of some abilities");
    writeText(xc + 10, 100, RGB_GREY127, 2, "GFXLIB does provide. Note that");
    writeText(xc + 10, 110, RGB_GREY127, 2, "this is only a small amount of");
    writeText(xc + 10, 120, RGB_GREY127, 2, "all available features.");
    writeText(xc + 10, 150, RGB_GREY127, 2, "%s", getVideoName());
    writeText(xc + 10, 160, RGB_GREY127, 2, "Driver Version   : %s", getDriverVersion());
    writeText(xc + 10, 170, RGB_GREY127, 2, "Video Memory     : %luMB", getVideoMemory());
    writeText(xc + 10, 180, RGB_GREY127, 2, "Video Mode       : %s", getVideoModeInfo());
    writeText(xc + 10, 190, RGB_GREY127, 2, "Render System    : %s", getRenderVersion());
    writeText(xc + 10, 200, RGB_GREY127, 2, "Image Library    : %s", getImageVersion());
    writeText(xc + 10, 220, RGB_GREY127, 2, "%s", getCpuName());
    writeText(xc + 10, 230, RGB_GREY127, 2, "CPU Features     : %s", getCpuFeatures());
    writeText(xc + 10, 240, RGB_GREY127, 2, "CPU Frequency    : %luMHz", getCpuSpeed());
    writeText(xc + 10, 250, RGB_GREY127, 2, "Physical Memory  : %luMB", getTotalMemory());
    writeText(xc + 10, 260, RGB_GREY127, 2, "Available Memory : %luMB", getAvailableMemory());
    render();

    fullSpeed = 1;
    showText(10, yc, &txt, "Please wait while loading images...");

    if (!loadImage("assets/fade1x.png", &fade1)) return;
    showText(10, yc, &txt, " - fade1x.png");

    if (!loadImage("assets/fade2x.png", &fade2)) return;
    showText(10, yc, &txt, " - fade2x.png");

    if (!loadImage("assets/flare0.png", &flare)) return;
    showText(10, yc, &txt, " - flare0.png");

    showText(10, yc, &txt, "");

    fullSpeed = 0;
    showText(10, yc, &txt, "This is an early demonstration of the abilities of");
    showText(10, yc, &txt, "GFXLIB. What you will see here are only some of");
    showText(10, yc, &txt, "the image manipulating effects which are currently");
    showText(10, yc, &txt, "built in. More to come and show you later...");
    delay(1000);
    showText(10, yc, &txt, "Starting...");
    runBlocking(20, 20);

    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "What you saw was a combination of the command");
    showText(10, yc, &txt, "BlockOut and the command BrightnessImage. The text");
    showText(10, yc, &txt, "is an alphamapped image. You may see: working with");
    showText(10, yc, &txt, "images got very easy in GFXLIB - no half things");
    showText(10, yc, &txt, "anymore! To the next... press enter!");
    while (!finished(SDL_SCANCODE_RETURN));
    runAddImage(20, 20);

    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "This was simply another flag of a draw-operation.");
    showText(10, yc, &txt, "It was used here to force draw to add the content");
    showText(10, yc, &txt, "of the image to the background of the image. You");
    showText(10, yc, &txt, "are also able to subtract the image and to work");
    showText(10, yc, &txt, "with an alphamap like PNG-images can contain one.");
    showText(10, yc, &txt, "The next effect - press enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runCrossFade(20, 20);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "This thing is called CrossFading or AlphaBlending.");
    showText(10, yc, &txt, "In GFXLIB, the procedure is called 'BlendImage'.");
    showText(10, yc, &txt, "This procedure makes of 2 images another, where");
    showText(10, yc, &txt, "you can decide which image covers more the other.");
    showText(10, yc, &txt, "Enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runBilinearRotateImage(20, 20);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "This is an image rotation. The responsible routine");
    showText(10, yc, &txt, "for this is called BilinearRotateImage. It doesn't");
    showText(10, yc, &txt, "seem to be very fast here, but in this demo the");
    showText(10, yc, &txt, "rotation is an optimize version of bilinear image");
    showText(10, yc, &txt, "interpolation. You can reach on a INTEL MMX-133 up");
    showText(10, yc, &txt, "to 20 fps at 640x480x32 bit. You can see another");
    showText(10, yc, &txt, "version of rotate image is so fast if only rotate");
    showText(10, yc, &txt, "and show image, check my source code for details.");
    showText(10, yc, &txt, "Enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runScaleUpImage(20, 20);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "Much more fancy than the other FX... Yeah, you see");
    showText(10, yc, &txt, "two effects combined here. scaleup and blur image");
    showText(10, yc, &txt, "are doing their work here. Check the source code");
    showText(10, yc, &txt, "to see the details. The next... Enter :)");
    while (!finished(SDL_SCANCODE_RETURN));
    runAntiAliased(20, 20);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "Antialiased lines, circles and ellipses. Possible");
    showText(10, yc, &txt, "with GFXLIB and also even faster than seen here");
    showText(10, yc, &txt, "(just slow for show). Perfect for 3D models and");
    showText(10, yc, &txt, "similar. Enter for the next...");
    while (!finished(SDL_SCANCODE_RETURN));
    runPlasmaScale(20, 20);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "Plasma effect with hight color, this also combine");
    showText(10, yc, &txt, "scale up image with bilinear interpolation to");
    showText(10, yc, &txt, "process image with hight quality. This version is");
    showText(10, yc, &txt, "optimized using integer number but not really fast");
    showText(10, yc, &txt, "here. You can still optimized.");
    showText(10, yc, &txt, "Enter for the next...");
    while (!finished(SDL_SCANCODE_RETURN));
    fillRect(20, 20, xc - 40, yc - 40, 0);

    GFX_IMAGE scr = { 0 };
    getImage(0, 0, texWidth, texHeight, &scr);
    runBumpImage();

    GFX_IMAGE old = { 0 }, im = { 0 };
    getImage(0, 0, texWidth, texHeight, &old);
    newImage(texWidth >> 1, texHeight >> 1, &im);
    scaleImage(&im, &old, 0);
    putImage(0, 0, &scr);
    putImage(20, 20, &im);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "Bumbing effect with full screen, this effect");
    showText(10, yc, &txt, "combined many images and use subtract, adding");
    showText(10, yc, &txt, "pixels to calculate render buffer. Scale down");
    showText(10, yc, &txt, "image using Bresenham algorithm for faster speed");
    showText(10, yc, &txt, "of image interpolation. Enter for next...");
    while (!finished(SDL_SCANCODE_RETURN));
    runLensFlare(&old);
    bilinearScaleImage(&im, &old);
    putImage(0, 0, &scr);
    putImage(20, 20, &im);
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "Yeah! Lens effect, this effect also combined many");
    showText(10, yc, &txt, "images too and other pixel manipulation such as");
    showText(10, yc, &txt, "substract, adding for each to render buffer. This");
    showText(10, yc, &txt, "also use bilinear algorithm with hight quality for");
    showText(10, yc, &txt, "scale down image. Using mouse hardware interrupts");
    showText(10, yc, &txt, "to tracking mouse event. Enter to continue...");
    while (!finished(SDL_SCANCODE_RETURN));
    
    fullSpeed = 0;
    showText(10, yc, &txt, "----");
    showText(10, yc, &txt, "That's all folks! More to come soon. In short time");
    showText(10, yc, &txt, "that's enought.See from my source code for another");
    showText(10, yc, &txt, "stuffs.If there occured something which seems tobe");
    showText(10, yc, &txt, "a bug or any suggestion,please contact me. Thanks!");
    showText(10, yc, &txt, "");
    showText(10, yc, &txt, "Nguyen Ngoc Van -- pherosiden@gmail.com");
    showText(10, yc, &txt, "");
    showText(10, yc, &txt, "Enter to exit ;-)");
    while (!finished(SDL_SCANCODE_RETURN));

    runExit();
    freeImage(&bg);
    freeImage(&txt);
    freeImage(&scr);
    freeImage(&old);
    freeImage(&im);
    cleanup();
}
