#include "gfxlib.h"

//max message lines
#define MAX_TEXT_LINE   23

//String buffer
typedef char STRBUFF[80];

//Show text intro message
int32_t     fullSpeed = 0;
STRBUFF     texts[MAX_TEXT_LINE] = {0};
GFX_IMAGE   fade1, fade2, flare;
GFX_IMAGE   bumpch, bumpimg, logo, sky;
GFX_IMAGE   flares[16] = { 0 };

//check and exit program
void runExit()
{
    int32_t i;
    GFX_IMAGE img;
    memset(&img, 0, sizeof(GFX_IMAGE));

    //capture current screen buffer
    getImage(0, 0, cresX, cresY, &img);

    //decrease rgb and push back to screen
    for (i = 0; i < 32; i++)
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
    freeImage(&bumpch);
    freeImage(&bumpimg);
    freeImage(&logo);
    freeImage(&sky);
    for (i = 0; i < 16; i++) freeImage(&flares[i]);
}

//Show intro message text string
void showText(int32_t sx, int32_t sy, GFX_IMAGE *img, const char *str)
{
    int32_t x, y, i, len;
    char msg[2] = {0};
    
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

    GFX_IMAGE ult, utb, trn, map;
    GFX_IMAGE scr, wcb, wci, gfx, gxb;
    
    int32_t i0, i1, i2;
    uint8_t mov = 0;
    uint8_t *buf1, *buf2;
    double startTime, waitTime;
    
    //load image
    loadImage("assets/gfxtheultimate.png", &ult);
    loadImage("assets/gfxlogo.png", &gfx);
    loadImage("assets/gfxwelcome.png", &wcb);
    loadImage("assets/map03.png", &map);

    //initialize buffer
    newImage(cresX, cresY, &scr);
    newImage(centerX, centerY, &trn);
    newImage(wcb.mWidth, wcb.mHeight, &wci);
    newImage(gfx.mWidth, gfx.mHeight, &gxb);
    newImage(ult.mWidth, ult.mHeight, &utb);

    i0 = trn.mWidth * trn.mHeight;

    //initialize turnel buffer
    buf1 = (uint8_t*)calloc(i0, sizeof(uint8_t));
    buf2 = (uint8_t*)calloc(i0, sizeof(uint8_t));
    if (!buf1 || !buf2) messageBox(GFX_ERROR, "RunIntro: cannot alloc memory.");

    //calculate turnel buffer
    prepareTunnel(&trn, buf1, buf2);

    //redirect draw buffer to image buffer
    setDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);

    i0 = 30;
    i1 = 25;
    i2 = 0;
    
    //start record time
    startTime = getTime();
    do {
        //draw and scale buffer
        waitTime = getTime();
        drawTunnel(&trn, &map, buf1, buf2, &mov, 1);
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

            if (i0 >= -1) putImageAdd(centerX - (wci.mWidth >> 1), centerY - (wci.mHeight >> 1), &wci);
            if (getElapsedTime(startTime) / 1000 >= tg) i0 = -1;
        }
        //logo GFXLIB
        else if ((getElapsedTime(startTime) / 1000 >= tg) && (i1 > 0))
        {
            blockOutMidImage(&gxb, &gfx, i1, i1);
            brightnessAlpha(&gxb, uint8_t(255 - i1 / 30.0 * 255.0));
            putImageAlpha(centerX - (gxb.mWidth >> 1), centerY - (gxb.mHeight >> 1), &gxb);
            i1--;
            if (getElapsedTime(startTime) / 1000 >= tu) i1 = 0;
        }
        //the ultimate message
        else if (i1 == 0)
        {
            putImageAlpha(centerX - (gfx.mWidth >> 1), centerY - (gfx.mHeight >> 1), &gfx);
            if ((getElapsedTime(startTime) / 1000 >= tu) && (i2 <= 15))
            {
                blurImageEx(&utb, &ult, 15 - (i2 & 15));
                brightnessImage(&utb, &utb, ((i2 & 15) + 1) * 15 + 15);
                putImageAdd(centerX - (ult.mWidth >> 1), centerY + (gfx.mHeight >> 1) + 30, &utb);
                i2++;
            }
            else
            {
                putImageAdd(centerX - (ult.mWidth >> 1), centerY + (gfx.mHeight >> 1) + 30, &utb);
                if (getElapsedTime(startTime) / 1000 >= to) fadeOutCircle(((getElapsedTime(startTime) / 1000 - to) / 3) * 100, 20, 3, 0);
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
    free(buf1);
    free(buf2);
}

void runBlocking(int32_t sx, int32_t sy)
{
    GFX_IMAGE img1, img2, img3;

    memset(&img1, 0, sizeof(GFX_IMAGE));
    memset(&img2, 0, sizeof(GFX_IMAGE));
    memset(&img3, 0, sizeof(GFX_IMAGE));

    //initialize buffer
    newImage(fade1.mWidth, fade1.mHeight, &img2);

    //blocking background
    int32_t dec = fade1.mWidth >> 2;
    do {
        dec--;
        blockOutMidImage(&img2, &fade1, dec << 1, dec << 1);
        brightnessImage(&img2, &img2, uint8_t(255.0 - double(dec) / (fade1.mWidth >> 2) * 255.0));
        putImage(sx, sy, &img2);
        render();
        delay(FPS_60);
    } while (dec > 0 && !finished(SDL_SCANCODE_RETURN));

    //save current background
    const int32_t width  = fade1.mWidth;
    const int32_t height = fade1.mHeight;

    putImage(sx, sy, &fade1);
    render();
    freeImage(&img2);

    //load next step
    loadImage("assets/gfxtext.png", &img1);
    newImage(img1.mWidth, img1.mHeight, &img2);

    //calculate current position and save current buffer
    const int32_t posx = (width - img2.mWidth) >> 1;
    const int32_t posy = (height - img2.mHeight) >> 1;
    getImage(sx + posx, sy + posy, img1.mWidth, img1.mHeight, &img3);

    //blocking next step
    dec = img1.mWidth >> 3;
    do {
        //start effect
        dec--;
        blockOutMidImage(&img2, &img1, dec << 1, dec << 1);
        brightnessAlpha(&img2, uint8_t(255.0 - double(dec) / (img1.mWidth >> 3) * 255.0));

        //render to screen
        putImage(sx + posx, sy + posy, &img3);
        putImageAlpha(sx + posx, sy + posy, &img2);
        render();
        delay(FPS_60);
    } while (dec > 0 && !finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&img1);
    freeImage(&img2);
    freeImage(&img3);
}

void runScaleUpImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE img1, img2, img3;

    //initialize buffer
    loadImage("assets/gfxspr.png", &img3);
    newImage(centerX, centerY, &img1);
    newImage(centerX, centerY, &img2);
       
    //setup lookup table
    int32_t* tables = (int32_t*)calloc(img2.mWidth, sizeof(int32_t));
    if (!tables) messageBox(GFX_ERROR, "ScaleUpImage: not enough memory for lookup tables.");

    do {
        //redirect render buffer to image buffer
        setDrawBuffer(img1.mData, img1.mWidth, img1.mHeight);

        //put some random pixel and GFX message
        for (int32_t i = 0; i < 400; i++) putPixel(random(img1.mWidth - 4) + 2, random(img1.mHeight - 4) + 2, RGB2INT(0, 255, 200));
        if (random(64) == 32) putImageAlpha((img1.mWidth - img3.mWidth) >> 1, (img1.mHeight - img3.mHeight) >> 1, &img3);

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
    } while (!finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&img1);
    freeImage(&img2);
    freeImage(&img3);
    free(tables);
}

void runCrossFade(int32_t sx, int32_t sy)
{
    GFX_IMAGE img;
    int32_t i = 0, up = 1, val = 1;

    //initialize render buffer
    newImage(fade1.mWidth, fade1.mHeight, &img);

    do {
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
    } while (!finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&img);
}

void runAddImage(int32_t sx, int32_t sy)
{
    int32_t i = 320;

    //change view port to screen
    setViewPort(sx, sy, centerX + 20 - 1, centerY + 20 - 1);

    do {
        i -= 4;
        //put lens image with adding background pixel
        putImage(sx, sy, &fade1);
        putImageAdd(int32_t(sx + (320 - cos(i / 160.0) * 320)), sy, &flare);
        render();
        delay(FPS_60);
    } while (i != 0 && !finished(SDL_SCANCODE_RETURN));

    //restore view port
    restoreViewPort();
}

void runRotateImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE img;
    uint32_t deg = 0;
    int32_t *tables = NULL;	

    //initialize render buffer
    newImage(fade2.mWidth, fade2.mHeight, &img);

    //pre-calculate lookup table
    tables = (int32_t*)calloc(intmax_t(fade2.mWidth) * 2 + fade2.mHeight + 2, sizeof(int32_t));
    if (!tables) messageBox(GFX_ERROR, "RotateImage: cannot alloc lookup tables.");

    do {
        deg++;

        //copy background
        memcpy(img.mData, fade1.mData, fade1.mSize);

        //rotate buffer
        rotateImage(&img, &fade2, tables, fade2.mWidth >> 1, fade2.mHeight >> 1, deg % 360, 1);
        putImage(sx, sy, &img);
        render();
        delay(FPS_60);
    } while (!finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&img);
    free(tables);
}

void runBilinearRotateImage(int32_t sx, int32_t sy)
{
    GFX_IMAGE img;
    int32_t angle = 0;

    //initialize render buffer
    newImage(fade2.mWidth, fade2.mHeight, &img);

    do {
        //copy background
        memcpy(img.mData, fade1.mData, fade1.mSize);

        //rotate buffer
        bilinearRotateImage(&img, &fade2, angle % 360);
        putImage(sx, sy, &img);
        render();
        delay(FPS_60);
        angle++;
    } while (!finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&img);
}

void runAntialias(int32_t sx, int32_t sy)
{
    GFX_IMAGE img, dst;

    //save current centerx, centery
    const int32_t xc = centerX;
    const int32_t yc = centerY;

    //initialize image buffer
    newImage(xc, yc, &img);
    newImage(xc, yc, &dst);

    do {
        //redirect drawing to image buffer
        setDrawBuffer(dst.mData, dst.mWidth, dst.mHeight);

        //draw anti-alias (smooth pixel) circle, line and ellipse
        for (int32_t i = 0; i < 3; i++)
        {
            //choose random color
            uint32_t col = RGB2INT(random(255) + 1, random(255) + 1, random(255) + 1);

            //which shape to be draw
            switch (random(3))
            {
            case 0: drawLineAlpha(random(xc), random(yc), random(xc), random(yc), col); break;
            case 1: drawCircleAlpha(random(xc), random(yc), random(xc) >> 2, col); break;
            case 2: drawEllipseAlpha(random(xc), random(yc), random(xc), random(yc), col); break;
            }
        }

        //restore draw buffer
        restoreDrawBuffer();

        //fade-out current buffer
        putImage(sx, sy, &dst);
        fadeOutImage(&dst, 4);
        render();
        delay(FPS_60);
    } while (!finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&img);
    freeImage(&dst);
}

void runLens(GFX_IMAGE* outImg)
{
    GFX_IMAGE scr;
    int32_t flareput[16] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    int32_t flarepos[16] = { -1110, -666, 0, 1087, 1221, 1309, 1776, 2197, 2819, 3130, 3220, 3263, 3663, 3707, 4440, 5125 };
    const char *str = "Drag your mouse to see details and left click to exit!";

    //load source image
    newImage(cresX, cresY, &scr);

    //current mouse pos and left button
    int32_t lmb = 0;
    int32_t mcx = centerX + 70;
    int32_t mdx = centerY - 80;

    //set mouse pointer limitation
    setMousePosition(mcx, mdx);
    showMouseCursor(SDL_DISABLE);
    
    //pre-calculate text position
    int32_t tx = (scr.mWidth - getFontWidth(str)) >> 1;
    int32_t ty = scr.mHeight - getFontHeight(str) - 4;

    //redirect to image buffer
    setDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);

    do {
        getMouseState(&mcx, &mdx, &lmb, NULL);
        putImage(0, 0, &sky);
        fillRectSub(0, 0, cmaxX, cmaxY, RGB2INT(0, uint8_t((double(mdx) / cmaxY) * 64), uint8_t((double(mdx) / cmaxY) * 64)));

        //put all flare image to render buffer
        for (int32_t i = 0; i < 16; i++)
        {
            //is show?
            if (flareput[i])
            {
                //merge current image buffer to background
                int32_t x = (centerX + ((centerX - mcx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mWidth >> 1);
                int32_t y = (centerY + ((centerY - mdx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mHeight >> 1);
                putImageAdd(x, y, &flares[i]);
            }
        }

        //put logo and draw text message
        putImageAlpha(cresX - logo.mWidth + 1, 0, &logo);
        writeText(tx, ty, RGB2INT(255, 255, 255), 2, str);
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
    GFX_IMAGE dst;
    int32_t cnt = 0;
    
    //loading source image
    newImage(cresX, cresY, &dst);

    do {
        //calculate position
        cnt++;
        int32_t lx = int32_t(cos(cnt / 13.0) * 133.0 + centerX);
        int32_t ly = int32_t(sin(cnt / 23.0) * 133.0 + centerY);

        //start bumping buffer
        bumpImage(&dst, &bumpch, &bumpimg, lx, ly);
        putImage(0, 0, &dst);
        render();
        delay(FPS_60);
        clearImage(&dst);
    } while (!finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&dst);
}

void runPlasmaScale(int32_t sx, int32_t sy)
{
    GFX_IMAGE plasma, screen;

    uint8_t sinx[256] = { 0 };
    uint32_t tectr, ofs;
    uint16_t x1, x2, x3, y1, y2, y3;
    uint16_t cr, cg, cb, a, b, c;

    //initialized lookup table and preload image
    for (int32_t y = 0; y < 256; y++) sinx[y] = uint8_t(sin(y * M_PI / 128) * 127 + 128);

    newImage(centerX >> 1, centerY >> 1, &plasma);
    newImage(centerX, centerY, &screen);

    uint32_t frames = 0;
    uint32_t* data = (uint32_t*)plasma.mData;
    uint16_t endx = plasma.mWidth >> 1;

    do {
        ofs = 0;
        tectr = frames * 10;
        x1 = sinx[(tectr / 12) & 0xFF];
        x2 = sinx[(tectr / 11) & 0xFF];
        x3 = sinx[frames & 0xFF];
        y1 = sinx[((tectr >> 3) + 64) & 0xFF];
        y2 = sinx[(tectr / 7 + 64) & 0xFF];
        y3 = sinx[(tectr / 12 + 64) & 0xFF];

        //calculate plasma buffer
        for (int32_t y = 0; y < plasma.mHeight; y++)
        {
            a = sqr(y - y1) + sqr(x1);
            b = sqr(y - y2) + sqr(x2);
            c = sqr(y - y3) + sqr(x3);
            cr = sinx[(a >> 6) & 0xFF];
            cg = sinx[(b >> 6) & 0xFF];
            cb = sinx[(c >> 6) & 0xFF];
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
            uint16_t col1;
            uint8_t sa, sb, sc;
            uint32_t col2, idx = ofs;

            for (int32_t x = 0; x < endx; x++)
            {
                c = x - x3 + c;
                sc = sinx[(c >> 6) & 0xFF];
                b = x - x2 + b;
                sb = sinx[(b >> 6) & 0xFF];
                a = x - x1 + a;
                sa = sinx[(a >> 6) & 0xFF];
                col2 = ((sa + cr) << 15) & 0xFFFF0000;
                col1 = (((sb + cg) << 7) & 0xFF00) + ((sc + cb) >> 1);
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
    } while (!finished(SDL_SCANCODE_RETURN));

    //clean up...
    freeImage(&plasma);
    freeImage(&screen);
}

void gfxDemo32()
{
    int32_t i, xc, yc;
    GFX_IMAGE bg, tx, scr, old, im;

    char sbuff[128] = { 0 };
    const char* initMsg = "Please wait while initialize GFXLIB...";

    memset(&tx, 0, sizeof(GFX_IMAGE));
    memset(&scr, 0, sizeof(GFX_IMAGE));
    memset(&old, 0, sizeof(GFX_IMAGE));
    
    loadFont("assets/sysfont.xfn", 0);
    initScreen(800, 600, 32, 0, "GFXLIB-Demo32");
    writeText(centerX - 8 * (int32_t(strlen(initMsg)) >> 1), centerY, RGB_GREY127, 2, initMsg);
    render();

    initSystemInfo();
    loadImage("assets/gfxbg5.png", &bg);
    loadImage("assets/gfxbumpchn.png", &bumpch);
    loadImage("assets/gfxbumpimg.png", &bumpimg);
    loadImage("assets/gfxlogosm.png", &logo);
    loadImage("assets/gfxsky.png", &sky);
    
    for (i = 0; i < 16; i++)
    {
        sprintf(sbuff, "assets/flare-%dx.png", i + 1);
        loadImage(sbuff, &flares[i]);
    }

    runIntro();
    putImage(0, 0, &bg);
    putImageAlpha(cresX - logo.mWidth + 1, cresY - logo.mHeight + 1, &logo);

    xc = centerX + 40;
    yc = centerY + 40;

    fillRectPatternAdd(10, 10, xc - 10, yc - 10, RGB_GREY32, ptnHatchX);
    fillRectSub(10, yc, xc - 10, cmaxY - 10, RGB_GREY32);
    fillRect(20, 20, xc - 20 - 1, yc - 20 - 1, 0);
    getImage(10, yc, xc - 20, cmaxY - yc - 10, &tx);

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
    showText(10, yc, &tx, "Please wait while loading images...");
    loadImage("assets/fade1x.png", &fade1);
    showText(10, yc, &tx, " - fade1x.png");
    loadImage("assets/fade2x.png", &fade2);
    showText(10, yc, &tx, " - fade2x.png");
    loadImage("assets/flare0.png", &flare);
    showText(10, yc, &tx, " - flare0.png");
    showText(10, yc, &tx, "");
    fullSpeed = 0;
    showText(10, yc, &tx, "This is an early demonstration of the abilities of");
    showText(10, yc, &tx, "GFXLIB. What you will see here are only some of");
    showText(10, yc, &tx, "the image manipulating effects which are currently");
    showText(10, yc, &tx, "built in. More to come and show you later...");
    delay(1000);
    showText(10, yc, &tx, "Starting...");
    runBlocking(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "What you saw was a combination of the command");
    showText(10, yc, &tx, "BlockOut and the command BrightnessImage. The text");
    showText(10, yc, &tx, "is an alphamapped image. You may see: working with");
    showText(10, yc, &tx, "images got very easy in GFXLIB - no half things");
    showText(10, yc, &tx, "anymore! To the next... press enter!");
    while (!finished(SDL_SCANCODE_RETURN));
    runAddImage(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "This was simply another flag of a draw-operation.");
    showText(10, yc, &tx, "It was used here to force draw to add the content");
    showText(10, yc, &tx, "of the image to the background of the image. You");
    showText(10, yc, &tx, "are also able to subtract the image and to work");
    showText(10, yc, &tx, "with an alphamap like PNG-images can contain one.");
    showText(10, yc, &tx, "The next effect - press enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runCrossFade(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "This thing is called CrossFading or AlphaBlending.");
    showText(10, yc, &tx, "In GFXLIB, the procedure is called 'BlendImage'.");
    showText(10, yc, &tx, "This procedure makes of 2 images another, where");
    showText(10, yc, &tx, "you can decide which image covers more the other.");
    showText(10, yc, &tx, "Enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runBilinearRotateImage(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "This is an image rotation. The responsible routine");
    showText(10, yc, &tx, "for this is called BilinearRotateImage. It doesn't");
    showText(10, yc, &tx, "seem to be very fast here, but in this demo the");
    showText(10, yc, &tx, "rotation is an optimize version of bilinear image");
    showText(10, yc, &tx, "interpolation. You can reach on a INTEL MMX-133 up");
    showText(10, yc, &tx, "to 20 fps at 640x480x32 bit. You can see another");
    showText(10, yc, &tx, "version of rotate image is so fast if only rotate");
    showText(10, yc, &tx, "and show image, check my source code for details.");
    showText(10, yc, &tx, "Enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runScaleUpImage(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Much more fancy than the other FX... Yeah, you see");
    showText(10, yc, &tx, "two effects combined here. scaleup and blur image");
    showText(10, yc, &tx, "are doing their work here. Check the source code");
    showText(10, yc, &tx, "to see the details. The next... Enter :)");
    while (!finished(SDL_SCANCODE_RETURN));
    runAntialias(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Antialiased lines, circles and ellipses. Possible");
    showText(10, yc, &tx, "with GFXLIB and also even faster than seen here");
    showText(10, yc, &tx, "(just slow for show). Perfect for 3D models and");
    showText(10, yc, &tx, "similar. Enter for the next...");
    while (!finished(SDL_SCANCODE_RETURN));
    runPlasmaScale(20, 20);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Plasma effect with hight color, this also combine");
    showText(10, yc, &tx, "scale up image with bilinear interpolation to");
    showText(10, yc, &tx, "process image with hight quality. This version is");
    showText(10, yc, &tx, "optimized using integer number but not really fast");
    showText(10, yc, &tx, "here. You can still optimized.");
    showText(10, yc, &tx, "Enter for the next...");
    while (!finished(SDL_SCANCODE_RETURN));
    fillRect(20, 20, xc - 20 - 1, yc - 20 - 1, 0);
    getImage(0, 0, cresX, cresY, &scr);
    runBumpImage();
    getImage(0, 0, cresX, cresY, &old);
    newImage(centerX, centerY, &im);
    scaleImage(&im, &old, 0);
    putImage(0, 0, &scr);
    putImage(20, 20, &im);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Bumbing effect with full screen, this effect");
    showText(10, yc, &tx, "combined many images and use subtract, adding");
    showText(10, yc, &tx, "pixels to calculate render buffer. Scale down");
    showText(10, yc, &tx, "image using Bresenham algorithm for faster speed");
    showText(10, yc, &tx, "of image interpolation. Enter for next...");
    while (!finished(SDL_SCANCODE_RETURN));
    runLens(&old);
    bilinearScaleImage(&im, &old);
    putImage(0, 0, &scr);
    putImage(20, 20, &im);
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "Yeah! Lens effect, this effect also combined many");
    showText(10, yc, &tx, "images too and other pixel manipulation such as");
    showText(10, yc, &tx, "substract, adding for each to render buffer. This");
    showText(10, yc, &tx, "also use bilinear algorithm with hight quality for");
    showText(10, yc, &tx, "scale down image. Using mouse hardware interrupts");
    showText(10, yc, &tx, "to tracking mouse event. Enter to continue...");
    while (!finished(SDL_SCANCODE_RETURN));
    fullSpeed = 0;
    showText(10, yc, &tx, "----");
    showText(10, yc, &tx, "That's all folks! More to come soon. In short time");
    showText(10, yc, &tx, "that's enought.See from my source code for another");
    showText(10, yc, &tx, "stuffs.If there occured something which seems tobe");
    showText(10, yc, &tx, "a bug or any suggestion,please contact me. Thanks!");
    showText(10, yc, &tx, "");
    showText(10, yc, &tx, "Nguyen Ngoc Van -- pherosiden@gmail.com");
    delay(1000);
    showText(10, yc, &tx, "");
    showText(10, yc, &tx, "Enter to exit ;-)");
    while (!finished(SDL_SCANCODE_RETURN));
    runExit();
    freeImage(&bg);
    freeImage(&tx);
    freeImage(&scr);
    freeImage(&old);
    freeImage(&im);
    cleanup();
}
