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
    getImage(0, 0, getBufferWidth(), getBufferHeight(), &img);

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
    if (!newImage(getBufferWidth(), getBufferHeight(), &scr)) return;
    if (!newImage(getBufferWidth() >> 1, getBufferHeight() >> 1, &trn)) return;
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
    const uint32_t startTime = getTime();
    const int32_t centerX = getCenterX();
    const int32_t centerY = getCenterY();

    do {
        //draw and scale buffer
        const uint32_t waitTime = getTime();
        drawTunnel(&trn, &map, buff1, buff2, &mov, 1);
        scaleImage(&scr, &trn, INTERPOLATION_TYPE_NORMAL);
        
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

            if (i0 >= -1) putImage(alignedSize(centerX - (wci.mWidth >> 1)), centerY - (wci.mHeight >> 1), &wci, BLEND_MODE_ADD);
            if (getElapsedTime(startTime) / 1000 >= tg) i0 = -1;
        }
        //logo GFXLIB
        else if ((getElapsedTime(startTime) / 1000 >= tg) && (i1 > 0))
        {
            blockOutMidImage(&gxb, &gfx, i1, i1);
            brightnessAlpha(&gxb, uint8_t(255 - i1 / 30.0 * 255.0));
            putImage(alignedSize(centerX - (gxb.mWidth >> 1)), centerY - (gxb.mHeight >> 1), &gxb, BLEND_MODE_ALPHA);

            i1--;
            if (getElapsedTime(startTime) / 1000 >= tu) i1 = 0;
        }
        //the ultimate message
        else if (i1 == 0)
        {
            putImage(alignedSize(centerX - (gfx.mWidth >> 1)), centerY - (gfx.mHeight >> 1), &gfx, BLEND_MODE_ALPHA);
            if ((getElapsedTime(startTime) / 1000 >= tu) && (i2 <= 15))
            {
                blurImageEx(&utb, &ult, 15 - (i2 & 15));
                brightnessImage(&utb, &utb, ((i2 & 15) + 1) * 15 + 15);
                putImage(alignedSize(centerX - (ult.mWidth >> 1)), centerY + (gfx.mHeight >> 1) + 30, &utb, BLEND_MODE_ADD);
                i2++;
            }
            else
            {
                putImage(alignedSize(centerX - (ult.mWidth >> 1)), centerY + (gfx.mHeight >> 1) + 30, &utb, BLEND_MODE_ADD);
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
    const int32_t posx = alignedSize((width - img2.mWidth) >> 1);
    const int32_t posy = alignedSize((height - img2.mHeight) >> 1);
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
    if (!newImage(getBufferWidth() >> 1, getBufferHeight() >> 1, &img1)) return;
    if (!newImage(getBufferWidth() >> 1, getBufferHeight() >> 1, &img2)) return;
       
    //setup lookup table
    int32_t* tables = (int32_t*)calloc(img2.mWidth, sizeof(int32_t));
    if (!tables)
    {
        messageBox(GFX_ERROR, "ScaleUpImage: not enough memory for lookup tables.");
        return;
    }

    //background color
    const uint32_t rcolor = rgb(0, 255, 200);

    //loop until enter key pressed
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //redirect render buffer to image buffer
        changeDrawBuffer(img1.mData, img1.mWidth, img1.mHeight);

        //put some random pixel and GFX message
        for (int32_t i = 0; i < 400; i++) putPixel(random(img1.mWidth - 4) + 2, random(img1.mHeight - 4) + 2, rcolor);
        if (random(64) == 32) putImage((img1.mWidth - img3.mWidth) >> 1, (img1.mHeight - img3.mHeight) >> 1, &img3, BLEND_MODE_ALPHA);

        //blur & scale buffer
        blurImage(&img1);
        scaleUpImage(&img2, &img1, tables, 7, 7);

        //restore to screen buffer to draw
        restoreDrawBuffer();
        putImage(sx, sy, &img2);
        render();
        delay(FPS_90);

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
        delay(FPS_90);

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
        putImage(alignedSize(int32_t((320 - cos(step / 160.0) * 320))), 0, &flare, BLEND_MODE_ADD);
        restoreDrawBuffer();
        putImage(sx, sy, &img);
        render();
        delay(FPS_90);
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
        delay(FPS_90);
    }

    //cleanup...
    freeImage(&img);
    free(tables);
}

void runFastRotateImage(int32_t sx, int32_t sy)
{
    //initialize render buffer
    GFX_IMAGE img = { 0 };
    if (!newImage(fade2.mWidth, fade2.mHeight, &img)) return;
    if (!img.mData) return;
    
    //start angle
    int32_t degree = 0;

    //loop until return
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //first step
        degree++;

        //copy background
        memcpy(img.mData, fade1.mData, fade1.mSize);

        //rotate buffer
        rotateImage(&img, &fade2, degree % 360, INTERPOLATION_TYPE_BILINEAR);
        putImage(sx, sy, &img);
        render();
        delay(FPS_90);
    }

    //cleanup...
    freeImage(&img);
}

void runAntiAliased(int32_t sx, int32_t sy)
{
    //save current midx, midy
    const int32_t midx = getBufferWidth() >> 1;
    const int32_t midy = getBufferHeight() >> 1;

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
        delay(FPS_90);
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
    if (!newImage(getBufferWidth(), getBufferHeight(), &scr)) return;
    
    const int32_t centerX = getCenterX();
    const int32_t centerY = getCenterY();

    //current mouse position and left button
    int32_t lmb = 0;
    int32_t mcx = centerX + 70;
    int32_t mdx = centerY - 80;

    //set mouse pointer limitation
    setMousePosition(mcx, mdx);
    showMouseCursor(SDL_DISABLE);
    
    //pre-calculate text position
    const int32_t tx = (scr.mWidth - getFontWidth(str)) >> 1;
    const int32_t ty = (scr.mHeight - getFontHeight(str)) - 4;

    //redirect to image buffer
    changeDrawBuffer(scr.mData, scr.mWidth, scr.mHeight);

    const int32_t cmaxX = getMaxX();
    const int32_t cmaxY = getMaxY();
    const int32_t logox = alignedSize(getBufferWidth() - gfxlogo.mWidth);

    //time for record FPS
    uint32_t time = 0, oldTime = 0;

    do {
        getMouseState(&mcx, &mdx, &lmb, NULL);
        putImage(0, 0, &gfxsky);
        fillRect(0, 0, alignedSize(cmaxX), cmaxY, rgb(0, uint8_t((double(mdx) / cmaxY) * 64), uint8_t((double(mdx) / cmaxY) * 64)), BLEND_MODE_SUB);

        //put all flares image to render buffer
        for (int32_t i = 0; i < 16; i++)
        {
            //is show?
            if (flareput[i])
            {
                //merge current image buffer to background
                const int32_t x = (centerX + ((centerX - mcx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mWidth >> 1);
                const int32_t y = (centerY + ((centerY - mdx) * (flarepos[i] - 2280) / 2280)) - (flares[i].mHeight >> 1);
                putImage(alignedSize(x), y, &flares[i], BLEND_MODE_ADD);
            }
        }

        //put logo and draw text message
        putImage(logox, 1, &gfxlogo, BLEND_MODE_ALPHA);
        writeText(tx, ty, RGB_WHITE, 2, str);

        //timing for input and FPS counter
        oldTime = time;
        time = getTime();

        //report FPS counter
        writeText(1, 1, RGB_WHITE, 0, "FPS:%.2f", 1.0 / ((time - oldTime) / 1000.0));

        render();
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
    if (!newImage(getBufferWidth(), getBufferHeight(), &dst)) return;

    const int32_t centerX = getCenterX();
    const int32_t centerY = getCenterY();

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
        delay(FPS_90);
        clearImage(&dst);
    }

    //cleanup...
    freeImage(&dst);
}

void runPlasmaScale(int32_t sx, int32_t sy)
{
    uint8_t sina[256] = { 0 };

    //initialized lookup table and pre-load image
    for (int32_t y = 0; y < 256; y++) sina[y] = uint8_t(sin(y * M_PI / 128) * 127 + 128);

    GFX_IMAGE plasma = { 0 }, screen = { 0 };
    if (!newImage(getBufferWidth() >> 2, getBufferHeight() >> 2, &plasma)) return;
    if (!newImage(getBufferWidth() >> 1, getBufferHeight() >> 1, &screen)) return;

    uint32_t frames = 0;
    uint32_t* data = (uint32_t*)plasma.mData;
    const uint16_t endx = plasma.mWidth >> 1;

    //loop until return
    while (!finished(SDL_SCANCODE_RETURN))
    {
        uint32_t ofs = 0;
        const uint32_t tectr = frames * 10;
        const uint16_t x1 = sina[(tectr / 12) & 0xff];
        const uint16_t x2 = sina[(tectr / 11) & 0xff];
        const uint16_t x3 = sina[frames & 0xff];
        const uint16_t y1 = sina[((tectr >> 3) + 64) & 0xff];
        const uint16_t y2 = sina[(tectr / 7 + 64) & 0xff];
        const uint16_t y3 = sina[(tectr / 12 + 64) & 0xff];

        //calculate plasma buffer
        for (int32_t y = 0; y < plasma.mHeight; y++)
        {
            uint16_t a = sqr(y - y1) + sqr(x1);
            uint16_t b = sqr(y - y2) + sqr(x2);
            uint16_t c = sqr(y - y3) + sqr(x3);
            uint16_t cr = sina[(a >> 6) & 0xff];
            uint16_t cg = sina[(b >> 6) & 0xff];
            uint16_t cb = sina[(c >> 6) & 0xff];
#ifdef _USE_ASM
            __asm {
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
                and     bx, 0xff
                mov     bl, byte ptr sina[ebx]
                mov     si, bx
                mov     bx, ax
                sub     bx, x2
                add     bx, b
                mov     b, bx
                shr     bx, cl
                and     bx, 0xff
                mov     dl, byte ptr sina[ebx]
                mov     bx, ax
                sub     bx, x1
                add     bx, a
                mov     a, bx
                shr     bx, cl
                and     bx, 0xff
                mov     bl, byte ptr sina[ebx]
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
                const uint8_t sc = sina[(c >> 6) & 0xff];
                b = x - x2 + b;
                const uint8_t sb = sina[(b >> 6) & 0xff];
                a = x - x1 + a;
                const uint8_t sa = sina[(a >> 6) & 0xff];
                const uint32_t col2 = ((sa + cr) << 15) & 0xffff0000;
                const uint16_t col1 = (((sb + cg) << 7) & 0xff00) + ((sc + cb) >> 1);
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
        scaleImage(&screen, &plasma, INTERPOLATION_TYPE_BICUBIC);
        putImage(sx, sy, &screen);
        render();
        delay(FPS_90);
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

    const int32_t centerX = getCenterX();
    const int32_t centerY = getCenterY();
    const int32_t cwidth = getBufferWidth();
    const int32_t cheight = getBufferHeight();
    
    writeText(centerX - 8 * (uint32_t(strlen(initMsg)) >> 1), centerY, RGB_GREY191, 2, initMsg);
    render();

    if (!initSystemInfo())
    {
        cleanup();
        return;
    }
    
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
    putImage(alignedSize(cwidth - gfxlogo.mWidth), cheight - gfxlogo.mHeight - 1, &gfxlogo, BLEND_MODE_ALPHA);

    GFX_IMAGE txt = { 0 };
    const int32_t xc = centerX + 40;
    const int32_t yc = centerY + 40;
    const int32_t tx = alignedSize(10);

    fillRectPattern(alignedSize(10), 10, alignedSize(xc - 23), yc - 19, RGB_GREY32, getPattern(PATTERN_TYPE_HATCH_X), BLEND_MODE_ADD);
    fillRect(alignedSize(10), yc, alignedSize(xc - 23), getMaxY() - yc - 9, RGB_GREY32, BLEND_MODE_SUB);
    fillRect(alignedSize(20), 20, alignedSize(xc - 39), yc - 39, 0);
    getImage(alignedSize(10), yc, alignedSize(xc - 23), getMaxY() - yc - 9, &txt);

    writeText(xc + tx,  70, RGB_GREY127, 2, "GFXLIB %s", GFX_VERSION);
    writeText(xc + tx,  90, RGB_GREY127, 2, "A short show of some abilities");
    writeText(xc + tx, 100, RGB_GREY127, 2, "GFXLIB does provide. Note that");
    writeText(xc + tx, 110, RGB_GREY127, 2, "this is only a small amount of");
    writeText(xc + tx, 120, RGB_GREY127, 2, "all available features.");
    writeText(xc + tx, 150, RGB_GREY127, 2, "%s", getVideoName());
    writeText(xc + tx, 160, RGB_GREY127, 2, "Driver Version   : %s", getDriverVersion());
    writeText(xc + tx, 170, RGB_GREY127, 2, "Video Memory     : %luMB", getVideoMemory());
    writeText(xc + tx, 180, RGB_GREY127, 2, "Video Mode       : %s", getVideoModeInfo());
    writeText(xc + tx, 190, RGB_GREY127, 2, "Render System    : %s", getRenderVersion());
    writeText(xc + tx, 200, RGB_GREY127, 2, "Image Library    : %s", getImageVersion());
    writeText(xc + tx, 220, RGB_GREY127, 2, "%s", getCpuName());
    writeText(xc + tx, 230, RGB_GREY127, 2, "CPU Features     : %s", getCpuFeatures());
    writeText(xc + tx, 240, RGB_GREY127, 2, "CPU Frequency    : %luMHz", getCpuSpeed());
    writeText(xc + tx, 250, RGB_GREY127, 2, "Physical Memory  : %luMB", getTotalMemory());
    writeText(xc + tx, 260, RGB_GREY127, 2, "Available Memory : %luMB", getAvailableMemory());
    render();

    fullSpeed = 1;
    showText(tx, yc, &txt, "Please wait while loading images...");

    if (!loadImage("assets/fade1x.png", &fade1)) return;
    showText(tx, yc, &txt, " - fade1x.png");

    if (!loadImage("assets/fade2x.png", &fade2)) return;
    showText(tx, yc, &txt, " - fade2x.png");

    if (!loadImage("assets/flare0.png", &flare)) return;
    showText(tx, yc, &txt, " - flare0.png");

    showText(tx, yc, &txt, "");

    fullSpeed = 0;
    showText(tx, yc, &txt, "This is an early demonstration of the abilities of");
    showText(tx, yc, &txt, "GFXLIB. What you'll see here are just a few of the");
    showText(tx, yc, &txt, "image manipulation effects that are currently");
    showText(tx, yc, &txt, "available. There will be more to show you later...");
    delay(1000);
    showText(tx, yc, &txt, "Starting...");
    runBlocking(alignedSize(20), 20);

    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "What you saw was a combination of the command");
    showText(tx, yc, &txt, "blockOut and the command brightnessImage. The text");
    showText(tx, yc, &txt, "is an alpha mapped image. You may see that working");
    showText(tx, yc, &txt, "with images has gotten very easy in GFXLIB-no");
    showText(tx, yc, &txt, "half-things anymore! Press the enter key!");
    while (!finished(SDL_SCANCODE_RETURN));
    runAddImage(alignedSize(20), 20);

    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "This was simply another flag of a draw-operation.");
    showText(tx, yc, &txt, "It was used here to force draw to add the content");
    showText(tx, yc, &txt, "of the image to the background of the image. You");
    showText(tx, yc, &txt, "are also able to subtract the image and to work");
    showText(tx, yc, &txt, "with an alpha map like PNG-images can contain one.");
    showText(tx, yc, &txt, "The next effect - press enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runCrossFade(alignedSize(20), 20);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "This thing is called crossFading or alphaBlending.");
    showText(tx, yc, &txt, "In GFXLIB, the procedure is called blendImage.");
    showText(tx, yc, &txt, "This procedure creates 2 images of one another,");
    showText(tx, yc, &txt, "where you can decide which image covers more of");
    showText(tx, yc, &txt, "the other. For the next, enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runFastRotateImage(alignedSize(20), 20);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "This is a smooth rotation. The responsible routine");
    showText(tx, yc, &txt, "for this is called bicubicRotateImage. It doesn't");
    showText(tx, yc, &txt, "seem to be very fast here because of the delay,");
    showText(tx, yc, &txt, "but in this demo the rotation is fully optimized.");
    showText(tx, yc, &txt, "You can reach up to 420 fps at 640x480x32bits with");
    showText(tx, yc, &txt, "the INTEL CORE I7-4770K. You can see another vers-");
    showText(tx, yc, &txt, "ion of the rotated image is so fast if you only");
    showText(tx, yc, &txt, "rotate and show the image. Check my source code");
    showText(tx, yc, &txt, "for an optimize version using hardware accelera-");
    showText(tx, yc, &txt, "tion of the AVX2 instructions. Press Enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    runScaleUpImage(alignedSize(20), 20);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "Much fancier than the other FX...Yeah, you see");
    showText(tx, yc, &txt, "two effects combined here. Scales and blurred");
    showText(tx, yc, &txt, "image are doing their work here. Check the source");
    showText(tx, yc, &txt, "code to see the details. Press enter... ;)");
    while (!finished(SDL_SCANCODE_RETURN));
    runAntiAliased(alignedSize(20), 20);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "Anti-aliased lines, circles and ellipses. Possible");
    showText(tx, yc, &txt, "with GFXLIB and also even faster than seen here");
    showText(tx, yc, &txt, "(just slow for show). Ideal for 3D models and the");
    showText(tx, yc, &txt, "like. Enter for the next...");
    while (!finished(SDL_SCANCODE_RETURN));
    runPlasmaScale(alignedSize(20), 20);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "Plasma effect with high colors. This also combines");
    showText(tx, yc, &txt, "the scaled up image with bi-cubic interpolation to");
    showText(tx, yc, &txt, "process the image with the best quality. This ver-");
    showText(tx, yc, &txt, "sion is fully optimized by using a fixed number");
    showText(tx, yc, &txt, "and SSE2 instructions to maximize speed (extremely");
    showText(tx, yc, &txt, "fast). Enter for the next...");
    while (!finished(SDL_SCANCODE_RETURN));
    fillRect(alignedSize(20), 20, alignedSize(xc - 39), yc - 39, 0);

    GFX_IMAGE scr = { 0 };
    getImage(0, 0, cwidth, cheight, &scr);
    runBumpImage();

    GFX_IMAGE old = { 0 }, im = { 0 };
    getImage(0, 0, cwidth, cheight, &old);
    newImage(cwidth >> 1, cheight >> 1, &im);
    scaleImage(&im, &old, 0);
    putImage(0, 0, &scr);
    putImage(alignedSize(20), 20, &im);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "2D bump mapping effect with full screen, this");
    showText(tx, yc, &txt, "effect also combines many images and uses sub-");
    showText(tx, yc, &txt, "tracting and adding pixels to calculate the render");
    showText(tx, yc, &txt, "buffer. Scale the image using Bresenham algorithm");
    showText(tx, yc, &txt, "for quick image interpolation. Enter for the next.");
    while (!finished(SDL_SCANCODE_RETURN));
    runLensFlare(&old);
    scaleImage(&im, &old, INTERPOLATION_TYPE_BICUBIC);
    putImage(0, 0, &scr);
    putImage(alignedSize(20), 20, &im);
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "The lens flare effect, this effect is a simulation");
    showText(tx, yc, &txt, "of the lens flare in photo shop. It's combined");
    showText(tx, yc, &txt, "many images too, and other pixel manipulations");
    showText(tx, yc, &txt, "such as subtracting and adding for each to the");
    showText(tx, yc, &txt, "render buffer. This is also used for bi-cubic");
    showText(tx, yc, &txt, "interpolation with the best quality for scale.");
    showText(tx, yc, &txt, "Use hardware mouse tracking events. Enter...");
    while (!finished(SDL_SCANCODE_RETURN));
    
    fullSpeed = 0;
    showText(tx, yc, &txt, "----");
    showText(tx, yc, &txt, "That's all, folks! More to come soon. In a short");
    showText(tx, yc, &txt, "time, that's enough. See my source code for other");
    showText(tx, yc, &txt, "stuff. If there is something which seems to be a");
    showText(tx, yc, &txt, "bug or any suggestion, please contact me at:");
    showText(tx, yc, &txt, "https://github.com/pherosiden/gfxlib. Many thanks!");
    showText(tx, yc, &txt, "");
    showText(tx, yc, &txt, "Nguyen Ngoc Van -- pherosiden@gmail.com");
    showText(tx, yc, &txt, "");
    showText(tx, yc, &txt, "Enter to exit ;-)");
    while (!finished(SDL_SCANCODE_RETURN));

    runExit();
    freeImage(&bg);
    freeImage(&txt);
    freeImage(&scr);
    freeImage(&old);
    freeImage(&im);
    cleanup();
}
