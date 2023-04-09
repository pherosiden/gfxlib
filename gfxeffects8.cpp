#include "gfxlib.h"

namespace juliaSet {

    void makePalette()
    {
        RGB pal[256] = { 0 };

        pal[0].r = 0;
        pal[0].g = 0;
        pal[0].b = 0;

        for (uint8_t a = 1; a <= 85; a++)
        {
            const uint8_t b = a * 255 / 85;
            pal[a      ].r = b;
            pal[85  + a].g = b;
            pal[170 + a].b = b;
            pal[170 + a].r = 0;
            pal[a      ].g = 0;
            pal[85  + a].b = 0;
            pal[171 - a].r = b;
            pal[256 - a].g = b;
            pal[86  - a].b = b;
        }
        
        setPalette(pal);
    }

    //call with juliaSet(256, -0.7, 0.27015, 1, 0, 0);
    void runJulia(int32_t iters, double cre, double cim, double zoom, double mx, double my)
    {
        if (!initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, "Julia-Set Demo")) return;

        int32_t mwidth = 0, mheight = 0;
        uint8_t* pixels = (uint8_t*)getDrawBuffer(&mwidth, &mheight);

        mwidth >>= 1;
        mheight >>= 1;

        const int32_t cwidth = getDrawBufferWidth();
        const int32_t cheight = getDrawBufferHeight();

        for (int32_t y = 0; y < cheight; y++)
        {
            const double preim = 1.0 * (intptr_t(y) - mheight) / (0.5 * zoom * cheight) + my;
            for (int32_t x = 0; x < cwidth; x++)
            {
                int32_t i = 0;
                double newim = preim;
                double newre = 1.5 * (intptr_t(x) - mwidth) / (0.5 * zoom * cwidth) + mx;

                for (i = 1; i <= iters; i++)
                {
                    const double oldre = newre;
                    const double oldim = newim;

                    newre = oldre * oldre - oldim * oldim + cre;
                    newim = 2 * oldre * oldim + cim;

                    if ((newre * newre + newim * newim) > 4) break;
                }

                *pixels++ = (i & 0xff);
            }
        }

        makePalette();
        rotatePalette(1, 255, 0, 10);
        cleanup();
    }

    //call with mandelbrotSet(512, 1179.8039, -0.743153, -0.139405);
    void runMandelbrot(int32_t iters, double zoom, double mx, double my)
    {
        if (!initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, "Mandelbrot-Set")) return;

        int32_t mwidth = 0, mheight = 0;
        uint8_t* pixels = (uint8_t*)getDrawBuffer(&mwidth, &mheight);

        mwidth >>= 1;
        mheight >>= 1;

        const int32_t cwidth = getDrawBufferWidth();
        const int32_t cheight = getDrawBufferHeight();

        for (int32_t y = 0; y < cheight; y++)
        {
            const double pi = 1.0 * (intptr_t(y) - mheight) / (0.5 * zoom * cheight) + my;

            for (int32_t x = 0; x < cwidth; x++)
            {
                int32_t i = 0;
                double newre = 0;
                double newim = 0;
                const double pr = 1.5 * (intptr_t(x) - mwidth) / (0.5 * zoom * cwidth) + mx;

                for (i = 1; i <= iters; i++)
                {
                    const double oldre = newre;
                    const double oldim = newim;

                    newre = oldre * oldre - oldim * oldim + pr;
                    newim = 2 * oldre * oldim + pi;

                    if ((newre * newre + newim * newim) > 4) break;
                }

                *pixels++ = (i & 0xff);
            }
        }

        makePalette();
        rotatePalette(1, 255, 0, 10);
        cleanup();
    }

    void run()
    {
        runJulia(256, -0.7, 0.27015, 1, 0, 0);
        runMandelbrot(512, 1179.8039, -0.743153, -0.139405);
    }
}

namespace fadePalette {
    void run()
    {
        const RGB dst[256] = { 0 };
        RGB src[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fade-IO")) return;
        uint8_t* vbuff = (uint8_t*)getDrawBuffer();

        if (!loadPNG(vbuff, src, "assets/arnold.png")) return;
        setPalette(dst);
        fadeIn(src, FPS_90);
        fadeMax(FPS_90);
        fadeOut(src, FPS_90);
        fadeMin(FPS_90);
        cleanup();
    }
}

namespace bumpMap {
    uint8_t dbuff[SIZE_256][SIZE_256] = { 0 };
    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void createEnvironmentMap()
    {
        FILE* fp = fopen("assets/envmap.dat", "rb");
        if (fp)
        {
            fread(dbuff, 1, SIZE_256 * SIZE_256, fp);
            fclose(fp);
        }
        else
        {
            for (int16_t y = 0; y < SIZE_256; y++)
            {
                for (int16_t x = 0; x < SIZE_256; x++)
                {
                    const double nx = (x - 128.0) / 128.0;
                    const double ny = (y - 128.0) / 128.0;
                    double nz = 1.0 - sqrt(nx * nx + ny * ny);
                    if (nz < 0) nz = 0;
                    dbuff[y][x] = uint8_t(nz * 128);
                }
            }

            fp = fopen("assets/envmap.dat", "wb");
            if (fp)
            {
                fwrite(dbuff, 1, SIZE_256 * SIZE_256, fp);
                fclose(fp);
            }
        }
    }

    void setFadePalette()
    {
        int16_t i = 0;
        RGB rgb[256] = { 0 };

        int16_t r1 = 0, r2 = 0;
        int16_t g1 = 0, g2 = 0;
        int16_t b1 = 0, b2 = 20;
        int16_t start = 0, end = 127;

        int16_t rval = r1 << 8;
        int16_t gval = g1 << 8;
        int16_t bval = b1 << 8;

        int16_t rstep = ((r2 - r1 + 1) << 8) / (end - start + 1);
        int16_t gstep = ((g2 - g1 + 1) << 8) / (end - start + 1);
        int16_t bstep = ((b2 - b1 + 1) << 8) / (end - start + 1);

        for (i = start; i <= end; i++)
        {
            rgb[i].r = rval >> 8;
            rgb[i].g = gval >> 8;
            rgb[i].b = bval >> 8;
            rval += rstep;
            gval += gstep;
            bval += bstep;
        }

        r1 = 0;  r2 = 63;
        g1 = 0;  g2 = 63;
        b1 = 20; b2 = 63;

        start = 128;
        end = 255;

        rval = r1 << 8;
        gval = g1 << 8;
        bval = b1 << 8;

        rstep = ((r2 - r1 + 1) << 8) / (end - start + 1);
        gstep = ((g2 - g1 + 1) << 8) / (end - start + 1);
        bstep = ((b2 - b1 + 1) << 8) / (end - start + 1);

        for (i = start; i <= end; i++)
        {
            rgb[i].r = rval >> 8;
            rgb[i].g = gval >> 8;
            rgb[i].b = bval >> 8;
            rval += rstep;
            gval += gstep;
            bval += bstep;
        }

        shiftPalette(rgb);
        setPalette(rgb);
    }

    void bumpScreen(int16_t lx, int16_t ly)
    {
        for (int16_t y = 1; y < MAX_HEIGHT; y++)
        {
            for (int16_t x = 0; x <= MAX_WIDTH; x++)
            {
                int16_t nx = vbuff2[y][x - 1] - vbuff2[y][x + 1];
                int16_t ny = vbuff2[y - 1][x] - vbuff2[y + 1][x];
                nx = nx - x + lx + 128;
                ny = ny - y + ly + 128;
                if (nx < 0 || nx > 255) nx = 0;
                if (ny < 0 || ny > 255) ny = 0;
                vbuff1[y][x] = dbuff[ny][nx] + 120;
            }
        }
    }

    void run()
    {
        int32_t x = 0, y = 0, lmb = 0;

        createEnvironmentMap();
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Bump-Mapping -- Move your mouse!")) return;
        if (!loadPNG(vbuff2[0], NULL, "assets/intro.png")) return;
        setFadePalette();
        setMousePosition(IMAGE_WIDTH, IMAGE_HEIGHT);
        showMouseCursor(SDL_DISABLE);

        do {
            getMouseState(&x, &y, &lmb);
            bumpScreen(x >> 1, y >> 1);
            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        } while (!finished(SDL_SCANCODE_RETURN) && !lmb);
        showMouseCursor(SDL_ENABLE);
        cleanup();
    }
}

namespace fireBump {
    uint16_t    num = 0;
    uint16_t    limit[8] = { 0 };
    uint8_t     vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void autoBump()
    {
        num++;
        const int16_t lx = int16_t(80 * cos(num / 20.0) + IMAGE_MIDX);
        const int16_t ly = int16_t(80 * sin(num / 20.0) + IMAGE_MIDY);

        for (int16_t y = 1; y < MAX_HEIGHT; y++)
        {
            int16_t vy = y - ly;
            for (int16_t x = 1; x < MAX_WIDTH; x++)
            {
                const int16_t vx = x - lx;
                const int16_t nx = vbuff1[y][x + 1] - vbuff1[y][x - 1];
                const int16_t ny = vbuff1[y + 1][x] - vbuff1[y - 1][x];
                const int16_t col = (abs(vx - nx) + abs(vy - ny) + vbuff1[y][x]) >> 1;
                if (col > 127) vbuff2[y][x] = 0;
                else vbuff2[y][x] = limit[uint16_t(vbuff1[y][x]) >> 8] - col;
            }
        }
    }

    void run()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire-Bump")) return;
        if (!loadPNG(vbuff1[0], NULL, "assets/author.png")) return;

        for (i = 0; i < 8; i++) limit[i] = 127 * (i + 1);
                
        for (i = 0; i < 64; i++)
        {
            pal[i     ].r = uint8_t(i);
            pal[i     ].g = 0;
            pal[i     ].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = uint8_t(i);
            pal[i + 64].b = 0;
        }

        shiftPalette(pal);
        setPalette(pal);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            autoBump();
            renderBuffer(vbuff2, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace circleEffect {
    double costab[400] = { 0 };
    double sinTab[400] = { 0 };

    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff3[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void preCalc()
    {
        double deg = 0.0;
        for (int16_t i = 0; i < 400; i++)
        {
            costab[i] = cos(deg);
            sinTab[i] = sin(deg);
            deg += 0.05;
        }
    }

    int16_t roundf(double x)
    {
        return (x >= 0) ? int16_t(x + 0.5) : int16_t(x - 0.5);
    }

    void paintCircle(int16_t x, int16_t y, int16_t rad)
    {
        if (x < 37) x = 37;
        if (y < 37) y = 37;
        if (x > IMAGE_WIDTH - 38) x = IMAGE_WIDTH - 38;
        if (y > IMAGE_HEIGHT - 38) y = IMAGE_HEIGHT - 38;

        for (int16_t i = 0; i < 400; i++)
        {
            const int16_t ox = roundf(rad * costab[i]);
            const int16_t oy = roundf(rad * sinTab[i]);
            vbuff2[y + oy][x + ox] = vbuff1[y + oy][x + ox];
        }
    }

    void run()
    {
        RGB pal[256] = { 0 };
        int32_t x = 0, y = 0, lmb = 0;
        
        preCalc();
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Circle -- Move your mouse!")) return;
        if (!loadPNG(vbuff1[0], pal, "assets/image.png")) return;
        if (!loadPNG(vbuff3[0], NULL, "assets/wall.png")) return;
        setPalette(pal);
        setMousePosition(40 << 1, 70 << 1);
        showMouseCursor(SDL_DISABLE);

        do {
            getMouseState(&x, &y, &lmb);
            memcpy(vbuff2, vbuff3, IMAGE_SIZE);
            for (int16_t i = 0; i < 38; i++) paintCircle(x >> 1, y >> 1, i);
            renderBuffer(vbuff2, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        } while (!finished(SDL_SCANCODE_RETURN) && !lmb);
        showMouseCursor(SDL_ENABLE);
        cleanup();
    }
}

namespace crossFade {
    RGB     src[256] = { 0 };
    RGB     dst[256] = { 0 };

    uint8_t dirt = 0;
    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff3[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void setItUp()
    {
        RGB pal1[256] = { 0 };
        RGB pal2[256] = { 0 };

        clearPalette();
        if (!loadPNG(vbuff1[0], pal2, "assets/to.png")) return;
        memcpy(vbuff3, vbuff1, IMAGE_SIZE);
        if (!loadPNG(vbuff1[0], pal1, "assets/from.png")) return;
    
        for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
        {
            for (int16_t x = 0; x < IMAGE_WIDTH; x++)
            {
                const uint8_t pix1 = vbuff1[y][x];
                const uint8_t pix2 = vbuff3[y][x];

                if (pix1 || pix2)
                {
                    uint8_t col = 0;
                    uint8_t change = 0;

                    const uint8_t rs = pal1[pix1].r;
                    const uint8_t gs = pal1[pix1].g;
                    const uint8_t bs = pal1[pix1].b;

                    const uint8_t rd = pal2[pix2].r;
                    const uint8_t gd = pal2[pix2].g;
                    const uint8_t bd = pal2[pix2].b;

                    for (uint8_t k = 0; k <= col; k++)
                    {
                        if ((src[k].r == rs) && (src[k].g == gs) && (src[k].b == bs) && (dst[k].r == rd) && (dst[k].g == gd) && (dst[k].b == bd))
                        {
                            vbuff2[y][x] = k;
                            change = 1;
                        }
                    }

                    if (!change)
                    {
                        col++;

                        src[col].r = rs;
                        src[col].g = gs;
                        src[col].b = bs;

                        dst[col].r = rd;
                        dst[col].g = gd;
                        dst[col].b = bd;

                        vbuff2[y][x] = col;
                    }
                }
            }
        }
    }

    void fadeColor(int16_t dirt, int16_t depth)
    {
        int16_t i = 0, j = 0;
        RGB pal[256] = { 0 };

        if (dirt)
        {
            memcpy(pal, src, sizeof(pal));
            setPalette(src);

            for (i = 0; i < depth; i++)
            {
                for (j = 0; j < 256; j++)
                {
                    if (pal[j].r < dst[j].r && pal[j].r < 252) pal[j].r += 4;
                    if (pal[j].r > dst[j].r && pal[j].r >   3) pal[j].r -= 4;
                    if (pal[j].g < dst[j].g && pal[j].g < 252) pal[j].g += 4;
                    if (pal[j].g > dst[j].g && pal[j].g >   3) pal[j].g -= 4;
                    if (pal[j].b < dst[j].b && pal[j].b < 252) pal[j].b += 4;
                    if (pal[j].b > dst[j].b && pal[j].b >   3) pal[j].b -= 4;
                    readKeys();
                    if (keyDown(SDL_SCANCODE_RETURN)) return;
                    if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
                }
                setPalette(pal);
                delay(FPS_90);
                readKeys();
                if (keyDown(SDL_SCANCODE_RETURN)) return;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }
        }
        else
        {
            memcpy(pal, dst, sizeof(pal));
            setPalette(dst);

            for (i = 0; i < depth; i++)
            {
                for (j = 0; j < 256; j++)
                {
                    if (pal[j].r < src[j].r && pal[j].r < 252) pal[j].r += 4;
                    if (pal[j].r > src[j].r && pal[j].r >   3) pal[j].r -= 4;
                    if (pal[j].g < src[j].g && pal[j].g < 252) pal[j].g += 4;
                    if (pal[j].g > src[j].g && pal[j].g >   3) pal[j].g -= 4;
                    if (pal[j].b < src[j].b && pal[j].b < 252) pal[j].b += 4;
                    if (pal[j].b > src[j].b && pal[j].b >   3) pal[j].b -= 4;
                    readKeys();
                    if (keyDown(SDL_SCANCODE_RETURN)) return;
                    if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
                }
                setPalette(pal);
                delay(FPS_90);
                readKeys();
                if (keyDown(SDL_SCANCODE_RETURN)) return;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Cross-Fade")) return;

        FILE *fp = fopen("assets/fade.dat", "rb");
        if (!fp)
        {
            fp = fopen("assets/fade.dat", "wb");
            if (!fp) return;

            setItUp();
            fwrite(src, sizeof(RGB), 256, fp);
            fwrite(dst, sizeof(RGB), 256, fp);
            fwrite(vbuff2, 1, IMAGE_SIZE, fp);
        }
        else
        {
            fread(src, sizeof(RGB), 256, fp);
            fread(dst, sizeof(RGB), 256, fp);
            fread(vbuff2, 1, IMAGE_SIZE, fp);
        }
        fclose(fp);

        setPalette(src);
        renderBuffer(vbuff2, SCREEN_MIDX, SCREEN_MIDY);

        dirt = 1;

        while (!keyDown(SDL_SCANCODE_RETURN)) 
        {
            fadeColor(dirt, 64);
            dirt = !dirt;
        }
        cleanup();
    }
}

namespace rainEffect {
    #define SEMILLA_INIT    0x9234
    #define ANCHO_OLAS      2
    #define DENSITY_INIT    5
    #define DENSITY_SET     6
    #define MAXIMO          150

    uint8_t     aborts = 0;
    uint8_t*    vmem = NULL;
    uint8_t     vbuff[IMAGE_SIZE] = { 0 };
    uint8_t     densityAdd = 0;

    int32_t     actualPage = 0;
    int32_t     otherPage = IMAGE_MIDY * IMAGE_WIDTH;
    
    uint16_t    randVal = 0;
    uint16_t    tblFil[5] = { 0 };
    uint16_t    tblCol[5] = { 0 };

    const uint8_t rgbPal[] = {
        0x3F,0x3F,0x3F,0x3C,0x3F,0x3F,0x3A,0x3F,0x3F,0x37,0x3E,0x3F,0x34,0x3E,0x3F,0x32,
        0x3E,0x3F,0x2F,0x3E,0x3F,0x2D,0x3D,0x3F,0x2A,0x3D,0x3F,0x29,0x3C,0x3F,0x28,0x3B,
        0x3F,0x27,0x3B,0x3F,0x27,0x3B,0x3F,0x26,0x3A,0x3F,0x25,0x39,0x3F,0x24,0x38,0x3F,
        0x23,0x37,0x3F,0x22,0x37,0x3F,0x21,0x36,0x3F,0x20,0x35,0x3F,0x1F,0x34,0x3F,0x1E,
        0x33,0x3F,0x1D,0x32,0x3F,0x1C,0x31,0x3F,0x1B,0x30,0x3F,0x1A,0x2F,0x3F,0x19,0x2E,
        0x3F,0x18,0x2D,0x3F,0x17,0x2C,0x3F,0x16,0x2A,0x3F,0x15,0x29,0x3F,0x14,0x28,0x3F,
        0x13,0x27,0x3F,0x12,0x25,0x3E,0x11,0x25,0x3D,0x11,0x23,0x3D,0x10,0x22,0x3C,0x0F,
        0x20,0x3B,0x0F,0x20,0x3A,0x0E,0x1E,0x39,0x0D,0x1D,0x39,0x0C,0x1B,0x38,0x0C,0x1B,
        0x37,0x0B,0x19,0x36,0x0B,0x18,0x35,0x0A,0x16,0x35,0x09,0x16,0x34,0x09,0x14,0x33,
        0x08,0x13,0x32,0x07,0x12,0x31,0x07,0x11,0x31,0x06,0x10,0x30,0x06,0x0F,0x2F,0x05,
        0x0E,0x2F,0x05,0x0D,0x2E,0x04,0x0C,0x2D,0x04,0x0B,0x2C,0x03,0x0A,0x2B,0x03,0x09,
        0x2B,0x02,0x08,0x2A,0x02,0x07,0x29,0x02,0x06,0x28,0x01,0x06,0x27,0x01,0x04,0x27,
        0x01,0x04,0x26,0x01,0x04,0x26,0x01,0x04,0x26,0x01,0x04,0x25,0x01,0x04,0x25,0x01,
        0x04,0x25,0x01,0x04,0x24,0x00,0x04,0x24,0x00,0x04,0x24,0x00,0x04,0x23,0x00,0x03,
        0x23,0x00,0x03,0x22,0x00,0x03,0x22,0x00,0x03,0x22,0x00,0x03,0x21,0x00,0x03,0x21,
        0x00,0x03,0x21,0x00,0x03,0x20,0x00,0x03,0x20,0x00,0x03,0x20,0x00,0x03,0x20,0x00,
        0x03,0x1F,0x00,0x03,0x1F,0x00,0x03,0x1F,0x00,0x03,0x1E,0x00,0x03,0x1E,0x00,0x02,
        0x1E,0x00,0x02,0x1D,0x00,0x02,0x1D,0x00,0x02,0x19,0x00,0x02,0x16,0x00,0x01,0x12,
        0x00,0x01,0x0F,0x00,0x00,0x0A,0x00,0x00,0x05,0x00,0x00,0x00,0x0E,0x00,0x00,0x13,
        0x00,0x00,0x18,0x00,0x00,0x1D,0x00,0x00,0x22,0x00,0x00,0x27,0x00,0x00,0x2C,0x00,
        0x00,0x31,0x00,0x00,0x34,0x00,0x00,0x38,0x00,0x00,0x37,0x00,0x00,0x36,0x00,0x02,
        0x35,0x00,0x04,0x35,0x00,0x05,0x34,0x00,0x08,0x33,0x00,0x09,0x33,0x00,0x0A,0x32,
        0x00,0x0B,0x31,0x00,0x0C,0x30,0x00,0x0D,0x30,0x01,0x0E,0x2F,0x00,0x0F,0x2E,0x00,
        0x10,0x2E,0x00,0x10,0x2D,0x00,0x12,0x2C,0x01,0x13,0x2C,0x01,0x14,0x2B,0x01,0x14,
        0x2A,0x01,0x15,0x29,0x01,0x16,0x29,0x01,0x16,0x28,0x01,0x17,0x29,0x03,0x19,0x2A,
        0x05,0x1B,0x2C,0x08,0x1D,0x2D,0x0B,0x1F,0x2E,0x0D,0x20,0x2F,0x10,0x22,0x31,0x13,
        0x25,0x32,0x17,0x27,0x33,0x1A,0x29,0x34,0x1E,0x2B,0x36,0x21,0x2D,0x37,0x25,0x30,
        0x38,0x29,0x32,0x3A,0x2D,0x34,0x3B,0x31,0x37,0x3C,0x36,0x3A,0x3E,0x3A,0x3C,0x3F,
        0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F
    };

    const uint16_t lkpFil[] = {
        25,25,26,27,28,3,32,34,37,39,42,45,
        48,52,55,58,61,63,66,68,7,72,73,74,
        75,75,75,74,73,72,7,68,66,63,61,58,
        55,52,48,45,42,39,37,34,32,3,28,27,
        26,25,0xff
    };

    const uint16_t lkpCol[] = {
         15, 15, 16, 16, 17, 18, 20, 21, 23, 25, 27, 30,
         33, 36, 39, 42, 45, 49, 52, 56, 60, 64, 68, 72,
         76, 80, 84, 88, 92, 96,  0,104,108,111,115,118,
        121,124,127,130,133,135,137,139,140,142,143,144,
        144,145,145,145,144,144,143,142,140,139,137,135,
        133,130,127,124,121,118,115,111,108,104,100, 96,
         92, 88, 84, 80, 76, 72, 68, 64, 60, 56, 52, 49,
         45, 42, 39, 36, 33, 30, 27, 25, 23, 21, 20, 18,
         17, 16, 16, 15, 0xff
    };

    void putFrame()
    {
        if (aborts) return;

#ifdef _USE_ASM
        __asm {
            lea     esi, vbuff
            mov     edi, vmem
            add     edi, MAX_SIZE - MAX_WIDTH
            mov     ecx, IMAGE_MIDX
        lp4:
            push    edi
            push    ecx
            xor     ax, ax
            xor     bx, bx
            xor     dx, dx
        lp3:
            xor     ecx, ecx
            mov     cx, [esi]
            sar     cx, 7
            add     cx, ax
            sub     cx, dx
            jle     lp1
            add     dx, cx
        lp2:
            mov     [edi], bx
            sub     di, IMAGE_WIDTH
            loop    lp2
        lp1:
            add     esi, 2
            add     bx, 0x101
            inc     ax
            cmp     ax, IMAGE_MIDY
            jnz     lp3
            mov     dh, dl
        lp5:
            sub     edi, IMAGE_WIDTH
            mov     [edi], dx
            add     dx, 0x101
            cmp     dx, MAXIMO + MAXIMO * SIZE_256;
            jnz     lp5
            pop     ecx
            pop     edi
            add     edi, 2
            loop    lp4
        }
#else
        uint16_t *si = (uint16_t*)vbuff;
        uint16_t *di = (uint16_t*)&vmem[MAX_SIZE - MAX_WIDTH];

        for (uint16_t cx = 0; cx < IMAGE_MIDX; cx++)
        {
            uint16_t bx = 0;
            uint16_t dx = 0;
            uint16_t *ofs = di;

            for (uint16_t ax = 0; ax < IMAGE_MIDY; ax++)
            {
                int16_t val = *si;
                val = (val >> 7) + ax - dx;
                if (val > 0)
                {
                    dx += val;
                    while (val--)
                    {
                        *ofs = bx;
                        ofs -= IMAGE_MIDX;
                    }
                }
                si++;
                bx += 0x101;
            }

            dx = (dx << 8) | (dx & 0x00FF);
            while (dx < MAXIMO * 0x101)
            {
                ofs -= IMAGE_MIDX;
                *ofs = dx;
                dx += 0x101;
            }
            di++;
        }
#endif
        render();
        readKeys();
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        if (keyDown(SDL_SCANCODE_RETURN)) aborts = 1;
        delay(FPS_90);
    }

    void stabylize()
    {
#ifdef _USE_ASM
        __asm {
            mov     esi, actualPage
            mov     edi, otherPage
            mov     actualPage, edi
            mov     otherPage, esi
            add     esi, IMAGE_HEIGHT
            add     edi, IMAGE_HEIGHT
            mov     cl, densityAdd
            mov     ebx, (IMAGE_MIDX * IMAGE_MIDY - IMAGE_HEIGHT) / 2
        again:
            mov     eax, dword ptr vbuff[esi - IMAGE_HEIGHT]
            add     eax, dword ptr vbuff[esi + IMAGE_HEIGHT]
            add     eax, dword ptr vbuff[esi + 2]
            add     eax, dword ptr vbuff[esi - 2]
            ror     eax, 16
            sar     ax, 1
            ror     eax, 16
            sar     ax, 1
            sub     eax, dword ptr vbuff[edi]
            mov     edx, eax
            sar     dx, cl
            ror     edx, 16
            sar     dx, cl
            ror     edx, 16
            sub     eax, edx
            mov     dword ptr vbuff[edi], eax
            add     edi, 4
            add     esi, 4
            dec     ebx
            jnz     again
        }
#else
        uint16_t si = actualPage;
        uint16_t di = otherPage;
        actualPage = di;
        otherPage = si;
        si += IMAGE_HEIGHT;
        di += IMAGE_HEIGHT;
        const uint16_t count = (IMAGE_MIDX * IMAGE_MIDY - IMAGE_HEIGHT) >> 1;

        for (uint16_t bx = 0; bx < count; bx++)
        {
            uint32_t eax = *(uint32_t*)&vbuff[si - IMAGE_HEIGHT];
            eax += *(uint32_t*)&vbuff[si + IMAGE_HEIGHT];
            eax += *(uint32_t*)&vbuff[si + 2];
            eax += *(uint32_t*)&vbuff[si - 2];
            eax = _rotr(eax, 16);
            eax = (eax & 0xffff0000) + (int16_t(eax) >> 1);
            eax = _rotr(eax, 16);
            eax = (eax & 0xffff0000) + (int16_t(eax) >> 1);
            eax -= *(uint32_t*)&vbuff[di];

            uint32_t edx = eax;
            edx = (edx & 0xffff0000) + (int16_t(edx) >> densityAdd);
            edx = _rotr(edx, 16);
            edx = (edx & 0xffff0000) + (int16_t(edx) >> densityAdd);
            edx = _rotr(edx, 16);
            eax -= edx;
            *(uint32_t*)&vbuff[di] = eax;
            di += 4;
            si += 4;
        }
#endif
    }

    void stabylize2()
    {
#ifdef _USE_ASM
        __asm {
            mov     esi, actualPage
            mov     edi, otherPage
            mov     actualPage, edi
            mov     otherPage, esi
            add     esi, IMAGE_HEIGHT
            add     edi, IMAGE_HEIGHT
            mov     ecx, IMAGE_MIDX * IMAGE_MIDY - IMAGE_HEIGHT - IMAGE_MIDY
        again:
            mov     ax, word ptr vbuff[esi - IMAGE_HEIGHT]
            add     ax, word ptr vbuff[esi + IMAGE_HEIGHT]
            add     ax, word ptr vbuff[esi + 2]
            add     ax, word ptr vbuff[esi - 2]
            add     ax, word ptr vbuff[esi - IMAGE_HEIGHT - 2]
            add     ax, word ptr vbuff[esi - IMAGE_HEIGHT - 2]
            add     ax, word ptr vbuff[esi + IMAGE_HEIGHT - 2]
            add     ax, word ptr vbuff[esi + IMAGE_HEIGHT - 2]
            sar     ax, 3
            mov     word ptr vbuff[edi], ax
            add     edi, 2
            add     esi, 2
            loop    again
        }
#else
        uint16_t si = actualPage;
        uint16_t di = otherPage;
        actualPage = di;
        otherPage = si;
        si += IMAGE_HEIGHT;
        di += IMAGE_HEIGHT;
        const uint16_t count = IMAGE_MIDX * IMAGE_MIDY - IMAGE_HEIGHT - IMAGE_MIDY;

        for (uint16_t bx = 0; bx < count; bx++)
        {
            int16_t ax = *(uint16_t*)&vbuff[si - IMAGE_HEIGHT];
            ax += *(uint16_t*)&vbuff[si + IMAGE_HEIGHT];
            ax += *(uint16_t*)&vbuff[si + 2];
            ax += *(uint16_t*)&vbuff[si - 2];
            ax += *(uint16_t*)&vbuff[si - IMAGE_HEIGHT - 2];
            ax += *(uint16_t*)&vbuff[si - IMAGE_HEIGHT - 2];
            ax += *(uint16_t*)&vbuff[si + IMAGE_HEIGHT - 2];
            ax += *(uint16_t*)&vbuff[si + IMAGE_HEIGHT - 2];
            ax >>= 3;
            *(uint16_t*)&vbuff[di] = ax;
            di += 2;
            si += 2;
        }
#endif
    }

    void putPoint(int16_t idx, int32_t rnd1, int32_t rnd2)
    {
#ifdef _USE_ASM
        __asm {
            mov     si, idx
            rol     esi, 16
            mov     si, idx
            mov     eax, IMAGE_MIDY
            mul     rnd1
            mov     ebx, rnd2
            add     ebx, eax
            shl     ebx, 1
            add     ebx, actualPage
            lea     edi, vbuff
            add     edi, ebx
            mov     [edi                     ], esi
            mov     [edi + 4                 ], esi
            mov     [edi + IMAGE_MIDY * 2    ], esi
            mov     [edi + IMAGE_MIDY * 2 + 4], esi
            mov     [edi + IMAGE_MIDY * 4    ], esi
            mov     [edi + IMAGE_MIDY * 4 + 4], esi
            mov     [edi + IMAGE_MIDY * 6    ], esi
            mov     [edi + IMAGE_MIDY * 6 + 4], esi
        }
#else
        const uint32_t esi = _rotl(idx, 16) + idx;
        const uint16_t di = ((rnd2 + IMAGE_MIDY * rnd1) << 1) + actualPage;

        *(uint32_t*)&vbuff[di                     ] = esi;
        *(uint32_t*)&vbuff[di + 4                 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2    ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2 + 4] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4    ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4 + 4] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6    ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6 + 4] = esi;
#endif
    }

    void putBigPoint(int16_t idx, int32_t rnd1, int32_t rnd2)
    {
#ifdef _USE_ASM
        __asm {
            mov     si, idx
            rol     esi, 16
            mov     si, idx
            mov     eax, IMAGE_MIDY
            mul     rnd1
            mov     ebx, rnd2
            add     ebx, eax
            shl     ebx, 1
            add     ebx, actualPage
            lea     edi, vbuff
            add     edi, ebx
            mov     [edi                       ], esi
            mov     [edi + 4                   ], esi
            mov     [edi + 8                   ], esi
            mov     [edi + 10                  ], esi
            mov     [edi + IMAGE_MIDY * 2      ], esi
            mov     [edi + IMAGE_MIDY * 2 + 4  ], esi
            mov     [edi + IMAGE_MIDY * 2 + 8  ], esi
            mov     [edi + IMAGE_MIDY * 2 + 10 ], esi
            mov     [edi + IMAGE_MIDY * 4      ], esi
            mov     [edi + IMAGE_MIDY * 4 + 4  ], esi
            mov     [edi + IMAGE_MIDY * 4 + 8  ], esi
            mov     [edi + IMAGE_MIDY * 4 + 10 ], esi
            mov     [edi + IMAGE_MIDY * 6      ], esi
            mov     [edi + IMAGE_MIDY * 6 + 4  ], esi
            mov     [edi + IMAGE_MIDY * 6 + 8  ], esi
            mov     [edi + IMAGE_MIDY * 6 + 10 ], esi
            mov     [edi + IMAGE_MIDY * 8      ], esi
            mov     [edi + IMAGE_MIDY * 8 + 4  ], esi
            mov     [edi + IMAGE_MIDY * 8 + 8  ], esi
            mov     [edi + IMAGE_MIDY * 8 + 10 ], esi
            mov     [edi + IMAGE_MIDY * 10     ], esi
            mov     [edi + IMAGE_MIDY * 10 + 4 ], esi
            mov     [edi + IMAGE_MIDY * 10 + 8 ], esi
            mov     [edi + IMAGE_MIDY * 10 + 10], esi
        }
#else
        const uint32_t esi = _rotl(idx, 16) + idx;
        const uint16_t di = ((rnd2 + IMAGE_MIDY * rnd1) << 1) + actualPage;

        *(uint32_t*)&vbuff[di                       ] = esi;
        *(uint32_t*)&vbuff[di + 4                   ] = esi;
        *(uint32_t*)&vbuff[di + 8                   ] = esi;
        *(uint32_t*)&vbuff[di + 10                  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2      ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2 + 4  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2 + 8  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2 + 10 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4      ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4 + 4  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4 + 8  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4 + 10 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6      ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6 + 4  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6 + 8  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6 + 10 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 8      ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 8 + 4  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 8 + 8  ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 8 + 10 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 10     ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 10 + 4 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 10 + 8 ] = esi;
        *(uint32_t*)&vbuff[di + IMAGE_MIDY * 10 + 10] = esi;
#endif
    }

    void calcRandValue()
    {
#ifdef _USE_ASM
        __asm {
            inc     randVal
            xor     dx, dx
            mov     ax, randVal
            mov     bx, 32719
            mul     bx
            add     ax, 3
            mov     bx, 32749
            div     bx
            add     dx, 0xF000
            mov     randVal, dx
        }
#else
        randVal++;
        randVal = ((randVal * 32719 + 3) % 32749) + 0xf000;
#endif
    }

    void readPoint(uint16_t* rnd1, uint16_t* rnd2)
    {
#ifdef _USE_ASM
        void* randFunc = calcRandValue;
        __asm {
            call    randFunc
            and     dx, 0FFh
            add     dx, 32
            push    dx
            call    randFunc
            mov     bx, dx
            pop     dx
            and     bx, 0FFh
            add     bx, 36
            mov     esi, rnd1
            mov     [esi], dx
            mov     esi, rnd2
            mov     [esi], bx
        }
#else
        calcRandValue();
        *rnd1 = (randVal & 0xff) + 32;
        calcRandValue();
        *rnd2 = (randVal & 0xff) + 36;
#endif
    }

    void touch(int16_t idx)
    {
        uint16_t rnd1, rnd2;
        readPoint(&rnd1, &rnd2);
        putPoint(idx, rnd1 >> 1, rnd2);
    }

    void bigTouch(int16_t idx)
    {
        uint16_t rnd1, rnd2;
        readPoint(&rnd1, &rnd2);
        putBigPoint(idx, rnd1 >> 1, rnd2);
    }

    void putLine(int16_t idx, int32_t rnd1)
    {
#ifdef _USE_ASM
        __asm {
            mov     si, idx
            rol     esi, 16
            mov     si, idx
            mov     eax, IMAGE_MIDY * 2
            mul     rnd1
            mov     ebx, eax
            add     ebx, ANCHO_OLAS + 2 * 8
            lea     edi, vbuff
            add     edi, ebx
            mov     ecx, (IMAGE_MIDY / 2 - ANCHO_OLAS - 2 * 8)
        again:
            mov     [edi                  ], esi
            mov     [edi + IMAGE_MIDY * 2 ], esi
            mov     [edi + IMAGE_MIDY * 4 ], esi
            mov     [edi + IMAGE_MIDY * 6 ], esi
            mov     [edi + IMAGE_MIDY * 8 ], esi
            mov     [edi + IMAGE_MIDY * 10], esi
            add     edi, 4
            loop    again
        }
#else
        const uint32_t esi = _rotl(idx, 16) + idx;
        const uint16_t count = (IMAGE_MIDY / 2 - ANCHO_OLAS - 2 * 8);
        uint16_t di = (ANCHO_OLAS + 2 * 8) + (2 * IMAGE_MIDY * rnd1);

        for (uint16_t cx = 0; cx < count; cx++)
        {
            *(uint32_t*)&vbuff[di                  ] = esi;
            *(uint32_t*)&vbuff[di + IMAGE_MIDY * 2 ] = esi;
            *(uint32_t*)&vbuff[di + IMAGE_MIDY * 4 ] = esi;
            *(uint32_t*)&vbuff[di + IMAGE_MIDY * 6 ] = esi;
            *(uint32_t*)&vbuff[di + IMAGE_MIDY * 8 ] = esi;
            *(uint32_t*)&vbuff[di + IMAGE_MIDY * 10] = esi;
            di += 4;
        }
#endif
    }

    void readSinus(int16_t val, uint16_t* rnd1, uint16_t* rnd2)
    {
#ifdef _USE_ASM
        __asm {
            xor     ebx, ebx
            xor     esi, esi
            add     bx, val
            shl     bx, 1
        lp0:
            mov     si, tblFil[ebx]
            mov     cx, lkpFil[esi]
            cmp     cx, 0xff
            jnz     lp1
            mov     tblFil[ebx], 0
            jmp     lp0
        lp1:
            mov     si, tblCol[ebx]
            mov     dx, lkpCol[esi]
            cmp     dx, 0xff
            jnz     lp2
            mov     tblCol[ebx], 0
            jmp     lp1
        lp2:
            add     tblCol[ebx], 2
            add     tblFil[ebx], 2
            mov     esi, rnd1
            mov     [esi], dx
            mov     esi, rnd2
            mov     [esi], cx
        }
#else
        uint16_t si = 0, cx = 0, dx = 0;

        while (1)
        {
            si = tblFil[val];
            cx = lkpFil[si];
            if (cx != 0xff) break;
            tblFil[val] = 0;
        }

        while (1)
        {
            si = tblCol[val];
            dx = lkpCol[si];
            if (dx != 0xff) break;
            tblCol[val] = 0;
        }

        tblCol[val]++;
        tblFil[val]++;

        *rnd1 = dx;
        *rnd2 = cx;
#endif
    }

    void clearSinus()
    {
        tblCol[0] = 10;
        tblCol[1] = 20;
        tblCol[2] = 40;
        tblCol[3] = 0;
        tblCol[4] = 0;
        tblFil[0] = 50;
        tblFil[1] = 20;
        tblFil[2] = 40;
        tblFil[3] = 40;
        tblFil[4] = 0;
    }

    void touchSinus(int16_t idx, int16_t val)
    {
        uint16_t rnd1, rnd2;
        readSinus(val, &rnd1, &rnd2);
        putPoint(idx, rnd1, rnd2);
    }

    void printFrame(int16_t num)
    {
        for (int16_t i = 0; i < num; i++)
        {
            putFrame();
            stabylize();
        }
    }

    void P001()
    {
        int16_t idx = 0;
        int16_t i = 0, j = 0;
        
        for (i = 0; i < 40; i++)
        {
            for (j = 0; j < 3; j++)
            {
                bigTouch(idx);
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                if (aborts) return;
            }
            idx += 25;
        }

        for (i = 0; i < 40; i++)
        {
            for (j = 0; j < 3; j++)
            {
                bigTouch(idx);
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                if (aborts) return;
            }
            idx -= 20;
        }
    }

    void P002()
    {
        int16_t i = 0, j = 0;

        for (i = 0; i < 5; i++)
        {
            putPoint(3000, 80, 150);
            for (j = 0; j < 50; j++)
            {
                touch(300);
                touch(200);
                touch(100);
                stabylize();
                putFrame();
                if (aborts) return;
            }
        }

        for (i = 0; i < 5; i++)
        {
            stabylize();
            putFrame();
            if (aborts) return;
        }
    }

    void P003()
    {
        int16_t i = 0;

        for (i = 0; i < 30; i++)
        {
            bigTouch(-4000);
            stabylize2();
            putFrame();
            bigTouch(-4000);
            stabylize2();
            putFrame();
            bigTouch(-4000);
            stabylize2();
            putFrame();
            if (aborts) return;
        }

        for (i = 0; i < 20; i++)
        {
            stabylize2();
            putFrame();
            if (aborts) return;
        }
    }

    void P004()
    {
        int16_t i = 0, j = 0;

        for (i = 0; i < 7; i++)
        {
            putPoint(3000, 80, 150);
            for (j = 0; j < 5; j++)
            {
                stabylize();
                putFrame();
                if (aborts) return;
            }
            putPoint(-2000, 80, 150);
            for (j = 0; j < 30; j++)
            {
                stabylize();
                putFrame();
                if (aborts) return;
            }
        }

        for (i = 0; i < 80; i++)
        {
            stabylize();
            putFrame();
            if (aborts) return;
        }
    }

    void P005(int16_t A, int16_t B)
    {
        for (int16_t i = 0; i < 150; i++)
        {
            if (A == 0)
            {
                touchSinus(800, 0);
                touchSinus(800, 1);
                touchSinus(800, 2);
                touchSinus(800, 3);
                touchSinus(800, 4);
            }
            else if (A == 1)
            {
                touchSinus(800, 1);
                touchSinus(800, 2);
                touchSinus(800, 3);
                touchSinus(800, 4);
            }
            else if (A == 2)
            {
                touchSinus(800, 2);
                touchSinus(800, 3);
                touchSinus(800, 4);
            }
            else if (A == 3)
            {
                touchSinus(800, 3);
                touchSinus(800, 4);
            }
            else if (A == 4)
            {
                touchSinus(800, 4);
            }

            printFrame(B);
            if (aborts) return;
        }
    }

    void P006(int16_t A, int16_t B)
    {
        for (int16_t i = 0; i < 4; i++)
        {
            if (A != 0) putLine(-1500, 1);
            for (int16_t j = 0; j < 50; j++)
            {
                if (B != 0) touch(250);
                stabylize();
                putFrame();
                stabylize();
                putFrame();
                if (aborts) return;
            }
        }
    }

    void setupPalette()
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 150; i++)
        {
            pal[i].r = rgbPal[i * 3 + 0] << 2;
            pal[i].g = rgbPal[i * 3 + 1] << 2;
            pal[i].b = rgbPal[i * 3 + 2] << 2;
        }
        setPalette(pal);
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Rain")) return;

        densityAdd = DENSITY_INIT;
        vmem = (uint8_t*)getDrawBuffer();
        memset(&vmem[IMAGE_WIDTH * IMAGE_MIDY], 0x63, IMAGE_WIDTH);

        setupPalette();
        printFrame(1);
        stabylize();

        while (!aborts)
        {
            randVal = SEMILLA_INIT;
            densityAdd = DENSITY_INIT;
            P001();
            P002();
            densityAdd = DENSITY_SET;
            P003();
            printFrame(100);
            P003();
            printFrame(100);
            densityAdd = DENSITY_INIT;
            P004();
            printFrame(100);
            clearSinus();
            P005(4, 2);
            P005(3, 2);
            P005(2, 2);
            P005(4, 1);
            P005(4, 1);
            P005(3, 1);
            densityAdd = DENSITY_SET;
            P006(1, 0);
            P006(1, 1);
            P006(0, 1);
        }
        cleanup();
    }
}

namespace waterEffect {
    #define MAXPAGE 2
    #define DENSITY	4

    uint16_t    sintab[256] = { 0 };
    uint16_t    costab[256] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     dbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    int16_t     water[MAXPAGE][IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void initSinCos()
    {
        for (int16_t i = 0; i < 256; i++)
        {
            sintab[i] = uint16_t(sin(i / 20.0) * 80 + IMAGE_MIDY);
            costab[i] = uint16_t(cos(i / 40.0) * 100 + IMAGE_MIDX);
        }
    }

    void showWater(uint16_t page)
    {
#ifdef _USE_ASM
        int32_t tmp = IMAGE_WIDTH;
        uint32_t cpage = 0, spage = 0;
        
        __asm {
            mov     ax, page
            and     ax, 0x01
            jz      setp1
            mov     cpage, 128000
            mov     spage, 0
            jmp     setp2
        setp1:
            mov     cpage, 0
            mov     spage, 128000
        setp2:
            xor     edi, edi
            xor     esi, esi
            mov     edi, 640
            mov     esi, edi
            add     edi, cpage
            add     esi, spage
            mov     dx, 1
            mov     cx, 1
        next:
            inc     tmp
            add     esi, 2
            add     edi, 2
            mov     ax, water[edi -   2]
            add     ax, water[edi - 638]
            add     ax, water[edi - 640]
            add     ax, water[edi - 642]
            add     ax, water[edi +   2]
            add     ax, water[edi + 638]
            add     ax, water[edi + 640]
            add     ax, water[edi + 642]
            sar     ax, 2
            mov     bx, water[esi]
            sub     ax, bx
            mov     bx, ax
            sar     bx, DENSITY
            sub     ax, bx
            mov     water[esi], ax
            xor     ebx, ebx
            mov     ax, water[esi]
            sub     ax, water[esi + 2]
            mov     bx, water[esi]
            sub     bx, water[esi + 640]
            sar     bx, 3
            add     bx, cx
            sar     ax, 3
            add     ax, dx
            shl     ax, 6
            add     bx, ax
            shl     ax, 2
            add     bx, ax
            mov     al, dbuff[ebx]
            mov     ebx, tmp
            mov     vbuff[ebx], al
            inc     cx
            cmp     cx, IMAGE_WIDTH - 2
            jbe     next
            add     esi, 4
            add     edi, 4
            add     tmp, 2
            mov     cx, 1
            inc     dx
            cmp     dx, IMAGE_HEIGHT - 2
            jbe     next
        }
#else
        const uint32_t cpage = page;
        const uint32_t spage = page ^ 1;

        for (int16_t y = 1; y < MAX_HEIGHT; y++)
        {
            for (int16_t x = 1; x < MAX_WIDTH; x++)
            {
                const int16_t nw = ((
                    water[cpage][y    ][x - 1] +
                    water[cpage][y    ][x + 1] +
                    water[cpage][y - 1][x    ] +
                    water[cpage][y - 1][x - 1] +
                    water[cpage][y - 1][x + 1] +
                    water[cpage][y + 1][x    ] +
                    water[cpage][y + 1][x - 1] +
                    water[cpage][y + 1][x + 1] ) >> 2) - water[spage][y][x];
                water[spage][y][x] = nw - (nw >> DENSITY);
                const int16_t dx = water[spage][y][x] - water[spage][y + 1][x];
                const int16_t dy = water[spage][y][x] - water[spage][y][x + 1];
                const int16_t sx = x + (dx >> 3);
                const int16_t sy = y + (dy >> 3);
                vbuff[y][x] = dbuff[sy][sx];
            }
        }
#endif
    }

    void initWater()
    {
#ifdef _USE_ASM
        __asm {
            mov     ax, 10000
            mov     edi, 100 + 60 * IMAGE_WIDTH
            shl     edi, 1
            mov     water[edi], ax
            mov     edi, 120 + 80 * IMAGE_WIDTH
            shl     edi, 1
            mov     water[edi], ax
            mov     edi, 160 + 100 * IMAGE_WIDTH
            shl     edi, 1
            mov     water[edi], ax
            mov     edi, 220 + 60 * IMAGE_WIDTH
            shl     edi, 1
            mov     water[edi], ax
        }
#else
        water[0][ 60][100] = 10000;
        water[0][ 60][220] = 10000;
        water[0][ 80][120] = 10000;
        water[0][100][160] = 10000;
#endif
    }

    void makeWater(int32_t idx)
    {
#ifdef _USE_ASM
        __asm {
            xor     edi, edi
            xor     ebx, ebx
            mov     esi, idx
            and     esi, 0FFh
            shl     esi, 1
            add     di, costab[esi]
            mov     bx, sintab[esi]
            shl     ebx, 6
            add     edi, ebx
            shl     ebx, 2
            add     edi, ebx
            shl     edi, 1
            mov     ax, 500
            mov     water[edi], ax
        }
#else
        const uint16_t i = idx & 0xff;
        const uint16_t nx = costab[i];
        const uint16_t ny = sintab[i];
        water[0][ny][nx] = 500;
#endif
    }

    void run()
    {
        RGB pal[256] = { 0 };
        uint32_t frames = 0;
        uint16_t page = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Water")) return;
        if (!loadPNG(dbuff[0], pal, "assets/sea.png")) return;
        setPalette(pal);
        memcpy(vbuff, dbuff, IMAGE_SIZE);

        initSinCos();
        initWater();
    
        const uint32_t tmstart = getTime();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            makeWater(frames);
            showWater(page);
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            page ^= 1;
            frames++;
            delay(FPS_90);
        }

        messageBox(GFX_INFO, "%.2f frames per second.", (1000.0 * frames) / getElapsedTime(tmstart));
        cleanup();
    }
}

namespace skyEffect {
    typedef struct {
        uint16_t posx, posy;
        uint16_t incx, incy;
        uint8_t sbuff[SIZE_256][SIZE_256];
    } TSkyLayer;

    TSkyLayer   layer[2] = { 0 };

    uint8_t     fromcol, tocol, curcol;
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     dist[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    uint8_t     angle[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    
    void swapWord(uint8_t* a, uint8_t* b)
    {
        *a ^= *b;
        *b ^= *a;
        *a ^= *b;
    }

    void newColor(uint16_t idx, uint16_t x0, uint16_t y0, uint16_t x, uint16_t y, uint16_t x1, uint16_t y1)
    {
        if (layer[idx].sbuff[uint8_t(y)][uint8_t(x)]) return;

        curcol = (x1 - x0) + (y1 - y0);
        curcol = random(curcol << 1) - curcol;
        curcol += (layer[idx].sbuff[uint8_t(y0)][uint8_t(x0)] + layer[idx].sbuff[uint8_t(y1)][uint8_t(x1)] + 1) >> 1;

        if (curcol < fromcol) curcol = fromcol;
        else if (curcol > tocol) curcol = tocol;

        layer[idx].sbuff[uint8_t(y)][uint8_t(x)] = curcol;
    }

    void subDivide(uint16_t idx, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
    {
        if ((x1 - x0) < 2 && (y1 - y0) < 2) return;

        const uint16_t x = (x0 + x1 + 1) >> 1;
        const uint16_t y = (y0 + y1 + 1) >> 1;

        newColor(idx, x0, y0, x, y0, x1, y0);
        newColor(idx, x1, y0, x1, y, x1, y1);
        newColor(idx, x0, y1, x, y1, x1, y1);
        newColor(idx, x0, y0, x0, y, x0, y1);

        curcol = (layer[idx].sbuff[uint8_t(y0)][uint8_t(x0)] + layer[idx].sbuff[uint8_t(y0)][uint8_t(x1)] + layer[idx].sbuff[uint8_t(y1)][uint8_t(x0)] + layer[idx].sbuff[uint8_t(y1)][uint8_t(x1)] + 2) >> 2;
        layer[idx].sbuff[uint8_t(y)][uint8_t(x)] = curcol;

        subDivide(idx, x0, y0, x, y);
        subDivide(idx, x, y0, x1, y);
        subDivide(idx, x, y, x1, y1);
        subDivide(idx, x0, y, x, y1);
    }

    void calcPlasma(uint16_t idx)
    {
        memset(layer[idx].sbuff, 0, SIZE_256 * SIZE_256);
        if (fromcol > tocol) swapWord(&fromcol, &tocol);
        const uint8_t range = tocol - fromcol + 1;
        layer[idx].sbuff[0][0] = fromcol + random(range);
        subDivide(idx, 0, 0, SIZE_256, SIZE_256);
    }

    void calcSky(uint16_t zoom)
    {
        int16_t x = 0, y = 0;

        for (y = 0; y < IMAGE_MIDY; y++) memset(dist[y], int32_t(log(100.0 - y) * (zoom >> 1)), IMAGE_WIDTH);

        for (y = 0; y < IMAGE_MIDY; y++)
        {
            for (x = 0; x < IMAGE_MIDX; x++)
            {
                curcol = zoom * x / (MAX_MIDX - y);
                angle[y][x + IMAGE_MIDX] = curcol + zoom;
                curcol = zoom * (MAX_MIDX - x) / (MAX_MIDX - y);
                angle[y][x] = zoom - curcol - 1;
            }
        }
    }

    void showSkyPixel(uint16_t i, uint16_t x, uint16_t y)
    {
        const uint8_t xm = angle[y][x] + layer[i].posx;
        const uint8_t ym = dist[y][x] + layer[i].posy;
        if (i > 0 && layer[i].sbuff[ym][xm] < 150) showSkyPixel(i - 1, x, y);
        else vbuff[y][x] = layer[i].sbuff[ym][xm];
    }

    void run()
    {
        int16_t x = 0, y = 0;
        RGB pal[256] = { 0 };

        memset(layer, 0, sizeof(layer));
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Sky-Plasma")) return;

        for (x = 1; x < 128; x++)
        {
            pal[x].r = 11 + x / 6;
            pal[x].g = 11 + x / 6;
            pal[x].b = 48 + x / 8;
        }

        for (x = 128; x < 256; x++)
        {
            pal[x].r = x >> 2;
            pal[x].g = x >> 2;
            pal[x].b = 63;
        }

        shiftPalette(pal);
        setPalette(pal);

        for (y = IMAGE_MIDY; y < IMAGE_HEIGHT; y++) memset(vbuff[y], IMAGE_HEIGHT - y, IMAGE_WIDTH);

        fromcol = 1;
        tocol = 255;
        calcSky(128);

        for (x = 0; x < 2; x++)
        {
            layer[x].posx = 0;
            layer[x].posy = 0;
            layer[x].incx = x + 5;
            layer[x].incy = x + 5;
            calcPlasma(x);
        }

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (y = 0; y < IMAGE_MIDY; y++)
            {
                for (x = 0; x < IMAGE_WIDTH; x++) showSkyPixel(1, x, y);
            }

            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_60);

            for (x = 0; x < 2; x++)
            {
                layer[x].posx += layer[x].incx;
                layer[x].posy += layer[x].incy;
            }
        }

        cleanup();
    }
}

namespace phongShader {
    int16_t faces[IMAGE_WIDTH][3] = { 0 };
    int16_t ut[IMAGE_MIDX] = { 0 };
    int16_t vt[IMAGE_MIDX] = { 0 };
    
    int16_t pind[IMAGE_WIDTH] = { 0 };
    int16_t polyz[IMAGE_WIDTH] = { 0 };

    int16_t fx[IMAGE_WIDTH] = { 0 };
    int16_t fy[IMAGE_WIDTH] = { 0 };
    int16_t fz[IMAGE_WIDTH] = { 0 };

    int16_t bx[IMAGE_MIDX] = { 0 }, by[IMAGE_MIDX] = { 0 }, bz[IMAGE_MIDX] = { 0 };
    int16_t px[IMAGE_MIDX] = { 0 }, py[IMAGE_MIDX] = { 0 }, pz[IMAGE_MIDX] = { 0 };
    int16_t nx[IMAGE_MIDX] = { 0 }, ny[IMAGE_MIDX] = { 0 }, nz[IMAGE_MIDX] = { 0 };

    int16_t xt1 = 0, yt1 = 0, zt1 = 0;
    int16_t nvisibles = 0, nverts = 0, nfaces = 0;
    uint8_t alpha = 0, beta = 0, gamma = 0;

    int16_t dist = 150;
    int16_t cost[SIZE_256] = { 0 };
    int16_t sint[SIZE_256] = { 0 };

    int32_t ul[IMAGE_HEIGHT] = { 0 }, ur[IMAGE_HEIGHT] = { 0 };
    int32_t vl[IMAGE_HEIGHT] = { 0 }, vr[IMAGE_HEIGHT] = { 0 };
    int32_t xl[IMAGE_HEIGHT] = { 0 }, xr[IMAGE_HEIGHT] = { 0 };

    uint8_t texture[SIZE_256][SIZE_256] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    int16_t roundf(double x)
    {
        return int16_t(ceil(x));
    }

    void calcVertexNormals()
    {
        for (int16_t i = 0; i < nverts; i++)
        {
            double nf = 0;
            double relx1 = 0;
            double rely1 = 0;
            double relz1 = 0;

            for (int16_t j = 0; j < nfaces; j++)
            {
                if (faces[j][0] == i || faces[j][1] == i || faces[j][2] == i)
                {
                    relx1 += fx[j];
                    rely1 += fy[j];
                    relz1 += fz[j];
                    nf++;
                }
            }

            if (nf)
            {
                relx1 /= nf;
                rely1 /= nf;
                relz1 /= nf;

                const double val = sqrt(sqr(relx1) + sqr(rely1) + sqr(relz1));

                nx[i] = roundf(relx1 / val * 120);
                ny[i] = roundf(rely1 / val * 120);
                nz[i] = roundf(relz1 / val * 120);
            }
        }
    }

    void createTorusData()
    {
        int16_t i = 0, j = 0, n = 0;

        nverts = IMAGE_MIDX;
        nfaces = IMAGE_WIDTH;

        for (i = 0; i < 16; i++)
        {
            const double cx = cos(i / 2.54647909) * 180;
            const double cy = sin(i / 2.54647909) * 180;

            for (j = 0; j < 10; j++)
            {
                px[n] = roundf(cx + cos(j / 1.59154931) * cos(i / 2.54647909) * 90);
                py[n] = roundf(cy + cos(j / 1.59154931) * sin(i / 2.54647909) * 90);
                pz[n] = roundf(sin(j / 1.59154931) * 90);
                n++;
            }
        }

        n = 0;

        for (i = 0; i < 16; i++)
        {
            for (j = 0; j < 10; j++)
            {
                faces[n][2] = i * 10 + j;
                faces[n][1] = i * 10 + (j + 1) % 10;
                faces[n][0] = (i * 10 + j + 10) % IMAGE_MIDX;
                n++;

                faces[n][2] = i * 10 + (j + 1) % 10;
                faces[n][1] = (i * 10 + (j + 1) % 10 + 10) % IMAGE_MIDX;
                faces[n][0] = (i * 10 + j + 10) % IMAGE_MIDX;
                n++;
            }
        }

        for (n = 0; n < IMAGE_WIDTH; n++)
        {
            const int16_t rx1 = px[faces[n][1]] - px[faces[n][0]];
            const int16_t ry1 = py[faces[n][1]] - py[faces[n][0]];
            const int16_t rz1 = pz[faces[n][1]] - pz[faces[n][0]];

            const int16_t rx2 = px[faces[n][2]] - px[faces[n][0]];
            const int16_t ry2 = py[faces[n][2]] - py[faces[n][0]];
            const int16_t rz2 = pz[faces[n][2]] - pz[faces[n][0]];

            fx[n] = ry1 * rz2 - ry2 * rz1;
            fy[n] = rz1 * rx2 - rz2 * rx1;
            fz[n] = rx1 * ry2 - rx2 * ry1;
        }
    }

    void initialize()
    {
        int16_t i = 0, j = 0;
        RGB pal[256] = { 0 };

        createTorusData();
        calcVertexNormals();

        for (i = 0; i < 256; i++)
        {
            cost[i] = roundf(cos(i / 40.58570747) * 128);
            sint[i] = roundf(sin(i / 40.58570747) * 128);
        }

        for (i = 1; i < 64; i++)
        {
            pal[i].r = uint8_t(i);
            pal[i].g = uint8_t(10 + i / 1.4);
            pal[i].b = uint8_t(20 + i / 1.6);
        }

        shiftPalette(pal);
        setPalette(pal);

        for (i = 0; i < 256; i++)
        {
            for (j = 0; j < 256; j++) texture[i][j] = uint8_t(roundf(sqr(sqr(sin(i / 81.487))) * sqr(sqr(sin(j / 81.487))) * 62 + 1));
        }
    }

    void quickSort(int16_t left, int16_t right)
    {
        int16_t tmp = 0;
        int16_t lo = left;
        int16_t hi = right;
        const int16_t mid = polyz[(lo + hi) >> 1];

        do {
            while (polyz[lo] > mid) lo++;
            while (mid > polyz[hi]) hi--;

            if (lo <= hi)
            {
                tmp = polyz[lo];
                polyz[lo] = polyz[hi];
                polyz[hi] = tmp;

                tmp = pind[lo];
                pind[lo] = pind[hi];
                pind[hi] = tmp;

                lo++;
                hi--;
            }
        } while (lo <= hi);

        if (left < hi) quickSort(left, hi);
        if (lo < right) quickSort(lo, right);
    }

    void newTexture(int16_t x1, int16_t y1, int16_t u1, int16_t v1, int16_t x2, int16_t y2, int16_t u2, int16_t v2, int16_t x3, int16_t y3, int16_t u3, int16_t v3)
    {
        int16_t osx = 0, osy = 0;
        int16_t xofs[IMAGE_WIDTH] = { 0 };
        int16_t yofs[IMAGE_WIDTH] = { 0 };

        uint16_t u = 0, v = 0, i = 0, j = 0, k = 0;

        int16_t du21 = 0, du31 = 0, du32 = 0;
        int16_t dx21 = 0, dx31 = 0, dx32 = 0;
        int16_t dv21 = 0, dv31 = 0, dv32 = 0;
        int16_t dy21 = 0, dy31 = 0, dy32 = 0;

        for (i = 0; i < 2; i++)
        {
            if (y3 < y2)
            {
                j = y3; y3 = y2; y2 = j;
                j = x3; x3 = x2; x2 = j;
                j = u3; u3 = u2; u2 = j;
                j = v3; v3 = v2; v2 = j;
            }

            if (y2 < y1)
            {
                j = y1; y1 = y2; y2 = j;
                j = x1; x1 = x2; x2 = j;
                j = u1; u1 = u2; u2 = j;
                j = v1; v1 = v2; v2 = j;
            }

            if (y3 < y1)
            {
                j = y1; y1 = y3; y3 = j;
                j = x1; x1 = x3; x3 = j;
                j = u1; u1 = u3; u3 = j;
                j = v1; v1 = v3; v3 = j;
            }
        }

        if (y1 == y2 && x1 > x2)
        {
            j = x1; x1 = x2; x2 = j;
            j = u1; u1 = u2; u2 = j;
            j = v1; v1 = v2; v2 = j;
        }

        dy21 = y2 - y1; dy31 = y3 - y1; dy32 = y3 - y2;
        dx21 = x2 - x1; dx31 = x3 - x1; dx32 = x3 - x2;
        du21 = u2 - u1; du31 = u3 - u1; du32 = u3 - u2;
        dv21 = v2 - v1; dv31 = v3 - v1; dv32 = v3 - v2;

        xl[0] = x1; xl[0] <<= 8;
        ul[0] = u1;	ul[0] <<= 8;
        vl[0] = v1; vl[0] <<= 8;

        if (y1 == y2)
        {
            xr[0] = x2; xr[0] <<= 8;
            ur[0] = u2; ur[0] <<= 8;
            vr[0] = v2; vr[0] <<= 8;
        }
        else
        {
            xr[0] = xl[0];
            ur[0] = ul[0];
            vr[0] = vl[0];
        }

        for (i = y1 + 1; i <= y2; i++)
        {
            xl[i - y1] = xl[i - y1 - 1] + (dx31 << 8) / dy31;
            xr[i - y1] = xr[i - y1 - 1] + (dx21 << 8) / dy21;
            ul[i - y1] = ul[i - y1 - 1] + (du31 << 8) / dy31;
            ur[i - y1] = ur[i - y1 - 1] + (du21 << 8) / dy21;
            vl[i - y1] = vl[i - y1 - 1] + (dv31 << 8) / dy31;
            vr[i - y1] = vr[i - y1 - 1] + (dv21 << 8) / dy21;
        }

        for (i = y2 + 1; i <= y3; i++)
        {
            xl[i - y1] = xl[i - y1 - 1] + (dx31 << 8) / dy31;
            xr[i - y1] = xr[i - y1 - 1] + (dx32 << 8) / dy32;
            ul[i - y1] = ul[i - y1 - 1] + (du31 << 8) / dy31;
            ur[i - y1] = ur[i - y1 - 1] + (du32 << 8) / dy32;
            vl[i - y1] = vl[i - y1 - 1] + (dv31 << 8) / dy31;
            vr[i - y1] = vr[i - y1 - 1] + (dv32 << 8) / dy32;
        }

        k = y2 - y1;

        if (xl[k] < xr[k])
        {
            u = ul[k];
            v = vl[k];
            osx = u >> 8;
            osy = v >> 8;

            for (i = 0; i <= (xr[k] >> 8) - (xl[k] >> 8); i++)
            {
                xofs[i] = (u >> 8) - osx;
                yofs[i] = (v >> 8) - osy;
                u += ((ur[k] - ul[k]) << 8) / (xr[k] - xl[k] + 1);
                v += ((vr[k] - vl[k]) << 8) / (xr[k] - xl[k] + 1);
            }
        }
        else
        {
            u = ur[k];
            v = vr[k];
            osx = u >> 8;
            osy = v >> 8;

            for (i = 0; i <= (xl[k] >> 8) - (xr[k] >> 8); i++)
            {
                xofs[i] = (u >> 8) - osx;
                yofs[i] = (v >> 8) - osy;
                u += ((ul[k] - ur[k]) << 8) / (xl[k] - xr[k] + 1);
                v += ((vl[k] - vr[k]) << 8) / (xl[k] - xr[k] + 1);
            }
        }

        if (xl[k] < xr[k])
        {
            for (i = 0; i <= y3 - y1; i++)
            {
                osx = ul[i] >> 8;
                osy = vl[i] >> 8;
                for (j = xl[i] >> 8; j <= xr[i] >> 8; j++)
                {
                    u = (osx + xofs[j - (xl[i] >> 8)]) & 0xff;
                    v = (osy + yofs[j - (xl[i] >> 8)]) & 0xff;
                    vbuff[i + y1][j] = texture[v][u];
                }
            }
        }
        else
        {
            for (i = 0; i <= y3 - y1; i++)
            {
                osx = ur[i] >> 8;
                osy = vr[i] >> 8;
                for (j = xr[i] >> 8; j <= xl[i] >> 8; j++)
                {
                    u = (osx + xofs[j - (xr[i] >> 8)]) & 0xff;
                    v = (osy + yofs[j - (xr[i] >> 8)]) & 0xff;
                    vbuff[i + y1][j] = texture[v][u];
                }
            }
        }
    }

    void rotate(int16_t* px, int16_t* py, int16_t* pz)
    {
        const int16_t y = (cost[alpha] * (*py) - sint[alpha] * (*pz)) >> 7;
        const int16_t z = (sint[alpha] * (*py) + cost[alpha] * (*pz)) >> 7;
        const int16_t x = (cost[beta ] * (*px) + sint[beta ] *    z ) >> 7;

        *pz = (cost[beta ] * z - sint[beta ] * (*px)) >> 7;
        *px = (cost[gamma] * x - sint[gamma] *    y ) >> 7;
        *py = (sint[gamma] * x + cost[gamma] *    y ) >> 7;
    }

    void run()
    {
        int16_t i = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Phong-Shader")) return;
        initialize();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (i = 0; i < nverts; i++)
            {
                xt1 = px[i];
                yt1 = py[i];
                zt1 = pz[i];

                rotate(&xt1, &yt1, &zt1);
                zt1 += 468;

                bx[i] = IMAGE_MIDX + xt1 * dist / zt1;
                by[i] = IMAGE_MIDY + yt1 * dist * 83 / 100 / zt1;
                bz[i] = zt1;

                xt1 = nx[i];
                yt1 = ny[i];
                zt1 = nz[i];

                rotate(&xt1, &yt1, &zt1);
                ut[i] = 128 + xt1;
                vt[i] = 128 + yt1;
            }

            nvisibles = 0;

            for (i = 0; i < nfaces; i++)
            {
                if ((bx[faces[i][2]] - bx[faces[i][0]]) * (by[faces[i][1]] - by[faces[i][0]]) - (bx[faces[i][1]] - bx[faces[i][0]]) * (by[faces[i][2]] - by[faces[i][0]]) > 0)
                {
                    pind[nvisibles] = i;
                    polyz[nvisibles] = bz[faces[i][0]] + bz[faces[i][1]] + bz[faces[i][2]];
                    nvisibles++;
                }
            }

            quickSort(0, nvisibles - 1);

            for (i = 0; i < nvisibles; i++) newTexture(
                bx[faces[pind[i]][0]], by[faces[pind[i]][0]],
                ut[faces[pind[i]][0]], vt[faces[pind[i]][0]],
                bx[faces[pind[i]][1]], by[faces[pind[i]][1]],
                ut[faces[pind[i]][1]], vt[faces[pind[i]][1]],
                bx[faces[pind[i]][2]], by[faces[pind[i]][2]],
                ut[faces[pind[i]][2]], vt[faces[pind[i]][2]]);

            alpha = (alpha + 2) % SIZE_256;
            beta = (beta + 255) % SIZE_256;
            gamma = (gamma + 1) % SIZE_256;

            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            memset(vbuff, 0, IMAGE_SIZE);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace plasmaTexture {
    #define LCOS 0.9993908
    #define LSIN 0.0348994

    const int16_t verts[][3] = {
        {-40,  40, -40}, {40,  40, -40}, {40,  40, 40}, {-40,  40, 40},
        {-40, -40, -40}, {40, -40, -40}, {40, -40, 40}, {-40, -40, 40}
    };

    const int16_t face[][4] = {
        {0, 3, 7, 4}, {3, 2, 6, 7}, {2, 1, 5, 6},
        {1, 0, 4, 5}, {0, 1, 2, 3}, {7, 6, 5, 4}
    };

    typedef struct {
        double x, y, z;
    } TVertex;

    typedef struct {
        int16_t x, y, z;
    } TPoint;

    TVertex     vertices[8] = { 0 };
    TPoint      points[8] = { 0 };

    uint8_t     zoom = 0;
    uint16_t    u, v, us, vs;
    int16_t     xofs, yofs, zofs;
    int16_t     faces[6][5] = { 0 };
    uint16_t    lookup[360] = { 0 };
    int16_t     polya[IMAGE_HEIGHT][2] = { 0 };
    int16_t     polyb[IMAGE_HEIGHT][2] = { 0 };
    int16_t     polyc[IMAGE_HEIGHT][2] = { 0 };
       
    uint8_t     texture[SIZE_128][SIZE_128] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void initData()
    {
        int16_t i = 0;

        xofs = IMAGE_MIDX;
        yofs = IMAGE_MIDY;
        zofs = -890;

        us = 40;
        vs = 50;
        zoom = 1;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Plasma-Mapping")) return;

        for (i = 0; i < 360; i++) lookup[i] = uint16_t(cos(i * M_PI / 180) * 128);

        for (i = 0; i < 8; i++)
        {
            vertices[i].x = verts[i][0];
            vertices[i].y = verts[i][1];
            vertices[i].z = verts[i][2];
        }

        for (i = 0; i < 6; i++)
        {
            faces[i][0] = face[i][0];
            faces[i][1] = face[i][1];
            faces[i][2] = face[i][2];
            faces[i][3] = face[i][3];
        }
    }

    void quickSort(int16_t left, int16_t right)
    {
        int16_t tmp[5] = { 0 };
        int16_t lo = left;
        int16_t hi = right;
        const int16_t mid = faces[(lo + hi) >> 1][4];

        do {
            while (faces[lo][4] > mid) lo++;
            while (mid > faces[hi][4]) hi--;
        
            if (lo <= hi)
            {
                memcpy(tmp, faces[lo], sizeof(faces[lo]));
                memcpy(faces[lo], faces[hi], sizeof(faces[hi]));
                memcpy(faces[hi], tmp, sizeof(tmp));
                lo++;
                hi--;
            }
        } while (lo <= hi);
    
        if (left < hi) quickSort(left, hi);
        if (lo < right) quickSort(lo, right);
    }

    void rotate()
    {
        int16_t i = 0;
        TVertex tmp = { 0 };

        for (i = 0; i < 8; i++)
        {
            tmp.y = LCOS * vertices[i].y - LSIN * vertices[i].z;
            tmp.z = LSIN * vertices[i].y + LCOS * vertices[i].z;

            vertices[i].y = tmp.y;
            vertices[i].z = tmp.z;

            tmp.x = LCOS * vertices[i].x - LSIN * vertices[i].y;
            tmp.y = LSIN * vertices[i].x + LCOS * vertices[i].y;

            vertices[i].x = tmp.x;
            vertices[i].y = tmp.y;

            points[i].x = int16_t((vertices[i].x * 256) / (vertices[i].z - zofs) + xofs);
            points[i].y = int16_t((vertices[i].y * 256) / (vertices[i].z - zofs) + yofs);
            points[i].z = int16_t(vertices[i].z);
        }

        for (i = 0; i < 6; i++) faces[i][4] = points[faces[i][0]].z + points[faces[i][1]].z + points[faces[i][2]].z + points[faces[i][3]].z;

        quickSort(0, 5);
    }

    void updatePlasma()
    {
        int16_t i = 0, j = 0;

        u = us;
        for (i = 0; i < 128; i++)
        {
            v = vs;
            for (j = 0; j < 128; j++)
            {
                uint16_t tdata = (lookup[i] + lookup[u] + lookup[j] + lookup[v]) << 8;
                texture[i][j] = (tdata >> 10) + 128;
                v = (v + 4) % 360;
            }
            u = (u + 3) % 360;
        }
    }

    void doSide(int16_t x1, int16_t y1, int16_t u1, int16_t v1, int16_t x2, int16_t y2, int16_t u2, int16_t v2)
    {
        if (y1 == y2) return;

        if (y2 < y1)
        {
            int16_t tmp = 0;
            tmp = y2; y2 = y1; y1 = tmp;
            tmp = x2; x2 = x1; x1 = tmp;
            tmp = u2; u2 = u1; u1 = tmp;
            tmp = v2; v2 = v1; v1 = tmp;
        }

        int16_t x = x1 << 7;
        int16_t u = u1 << 7;
        int16_t v = v1 << 7;

        const int16_t xinc = ((x2 - x1) << 7) / (y2 - y1);
        const int16_t uinc = ((u2 - u1) << 7) / (y2 - y1);
        const int16_t vinc = ((v2 - v1) << 7) / (y2 - y1);

        for (int16_t y = y1; y <= y2; y++)
        {
            if (y >= 0 && y <= MAX_HEIGHT)
            {
                const int16_t xdec = x >> 7;
                if (xdec < polya[y][0])
                {
                    polya[y][0] = xdec;
                    polyb[y][0] = u;
                    polyc[y][0] = v;
                }

                if (xdec > polya[y][1])
                {
                    polya[y][1] = xdec;
                    polyb[y][1] = u;
                    polyc[y][1] = v;
                }
            }

            x += xinc;
            u += uinc;
            v += vinc;
        }
    }

    void plasmaMapPoly(int16_t x1, int16_t y1, int16_t u1, int16_t v1, int16_t x2, int16_t y2, int16_t u2, int16_t v2, int16_t x3, int16_t y3, int16_t u3, int16_t v3, int16_t x4, int16_t y4, int16_t u4, int16_t v4)
    {
        int16_t x = 0, y = 0;

        for (y = 0; y < IMAGE_HEIGHT; y++)
        {
            polya[y][0] = 21474;
            polya[y][1] = -21474;
            polyb[y][0] = 21474;
            polyb[y][1] = -21474;
            polyc[y][0] = 21474;
            polyc[y][1] = -21474;
        }

        int16_t miny = y1;
        int16_t maxy = y1;

        if (y2 < miny) miny = y2;
        if (y3 < miny) miny = y3;
        if (y4 < miny) miny = y4;
        if (y2 > maxy) maxy = y2;
        if (y3 > maxy) maxy = y3;
        if (y4 > maxy) maxy = y4;

        if (miny < 0)  miny = 0;
        if (maxy > MAX_HEIGHT) maxy = MAX_HEIGHT;
        if (miny > MAX_HEIGHT || maxy < 0) return;

        doSide(x1, y1, u1, v1, x2, y2, u2, v2);
        doSide(x2, y2, u2, v2, x3, y3, u3, v3);
        doSide(x3, y3, u3, v3, x4, y4, u4, v4);
        doSide(x4, y4, u4, v4, x1, y1, u1, v1);

        for (y = miny; y <= maxy; y++)
        {
            int16_t uval = polyb[y][0];
            const int16_t ui = (polya[y][0] != polya[y][1]) ? (polyb[y][1] - polyb[y][0]) / (polya[y][1] - polya[y][0]) : 0;

            int16_t vval = polyc[y][0];
            const int16_t vi = (polya[y][0] != polya[y][1]) ? (polyc[y][1] - polyc[y][0]) / (polya[y][1] - polya[y][0]) : 0;

            if (polya[y][0] < 0) polya[y][0] = 0;
            if (polya[y][1] > MAX_WIDTH) polya[y][1] = MAX_WIDTH;

            for (x = polya[y][0]; x <= polya[y][1]; x++)
            {
                vbuff[y][x] = texture[uval >> 7][vval >> 7];
                uval += ui;
                vval += vi;
            }
        }
    }

    void drawCube()
    {
        for (int16_t i = 0; i < 6; i++)
        {
            if ((points[faces[i][1]].x - points[faces[i][0]].x) *
                (points[faces[i][0]].y - points[faces[i][2]].y) -
                (points[faces[i][1]].y - points[faces[i][0]].y) *
                (points[faces[i][0]].x - points[faces[i][2]].x) < 0)

                plasmaMapPoly(
                    points[faces[i][0]].x, points[faces[i][0]].y, 0, 0,
                    points[faces[i][1]].x, points[faces[i][1]].y, 127, 0,
                    points[faces[i][2]].x, points[faces[i][2]].y, 127, 127,
                    points[faces[i][3]].x, points[faces[i][3]].y, 0, 127);
        }
    }

    void run()
    {
        RGB pal[256] = { 0 };

        initData();

        for (int16_t i = 0; i < 64; i++)
        {
            pal[i + 128].r = 0;
            pal[i + 128].g = uint8_t(i);
            pal[i + 128].b = 63;
            pal[i + 192].r = uint8_t(i);
            pal[i + 192].g = 63;
            pal[i + 192].b = 63;
        }

        shiftPalette(pal);
        setPalette(pal);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            rotate();

            vs = (vs + 4) % 360;
            us = (us + 3) % 360;

            if (zoom)
            {
                zofs += 15;
                if (zofs > -190) zoom = !zoom;
            }
            else
            {
                zofs -= 15;
                if (zofs < -890) zoom = !zoom;
            }

            updatePlasma();
            memset(vbuff, 0, IMAGE_SIZE);
            drawCube();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace fireDown {
    typedef struct {
        int16_t cx, cy;
        int16_t dx, dy;
        int16_t life;
    } TPart;

    TPart particles[1024] = { 0 };

    int16_t frames = 0;
    int16_t sinTab[256] = { 0 };

    RGB pal[256] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    int16_t roundf(double x)
    {
        if (x >= 0.0) return int16_t(x + 0.5);
        return int16_t(x - 0.5);
    }

    void blurBuffer()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            mov     ecx, IMAGE_SIZE - 2 * IMAGE_WIDTH
            xor     bx, bx
        again:
            xor     ax, ax
            mov     bl, [edi]
            add     ax, bx
            mov     bl, [edi + MAX_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH + 1]
            add     ax, bx
            shr     ax, 2
            and     al, al
            jz      skip
            dec     al
        skip:
            stosb
            loop   again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++)
            {
                uint8_t col = (vbuff[i - 1][j] + vbuff[i][j - 1] + vbuff[i][j] + vbuff[i][j + 1]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    int16_t range(int16_t b, int16_t min, int16_t max)
    {
        if (b > max) return max;
        if (b < min) return min;
        return b;
    }

    int16_t pike(int16_t x, int16_t arg, int16_t a, int16_t b, int16_t c)
    {
        if (x < arg) return a + (b - a) * x / arg;
        return b + (c - b) * (x - arg) / (256 - arg);
    }

    void setPalette(uint8_t brightness)
    {
        for (int16_t i = 0; i < 256; i++)
        {
            int16_t val = 127 - pal[i].r;
            val = ((val << 8) * val) << 2;
            int16_t col = (val & 0xFF00) | pal[i].r;
            val = ((val & 0xFF00) + col) * brightness;
            pal[i].r = val >> 8;

            val = 127 - pal[i].g;
            val = ((val << 8) * val) << 2;
            col = (val & 0xFF00) | pal[i].g;
            val = ((val & 0xFF00) + col) * brightness;
            pal[i].g = val >> 8;

            val = 127 - pal[i].b;
            val = ((val << 8) * val) << 2;
            col = (val & 0xFF00) | pal[i].b;
            val = ((val & 0xFF00) + col) * brightness;
            pal[i].b = val >> 8;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void showFire()
    {
        int16_t i = 0;

        for (i = 0; i < 256; i++)
        {
            pal[i].r = uint8_t(range(i, 0, 128));
            pal[i].g = uint8_t(range(i - 48, 0, 120));
            pal[i].b = pike(i, 32, 0, 32, 0) + pike(i, 128, 0, 0, 200);
        }
        
        frames = 0;
        setPalette(120);
        int16_t cnt = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            if (frames > 800) frames = 0;

            const int16_t x = IMAGE_MIDX + sinTab[(frames / 3) & 0xff];
            const int16_t y = 80 - (sinTab[(frames + 60) & 0xff] >> 1);

            for (i = 0; i < 4; i++)
            {
                particles[cnt].cx = (x << 5) + random(100);
                particles[cnt].cy = (y << 5) + random(100);
                particles[cnt].dx = -16383 + random(32767);
                particles[cnt].dy = -16383 + random(32767);
                particles[cnt].life = 10 + random(400);
                cnt++;
            }

            i = 0;

            do {
                particles[i].life--;

                if (particles[i].life <= 0)
                {
                    particles[i] = particles[cnt - 1];
                    cnt--;
                }
                else
                {
                    vbuff[particles[i].cy >> 5][particles[i].cx >> 5] = 64 + (particles[i].life >> 1);

                    particles[i].cx += (particles[i].dx >> 8);
                    particles[i].cy += (particles[i].dy >> 8);
                    particles[i].dy += 256;
                    particles[i].dx -= (particles[i].dx >> 6);
                    particles[i].dy -= (particles[i].dy >> 6);

                    if (particles[i].cx < 200 || particles[i].cx > 10100) particles[i].dx = -particles[i].dx;
                    if (particles[i].cy > 6200) particles[i].dy = -particles[i].dy;
                }

                i++;
            } while (i < cnt);

            blurBuffer();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            frames++;
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire-Blur")) return;
        for (int16_t i = 0; i < 256; i++) sinTab[i] = roundf(sin(i * M_PI / 128) * 64);
        showFire();
        cleanup();
    }
}

namespace fireTexture {
    #define LCOS 0.9993908
    #define LSIN 0.0348994

    const int16_t verts[][3] = {
        {-50,  50, -50}, {50,  50, -50}, {50,  50, 50}, {-50,  50, 50},
        {-50, -50, -50}, {50, -50, -50}, {50, -50, 50}, {-50, -50, 50}
    };

    const int16_t faces[][4] = {
        {0, 3, 7, 4}, {3, 2, 6, 7}, {2, 1, 5, 6},
        {1, 0, 4, 5}, {0, 1, 2, 3}, {7, 6, 5, 4}
    };

    typedef struct {
        double x, y, z;
    } TVertex;

    typedef struct {
        int16_t x, y, z;
    } TPoint;

    RGB pal[256] = { 0 };
    TVertex verties[8] = { 0 };
    TPoint points[8] = { 0 };

    int16_t face[6][5] = { 0 };
    int16_t xofs, yofs, zofs;

    int16_t poly1[IMAGE_HEIGHT][2] = { 0 };
    int16_t poly2[IMAGE_HEIGHT][2] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void HSI2RGB(double H, double S, double I, RGB* rgb)
    {
        const double rv = 1 + S * sin(H - 2 * M_PI / 3);
        const double gv = 1 + S * sin(H);
        const double bv = 1 + S * sin(H + 2 * M_PI / 3);
        const double t = 63.999 * I / 2;

        rgb->r = uint8_t(rv * t);
        rgb->g = uint8_t(gv * t);
        rgb->b = uint8_t(bv * t);
    }

    void makePalette()
    {
        int16_t i = 0;

        for (i = 1; i <= 80; i++) HSI2RGB(4.6 - 1.5 * i / 80.0, i / 80.0, i / 80.0, &pal[i]);

        for (i = 81; i < 256; i++)
        {
            pal[i] = pal[i - 1];

            if (pal[i].r < 63) pal[i].r++;
            if (pal[i].r < 63) pal[i].r++;

            if (!(i % 2) && pal[i].g < 63) pal[i].g++;
            if (!(i % 2) && pal[i].b < 63) pal[i].b++;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void initialize()
    {
        int16_t k = 0;

        xofs = 160;
        yofs = 120;
        zofs = -400;

        for (k = 0; k < 8; k++)
        {
            verties[k].x = verts[k][0];
            verties[k].y = verts[k][1];
            verties[k].z = verts[k][2];
        }

        for (k = 0; k < 6; k++)
        {
            face[k][0] = faces[k][0];
            face[k][1] = faces[k][1];
            face[k][2] = faces[k][2];
            face[k][3] = faces[k][3];
        }
    }

    void quickSort(int16_t left, int16_t right)
    {
        int16_t tmp[5] = { 0 };
        int16_t lo = left;
        int16_t hi = right;
        const int16_t mid = face[(lo + hi) >> 1][4];

        do {
            while (face[lo][4] > mid) lo++;
            while (mid > face[hi][4]) hi--;

            if (lo <= hi)
            {
                memcpy(tmp, face[lo], sizeof(face[lo]));
                memcpy(face[lo], face[hi], sizeof(face[hi]));
                memcpy(face[hi], tmp, sizeof(tmp));

                lo++;
                hi--;
            }
        } while (lo <= hi);

        if (left < hi) quickSort(left, hi);
        if (lo < right) quickSort(lo, right);
    }

    void rotate()
    {
        int16_t k = 0;
        TVertex temp = { 0 };

        for (k = 0; k < 8; k++)
        {
            temp.y = LCOS * verties[k].y - LSIN * verties[k].z;
            temp.z = LSIN * verties[k].y + LCOS * verties[k].z;

            verties[k].y = temp.y;
            verties[k].z = temp.z;

            temp.x = LCOS * verties[k].x - LSIN * verties[k].y;
            temp.y = LSIN * verties[k].x + LCOS * verties[k].y;

            verties[k].x = temp.x;
            verties[k].y = temp.y;

            points[k].x = int16_t((verties[k].x * 256) / (verties[k].z - zofs) + xofs);
            points[k].y = int16_t((verties[k].y * 256) / (verties[k].z - zofs) + yofs);
            points[k].z = int16_t(verties[k].z + 128);
        }

        for (k = 0; k < 6; k++)
        {
            face[k][4] = points[face[k][0]].z + points[face[k][1]].z + points[face[k][2]].z + points[face[k][3]].z;
        }

        quickSort(0, 5);
    }

    void doSide(int16_t x1, int16_t y1, int16_t z1, int16_t x2, int16_t y2, int16_t z2)
    {
        if (y1 == y2) return;

        if (y2 < y1)
        {
            int16_t tmp = 0;
            tmp = y2; y2 = y1; y1 = tmp;
            tmp = x2; x2 = x1; x1 = tmp;
            tmp = z2; z2 = z1; z1 = tmp;
        }

        int16_t x = x1 << 7;
        int16_t z = z1 << 7;

        const int16_t xinc = ((x2 - x1) << 7) / (y2 - y1);
        const int16_t zinc = ((z2 - z1) << 7) / (y2 - y1);

        for (int16_t y = y1; y <= y2; y++)
        {
            if (y >= 0 && y <= MAX_HEIGHT)
            {
                const int16_t xdec = x >> 7;
                if (xdec < poly1[y][0]) { poly1[y][0] = xdec; poly2[y][0] = z; }
                if (xdec > poly1[y][1]) { poly1[y][1] = xdec; poly2[y][1] = z; }
            }

            x += xinc;
            z += zinc;
        }
    }

    void gouraudPoly(int16_t x1, int16_t y1, int16_t z1, int16_t x2, int16_t y2, int16_t z2, int16_t x3, int16_t y3, int16_t z3, int16_t x4, int16_t y4, int16_t z4)
    {
        for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
        {
            poly1[y][0] = 500; poly1[y][1] = -500;
            poly2[y][0] = 500; poly2[y][1] = -500;
        }

        uint16_t miny = y1;
        uint16_t maxy = y1;

        if (y2 < miny) miny = y2; if (y3 < miny) miny = y3;
        if (y4 < miny) miny = y4; if (y2 > maxy) maxy = y2;
        if (y3 > maxy) maxy = y3; if (y4 > maxy) maxy = y4;

        if (miny < 0) miny = 0;
        if (maxy > MAX_HEIGHT) maxy = MAX_HEIGHT;
        if (miny > MAX_HEIGHT || maxy < 0) return;

        doSide(x1, y1, z1, x2, y2, z2); doSide(x2, y2, z2, x3, y3, z3);
        doSide(x3, y3, z3, x4, y4, z4); doSide(x4, y4, z4, x1, y1, z1);

        for (int16_t y = miny; y < maxy; y++)
        {
            if (poly1[y][0] < 0) poly1[y][0] = 0;
            if (poly1[y][1] > MAX_WIDTH) poly1[y][1] = MAX_WIDTH;

            uint16_t cinc = 0;
            uint16_t col = poly2[y][0];

            if (poly1[y][0] != poly1[y][1]) cinc = (poly2[y][1] - poly2[y][0]) / (poly1[y][1] - poly1[y][0]);
            else cinc = 0;

            for (int16_t x = poly1[y][0]; x < poly1[y][1]; x++)
            {
                vbuff[y][x] = col >> 7;
                col += cinc;
            }
        }
    }

    void drawCube()
    {
        for (int16_t k = 0; k < 6; k++)
        {
            if (((points[face[k][1]].x - points[face[k][0]].x) *
                (points[face[k][0]].y - points[face[k][2]].y) -
                (points[face[k][1]].y - points[face[k][0]].y) *
                (points[face[k][0]].x - points[face[k][2]].x)) < 0)

                gouraudPoly(
                    points[face[k][0]].x, points[face[k][0]].y, points[face[k][0]].z,
                    points[face[k][1]].x, points[face[k][1]].y, points[face[k][1]].z,
                    points[face[k][2]].x, points[face[k][2]].y, points[face[k][2]].z,
                    points[face[k][3]].x, points[face[k][3]].y, points[face[k][3]].z
                );
        }
    }

    void motionBlur()
    {
#ifdef _USE_ASM
    __asm {
            lea     edi, vbuff
            mov     esi, edi
            add     edi, IMAGE_WIDTH
            add     esi, IMAGE_SIZE - IMAGE_WIDTH
        again:
            xor     ah, ah
            mov     al, [edi - 1]
            mov     dx, ax
            mov     al, [edi + 1]
            add     dx, ax
            mov     al, [edi - IMAGE_WIDTH]
            add     dx, ax
            mov     al, [edi + IMAGE_WIDTH]
            add     dx, ax
            shr     dx, 2
            or      dl, dl
            jz    	skip
            dec     dl
        skip:
            mov     [edi - IMAGE_WIDTH], dl
            inc     edi
            cmp     edi, esi
            jnz     again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++)
            {
                uint8_t col = (vbuff[i][j - 1] + vbuff[i][j + 1] + vbuff[i - 1][j] + vbuff[i + 1][j]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire-Mapping")) return;
        initialize();
        makePalette();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            rotate();
            motionBlur();
            drawCube();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace fireTexture2 {
    #define MPOS	200
    #define DIVD	128
    #define FDST	150
    #define XSTR	1
    #define YSTR	2
    #define ZSTR	-1

    #define SIN(i)		(stab[i & 0xff])
    #define COS(i)		(stab[(i + 192) & 0xff])

    const int16_t points[][3] = {
        {-32, -32, -32}, {-32, -32, 32}, {32, -32, 32}, {32, -32, -32},
        {-32,  32, -32}, {-32,  32, 32}, {32,  32, 32}, {32,  32, -32}
    };

    const uint8_t planes[][4] = {
        {0, 4, 5, 1}, {0, 3, 7, 4}, {0, 1, 2, 3},
        {4, 5, 6, 7}, {7, 6, 2, 3}, {1, 2, 6, 5}
    };

    int16_t stab[256] = { 0 };
    int16_t polyz[6] = { 0 };
    uint8_t pind[6] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void horzLine(int16_t x1, int16_t x2, int16_t y, uint8_t col)
    {
        if (x2 < x1) { x1 += x2; x2 = x1 - x2; x1 -= x2; }
        if (x2 < 0 || x1 > MAX_WIDTH || y < 0 || y > MAX_HEIGHT) return;
        if (x1 < 0) x1 = 0;
        if (x2 > MAX_WIDTH) x2 = MAX_WIDTH;
        for (int16_t x = x1; x <= x2; x++) vbuff[y][x] = col;
    }

    void linePolygon(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, uint8_t col)
    {
        int16_t xpos[MPOS][2] = { 0 };

        int16_t mny = y1;
        if (y2 < mny) mny = y2;
        if (y3 < mny) mny = y3;
        if (y4 < mny) mny = y4;

        int16_t mxy = y1;
        if (y2 > mxy) mxy = y2;
        if (y3 > mxy) mxy = y3;
        if (y4 > mxy) mxy = y4;

        const int16_t s1 = ((y1 < y2) << 1) - 1;
        const int16_t s2 = ((y2 < y3) << 1) - 1;
        const int16_t s3 = ((y3 < y4) << 1) - 1;
        const int16_t s4 = ((y4 < y1) << 1) - 1;

        int16_t y = y1;
        if (y1 != y2)
        {
            do
            {
                xpos[y][y1 < y2] = (x2 - x1) * (y - y1) / (y2 - y1) + x1;
                y += s1;
            } while (y != (y2 + s1));
        }
        else xpos[y][y1 < y2] = x1;

        y = y2;
        if (y2 != y3)
        {
            do
            {
                xpos[y][y2 < y3] = (x3 - x2) * (y - y2) / (y3 - y2) + x2;
                y += s2;
            } while (y != (y3 + s2));
        }
        else xpos[y][y2 < y3] = x2;

        y = y3;
        if (y3 != y4)
        {
            do
            {
                xpos[y][y3 < y4] = (x4 - x3) * (y - y3) / (y4 - y3) + x3;
                y += s3;
            } while (y != (y4 + s3));
        }
        else xpos[y][y3 < y4] = x3;

        y = y4;
        if (y4 != y1)
        {
            do
            {
                xpos[y][y4 < y1] = (x1 - x4) * (y - y4) / (y1 - y4) + x4;
                y += s4;
            } while (y != (y1 + s4));
        }
        else xpos[y][y1 < y4] = x4;

        for (y = mny; y < mxy; y++) horzLine(xpos[y][0], xpos[y][1], y, col);
    }

    void motionBlur()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            mov     esi, edi
            add     edi, IMAGE_WIDTH
            add     esi, IMAGE_SIZE - IMAGE_WIDTH
        again:
            xor     ah, ah
            mov     al, [edi - 1]
            mov     dx, ax
            mov     al, [edi + 1]
            add     dx, ax
            mov     al, [edi - IMAGE_WIDTH]
            add     dx, ax
            mov     al, [edi + IMAGE_WIDTH]
            add     dx, ax
            shr     dx, 2
            or      dl, dl
            jz    	skip
            dec     dl
        skip :
            mov     [edi - IMAGE_WIDTH], dl
            inc     edi
            cmp     edi, esi
            jnz     again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++)
            {
                uint8_t col = (vbuff[i][j - 1] + vbuff[i][j + 1] + vbuff[i - 1][j] + vbuff[i + 1][j]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void quickSort(int16_t left, int16_t right)
    {
        int16_t lo = left;
        int16_t hi = right;
        const int16_t mid = polyz[(lo + hi) >> 1];

        do {
            while (polyz[lo] < mid) lo++;
            while (mid < polyz[hi]) hi--;

            if (lo <= hi)
            {
                const int16_t ta = polyz[lo];
                polyz[lo] = polyz[hi];
                polyz[hi] = ta;

                const uint8_t tb = pind[lo];
                pind[lo] = pind[hi];
                pind[hi] = tb;

                lo++;
                hi--;
            }

        } while (lo <= hi);

        if (left < hi) quickSort(left, hi);
        if (lo < right) quickSort(lo, right);
    }

    void rotateCube()
    {
        int16_t px[8] = { 0 };
        int16_t py[8] = { 0 };
        int16_t pz[8] = { 0 };

        uint8_t n = 0;
        uint8_t ax = 1;
        uint8_t ay = 1;
        uint8_t az = 1;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (n = 0; n < 8; n++)
            {
                const int16_t i = (COS(ay) * points[n][0] - SIN(ay) * points[n][2]) / DIVD;
                const int16_t j = (COS(az) * points[n][1] - SIN(az) * i) / DIVD;
                const int16_t k = (COS(ay) * points[n][2] + SIN(ay) * points[n][0]) / DIVD;

                const int16_t x = (COS(az) * i + SIN(az) * points[n][1]) / DIVD;
                const int16_t y = (COS(ax) * j + SIN(ax) * k) / DIVD;
                const int16_t z = (COS(ax) * k - SIN(ax) * j) / DIVD;

                px[n] = 160 + (-x * FDST) / (z - FDST);
                py[n] = 100 + (-y * FDST) / (z - FDST);
                pz[n] = z;
            }

            for (n = 0; n < 6; n++)
            {
                polyz[n] = (pz[planes[n][0]] + pz[planes[n][1]] + pz[planes[n][2]] + pz[planes[n][3]]) >> 2;
                pind[n] = n;
            }

            quickSort(0, 5);

            for (n = 0; n < 6; n++)
                linePolygon(px[planes[pind[n]][0]], py[planes[pind[n]][0]],
                    px[planes[pind[n]][1]], py[planes[pind[n]][1]],
                    px[planes[pind[n]][2]], py[planes[pind[n]][2]],
                    px[planes[pind[n]][3]], py[planes[pind[n]][3]], polyz[n] + 75);

            ax += XSTR;
            ay += YSTR;
            az += ZSTR;

            motionBlur();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
    }

    void run()
    {
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire-Mapping")) return;

        for (int16_t j = 0; j < 256; j++) stab[j] = int16_t(sin(j * M_PI / 128) * DIVD);

        for (uint8_t i = 0; i < 64; i++)
        {
            pal[i].r = i;
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = i;
            pal[i + 64].b = 0;
            pal[i + 128].r = 63;
            pal[i + 128].g = 63;
            pal[i + 128].b = i;
            pal[i + 192].r = 63;
            pal[i + 192].g = 63;
            pal[i + 192].b = 63;
        }

        shiftPalette(pal);
        setPalette(pal);
        rotateCube();
        cleanup();
    }
}

namespace fireTexture3 {
    #define MPOS    200
    #define DIVD    128
    #define DIST    400
    #define XSTR    1
    #define YSTR    2
    #define ZSTR    -1

    #define SIN(i)  (stab[i & 0xff])
    #define COS(i)  (stab[(i + 192) & 0xff])

    const int16_t points[][3] = {
        {-25, -25, -25}, {-25, -25, 25}, {25, -25, 25}, {25, -25, -25},
        {-25,  25, -25}, {-25,  25, 25}, {25,  25, 25}, {25,  25, -25}
    };

    const uint8_t planes[][4] = {
        {0, 4, 5, 1}, {0, 3, 7, 4}, {0, 1, 2, 3},
        {4, 5, 6, 7}, {7, 6, 2, 3}, {1, 2, 6, 5}
    };

    int16_t stab[256] = { 0 };
    int16_t polyz[6] = { 0 };
    uint8_t pind[6] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void horzLine(int16_t x1, int16_t x2, int16_t y, uint8_t col)
    {
        if (x2 < x1) { x1 += x2; x2 = x1 - x2; x1 -= x2; }
        if (x2 < 0 || x2 > MAX_WIDTH || x1 < 0 || x1 > MAX_WIDTH || y < 0 || y > MAX_HEIGHT) return;
        for (int16_t x = x1; x <= x2; x++) vbuff[y][x] = col;
    }

    void linePolygon(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, uint8_t col)
    {
        int16_t xpos[MPOS][2] = { 0 };

        int16_t mx = y1;
        if (y2 < mx) mx = y2;
        if (y3 < mx) mx = y3;
        if (y4 < mx) mx = y4;

        int16_t my = y1;
        if (y2 > my) my = y2;
        if (y3 > my) my = y3;
        if (y4 > my) my = y4;

        const int16_t s1 = ((y1 < y2) << 1) - 1;
        const int16_t s2 = ((y2 < y3) << 1) - 1;
        const int16_t s3 = ((y3 < y4) << 1) - 1;
        const int16_t s4 = ((y4 < y1) << 1) - 1;

        int16_t y = y1;
        if (y1 != y2)
        {
            do
            {
                xpos[y][y1 < y2] = (x2 - x1) * (y - y1) / (y2 - y1) + x1;
                y += s1;
            } while (y != (y2 + s1));
        }
        else xpos[y][y1 < y2] = x1;

        y = y2;
        if (y2 != y3)
        {
            do
            {
                xpos[y][y2 < y3] = (x3 - x2) * (y - y2) / (y3 - y2) + x2;
                y += s2;
            } while (y != (y3 + s2));
        }
        else xpos[y][y2 < y3] = x2;

        y = y3;
        if (y3 != y4)
        {
            do
            {
                xpos[y][y3 < y4] = (x4 - x3) * (y - y3) / (y4 - y3) + x3;
                y += s3;
            } while (y != (y4 + s3));
        }
        else xpos[y][y3 < y4] = x3;

        y = y4;
        if (y4 != y1)
        {
            do
            {
                xpos[y][y4 < y1] = (x1 - x4) * (y - y4) / (y1 - y4) + x4;
                y += s4;
            } while (y != (y1 + s4));
        }
        else xpos[y][y1 < y4] = x4;

        for (y = mx; y < my; y++) horzLine(xpos[y][0], xpos[y][1], y, col);
    }

    void motionBlur()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            mov     esi, edi
            add     edi, IMAGE_WIDTH
            add     esi, IMAGE_SIZE - IMAGE_WIDTH
        again:
            xor     ah, ah
            mov     al, [edi - 1]
            mov     dx, ax
            mov     al, [edi + 1]
            add     dx, ax
            mov     al, [edi - IMAGE_WIDTH]
            add     dx, ax
            mov     al, [edi + IMAGE_WIDTH]
            add     dx, ax
            shr     dx, 2
            or      dl, dl
            jz    	skip
            dec     dl
        skip :
            mov     [edi - IMAGE_WIDTH], dl
            inc     edi
            cmp     edi, esi
            jnz     again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++)
            {
                uint8_t col = (vbuff[i][j - 1] + vbuff[i][j + 1] + vbuff[i - 1][j] + vbuff[i + 1][j]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void quickSort(int16_t left, int16_t right)
    {
        int16_t lo = left;
        int16_t hi = right;
        const int16_t mid = polyz[(lo + hi) >> 1];

        do {
            while (polyz[lo] < mid) lo++;
            while (mid < polyz[hi]) hi--;

            if (lo <= hi)
            {
                const int16_t ta = polyz[lo];
                polyz[lo] = polyz[hi];
                polyz[hi] = ta;

                const uint8_t tb = pind[lo];
                pind[lo] = pind[hi];
                pind[hi] = tb;

                lo++;
                hi--;
            }

        } while (lo <= hi);

        if (left < hi) quickSort(left, hi);
        if (lo < right) quickSort(lo, right);
    }

    void rotateCube()
    {
        int16_t px[8] = { 0 };
        int16_t py[8] = { 0 };
        int16_t pz[8] = { 0 };

        uint8_t n = 0;
        uint8_t ax = 0;
        uint8_t ay = 0;
        uint8_t az = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (n = 0; n < 8; n++)
            {
                const int16_t i = (COS(ay) * points[n][0] - SIN(ay) * points[n][2]) / DIVD;
                const int16_t j = (COS(az) * points[n][1] - SIN(az) * i) / DIVD;
                const int16_t k = (COS(ay) * points[n][2] + SIN(ay) * points[n][0]) / DIVD;

                const int16_t x = (COS(az) * i + SIN(az) * points[n][1]) / DIVD;
                const int16_t y = (COS(ax) * j + SIN(ax) * k) / DIVD;

                pz[n] = (COS(ax) * k - SIN(ax) * j) / DIVD + COS(ax) / 3;
                px[n] = 160 + (SIN(ax) >> 1) + (-x * DIST) / (pz[n] - DIST);
                py[n] = 100 + (-y * DIST) / (pz[n] - DIST);
            }

            for (n = 0; n < 6; n++)
            {
                polyz[n] = (pz[planes[n][0]] + pz[planes[n][1]] + pz[planes[n][2]] + pz[planes[n][3]]) >> 2;
                pind[n] = n;
            }

            quickSort(0, 5);

            for (n = 0; n < 6; n++)
                linePolygon(px[planes[pind[n]][0]], py[planes[pind[n]][0]],
                    px[planes[pind[n]][1]], py[planes[pind[n]][1]],
                    px[planes[pind[n]][2]], py[planes[pind[n]][2]],
                    px[planes[pind[n]][3]], py[planes[pind[n]][3]], polyz[n] + 75);

            ax += XSTR;
            ay += YSTR;
            az += ZSTR;

            motionBlur();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
    }

    void run()
    {
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire-Mapping")) return;

        for (int16_t j = 0; j < 256; j++) stab[j] = int16_t(sin(j * M_PI / 128) * DIVD);

        for (uint8_t i = 0; i < 32; i++)
        {
            pal[i].r = 0;
            pal[i].g = 0;
            pal[i].b = i;
            pal[i + 32].r = i;
            pal[i + 32].g = i >> 1;
            pal[i + 32].b = 31 - (i >> 1);
            pal[i + 64].r = 31 + i;
            pal[i + 64].g = (31 + i) >> 1;
            pal[i + 64].b = 31 - ((31 + i) >> 1);
            pal[i + 96].r = 63;
            pal[i + 96].g = 31 + i;
            pal[i + 96].b = 0;
            pal[i + 128].r = 63;
            pal[i + 128].g = 63;
            pal[i + 128].b = i;
        }

        shiftPalette(pal);
        setPalette(pal);
        rotateCube();
        cleanup();
    }
}

namespace tunnelEffect {
    RGB     pal[256] = { 0 };
    uint8_t brightness = 120;
    int32_t xofs[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    int32_t yofs[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void setPalette()
    {
        for (int16_t i = 0; i < 256; i++)
        {
            int16_t val = 127 - pal[i].r;
            val = ((val << 8) * val) << 2;
            int16_t col = (val & 0xFF00) | pal[i].r;
            val = ((val & 0xFF00) + col) * brightness;
            pal[i].r = val >> 8;

            val = 127 - pal[i].g;
            val = ((val << 8) * val) << 2;
            col = (val & 0xFF00) | pal[i].g;
            val = ((val & 0xFF00) + col) * brightness;
            pal[i].g = val >> 8;

            val = 127 - pal[i].b;
            val = ((val << 8) * val) << 2;
            col = (val & 0xFF00) | pal[i].b;
            val = ((val & 0xFF00) + col) * brightness;
            pal[i].b = val >> 8;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void tunnelBlur()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            add     edi, IMAGE_WIDTH
            mov     ecx, IMAGE_SIZE - (IMAGE_WIDTH << 1)
            xor     bx, bx
        again:
            xor     ax, ax
            mov     bl, [edi - 1]
            add     ax, bx
            mov     bl, [edi + 1]
            add     ax, bx
            mov     bl, [edi - IMAGE_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH]
            add     ax, bx
            shr     ax, 2
            stosb
            loop    again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++) vbuff[i][j] = (vbuff[i][j - 1] + vbuff[i][j + 1] + vbuff[i - 1][j] + vbuff[i + 1][j]) >> 2;
        }
#endif
    }

    uint8_t pike(int16_t x, int16_t arg, int16_t a, int16_t b, int16_t c)
    {
        if (x < arg) return a + (b - a) * x / arg;
        return b + (c - b) * (x - arg) / (256 - arg);
    }

    void makeTunnel()
    {
#ifdef _USE_ASM
        __asm {
            xor     esi, esi
            xor     edi, edi
            mov     ecx, 32000
        next:
            xor     eax, eax
            xor     edx, edx
            xor     ebx, ebx
            mov     edx, yofs[edi]
            shl     edx, 6
            add     ebx, edx
            shl     edx, 2
            add     ebx, edx
            add     ebx, xofs[edi]
            mov     al, vbuff[esi]
            shr     al, 1
            mov     ah, vbuff[esi + ebx]
            shr     ah, 1
            add     al, ah
            mov     vbuff[esi], al
            not     esi
            not     ebx
            mov     al, vbuff[IMAGE_SIZE + esi]
            shr     al, 1
            mov     ah, vbuff[IMAGE_SIZE + esi + ebx]
            shr     ah, 1
            add     al, ah
            mov     vbuff[IMAGE_SIZE + esi], al
            not     esi
            inc     esi
            add     edi, 4
            loop    next
        }
#else
        for (int16_t i = 0; i < IMAGE_MIDY; i++)
        {
            for (int16_t j = 0; j < IMAGE_WIDTH; j++)
            {
                vbuff[i][j] = (vbuff[i][j] + vbuff[i + yofs[i][j]][j + xofs[i][j]]) >> 1;
                vbuff[MAX_HEIGHT - i][MAX_WIDTH - j] = (vbuff[MAX_HEIGHT - i][MAX_WIDTH - j] + vbuff[MAX_HEIGHT - i - yofs[i][j]][MAX_WIDTH - j - xofs[i][j]]) >> 1;
            }
        }
#endif
    }

    void showTunnel()
    {
        int16_t i = 0, j = 0;
        int16_t x = 0, y = 0;

        for (i = 0; i < 256; i++)
        {
            pal[i].r = pike(i, 128, 0, 64, 64);
            pal[i].g = pike(i, 128, 0, 127, 64);
            pal[i].b = pike(i, 192, 0, 127, 32);
        }

        setPalette();

        for (i = 0; i < IMAGE_MIDY; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                x = IMAGE_MIDX - j;
                y = IMAGE_MIDY - i;
                xofs[i][j] = (x >> 4) + random(3) + (y >> 5) - 1;
                yofs[i][j] = (y >> 4) + random(3) - (x >> 5) - 1;
            }
        }

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (i = 0; i < IMAGE_MIDY; i++)
            {
                x = rand() % MAX_WIDTH;
                y = rand() % MAX_HEIGHT;
                vbuff[y][x] = 240;
                vbuff[y][x + 1] = 240;
                vbuff[y + 1][x] = 240;
                vbuff[y + 1][x + 1] = 240;
            }

            makeTunnel();
            tunnelBlur();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Tunnel")) return;
        showTunnel();
    }
}

namespace textureMappingEffect {
    #define MAXP	18
    #define YOFS	100
    #define XOFS	160
    #define ZOFS	-500
    #define RADI	0.01745329
    #define TEXSIZE	16384

    typedef struct {
        int16_t x, y, z;
    } TPoint;

    const int16_t faces[MAXP][4][3] = {
        {{-10, -10,  10}, { 10, -10,  10}, { 10,  10,  10}, {-10,  10,  10}},
        {{-10,  10, -10}, { 10,  10, -10}, { 10, -10, -10}, {-10, -10, -10}},
        {{-10,  10,  10}, {-10,  10, -10}, {-10, -10, -10}, {-10, -10,  10}},
        {{ 10, -10,  10}, { 10, -10, -10}, { 10,  10, -10}, { 10,  10,  10}},
        {{ 10,  10,  10}, { 10,  10, -10}, {-10,  10, -10}, {-10,  10,  10}},
        {{-10, -10,  10}, {-10, -10, -10}, { 10, -10, -10}, { 10, -10,  10}},

        {{-10, -10, -20}, { 10, -10, -20}, { 10,  10, -20}, {-10,  10, -20}},
        {{-10,  10, -30}, { 10,  10, -30}, { 10, -10, -30}, {-10, -10, -30}},
        {{-10,  10, -20}, {-10,  10, -30}, {-10, -10, -30}, {-10, -10, -20}},
        {{ 10, -10, -20}, { 10, -10, -30}, { 10,  10, -30}, { 10,  10, -20}},
        {{ 10,  10, -20}, { 10,  10, -30}, {-10,  10, -30}, {-10,  10, -20}},
        {{-10, -10, -20}, {-10, -10, -30}, { 10, -10, -30}, { 10, -10, -20}},

        {{-30, -10,  10}, {-20, -10,  10}, {-20,  10,  10}, {-30,  10,  10}},
        {{-30,  10, -10}, {-20,  10, -10}, {-20, -10, -10}, {-30, -10, -10}},
        {{-30,  10,  10}, {-30,  10, -10}, {-30, -10, -10}, {-30, -10,  10}},
        {{-20, -10,  10}, {-20, -10, -10}, {-20,  10, -10}, {-20,  10,  10}},
        {{-20,  10,  10}, {-20,  10, -10}, {-30,  10, -10}, {-30,  10,  10}},
        {{-30, -10,  10}, {-30, -10, -10}, {-20, -10, -10}, {-20, -10,  10}}
    };

    TPoint lines[MAXP][4] = { 0 };
    TPoint trans[MAXP][4] = { 0 };
    TPoint center1[MAXP] = { 0 };
    TPoint center2[MAXP] = { 0 };

    int16_t miny = 0, maxy = 0;
    int16_t clipx = 0, clipy = 0;

    int16_t order[MAXP] = { 0 };
    int16_t lookup[360][2] = { 0 };

    int16_t left[402][3] = { 0 };
    int16_t right[402][3] = { 0 };

    uint8_t texture[SIZE_128][SIZE_128] = { 0 };
    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void setUpPoints()
    {
        int16_t i = 0;

        for (i = 0; i < 360; i++)
        {
            lookup[i][0] = int16_t(sin(i * RADI) * TEXSIZE);
            lookup[i][1] = int16_t(cos(i * RADI) * TEXSIZE);
        }

        for (i = 0; i < MAXP; i++)
        {
            center1[i].x = (lines[i][0].x + lines[i][1].x + lines[i][2].x + lines[i][3].x) >> 2;
            center1[i].y = (lines[i][0].y + lines[i][1].y + lines[i][2].y + lines[i][3].y) >> 2;
            center1[i].z = (lines[i][0].z + lines[i][1].z + lines[i][2].z + lines[i][3].z) >> 2;
        }
    }

    int16_t loadTexture()
    {
        RGB rgb[256] = { 0 };
        if (!loadPNG(vbuff2[0], rgb, "assets/tface.png")) return 0;
        if (!loadPNG(texture[0], NULL, "assets/robot.png")) return 0;
        setPalette(rgb);
        return 1;
    }

    void rotatePoints(int16_t x, int16_t y, int16_t z)
    {
        int16_t i = 0, j = 0;
        int16_t a = 0, b = 0, c = 0;

        for (i = 0; i < MAXP; i++)
        {
            for (j = 0; j < 4; j++)
            {
                b = lookup[y][1];
                c = lines[i][j].x;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    mov     a, dx
                }
#else
                a = b * c >> 14; 
#endif
                b = lookup[y][0];
                c = lines[i][j].z;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    add     a, dx
                }
#else
                a += b * c >> 14;
#endif
                trans[i][j].x = a;
                trans[i][j].y = lines[i][j].y;

                b = -lookup[y][0];
                c = lines[i][j].x;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    mov     a, dx
                }
#else
                a = b * c >> 14;
#endif
                b = lookup[y][1];
                c = lines[i][j].z;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    add     a, dx
                }
#else
                a += b * c >> 14;
#endif
                trans[i][j].z = a;

                if (x)
                {
                    b = lookup[x][1];
                    c = trans[i][j].y;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        mov     a, dx
                    }
#else
                    a = b * c >> 14;
#endif
                    b = lookup[x][0];
                    c = trans[i][j].z;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        sub     a, dx
                    }
#else
                    a -= b * c >> 14;
#endif
                    b = lookup[x][0];
                    c = trans[i][j].y;
                    trans[i][j].y = a;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        mov     a, dx
                    }
#else
                    a = b * c >> 14;
#endif
                    b = lookup[x][1];
                    c = trans[i][j].z;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        add     a, dx
                    }
#else
                    a += b * c >> 14;
#endif
                    trans[i][j].z = a;
                }

                if (z)
                {
                    b = lookup[z][1];
                    c = trans[i][j].x;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        mov     a, dx
                    }
#else
                    a = b * c >> 14;
#endif
                    b = lookup[z][0];
                    c = trans[i][j].y;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        sub     a, dx
                    }
#else
                    a -= b * c >> 14;
#endif
                    b = lookup[z][0];
                    c = trans[i][j].x;
                    trans[i][j].x = a;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        mov     a, dx
                    }
#else
                    a = b * c >> 14;
#endif
                    b = lookup[z][1];
                    c = trans[i][j].y;
#ifdef _USE_ASM
                    __asm {
                        mov     ax, b
                        imul    c
                        sal     ax, 1
                        rcl     dx, 1
                        sal     ax, 1
                        rcl     dx, 1
                        add     a, dx
                    }
#else
                    a += b * c >> 14;
#endif
                    trans[i][j].y = a;
                }
            }
        }

        for (i = 0; i < MAXP; i++)
        {
            b = lookup[y][1];
            c = center1[i].x;
#ifdef _USE_ASM
            __asm {
                mov     ax, b
                imul    c
                sal     ax, 1
                rcl     dx, 1
                sal     ax, 1
                rcl     dx, 1
                mov     a, dx
            }
#else
            a = b * c >> 14;
#endif
            b = lookup[y][0];
            c = center1[i].z;
#ifdef _USE_ASM
            __asm {
                mov     ax, b
                imul    c
                sal     ax, 1
                rcl     dx, 1
                sal     ax, 1
                rcl     dx, 1
                add     a, dx
            }
#else
            a += b * c >> 14;
#endif
            center2[i].x = a;
            center2[i].y = center1[i].y;

            b = -lookup[y][0];
            c = center1[i].x;
#ifdef _USE_ASM
            __asm {
                mov     ax, b
                imul    c
                sal     ax, 1
                rcl     dx, 1
                sal     ax, 1
                rcl     dx, 1
                mov     a, dx
            }
#else
            a = b * c >> 14;
#endif
            b = lookup[y][1];
            c = center1[i].z;
#ifdef _USE_ASM
            __asm {
                mov     ax, b
                imul    c
                sal     ax, 1
                rcl     dx, 1
                sal     ax, 1
                rcl     dx, 1
                add     a, dx
            }
#else
            a += b * c >> 14;
#endif
            center2[i].z = a;

            if (x)
            {
                b = lookup[x][1];
                c = center2[i].y;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    mov     a, dx
                }
#else
                a = b * c >> 14;
#endif
                b = lookup[x][0];
                c = center2[i].z;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    sub     a, dx
                }
#else
                a -= b * c >> 14;
#endif
                b = lookup[x][0];
                c = center2[i].y;
                center2[i].y = a;

#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    mov     a, dx
                }
#else
                a = b * c >> 14;
#endif
                b = lookup[x][1];
                c = center2[i].z;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    add     a, dx
                }
#else
                a += b * c >> 14;
#endif
                center2[i].z = a;
            }

            if (z)
            {
                b = lookup[z][1];
                c = center2[i].x;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    mov     a, dx
                }
#else
                a = b * c >> 14;
#endif
                b = lookup[z][0];
                c = center2[i].y;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    sub     a, dx
                }
#else
                a -= b * c >> 14;
#endif
                b = lookup[z][0];
                c = center2[i].x;
                center2[i].x = a;

#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    mov     a, dx
                }
#else
                a = b * c >> 14;
#endif
                b = lookup[z][1];
                c = center2[i].y;
#ifdef _USE_ASM
                __asm {
                    mov     ax, b
                    imul    c
                    sal     ax, 1
                    rcl     dx, 1
                    sal     ax, 1
                    rcl     dx, 1
                    add     a, dx
                }
#else
                a += b * c >> 14;
#endif
                center2[i].y = a;
            }
        }
    }

    void scanLeftSide(int16_t x1, int16_t x2, int16_t ytop, int16_t height, uint8_t side)
    {
        int16_t x = 0, y = 0, xadd = 0, ofs = 0;
        int16_t px = 0, py = 0, pxadd = 0, pyadd = 0;

        if (height == 0) return;

        xadd = ((x2 - x1) << 6) / height;

        switch (side)
        {
        case 1:
            px = 127 << 6;
            py = 0;
            pxadd = -(127 << 6) / height;
            pyadd = 0;
            break;

        case 2:
            px = 127 << 6;
            py = 127 << 6;
            pxadd = 0;
            pyadd = -(127 << 6) / height;
            break;

        case 3:
            px = 0;
            py = 127 << 6;
            pxadd = (127 << 6) / height;
            pyadd = 0;
            break;

        case 4:
            px = 0;
            py = 0;
            pxadd = 0;
            pyadd = (127 << 6) / height;
            break;

        default:
            break;
        }

        x = x1 << 6;

        for (y = 0; y <= height; y++)
        {
            ofs = ytop + y;
            if (ofs > IMAGE_HEIGHT) ofs = IMAGE_HEIGHT;

            left[ofs + IMAGE_HEIGHT][0] = x >> 6;
            left[ofs + IMAGE_HEIGHT][1] = px >> 6;
            left[ofs + IMAGE_HEIGHT][2] = py >> 6;

            x += xadd;
            px += pxadd;
            py += pyadd;
        }
    }

    void scanRightSide(int16_t x1, int16_t x2, int16_t ytop, int16_t height, uint8_t side)
    {
        int16_t x = 0, y = 0, xadd = 0, ofs = 0;
        int16_t px = 0, py = 0, pxadd = 0, pyadd = 0;

        if (height == 0) return;

        xadd = ((x2 - x1) << 6) / height;

        switch (side)
        {
        case 1:
            px = 0;
            py = 0;
            pxadd = (127 << 6) / height;
            pyadd = 0;
            break;

        case 2:
            px = 127 << 6;
            py = 0;
            pxadd = 0;
            pyadd = (127 << 6) / height;
            break;

        case 3:
            px = 127 << 6;
            py = 127 << 6;
            pxadd = -(127 << 6) / height;
            pyadd = 0;
            break;

        case 4:
            px = 0;
            py = 127 << 6;
            pxadd = 0;
            pyadd = -(127 << 6) / height;
            break;

        default:
            break;
        }

        x = x1 << 6;

        for (y = 0; y <= height; y++)
        {
            ofs = ytop + y;
            if (ofs > IMAGE_HEIGHT) ofs = IMAGE_HEIGHT;

            right[ofs + IMAGE_HEIGHT][0] = x >> 6;
            right[ofs + IMAGE_HEIGHT][1] = px >> 6;
            right[ofs + IMAGE_HEIGHT][2] = py >> 6;

            x += xadd;
            px += pxadd;
            py += pyadd;
        }
    }

    void textureMapping()
    {
        if (miny < 0) miny = 0;
        if (maxy > MAX_HEIGHT) maxy = MAX_HEIGHT;

        if (miny < clipx) miny = clipx;
        if (maxy > clipy) maxy = clipy;

        if (maxy - miny < 2) return;
        if (miny > MAX_HEIGHT) return;
        if (maxy < 0) return;

        for (int16_t y = miny; y <= maxy; y++)
        {
            const int16_t lx1 = left[y + IMAGE_HEIGHT][0];
            int16_t px1 = left[y + IMAGE_HEIGHT][1] << 7;
            int16_t py1 = left[y + IMAGE_HEIGHT][2] << 7;
            
            const int16_t lx2 = right[y + IMAGE_HEIGHT][0];
            const int16_t px2 = right[y + IMAGE_HEIGHT][1] << 7;
            const int16_t py2 = right[y + IMAGE_HEIGHT][2] << 7;

            const int16_t width = abs(lx2 - lx1) + 1;
            const int16_t pxadd = (px2 - px1) / width;
            const int16_t pyadd = (py2 - py1) / width;

#ifdef _USE_ASM
            __asm {
                xor     eax, eax
                mov     ax, lx1
                lea     edi, vbuff1
                add     edi, eax
                mov     ax, y
                shl     eax, 6
                add     edi, eax
                shl     eax, 2
                add     edi, eax
                xor     ebx, ebx
                xor     edx, edx
                add     bx, px1
                add     dx, py1
                xor     ecx, ecx
                add     cx, width
            again:
                lea     esi, texture
                mov     eax, ebx
                and     eax, 0FF80h
                add     esi, eax
                mov     eax, edx
                shr     eax, 7
                add     esi, eax
                movsb
                add     bx, pxadd
                add     dx, pyadd
                loop    again
            }
#else
            for (int16_t x = 0; x < width; x++)
            {
                vbuff1[y][x + lx1] = texture[px1 >> 7][py1 >> 7];
                px1 += pxadd;
                py1 += pyadd;
            }
#endif
        }
    }

    void textureMapPoly(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4)
    {
        maxy = 0;
        miny = 32767;
        
        if (y1 < miny) miny = y1;
        if (y1 > maxy) maxy = y1;
        if (y2 < miny) miny = y2;
        if (y2 > maxy) maxy = y2;
        if (y3 < miny) miny = y3;
        if (y3 > maxy) maxy = y3;
        if (y4 < miny) miny = y4;
        if (y4 > maxy) maxy = y4;

        if (miny > maxy - 5) return;

        if (y2 < y1) scanLeftSide(x2, x1, y2, y1 - y2 + 1, 1);
        else scanRightSide(x1, x2, y1, y2 - y1 + 1, 1);

        if (y3 < y2) scanLeftSide(x3, x2, y3, y2 - y3 + 1, 2);
        else scanRightSide(x2, x3, y2, y3 - y2 + 1, 2);

        if (y4 < y3) scanLeftSide(x4, x3, y4, y3 - y4 + 1, 3);
        else scanRightSide(x3, x4, y3, y4 - y3 + 1, 3);

        if (y1 < y4) scanLeftSide(x1, x4, y1, y4 - y1 + 1, 4);
        else scanRightSide(x4, x1, y4, y1 - y4 + 1, 4);

        textureMapping();
    }

    void drawPoints()
    {
        for (int16_t j = 0; j < MAXP; j++)
        {
            const int16_t i = order[j];

            if (trans[i][0].z + ZOFS < 0 && trans[i][1].z + ZOFS < 0 && trans[i][2].z + ZOFS < 0 && trans[i][3].z + ZOFS < 0)
            {
                int16_t temp = trans[i][0].z + ZOFS;
                int32_t nx = trans[i][0].x;
                const int16_t x1 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
                nx = trans[i][0].y;
                const int16_t y1 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

                temp = trans[i][1].z + ZOFS;
                nx = trans[i][1].x;
                const int16_t x2 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
                nx = trans[i][1].y;
                const int16_t y2 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

                temp = trans[i][2].z + ZOFS;
                nx = trans[i][2].x;
                const int16_t x3 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
                nx = trans[i][2].y;
                const int16_t y3 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

                temp = trans[i][3].z + ZOFS;
                nx = trans[i][3].x;
                const int16_t x4 = ((nx << 8) + (nx >> 8)) / temp + XOFS;
                nx = trans[i][3].y;
                const int16_t y4 = ((nx << 8) + (nx >> 8)) / temp + YOFS;

                const int16_t normal = (y1 - y3) * (x2 - x1) - (x1 - x3) * (y2 - y1);
                if (normal < 0) textureMapPoly(x1, y1, x2, y2, x3, y3, x4, y4);
            }
        }
    }

    void sortPoints()
    {
        int16_t i = 0;

        for (i = 0; i < MAXP; i++) order[i] = i;

        i = 0;

        while (i < MAXP - 1)
        {
            if (center2[i].z > center2[i + 1].z)
            {
                int16_t temp = center2[i + 1].x;
                center2[i + 1].x = center2[i].x;
                center2[i].x = temp;

                temp = center2[i + 1].y;
                center2[i + 1].y = center2[i].y;
                center2[i].y = temp;

                temp = center2[i + 1].z;
                center2[i + 1].z = center2[i].z;
                center2[i].z = temp;

                temp = order[i + 1];
                order[i + 1] = order[i];
                order[i] = temp;

                i = 0;
            }

            i++;
        }
    }

    void run()
    {
        int16_t deg1 = 0;
        int16_t deg2 = 0;

        for (int16_t i = 0; i < MAXP; i++)
        {
            for (int16_t j = 0; j < 4; j++)
            {
                lines[i][j].x = faces[i][j][0] << 3;
                lines[i][j].y = faces[i][j][1] << 3;
                lines[i][j].z = faces[i][j][2] << 3;
            }
        }

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Texture-Mapping")) return;
        if (!loadTexture()) return;
        
        setUpPoints();

        clipx = 0;
        clipy = MAX_HEIGHT;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            rotatePoints(deg2, deg1, deg2);
            sortPoints();
            drawPoints();
            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            memcpy(vbuff1, vbuff2, IMAGE_SIZE);
            delay(FPS_90);
            deg1 = (deg1 + 1) % 360;
            deg2 = (deg2 + 1) % 360;
        }
        cleanup();
    }
}

namespace bitmapRotate {
    int16_t sintab[256] = { 0 };
    int16_t costab[256] = { 0 };
    int16_t sin2tab[256] = { 0 };
    int16_t cos2tab[256] = { 0 };

    uint8_t texture[SIZE_256][SIZE_256] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void makeTables()
    {
        int16_t i;
        double angle;

        for (i = 0; i < 256; i++)
        {
            angle = i * M_PI / 128;
            sintab[i]  = int16_t(sin(angle) * 256);
            costab[i]  = int16_t(cos(angle) * 256);
            sin2tab[i] = int16_t(sin(angle + M_PI / 2) * 256 * 1.2);
            cos2tab[i] = int16_t(cos(angle + M_PI / 2) * 256 * 1.2);
        }
    }

    void drawScreen(uint16_t x, uint16_t y, uint16_t scale, uint8_t rot)
    {
        int16_t d1x, d1y, d2x, d2y;
        uint16_t i, j, w, h, a, b;

        d1x = costab[rot] * scale >> 8;
        d1y = sintab[rot] * scale >> 8;

        d2x = cos2tab[rot] * scale >> 8;
        d2y = sin2tab[rot] * scale >> 8;

        i = x - d1x * IMAGE_MIDX - d2x * IMAGE_MIDY;
        j = y - d1y * IMAGE_MIDX - d2y * IMAGE_MIDY;

        for (h = 0; h < IMAGE_HEIGHT; h++)
        {
            a = i;
            b = j;
            for (w = 0; w < IMAGE_WIDTH; w++)
            {
                a += d1x;
                b += d1y;
                vbuff[h][w] = texture[b >> 8][a >> 8];
            }
            i += d2x;
            j += d2y;
        }
    }

    int16_t loadTexture()
    {
        RGB rgb[256] = { 0 };
        if (!loadPNG(texture[0], rgb, "assets/crew.png")) return 0;
        setPalette(rgb);
        return 1;
    }

    void run()
    {
        const uint16_t x = 32767;
        uint16_t y = 0;
        uint16_t rot = 0;
        uint16_t dir = 1;
        uint16_t dist = 800;
        uint16_t inc = 65535;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Rotate-Bitmap")) return;
        if (!loadTexture()) return;

        makeTables();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            drawScreen(x, y, dist, rot & 0xff);
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);

            y += 128;
            rot += dir;
            dist += inc;

            if (dist == 2000 || dist == 2) inc = -inc;
            if (random(150) == 1) dir = random(7) - 3;
        }

        cleanup();
    }
}

namespace intro16k {
    #define FONT_HEIGHT 32
    #define FONT_WIDTH  16

    typedef struct {
        uint8_t r, g, b;
    } RGB8;

    typedef struct {
        int16_t x, y, z;
        uint8_t color;
    } T3DPoint;

    typedef struct {
        int16_t v1, v2, v3;
        uint8_t color;
        int16_t z;
    } TFace;

    typedef struct {
        int16_t cx, cy;
        int16_t dx, dy;
        int16_t life;
    } TParticle;

    const TFace baseFigure[] = {
        {0, 3, 1, 200, 0},
        {0, 1, 2, 180, 0},
        {0, 2, 3, 160, 0},
        {1, 3, 2, 140, 0}
    };

    const TFace logoData[] = {
        { 1,  5,  4, 0, 0},
        { 0,  2,  3, 0, 0},
        { 3,  2,  6, 0, 0},
        { 7,  9, 10, 0, 0},
        { 9, 10, 13, 0, 0},
        { 8, 11, 12, 0, 0},
        {14, 15, 16, 0, 0},
        {17, 15, 16, 0, 0},
        {17, 16, 18, 0, 0},
        {17, 19, 18, 0, 0},
        {20, 21, 22, 0, 0},
        {21, 27, 23, 0, 0},
        {22, 24, 28, 0, 0},
        {25, 27, 29, 0, 0},
        {26, 30, 28, 0, 0},
        {37, 31, 32, 0, 0},
        {33, 34, 35, 0, 0},
        {31, 36, 32, 0, 0}
    };

    const T3DPoint baseVertices[] = {
        { 80,  80,  80, 200},
        {-80, -80,  80, 200},
        {-80,  80, -80, 200},
        { 80, -80, -80, 200}
    };

    const T3DPoint logoVertices[] = {
        {-68,  20, 0, 255},
        {-72,  -4, 0, 255},
        {-60, -20, 0, 255},
        {-52, -16, 0, 255},
        {-52,   0, 0, 255},
        {-48,  -4, 0, 255},
        {-20, -24, 0, 255},
        {-56,  20, 0, 255},
        {-56,   8, 0, 255},
        {-32, -16, 0, 255},
        {-20, -20, 0, 255},
        {-20,   0, 0, 255},
        {-20,   8, 0, 255},
        {-20,  20, 0, 255},
        {-16,  20, 0, 255},
        {-16, -16, 0, 255},
        { -8, -20, 0, 255},
        {  4,  20, 0, 255},
        { 12,  16, 0, 255},
        { 12, -20, 0, 255},
        { 16,   0, 0, 255},
        { 24, -20, 0, 255},
        { 24,  20, 0, 255},
        { 23, -12, 0, 255},
        { 23,  12, 0, 255},
        { 28, -18, 0, 255},
        { 28,  18, 0, 255},
        { 32, -20, 0, 255},
        { 32,  20, 0, 255},
        { 40, -16, 0, 255},
        { 40,  16, 0, 255},
        { 48,  16, 0, 255},
        { 56,  20, 0, 255},
        { 56,   8, 0, 255},
        { 44, -20, 0, 255},
        { 52, -20, 0, 255},
        { 68, -20, 0, 255},
        { 20,  28, 0, 255}
    };

    int16_t     xorg = 0;
    int16_t     yorg = 0;
    int16_t	    clipx1 = 0;
    int16_t     clipx2 = 319;
    int16_t     clipy1 = 0;
    int16_t     clipy2 = 199;
    int16_t     deltaZ = 4096;
    int16_t     frames = 0;
    int16_t     beatFunc = 0;
    int16_t     matrix[3][3] = { 0 };

    uint8_t	    br1 = 0;
    uint8_t     br2 = 0;
    uint8_t     sat = 0;
    uint16_t    firstIndex = 255;

    RGB8        rgbpal[256] = { 0 };
    RGB8        outrgb[256] = { 0 };
    RGB         setrgb[256] = { 0 };

    int8_t      sinTab[256] = { 0 };
    uint8_t     sqrTab[4096] = { 0 };

    uint8_t     blobs[SIZE_128][SIZE_128] = { 0 };
    uint8_t     texture[SIZE_128][SIZE_128] = { 0 };
    uint8_t     wiredFont[SIZE_16K] = { 0 };

    int32_t     xofs[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    int32_t     yofs[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    TFace       faces[200] = { 0 };
    T3DPoint    scenes[200] = { 0 };
    T3DPoint    vertices[200] = { 0 };
    TParticle   particles[1000] = { 0 };
    
    int16_t     *order1 = NULL;
    int16_t     *order2 = NULL;

    void flipBuffer()
    {
        beatFunc = (beatFunc * 7) >> 3;
        renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
    }

    void doBeat(int16_t value, int16_t start, int16_t step)
    {
        if (frames % step == start) beatFunc = value;
    }

    int32_t exSin(int16_t x)
    {
        return sinTab[(x >> 2) & 0xff] * (4 - (x & 3)) + sinTab[((x >> 2) + 1) & 0xff] * (x & 3);
    }

    void multMatrix(int32_t m1[][3], int32_t m2[][3])
    {
        int32_t m3[3][3] = { 0 };
        memset(m3, 0, sizeof(m3));

        for (int16_t i = 0; i < 3; i++)
        {
            for (int16_t j = 0; j < 3; j++)
            {
                for (int16_t k = 0; k < 3; k++) m3[i][j] += m1[i][k] * m2[k][j];
            }
        }

        memcpy(m1, m3, sizeof(m3));
        memset(m2, 0, sizeof(m3));
    }

    void genMatrix(int16_t a, int16_t b, int16_t c)
    {
        int32_t m1[3][3], m2[3][3];

        memset(m1, 0, sizeof(m1));
        memset(m2, 0, sizeof(m2));

        int16_t asin = sinTab[a & 0xff];
        int16_t acos = sinTab[(a + 64) & 0xff];
        
        m1[0][0] = acos;
        m1[0][1] = -asin;
        m1[1][0] = asin;
        m1[1][1] = acos;
        m1[2][2] = 64;

        asin = sinTab[b & 0xff];
        acos = sinTab[(b + 64) & 0xff];
        m2[0][0] = acos;
        m2[0][2] = -asin;
        m2[2][0] = asin;
        m2[2][2] = acos;
        m2[1][1] = 64;
        multMatrix(m1, m2);

        asin = sinTab[c & 0xff];
        acos = sinTab[(c + 64) & 0xff];
        m2[1][1] = acos;
        m2[1][2] = -asin;
        m2[2][1] = asin;
        m2[2][2] = acos;
        m2[0][0] = 64;
        multMatrix(m1, m2);

        for (int16_t i = 0; i < 3; i++)
        {
            for (int16_t j = 0; j < 3; j++) matrix[i][j] = m1[i][j] >> 7;
        }
    }

    void project(int16_t cnt)
    {
        int32_t val[3] = { 0 };

        for (int16_t i = 0; i < cnt; i++)
        {
            for (int16_t j = 0; j < 3; j++) val[j] = vertices[i].x * matrix[j][0] + vertices[i].y * matrix[j][1] + vertices[i].z * matrix[j][2];
            scenes[i].z = val[2] >> 11;
            val[2] = (val[2] >> 6) + deltaZ;
            scenes[i].x = (val[0] << 3) / (val[2] * 7);
            scenes[i].y = val[1] / val[2];
            scenes[i].color = vertices[i].color;
        }
    }

    void triangle(int16_t p1, int16_t p2, int16_t p3)
    {
        int16_t v1 = 0, v2 = 0, v3 = 0, y = 0, i = 0;
        int16_t x1 = 0, x2 = 0, dx1 = 0, dx2 = 0;
        int16_t c1 = 0, c2 = 0, dc1 = 0, dc2 = 0;
        int16_t xl = 0, xr = 0, cl = 0, cr = 0, dc = 0;

        if (scenes[p1].y < scenes[p2].y) v1 = p1; else v1 = p2;
        if (scenes[p3].y < scenes[v1].y) v1 = p3;
        if (scenes[p1].y > scenes[p2].y) v2 = p1; else v2 = p2;
        if (scenes[p3].y > scenes[v2].y) v2 = p3;

        if (v1 != p1 && v2 != p1) v3 = p1;
        if (v1 != p2 && v2 != p2) v3 = p2;
        if (v1 != p3 && v2 != p3) v3 = p3;

        int16_t y1 = scenes[v1].y;
        const int16_t vc1 = scenes[v1].color;
        const int16_t vc2 = scenes[v2].color;
        const int16_t vc3 = scenes[v3].color;

        if (scenes[v1].y == scenes[v3].y)
        {
            x1 = scenes[v1].x << 8;
            x2 = scenes[v3].x << 8;
            i = scenes[v2].y - y1;

            if (i == 0) return;

            dx1 = ((scenes[v2].x - scenes[v1].x) << 8) / i;
            dx2 = ((scenes[v2].x - scenes[v3].x) << 8) / i;

            c1 = vc1 << 8;
            c2 = vc3 << 8;

            dc1 = ((vc2 - vc1) << 8) / i;
            dc2 = ((vc2 - vc3) << 8) / i;
        }
        else
        {
            if (y1 >= clipy1) vbuff[(yorg + y1) % IMAGE_HEIGHT][(xorg + scenes[v1].x) % IMAGE_WIDTH] = uint8_t(vc1);

            x1 = scenes[v1].x << 8;
            x2 = x1;

            dx1 = ((scenes[v2].x - scenes[v1].x) << 8) / (scenes[v2].y - y1);
            dx2 = ((scenes[v3].x - scenes[v1].x) << 8) / (scenes[v3].y - y1);

            c1 = vc1 << 8;
            c2 = c1;

            dc1 = ((vc2 - vc1) << 8) / (scenes[v2].y - y1);
            dc2 = ((vc3 - vc1) << 8) / (scenes[v3].y - y1);

            y1++;

            x1 += dx1;
            x2 += dx2;
            c1 += dc1;
            c2 += dc2;
        }

        for (y = y1; y < scenes[v2].y; y++)
        {
            if (y >= clipy1 && y <= clipy2)
            {
                if (x1 < x2)
                {
                    xl = (x1 + 255) >> 8;
                    xr = (x2 + 128) >> 8;
                    cl = c1;
                    cr = c2;
                }
                else
                {
                    xl = (x2 + 255) >> 8;
                    xr = (x1 + 128) >> 8;
                    cl = c2;
                    cr = c1;
                }

                if (xr > xl) dc = int16_t(cr - cl) / int16_t(xr - xl);

                if (xl < clipx1)
                {
                    cl += dc * (clipx1 - xl);
                    xl = clipx1;
                }

                if (xr > clipx2) xr = clipx2;

                for (i = xl; i <= xr; i++)
                {
                    vbuff[(yorg + y) % IMAGE_HEIGHT][(xorg + i) % IMAGE_WIDTH] = cl >> 8;
                    cl += dc;
                }
            }

            if (y == scenes[v3].y)
            {
                dx2 = ((scenes[v2].x - scenes[v3].x) << 8) / (scenes[v2].y - scenes[v3].y);
                dc2 = ((vc2 - vc3) << 8) / (scenes[v2].y - scenes[v3].y);
            }

            x1 += dx1;
            x2 += dx2;
            c1 += dc1;
            c2 += dc2;
        }
    }

    void drawScene(int16_t vert, int16_t fac, int16_t x, int16_t y)
    {
        int16_t i = 0, j = 0;

        xorg = x;
        yorg = y;
        clipx1 = -x + 2;
        clipx2 = MAX_WIDTH - x - 2;
        clipy1 = -y + 2;
        clipy2 = MAX_HEIGHT - y - 2;

        int16_t cnt = 0;
        project(vert);
        
        for (i = 0; i < fac; i++)
        {
            faces[i].z = (scenes[faces[i].v1].z + scenes[faces[i].v2].z + scenes[faces[i].v3].z) / 3;
            const int16_t v1x = scenes[faces[i].v1].x - scenes[faces[i].v2].x;
            const int16_t v1y = scenes[faces[i].v1].y - scenes[faces[i].v2].y;
            const int16_t v2x = scenes[faces[i].v1].x - scenes[faces[i].v3].x;
            const int16_t v2y = scenes[faces[i].v1].y - scenes[faces[i].v3].y;
            if (v1x * v2y - v2x * v1y > 0) order1[cnt++] = i;
        }

        int16_t m = 1;
        for (i = 0; i < 8; i++)
        {
            int16_t c = 0;
            for (j = 0; j < cnt; j++)
            {
                if (!((faces[order1[j]].z ^ 0x80) & m)) c++;
            }

            int16_t n1 = 0;
            int16_t n2 = c;

            for (j = 0; j < cnt; j++)
            {
                if (!((faces[order1[j]].z ^ 0x80) & m)) order2[n1++] = order1[j];
                else order2[n2++] = order1[j];
            }

            m <<= 1;

            int16_t *torder = order1;
            order1 = order2;
            order2 = torder;
        }

        for (i = cnt - 1; i >= 0; i--)
        {
            if (faces[order1[i]].color)
            {
                scenes[faces[order1[i]].v1].color = faces[order1[i]].color;
                scenes[faces[order1[i]].v2].color = faces[order1[i]].color;
                scenes[faces[order1[i]].v3].color = faces[order1[i]].color;
            }
            triangle(faces[order1[i]].v1, faces[order1[i]].v2, faces[order1[i]].v3);
        }
    }

    void setPalette()
    {
#ifdef _USE_ASM
        int32_t startIndex = firstIndex * 3;

        __asm {
            lea     esi, rgbpal
            lea     edi, outrgb
            mov     ecx, startIndex
        lp1:
            mov     al, [esi]
            mov     ah, 127
            sub     ah, al
            mov     al, sat
            mul     ah
            shl     ax, 2
            mov     al, [esi]
            add     al, ah
            mul     br1
            mov     al, ah
            cmp     al, 0x3F
            jbe     lp3
            mov     al, 0x3F
        lp3:
            mov     [edi], al
            inc     edi
            inc     esi
            loop    lp1
            mov     ecx, 768
            sub     ecx, startIndex
        lp2:
            mov     al, [esi]
            mov     ah, 127
            sub     ah, al
            mov     al, sat
            mul     ah
            shl     ax, 2
            mov     al, [esi]
            add     al, ah
            mul     br2
            mov     al, ah
            cmp     al, 0x3F
            jbe     lp4
            mov     al, 0x3F
        lp4:
            mov     [edi], al
            inc     esi
            inc     edi
            loop    lp2
            lea     esi, outrgb
            lea     edi, setrgb
            mov     ecx, 256
        again:
            movsw
            movsb
            inc     edi
            loop    again
        }
#else
        uint8_t col = 0;
        uint16_t i = 0, val = 0;

        for (i = 0; i < firstIndex; i++)
        {
            val = 127 - rgbpal[i].r;
            val = ((((val << 8) + sat) & 0xff) * val) << 2;
            col = (val >> 8) + rgbpal[i].r;
            col = ((((val & 0xFF00) + col) & 0xff) * br1) >> 8;
            if (col > 0x3F) col = 0x3F;
            outrgb[i].r = col;

            val = 127 - rgbpal[i].g;
            val = ((((val << 8) + sat) & 0xff) * val) << 2;
            col = (val >> 8) + rgbpal[i].g;
            col = ((((val & 0xFF00) + col) & 0xff) * br1) >> 8;
            if (col > 0x3F) col = 0x3F;
            outrgb[i].g = col;

            val = 127 - rgbpal[i].b;
            val = ((((val << 8) + sat) & 0xff) * val) << 2;
            col = (val >> 8) + rgbpal[i].b;
            col = ((((val & 0xFF00) + col) & 0xff) * br1) >> 8;
            if (col > 0x3F) col = 0x3F;
            outrgb[i].b = col;
        }

        for (i = firstIndex; i < 256; i++)
        {
            val = 127 - rgbpal[i].r;
            val = ((((val << 8) + sat) & 0xff) * val) << 2;
            col = (val >> 8) + rgbpal[i].r;
            col = ((((val & 0xFF00) + col) & 0xff) * br2) >> 8;
            if (col > 0x3F) col = 0x3F;
            outrgb[i].r = col;

            val = 127 - rgbpal[i].g;
            val = ((((val << 8) + sat) & 0xff) * val) << 2;
            col = (val >> 8) + rgbpal[i].g;
            col = ((((val & 0xFF00) + col) & 0xff) * br2) >> 8;
            if (col > 0x3F) col = 0x3F;
            outrgb[i].g = col;

            val = 127 - rgbpal[i].b;
            val = ((((val << 8) + sat) & 0xff) * val) << 2;
            col = (val >> 8) + rgbpal[i].b;
            col = ((((val & 0xFF00) + col) & 0xff) * br2) >> 8;
            if (col > 0x3F) col = 0x3F;
            outrgb[i].b = col;
        }

        uint8_t* rgb = (uint8_t*)outrgb;
        uint8_t* pal = (uint8_t*)setrgb;
        for (i = 0; i < 256; i++)
        {
            memcpy(pal, rgb, sizeof(RGB8));
            rgb += 3;
            pal += 4;
        }
#endif
        shiftPalette(setrgb);
        setPalette(setrgb);
    }

    void tunnelBlur()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            add     edi, IMAGE_WIDTH
            mov     ecx, IMAGE_SIZE - (IMAGE_WIDTH << 1)
            xor     bx, bx
        again:
            xor     ax, ax
            mov     bl, [edi - 1]
            add     ax, bx
            mov     bl, [edi + 1]
            add     ax, bx
            mov     bl, [edi - IMAGE_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH]
            add     ax, bx
            shr     ax, 2
            stosb
            loop    again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++) vbuff[i][j] = (vbuff[i][j - 1] + vbuff[i][j + 1] + vbuff[i - 1][j] + vbuff[i + 1][j]) >> 2;
        }
#endif
    }

    void fireBlur()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            mov     ecx, IMAGE_SIZE - (IMAGE_WIDTH << 1)
            xor     bx, bx
        again:
            xor     ax, ax
            mov     bl, [edi]
            add     ax, bx
            mov     bl, [edi + MAX_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH + 1]
            add     ax, bx
            shr     ax, 2
            and     al, al
            jz      skip
            dec     al
        skip:
            stosb
            loop    again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++)
            {
                uint8_t col = (vbuff[i - 1][j] + vbuff[i][j - 1] + vbuff[i][j] + vbuff[i][j + 1]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void printCharIntro(char chr, int16_t x, int16_t y, uint8_t col, uint8_t col2)
    {
#ifdef _USE_ASM
        __asm {
            xor     esi, esi
            xor     edi, edi
            mov     ax, y
            shl     ax, 6
            add     di, ax
            shl     ax, 2
            add     di, ax
            add     di, x
            xor     ax, ax
            mov     al, chr
            shl     ax, 7
            mov     si, ax
            mov     dx, 32
            mov     bl, col
            mov     bh, col2
        lp1:
            push    dx
            mov     dx, word ptr wiredFont[esi]
            add     esi, 2
            mov     ax, word ptr wiredFont[esi]
            add     esi, 2
            mov     ecx, 16
        lp2:
            rcr     ax, 1
            jnc     lp3
            mov     vmem[edi], bh
        lp3:
            rcr     dx, 1
            jnc     lp4
            mov     vmem[edi], bl
        lp4:
            inc     edi
            loop    lp2
            add     edi, 304
            pop     dx
            dec     dx
            jnz     lp1
        }
#else
        uint16_t* si = (uint16_t*)&wiredFont[chr << 7];

        for (int16_t i = 0; i < FONT_HEIGHT; i++)
        {
            uint16_t dx = *si++;
            uint16_t ax = *si++;
            for (int16_t j = 0; j < FONT_WIDTH; j++)
            {
                if (dx & 1) vmem[y + i][x + j] = col;
                if (ax & 1) vmem[y + i][x + j] = col2;
                ax >>= 1;
                dx >>= 1;
            }
        }
#endif
    }

    void printChar(char chr, int16_t x, int16_t y, uint8_t col, uint8_t col2)
    {
#ifdef _USE_ASM
        __asm {
            xor     esi, esi
            xor     edi, edi
            mov     ax, y
            shl     ax, 6
            add     di, ax
            shl     ax, 2
            add     di, ax
            add     di, x
            xor     ax, ax
            mov     al, chr
            shl     ax, 7
            mov     si, ax
            mov     dx, FONT_HEIGHT
            mov     bl, col
            mov     bh, col2
        lp1:
            push    dx
            mov     dx, word ptr wiredFont[esi]
            add     esi, 2
            mov     ax, word ptr wiredFont[esi]
            add     esi, 2
            mov     ecx, FONT_WIDTH
        lp2:
            rcr     ax, 1
            jnc     lp3
            mov     vbuff[edi], bh
        lp3:
            rcr     dx, 1
            jnc     lp4
            mov     vbuff[edi], bl
        lp4:
            inc     edi
            loop    lp2
            add     edi, 304
            pop     dx
            dec     dx
            jnz     lp1
        }
#else
        uint16_t* si = (uint16_t*)&wiredFont[chr << 7];

        for (int16_t i = 0; i < FONT_HEIGHT; i++)
        {
            uint16_t dx = *si++;
            uint16_t ax = *si++;
            for (int16_t j = 0; j < FONT_WIDTH; j++)
            {
                if (dx & 1) vbuff[y + i][x + j] = col;
                if (ax & 1) vbuff[y + i][x + j] = col2;
                ax >>= 1;
                dx >>= 1;
            }
        }
#endif
    }

    void printString(uint8_t place, const char* str, int16_t x, int16_t y, uint8_t color, uint8_t color2, int8_t fl)
    {
        int16_t xadd = 0, yadd = 0;

        while (*str != '\0')
        {
            if (fl)
            {
                xadd = random(3);
                yadd = random(3);
            }
            if (!place) printCharIntro(*str, x + xadd, y + yadd, color, color2);
            else printChar(*str, x + xadd, y + yadd, color, color2);
            x += FONT_WIDTH + 2;
            str++;
        }
    }

    int16_t range(int16_t b, int16_t min, int16_t max)
    {
        if (b > max) return max;
        if (b < min) return min;
        return b;
    }

    int16_t pike(int16_t x, int16_t arg, int16_t a, int16_t b, int16_t c)
    {
        if (x < arg) return a + (b - a) * x / arg;
        return b + (c - b) * (x - arg) / (256 - arg);
    }

    void initSintab()
    {
#ifdef _USE_ASM
        int16_t val = 0;
        const int16_t scale = 64, diver = 128;

        __asm {
            mov     ecx, 256
            lea     esi, sinTab
            fninit
            fldpi
            fidiv   diver
            fldz
        again:
            fld     ST(0)
            fsin
            fimul   scale
            fistp   val
            mov     ax, val
            mov     [esi], al
            inc     esi
            fadd    ST, ST(1)
            loop    again
        }
#else
        for (int16_t i = 0; i < 256; i++) sinTab[i] = int8_t(sin(i * M_PI / 128) * 64);
#endif
    }

    void initBlobs()
    {
        int16_t i, j = 0;
        for (i = 0; i < 4096; i++)
        {
            if (j * j < i) j++;
            sqrTab[i] = uint8_t(j);
        }

        for (i = 0; i < 128; i++)
        {
            for (j = 0; j < 128; j++) blobs[i][j] = uint8_t(range(32 - sqrTab[range((64 - i) * (64 - i) + (64 - j) * (64 - j), 0, 4095)], 0, 32));
        }
    }

    void initTextures()
    {
        const int16_t dirTab[][2] = { {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1} };

        memset(texture, 0, sizeof(texture));

        for (int16_t i = 1; i <= 25; i++)
        {
            int16_t x = random(256);
            int16_t y = random(256);
            int16_t c = random(8);

            for (int16_t j = 1; j <= 2000; j++)
            {
                const int16_t col = (x >> 1) & 127;
                const int16_t row = (y >> 1) & 127;
                texture[row][col] += 4;
                x += dirTab[c][0];
                y += dirTab[c][1];
                if (!(j & 1)) c = (c + random(3) - 1) & 7;
            }
        }
    }

    void initFonts()
    {
        FILE* fp = fopen("assets/wirefont.fnt", "rb");
        if (!fp)
        {
            messageBox(GFX_WARNING, "Cannot load wired font!");
            return;
        }

        fread(wiredFont, 1, SIZE_16K, fp);
        fclose(fp);
    }

    void initialize()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "16K-Intro")) return;
        
        initSintab();
        initBlobs();
        initTextures();
        initFonts();

        order1 = (int16_t*)calloc(1024, sizeof(int16_t));
        order2 = (int16_t*)calloc(1024, sizeof(int16_t));
        if (!order1 || !order2) return;
    }

    void makeTunnel()
    {
#ifdef _USE_ASM
        __asm {
            xor     esi, esi
            xor     edi, edi
            mov     ecx, 32000
        next:
            xor     eax, eax
            xor     edx, edx
            xor     ebx, ebx
            mov     edx, yofs[edi]
            shl     edx, 6
            add     ebx, edx
            shl     edx, 2
            add     ebx, edx
            add     ebx, xofs[edi]
            mov     al, vbuff[esi]
            shr     al, 1
            mov     ah, vbuff[esi + ebx]
            shr     ah, 1
            add     al, ah
            mov     vbuff[esi], al
            not     esi
            not     ebx
            mov     al, vbuff[IMAGE_SIZE + esi]
            shr     al, 1
            mov     ah, vbuff[IMAGE_SIZE + esi + ebx]
            shr     ah, 1
            add     al, ah
            mov     vbuff[IMAGE_SIZE + esi], al
            not     esi
            inc     esi
            add     edi, 4
            loop    next
        }
#else
        for (int16_t i = 0; i < IMAGE_MIDY; i++)
        {
            for (int16_t j = 0; j < IMAGE_WIDTH; j++)
            {
                vbuff[i][j] = (vbuff[i][j] + vbuff[i + yofs[i][j]][j + xofs[i][j]]) >> 1;
                vbuff[MAX_HEIGHT - i][MAX_WIDTH - j] = (vbuff[MAX_HEIGHT - i][MAX_WIDTH - j] + vbuff[MAX_HEIGHT - i - yofs[i][j]][MAX_WIDTH - j - xofs[i][j]]) >> 1;
            }
        }
#endif
    }

    void part1()
    {
        int16_t i = 0, j = 0;

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(pike(i, 128, 0, 64, 64));
            rgbpal[i].g = uint8_t(pike(i, 128, 0, 127, 64));
            rgbpal[i].b = uint8_t(pike(i, 192, 0, 127, 32));
        }

        for (i = 0; i < IMAGE_MIDY; i++)
        {
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                const int16_t x = IMAGE_MIDX - j;
                const int16_t y = IMAGE_MIDY - i;
                xofs[i][j] = (x >> 4) + random(3) + (y >> 5) - 1;
                yofs[i][j] = (y >> 4) + random(3) - (x >> 5) - 1;
            }
        }

        rgbpal[255].r = 120;
        rgbpal[255].g = 120;
        rgbpal[255].b = 0;

        frames = 0;
        br1 = 120;
        sat = 1;

        do {
            doBeat(30, 1, 123);
            doBeat(30, 1, 127);

            if (frames <= 120 && br1 < 110) br1 += 10;
            if (frames > 460 && br1 > 0) br1 -= 5;

            br2 = br1;
            sat = beatFunc >> 3;
            br1 += beatFunc;
            setPalette();
            br1 -= beatFunc;

            for (i = 0; i < IMAGE_MIDY; i++)
            {
                const int16_t x = rand() % MAX_WIDTH;
                const int16_t y = rand() % MAX_HEIGHT;
                vbuff[y][x] = 240;
                vbuff[y][x + 1] = 240;
                vbuff[y + 1][x] = 240;
                vbuff[y + 1][x + 1] = 240;
            }

            makeTunnel();
            tunnelBlur();
            memcpy(vmem, vbuff, IMAGE_SIZE);

            switch (frames / 120)
            {
            case 0:
                printString(0, "THE PSYCHO TEAM", 24, 84, 255, 180, 0);
                break;
            case 1:
                printString(0, "presents", 84, 84, 255, 180, 0);
                break;
            case 2:
                printString(0, "specially for", 40, 32, 250, 180, 0);
                printString(0, "MILLENNIUM", 32, 84, 255, 180, 0);
                printString(0, "DEMOPARTY", 120, 120, 255, 180, 0);
                break;
            default:
                printString(0, "16K Intro", 80, 84, 255, 180, 0);
                break;
            }

            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);

            beatFunc = (beatFunc * 7) >> 3;
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 480);
    }

    void makeTexture()
    {
#ifdef _USE_ASM
        __asm {
            xor     ebx, ebx
            lea     edi, vbuff
            mov     edx, IMAGE_HEIGHT
        lp1:
            mov     ecx, IMAGE_WIDTH
        lp2:
            mov     bh, dl
            mov     bl, cl
            shl     bl, 1
            shr     bx, 1
            and     bx, 3FFFh
            mov     al, texture[ebx]
            shl     al, 1
            stosb
            loop    lp2
            dec     edx
            jnz     lp1
        }
#else
        for (int16_t i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (int16_t j = 0; j < IMAGE_WIDTH; j++) vbuff[i][j] = texture[i & 0x7f][j & 0x7f] << 1;
        }
#endif
    }

    void partTitle()
    {
        int16_t i = 0, j = 0;

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i - 160, 0, 127));
            rgbpal[i].g = uint8_t(range(i, 0, 127));
            rgbpal[i].b = uint8_t(range(i - 48, 0, 127));
        }

        memcpy(faces, logoData, sizeof(logoData));
        memcpy(vertices, logoVertices, sizeof(logoVertices));

        int16_t dz = 155;
        deltaZ = 13500;
        frames = 0;

        do {
            br1 = uint8_t(range(frames * 2, 0, 120));
            sat = uint8_t(range((frames - 140) * 3, 0, 60));
            setPalette();
            makeTexture();
            genMatrix(range(frames * 2 - 20, 0, 250), 0, -10);
            deltaZ -= dz;
            if (dz > 0) dz--;
            project(38);
            for (i = 0; i < 38; i++) vertices[i].color = ((scenes[i].y * 3) >> 1) + IMAGE_MIDX;
            drawScene(38, 18, IMAGE_MIDX, IMAGE_MIDY);
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames <= 160);

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i - 128, 0, 100));
            rgbpal[i].g = uint8_t(pike(i, 64, 0, 100, 0));
            rgbpal[i].b = uint8_t(range(i - 48, 0, 127));
        }

        memset(vbuff, 0, IMAGE_SIZE);
        drawScene(38, 18, IMAGE_MIDX, IMAGE_MIDY);

        for (i = 0; i < 4; i++) tunnelBlur();
        drawScene(38, 18, IMAGE_MIDX, IMAGE_MIDY);

        tunnelBlur();
        tunnelBlur();

        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            const int16_t row = i & 0x7f;
            for (j = 0; j < IMAGE_WIDTH; j++)
            {
                const int16_t col = j & 0x7f;
                const uint8_t c = vbuff[i][j];

                if (c < 12) vbuff[i][j] = texture[row][col];
                else if (c < 24) vbuff[i][j] = 60 - (c - 16) * (c - 16);
                else if (c > 64) vbuff[i][j] = 180 - (sinTab[(j * 3 + i * 2) & 0xff] - sinTab[(i * 5) & 0xff] - sinTab[(i * 4 - j * 7) & 0xff]) / 3;
                else vbuff[i][j] = 0;
            }
        }

        flipBuffer();
        frames = 0;

        do {
            doBeat(24, 1, 60);
            br1 = 120 - range(frames - IMAGE_MIDX, 0, 120);
            sat = range(60 - frames * 3, 0, 60) + beatFunc;
            setPalette();
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 280);
    }

    void makeBlob(int32_t x, int32_t y)
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            lea     esi, blobs
            xor     eax, eax
            add     edi, x
            mov     eax, y
            shl     eax, 6
            add     edi, eax
            shl     eax, 2
            add     edi, eax
            mov     dx, 128
        lp1:
            mov     ecx, 128
        lp2:
            mov     al, [esi]
            inc     esi
            shl     al, 1
            add     al, [edi]
            jnc     lp3
            mov     al, 255
        lp3:
            stosb
            loop    lp2
            add     edi, IMAGE_WIDTH - 128
            dec     dx
            jnz     lp1
        }
#else
        for (int16_t i = 0; i < SIZE_128; i++)
        {
            for (int16_t j = 0; j < SIZE_128; j++)
            {
                int16_t col = vbuff[y + i][x + j] + (blobs[i][j] << 1);
                if (col > 255) col = 255;
                vbuff[y + i][x + j] = uint8_t(col);
            }
        }
#endif
    }

    void makeFloor(int32_t my, uint8_t dt, int32_t x, int32_t y, int32_t px, int32_t py)
    {
#ifdef _USE_ASM
        __asm {
            lea     esi, vbuff
            lea     edi, vbuff
            add     edi, 31680
            add     esi, 32000
            mov     eax, my
            mov     ecx, IMAGE_WIDTH
            mul     ecx
            sub     edi, eax
            add     esi, eax
            mov     ebx, x
            mov     edx, y
        lp1:
            mov     eax, edx
            xor     eax, ebx
            shr     eax, 2
            shr     ah, 2
            sub     ah, dt
            jb      lp2
        lp3:
            mov     [edi], ah
            mov     [esi], ah
            add     ebx, px
            add     edx, py
            inc     edi
            inc     esi
            loop    lp1
            jmp     lp4
        lp2: 
            xor     ah, ah
            jmp     lp3
        lp4:
        }
#else
        for (int16_t i = 0; i < IMAGE_WIDTH; i++)
        {
            int32_t val = y;
            val ^= x;
            val >>= 2;
            val = (val & 0x0000FF00) >> 10;
            val -= dt;
            if (val < 0) val = 0;
            vbuff[MAX_MIDY   - my][i] = val;
            vbuff[IMAGE_MIDY + my][i] = val;
            x += px;
            y += py;
        }
#endif
    }

    void part6()
    {
        int16_t i = 0;

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i - 64, 0, 100));
            rgbpal[i].g = uint8_t(range(i, 0, 100));
            rgbpal[i].b = uint8_t(range(i - 32, 0, 120));
        }

        br1 = 0;
        frames = 0;

        do {
            doBeat(16, 1, 60);

            if (frames <= 140 && br1 < 150) br1 += 5;
            if (frames > 800 && br1 > 0) br1 -= 5;

            br2 = br1;
            sat = uint8_t(beatFunc);
            setPalette();

            for (i = 0; i < IMAGE_MIDY; i++)
            {
                uint8_t v = 0;
                const int32_t x1 = (exSin(frames + 256) << 16) / (i + 2);
                const int32_t y1 = (exSin(frames) << 16) / (i + 2);
                const int32_t x2 = y1;
                const int32_t y2 = -x1;
                const int32_t px = (x2 - x1) / IMAGE_WIDTH;
                const int32_t py = (y2 - y1) / IMAGE_WIDTH;
                const int32_t x = x1;
                const int32_t y = y1;

                if (i < 48) v = ((48 - i) * (48 - i)) >> 4;
                else v = 0;

                makeFloor(i, v, x, y, px, py);
            }

            for (i = 1; i <= 9; i++)
            {
                const int16_t cx = 96 + (sinTab[(frames + i * 24) & 0xff] >> 1) + (sinTab[((i << 4) + frames / 3) & 0xff] >> 1);
                const int16_t cy = 32 + (sinTab[((frames << 1) + 100 + i * 20) & 0xff] >> 2);
                makeBlob(cx, cy);
            }

            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 840);
    }

    void makeCopper(int32_t x, int32_t y)
    {
#ifdef _USE_ASM
        __asm {
            lea     esi, vbuff
            mov     eax, y
            shl     eax, 6
            add     esi, eax
            shl     eax, 2
            add     esi, eax
            mov     eax, x
            shr     al, 3
            add     al, 20
            mov     ecx, IMAGE_WIDTH
        next:
            add     [esi], al
            inc     esi
            loop    next
        }
#else
        const uint8_t col = (x >> 3) + 20;
        for (int16_t i = 0; i < IMAGE_WIDTH; i++) vbuff[y][(x + i) % IMAGE_WIDTH] += col;
#endif
    }

    void makeSprings()
    {
        int16_t i = 0, j = 0, w = 0, k = 0;

        do {
            if (0 <= frames && frames <= 480)
            {
                vertices[0].x = sinTab[(i << 3) & 0xff] >> 1;
                vertices[0].y = sinTab[((i << 3) + 64) & 0xff] >> 1;
                vertices[0].z = (i >> 1) - 80;
            }
            else
            {
                j = 100 + sinTab[(i * 6 + (i << 1)) & 0xff] / 3;
                vertices[0].x = j * sinTab[i & 0xff] >> 7;
                vertices[0].y = j * sinTab[(i * 3 + 64) & 0xff] >> 7;
                vertices[0].z = j - 100;
            }

            project(1);
            scenes[1] = scenes[0];
            i++;

            if (0 <= frames && frames <= 480)
            {
                vertices[0].x = sinTab[(i << 3) & 0xff] >> 1;
                vertices[0].y = sinTab[((i << 3) + 64) & 0xff] >> 1;
                vertices[0].z = (i >> 1) - 80;
            }
            else
            {
                j = 100 + sinTab[(i * 6 + i * 2) & 0xff] / 3;
                vertices[0].x = j * sinTab[i & 0xff] >> 7;
                vertices[0].y = j * sinTab[(i * 3 + 64) & 0xff] >> 7;
                vertices[0].z = j - 100;
            }

            project(1);

            for (j = 0; j < 8; j++)
            {
                k = IMAGE_MIDX + ((scenes[0].x * j + scenes[1].x * (8 - j)) >> 3);
                w = IMAGE_MIDY + ((scenes[0].y * j + scenes[1].y * (8 - j)) >> 3);
                vbuff[w][k] = 140 - scenes[0].z;
            }
        } while (i <= IMAGE_WIDTH);
    }

    void makeRecbar(int32_t my, int32_t mx)
    {
#ifdef _USE_ASM
        __asm {
            xor     esi, esi
            mov     eax, my
            shl     eax, 6
            add     esi, eax
            shl     eax, 2
            add     esi, eax
            mov     ecx, mx
        next:
            mov     al, vbuff[esi]
            shr     al, 1
            add     al, 96
            mov     vbuff[esi], al
            not     esi
            mov     al, vbuff[IMAGE_SIZE + esi]
            shr     al, 1
            add     al, 96
            mov     vbuff[IMAGE_SIZE + esi], al
            not     esi
            inc     esi
            loop    next
        }
#else
        for (uint16_t i = 0; i < mx; i++)
        {
            vbuff[my][i] = (vbuff[my][i] >> 1) + 96;
            vbuff[MAX_HEIGHT - my][MAX_WIDTH - i] = (vbuff[MAX_HEIGHT - my][MAX_WIDTH - i] >> 1) + 96;
        }
#endif
    }

    void part5()
    {
        int16_t i = 0, j = 0;
        int16_t k = 0, dpos = 0, tpos = 0, ddz = 0;
        int16_t pos[16] = { 0 }, wid[16] = { 0 }, spd[16] = { 0 };

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i - 128, 0, 100));
            rgbpal[i].g = uint8_t(range(i - 32, 0, 120));
            rgbpal[i].b = uint8_t(range(i + 32, 0, 120));
        }

        for (i = 0; i < 16; i++)
        {
            pos[i] = random(1000);
            wid[i] = 8 + random(32);
            spd[i] = 1 + random(3);
            if (i > 7) spd[i] = -spd[i];
        }

        br1 = 0;
        frames = 0;
        ddz = 1024;
        deltaZ = 21000;

        do {
            doBeat(800, 1, 60);

            if (frames <= 120 && br1 < 120) br1 += 5;
            if (900 < frames && br1 > 0) br1 -= 5;

            br2 = br1;
            sat = beatFunc / 48;

            memset(vbuff, 0, IMAGE_SIZE);
            setPalette();

            for (i = 0; i < 16; i++)
            {
                k = pos[i] >> 2;
                dpos = wid[i];
                for (j = k; j <= k + dpos; j++)
                {
                    if (j >= 0 && j <= MAX_HEIGHT) makeCopper(i, j);
                }
                pos[i] = (pos[i] + spd[i]) & 0x3FF;
            }

            if (((0 <= frames && frames <= 120) || frames >= 484) && ddz > 0)
            {
                deltaZ -= ddz;
                ddz -= 32;
            }

            if ((460 < frames && frames < 480) && ddz < 1024)
            {
                ddz += 32;
                deltaZ += ddz;
            }

            if (frames > 900 && ddz < 1024)
            {
                deltaZ -= ddz;
                ddz -= 32;
            }

            deltaZ -= beatFunc;
            genMatrix(frames, frames >> 1, (frames * 3) >> 1);
            makeSprings();
            deltaZ += beatFunc;

            tpos = range((frames << 1) - 64, 0, IMAGE_HEIGHT) - range((frames << 1) - 1600, 0, IMAGE_HEIGHT);
            if (tpos > 0)
            {
                for (i = 0; i < 32; i++)
                {
                    dpos = tpos - ((i * i) >> 4);
                    if (dpos > 0) makeRecbar(i, dpos);
                }
            }

            if (154 < frames && frames < 460)
            {
                printString(1, "CODE:", 32, 2, 0, 224, 0);
                printString(1, "COOLER", 192, 172, 0, 224, 0);
            }

            if (486 < frames && frames < 800)
            {
                printString(1, "MUSIC:", 24, 2, 0, 224, 0);
                printString(1, "DJ MoHaX", 170, 172, 0, 224, 0);
            }

            tunnelBlur();
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 920);
    }

    void genWheels(int16_t r, int16_t n)
    {
        int16_t i = 0, j = 0;

        vertices[0].x = 0;
        vertices[0].y = 0;
        vertices[0].z = 0;

        for (i = 1; i <= n + 1; i++)
        {
            j = (i << 8) / n;
            vertices[i].x = sinTab[(j + 64) & 0xff] * r / 80;
            vertices[i].y = sinTab[j & 0xff] * r / 80;
            vertices[i].z = 0;

            j = j - 2 + ((i & 1) << 2);
            vertices[i + n + 1].x = sinTab[(j + 64) & 0xff] * r >> 6;
            vertices[i + n + 1].y = sinTab[j & 0xff] * r >> 6;
            vertices[i + n + 1].z = 0;
        }

        for (i = 0; i < n; i++)
        {
            faces[i].v1 = 0;
            faces[i].v3 = i + 2;
            faces[i].v2 = i + 1;
            faces[i].color = 16;

            if (!(i & 1))
            {
                faces[i + n].v1 = n + i + 3;
                faces[i + n].v3 = n + i + 2;
                faces[i + n].v2 = i + 1;
                faces[i + n].color = 16;

                faces[i + n + 1].v1 = n + i + 3;
                faces[i + n + 1].v3 = i + 1;
                faces[i + n + 1].v2 = i + 2;
                faces[i + n + 1].color = 16;
            }
        }
    }

    void part4()
    {
        int16_t i = 0;

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(120 - i, 24, 120));
            rgbpal[i].g = uint8_t(range(100 - i, 32, 120));
            rgbpal[i].b = uint8_t(range(i - 128, 0, 120));
        }

        frames = 0;
        sat = 0;
        br1 = br2 = 0;

        do {
            if (frames <= 120 && br1 < 120) br1 += 5;
            if (900 < frames && br1 > 0) br1 -= 3;

            setPalette();
            memset(vbuff, 0, IMAGE_SIZE);

            deltaZ = 4096;
            genMatrix((frames << 2) / 3, 0, 0);
            genWheels(100, 24);
            drawScene(50, 48, 100, 120);

            genMatrix(frames + 10, 0, 0);
            genWheels(120, 32);
            drawScene(66, 64, 224, 150);

            genMatrix((-frames << 1) + 5, 0, 0);
            genWheels(64, 16);
            drawScene(34, 32, 170, 80);

            genMatrix((-frames << 3) / 3 + 5, 0, 0);
            genWheels(45, 12);
            drawScene(26, 24, 40, 160);

            deltaZ = 7200;
            genMatrix(frames / 3, frames, frames >> 1);

            memcpy(faces, baseFigure, sizeof(baseFigure));
            memcpy(vertices, baseVertices, sizeof(baseVertices));

            for (i = 0; i < 4; i++)
            {
                vertices[i].color = 160 + i * 30;
                if (frames < 450) faces[i].color = 175 + i * 20;
                else faces[i].color = 0;
            }

            drawScene(4, 4, IMAGE_MIDX, IMAGE_MIDY + (128 - ((3 * frames / 5) & 0xff) - sinTab[(3 * frames / 5) & 0xff]));
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 950);
    }

    int16_t getValues(int16_t a, int16_t b)
    {
        int16_t p = 0;
        if (frames < 256) p = 48;
        else p = 176 + (frames >> 1);
        if (a < 4 && b < 4) return 64 + sinTab[p & 0xff] / 3;
        return 64 + sinTab[(p + 64) & 0xff] / 3;
    }

    void makePlasma(int32_t j, int32_t v, int32_t u)
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            xor     esi, esi
            add     esi, j
            mov     ebx, IMAGE_HEIGHT
        lp1:
            mov     ecx, IMAGE_WIDTH
            push    esi
        lp2:
            and     esi, 0FFh
            mov     al, sinTab[esi]
            add     al, 80
            shr     al, 3
            add     esi, v
            add     [edi], al
            inc     edi
            loop    lp2
            pop     esi
            add     esi, u
            dec     ebx
            jnz     lp1
        }
#else
        for (int16_t k = 0; k < IMAGE_HEIGHT; k++)
        {
            int16_t s = j;
            for (int16_t i = 0; i < IMAGE_WIDTH; i++)
            {
                vbuff[k][i] += (sinTab[s & 0xff] + 80) >> 3;
                s += v;
            }
            j += u;
        }
#endif
    }

    void part4a()
    {
        const int8_t a[6] = { 1, 1, 5, 11, 6, -2 };
        const int8_t b[6] = { -1, 3, 2, 7, 13, -8 };
        const int8_t c[6] = { 3, 0, -1, 2, 3, 0 };

        int32_t koef = 0, pos = 0;
        int16_t mask[IMAGE_HEIGHT] = { 0 };
        int16_t u = 0, v = 0;
        int16_t vcnt = 0, fcnt = 0;
        int16_t i = 0, j = 0, x = 0, y = 0;

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(pike(i, 128, 0, 60, 100));
            rgbpal[i].g = uint8_t(pike(i, 96, 90, 0, 110));
            rgbpal[i].b = uint8_t(pike(i, 48, 40, 60, 0));
        }

        firstIndex = 128;
        memset(rgbpal, 0, 3);

        frames = 0;
        br1 = br2 = 0;

        do {
            doBeat(20, 1, 70);

            if (frames <= 120 && br1 < 120)
            {
                br1 += 2;
                br2 += 2;
            }

            if (frames > 480 && br1 > 0)
            {
                br1 -= 2;
                br2 -= 2;
            }

            br1 += beatFunc;
            setPalette();
            br1 -= beatFunc;

            memset(mask, 0, sizeof(mask));

            for (i = 0; i < 16; i++)
            {
                j = (i << 6) - (frames & 0xff);
                if (j >= 0 && j <= 255)
                {
                    koef = (exSin(j) << 12) / exSin(j + 256);
                    pos = koef << 2;
                    y = 199;
                    u = i + 1;

                    do {
                        x = pos >> 12;
                        if (x > MAX_WIDTH) x = MAX_WIDTH;
                        if (x < mask[y]) break;
                        memset(&vbuff[y][mask[y]], (i << 5) & 63, intptr_t(x) - mask[y] + 1);
                        mask[y] = x;
                        pos += koef;
                        y--;
                    } while (y);
                }
            }

            for (i = 0; i < IMAGE_HEIGHT; i++)
            {
                if (mask[i] < IMAGE_WIDTH) memset(&vbuff[i][mask[i]], (u << 5) & 63, intptr_t(IMAGE_WIDTH) - mask[i]);
            }

            for (i = 0; i < 5; i++)
            {
                j = frames * c[i];
                u = a[i];
                v = b[i];
                makePlasma(j, v, u);
            }

            deltaZ = 0x1800;
            genMatrix(frames / 3, frames, frames >> 1);

            memcpy(faces, baseFigure, sizeof(baseFigure));
            memcpy(vertices, baseVertices, sizeof(baseVertices));

            for (i = 0; i < 4; i++) vertices[i].color = 255;

            vcnt = 4;
            fcnt = 4;

            for (i = 0; i < 4; i++)
            {
                vertices[vcnt].x = ((vertices[faces[i].v1].x + vertices[faces[i].v2].x) * getValues(faces[i].v1, faces[i].v2)) >> 7;
                vertices[vcnt].y = ((vertices[faces[i].v1].y + vertices[faces[i].v2].y) * getValues(faces[i].v1, faces[i].v2)) >> 7;
                vertices[vcnt].z = ((vertices[faces[i].v1].z + vertices[faces[i].v2].z) * getValues(faces[i].v1, faces[i].v2)) >> 7;

                vertices[vcnt + 1].x = ((vertices[faces[i].v1].x + vertices[faces[i].v3].x) * getValues(faces[i].v1, faces[i].v3)) >> 7;
                vertices[vcnt + 1].y = ((vertices[faces[i].v1].y + vertices[faces[i].v3].y) * getValues(faces[i].v1, faces[i].v3)) >> 7;
                vertices[vcnt + 1].z = ((vertices[faces[i].v1].z + vertices[faces[i].v3].z) * getValues(faces[i].v1, faces[i].v3)) >> 7;

                vertices[vcnt + 2].x = ((vertices[faces[i].v3].x + vertices[faces[i].v2].x) * getValues(faces[i].v3, faces[i].v2)) >> 7;
                vertices[vcnt + 2].y = ((vertices[faces[i].v3].y + vertices[faces[i].v2].y) * getValues(faces[i].v3, faces[i].v2)) >> 7;
                vertices[vcnt + 2].z = ((vertices[faces[i].v3].z + vertices[faces[i].v2].z) * getValues(faces[i].v3, faces[i].v2)) >> 7;

                faces[fcnt].v1 = faces[i].v1;
                faces[fcnt].v2 = vcnt;
                faces[fcnt].v3 = vcnt + 1;
                faces[fcnt].color = 0;

                faces[fcnt + 1].v1 = faces[i].v2;
                faces[fcnt + 1].v2 = vcnt + 2;
                faces[fcnt + 1].v3 = vcnt;
                faces[fcnt + 1].color = 0;

                faces[fcnt + 2].v1 = faces[i].v3;
                faces[fcnt + 2].v2 = vcnt + 1;
                faces[fcnt + 2].v3 = vcnt + 2;
                faces[fcnt + 2].color = 0;

                faces[i].v1 = vcnt;
                faces[i].v2 = vcnt + 2;
                faces[i].v3 = vcnt + 1;
                faces[i].color = 0;

                fcnt += 3;
                vcnt += 3;
            }

            for (i = 0; i < 16; i++)
            {
                vertices[vcnt].x = ((vertices[faces[i].v1].x + vertices[faces[i].v2].x) * getValues(faces[i].v1, faces[i].v2)) >> 7;
                vertices[vcnt].y = ((vertices[faces[i].v1].y + vertices[faces[i].v2].y) * getValues(faces[i].v1, faces[i].v2)) >> 7;
                vertices[vcnt].z = ((vertices[faces[i].v1].z + vertices[faces[i].v2].z) * getValues(faces[i].v1, faces[i].v2)) >> 7;

                vertices[vcnt + 1].x = ((vertices[faces[i].v1].x + vertices[faces[i].v3].x) * getValues(faces[i].v1, faces[i].v3)) >> 7;
                vertices[vcnt + 1].y = ((vertices[faces[i].v1].y + vertices[faces[i].v3].y) * getValues(faces[i].v1, faces[i].v3)) >> 7;
                vertices[vcnt + 1].z = ((vertices[faces[i].v1].z + vertices[faces[i].v3].z) * getValues(faces[i].v1, faces[i].v3)) >> 7;

                vertices[vcnt + 2].x = ((vertices[faces[i].v3].x + vertices[faces[i].v2].x) * getValues(faces[i].v3, faces[i].v2)) >> 7;
                vertices[vcnt + 2].y = ((vertices[faces[i].v3].y + vertices[faces[i].v2].y) * getValues(faces[i].v3, faces[i].v2)) >> 7;
                vertices[vcnt + 2].z = ((vertices[faces[i].v3].z + vertices[faces[i].v2].z) * getValues(faces[i].v3, faces[i].v2)) >> 7;

                faces[fcnt].v1 = faces[i].v1;
                faces[fcnt].v2 = vcnt;
                faces[fcnt].v3 = vcnt + 1;
                faces[fcnt].color = 0;

                faces[fcnt + 1].v1 = faces[i].v2;
                faces[fcnt + 1].v2 = vcnt + 2;
                faces[fcnt + 1].v3 = vcnt;
                faces[fcnt + 1].color = 0;

                faces[fcnt + 2].v1 = faces[i].v3;
                faces[fcnt + 2].v2 = vcnt + 1;
                faces[fcnt + 2].v3 = vcnt + 2;
                faces[fcnt + 2].color = 0;

                faces[i].v1 = vcnt;
                faces[i].v2 = vcnt + 2;
                faces[i].v3 = vcnt + 1;
                faces[i].color = 0;

                fcnt += 3;
                vcnt += 3;
            }

            for (i = 0; i < vcnt; i++) vertices[i].color = 191 - sinTab[vertices[i].z & 0xff];

            drawScene(vcnt, fcnt, IMAGE_MIDX, IMAGE_MIDY);
            if (frames & 32) printString(1, "VOTE!", 224, 172, 192 + beatFunc, 240 + beatFunc, 0);

            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 550);

        firstIndex = 255;
    }

    void makeGreets(int16_t acos, int16_t asin)
    {
#ifdef _USE_ASM
        __asm {
            xor     edi, edi
            xor     esi, esi
            xor     dx, dx
            xor     bx, bx
            mov     ch, IMAGE_MIDY
            add     esi, MAX_WIDTH
        lp1:
            push    dx
            push    bx
            mov     cl, IMAGE_MIDX
        lp2:
            mov     al, bh
            xor     al, dh
            shr     al, 2
            mov     vbuff[edi], al
            mov     vbuff[esi], al
            add     al, 40h
            not     edi
            not     esi
            mov     vbuff[IMAGE_SIZE + edi], al
            mov     vbuff[IMAGE_SIZE + esi], al
            not     edi
            not     esi
            add     edi, 2
            sub     esi, 2
            add     bx, acos
            add     dx, asin
            dec     cl
            jnz     lp2
            pop     bx
            pop     dx
            add     bx, asin
            sub     dx, acos
            add     edi, IMAGE_WIDTH
            add     esi, IMAGE_WIDTH * 3
            dec     ch
            jnz     lp1
        }
#else
        int16_t ax = 0, bx = 0;

        for (int16_t j = 0; j < IMAGE_HEIGHT; j += 2)
        {
            int16_t cx = ax;
            int16_t dx = bx;
            for (int16_t i = 0; i < IMAGE_WIDTH; i += 2)
            {
                uint8_t col = (cx >> 8) ^ (dx >> 8);
                col >>= 2;
                vbuff[j][i] = col;
                vbuff[j][MAX_WIDTH - i] = col;
                col += 0x40;
                vbuff[MAX_HEIGHT - j][i] = col;
                vbuff[MAX_HEIGHT - j][MAX_WIDTH - i] = col;
                cx += acos;
                dx += asin;
            }
            ax += asin;
            bx -= acos;
        }
#endif
    }

    void part10()
    {
        for (int16_t i = 0; i < 256; i++)
        {
            rgbpal[i].r = (i >> 1) & 0x7f;
            rgbpal[i].g = i & 0x7f;
            rgbpal[i].b = (i << 1) & 0x7f;
        }

        frames = 0;
        br1 = sat = 0;

        do {
            if (frames <= 80 && br1 < 120) br1 += 5;
            if (frames > 120 && br1 > 0) br1 -= 5;

            const int16_t asin = exSin(frames);
            const int16_t acos = exSin(frames + 256);

            setPalette();
            makeGreets(acos, asin);
            printString(1, "And now", 96, 70, 255, 254, 0);
            printString(1, "some greetz...", 32, 100, 255, 254, 0);
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 140);
    }

    void makeIntro0(int16_t acos, int16_t asin)
    {
        memset(vbuff, 255, IMAGE_SIZE);

#ifdef _USE_ASM
        __asm {
            xor     edi, edi
            xor     esi, esi
            mov     ecx, IMAGE_HEIGHT
            xor     dx, dx
            xor     bx, bx
            add     esi, MAX_WIDTH
        lp1:
            push    dx
            push    bx
            mov     ax, IMAGE_WIDTH
        lp2:
            push    ax
            mov     al, bh
            xor     al, dh
            and     vbuff[edi], al
            and     vbuff[esi], al
            not     edi
            not     esi
            and     vbuff[IMAGE_SIZE + edi], al
            and     vbuff[IMAGE_SIZE + esi], al
            not     edi
            not     esi
            inc     edi
            dec     esi
            add     bx, acos
            add     dx, asin
            pop     ax
            dec     ax
            jnz     lp2
            pop     bx
            pop     dx
            add     bx, asin
            sub     dx, acos
            add     esi, IMAGE_WIDTH * 2
            loop    lp1
        }
#else
        int16_t ax = 0, bx = 0;

        for (int16_t j = 0; j < IMAGE_HEIGHT; j++)
        {
            int16_t cx = ax;
            int16_t dx = bx;
            for (int16_t i = 0; i < IMAGE_WIDTH; i++)
            {
                uint8_t col = (cx >> 8) ^ (dx >> 8);
                vbuff[j][i] &= col;
                vbuff[j][MAX_WIDTH - i] &= col;
                vbuff[MAX_HEIGHT - j][i] &= col;
                vbuff[MAX_HEIGHT - j][MAX_WIDTH - i] &= col;
                cx += acos;
                dx += asin;
            }
            ax += asin;
            bx -= acos;
        }
#endif
    }

    void makeIntro1(int16_t acos, int16_t asin)
    {
        memset(vbuff, 0, IMAGE_SIZE);

#ifdef _USE_ASM
        __asm {
            xor     edi, edi
            xor     esi, esi
            mov     ecx, IMAGE_HEIGHT
            xor     dx, dx
            xor     bx, bx
            add     esi, MAX_WIDTH
        lp1:
            push    dx
            push    bx
            mov     ax, IMAGE_WIDTH
        lp2:
            push    ax
            mov     al, bh
            xor     al, dh
            shr     al, 1
            xor     vbuff[edi], al
            xor     vbuff[esi], al
            not     edi
            not     esi
            xor     vbuff[IMAGE_SIZE + edi], al
            xor     vbuff[IMAGE_SIZE + esi], al
            not     edi
            not     esi
            inc     edi
            dec     esi
            add     bx, acos
            add     dx, asin
            pop     ax
            dec     ax
            jnz     lp2
            pop     bx
            pop     dx
            add     bx, asin
            sub     dx, acos
            add     esi, IMAGE_WIDTH * 2
            loop    lp1
        }
#else
        int16_t ax = 0, bx = 0;

        for (int16_t j = 0; j < IMAGE_HEIGHT; j++)
        {
            int16_t cx = ax;
            int16_t dx = bx;
            for (int16_t i = 0; i < IMAGE_WIDTH; i++)
            {
                uint8_t col = (cx >> 8) ^ (dx >> 8);
                col >>= 1;
                vbuff[j][i] ^= col;
                vbuff[j][MAX_WIDTH - i] ^= col;
                vbuff[MAX_HEIGHT - j][i] ^= col;
                vbuff[MAX_HEIGHT - j][MAX_WIDTH- i] ^= col;
                cx += acos;
                dx += asin;
            }
            ax += asin;
            bx -= acos;
        }
#endif
    }

    void makeIntro2(int16_t acos, int16_t asin)
    {
        memset(vbuff, 0, IMAGE_SIZE);

#ifdef _USE_ASM
        __asm {
            xor     edi, edi
            xor     esi, esi
            xor     dx, dx
            xor     bx, bx
            add     esi, MAX_WIDTH
            mov     ecx, IMAGE_HEIGHT
        lp1:
            push    dx
            push    bx
            mov     ax, IMAGE_WIDTH
        lp2:
            push    ax
            mov     al, bh
            xor     al, dh
            shr     al, 3
            add     vbuff[edi], al
            add     vbuff[esi], al
            not     edi
            not     esi
            add     vbuff[IMAGE_SIZE + edi], al
            add     vbuff[IMAGE_SIZE + esi], al
            not     edi
            not     esi
            inc     edi
            dec     esi
            add     bx, acos
            add     dx, asin
            pop     ax
            dec     ax
            jnz     lp2
            pop     bx
            pop     dx
            add     bx, asin
            sub     dx, acos
            add     esi, IMAGE_WIDTH * 2
            loop    lp1
        }
#else
        int16_t ax = 0, bx = 0;

        for (int16_t j = 0; j < IMAGE_HEIGHT; j++)
        {
            int16_t cx = ax;
            int16_t dx = bx;
            for (int16_t i = 0; i < IMAGE_WIDTH; i++)
            {
                uint8_t col = (cx >> 8) ^ (dx >> 8);
                col >>= 3;
                vbuff[j][i] += col;
                vbuff[j][MAX_WIDTH - i] += col;
                vbuff[MAX_HEIGHT - j][i] += col;
                vbuff[MAX_HEIGHT - j][MAX_WIDTH - i] += col;
                cx += acos;
                dx += asin;
            }
            ax += asin;
            bx -= acos;
        }
#endif
    }

    void part11()
    {
        int16_t i = 0, j = 0, k = 0;
        int16_t m = 0, asin = 0, acos = 0;

        const char* const greets[] = {
            "Accept Corp.",
            "ANTARES",
            "FENOMEN",
            "FREEART",
            "FishBone",
            "Infinite",
            "MonSteR",
            "SkyProject",
            "Perforation",
            "PHG",
            "PROXiUM",
            "T-Rex"
        };

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i - 128, 0, 127));
            rgbpal[i].g = uint8_t(pike(i, 160, 20, 120, 120));
            rgbpal[i].b = uint8_t(pike(i, 64, 0, 100, 60));
        }

        br1 = 0;
        frames = 0;

        do {
            doBeat(30, 0, 60);

            if (frames <= 120 && br1 < 120) br1 += 5;
            if (frames > 1420 && br1 > 0) br1 -= 5;

            br2 = br1;
            sat = uint8_t(beatFunc);
            setPalette();

            m = frames / 480;
            switch (m)
            {
            case 0:
                asin = exSin(frames);
                acos = exSin(frames + 256);
                makeIntro0(acos, asin);
                break;
            case 1:
                asin = exSin(frames);
                acos = exSin(frames + 256);
                makeIntro1(acos, asin);
                break;
            case 2:
                asin = exSin(frames);
                acos = exSin(frames + 256);
                makeIntro2(acos, asin);
                break;
            default:
                break;
            }

            if (frames < 240 + 480 * m)
            {
                k = (frames / 60) % 4;
                for (i = 0; i <= k; i++)
                {
                    j = int16_t(160 - int16_t(strlen(greets[i + (m << 2)])) * 9);
                    printString(1, greets[i + (m << 2)], j, i * 40 + 24, 96, 255 - ((k - i) << 5), 1);
                }
            }
            else
            {
                k = (frames / 60) - (m << 3) - 4;
                for (i = k + 1; i <= 3; i++)
                {
                    j = int16_t(160 - int16_t(strlen(greets[i + (m << 2)])) * 9);
                    printString(1, greets[i + (m << 2)], j, i * 40 + 24, 96, 255 - ((i - k - 1) << 5), 1);
                }
            }

            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 1440);
    }

    void part2()
    {
        int16_t x = 0, y = 0;
        int16_t i = 0, cnt = 0;

        for (i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i, 0, 127));
            rgbpal[i].g = uint8_t(range(i - 48, 0, 120));
            rgbpal[i].b = uint8_t(pike(i, 32, 0, 32, 0) + pike(i, 128, 0, 0, 200));
        }

        memcpy(faces, baseFigure, sizeof(baseFigure));
        memcpy(vertices, baseVertices, sizeof(baseVertices));

        memset(vbuff, 0, IMAGE_SIZE);
        flipBuffer();

        br1 = br2 = 0;
        sat = 0;
        cnt = 0;
        frames = -800;

        do {
            if (frames <= -680 && br1 < 120) br1 += 4;
            if (frames > 600 && sat < 60) sat += 1;

            setPalette();

            if (frames < 0)
            {
                x = 160 + sinTab[(frames / 3) & 0xff];
                y = 80 - (sinTab[(frames + 60) & 0xff] >> 1);

                for (i = 0; i < 4; i++)
                {
                    particles[cnt].cx = (x << 5) + random(100);
                    particles[cnt].cy = (y << 5) + random(100);
                    particles[cnt].dx = random(32767) - 16383;
                    particles[cnt].dy = random(32767) - 16383;
                    particles[cnt].life = 10 + random(400);
                    cnt++;
                }
            }
            else
            {
                deltaZ = 0x3A00;
                genMatrix(frames, frames << 1, frames << 2);
                drawScene(4, 4, IMAGE_WIDTH - (frames >> 1) + sinTab[(frames + 40) & 0xff], IMAGE_MIDY + sinTab[frames & 0xff]);
            }

            i = 0;
            do {
                particles[i].life--;
                if (particles[i].life <= 0)
                {
                    particles[i] = particles[cnt - 1];
                    cnt--;
                }
                else
                {
                    x = particles[i].cx >> 5;
                    y = particles[i].cy >> 5;
                    vbuff[y][x] = 64 + (particles[i].life >> 1);
                    particles[i].cx += (particles[i].dx >> 8);
                    particles[i].cy += (particles[i].dy >> 8);
                    particles[i].dy += 256;
                    particles[i].dx -= (particles[i].dx >> 6);
                    particles[i].dy -= (particles[i].dy >> 6);
                    if (particles[i].cx < 200 || particles[i].cx > 10100) particles[i].dx = -particles[i].dx;
                    if (particles[i].cy > 6200) particles[i].dy = -particles[i].dy;
                }
                i++;
            } while (i < cnt);

            fireBlur();
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && frames < 660);
    }

    void finalPart()
    {
        for (int16_t i = 0; i < 256; i++)
        {
            rgbpal[i].r = uint8_t(range(i - 160, 0, 127));
            rgbpal[i].g = uint8_t(range(i, 0, 127));
            rgbpal[i].b = uint8_t(range(i - 48, 0, 127));
        }

        makeTexture();
        printString(1, "WIN32 SDL2", 64, 64, 192, 96, 0);
        printString(1, "16K Intro!", 128, 96, 192, 96, 0);
        printString(1, "(The End)", 80, 160, 192, 64, 0);

        frames = br2 = 0;

        do {
            sat = uint8_t(range(60 - (frames << 1), 0, 60));
            br1 = uint8_t(range(400 - frames, 0, 100));
            setPalette();
            flipBuffer();
            delay(FPS_90);
            frames++;
        } while (!finished(SDL_SCANCODE_RETURN) && br1);
    }

    void run()
    {
        initialize();
        part1();
        partTitle();
        part6();
        part5();
        part4();
        part4a();
        part10();
        part11();
        part2();
        finalPart();
        cleanup();
        free(order1);
        free(order2);
    }
}

namespace textScrolling {
    #define FONT_SIZE   8192
    #define OFS 	    80
    #define AMP 	    8
    #define LEN 	    256
    #define SIZE 	    2
    #define CURVE 	    2
    #define SCR_WIDTH   (309 / SIZE)
    #define SCR_HEIGHT  8
    #define SPEED 	    -1

    const uint8_t mask[] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    const char* text = "FONT SCROLLING - (c) 1998 by Nguyen Ngoc Van ";

    uint8_t fbuff[FONT_SIZE] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    uint16_t sint[LEN] = { 0 };
    int16_t xpos[SCR_WIDTH][SCR_HEIGHT] = { 0 };
    int16_t ypos[SCR_WIDTH][SCR_HEIGHT] = { 0 };
    uint8_t bitmap[SCR_WIDTH][SCR_HEIGHT] = { 0 };

    void loadFont()
    {
        FILE* fp = fopen("assets/font8x8.fnt", "rb");
        fread(fbuff, 1, FONT_SIZE, fp);
        fclose(fp);
    }

    void scrollText()
    {
        uint16_t pos = 0;
        uint16_t x = 0, y = 0, j = 0;
        const uint16_t len = uint16_t(strlen(text));
        
        do {
            const uint16_t c = text[pos];

            for (uint16_t i = 0; i < SCR_HEIGHT; i++)
            {
                memcpy(&bitmap[0], &bitmap[1], SCR_HEIGHT * SCR_WIDTH);

                for (y = 0; y < SCR_HEIGHT; y++)
                {
                    if (mask[i] & fbuff[SCR_HEIGHT * c + y]) bitmap[SCR_WIDTH - 1][y] = 15;
                    else bitmap[SCR_WIDTH - 1][y] = 0;
                }

                for (x = 0; x < SCR_WIDTH; x++)
                {
                    for (y = 0; y < SCR_HEIGHT; y++)
                    {
                        vmem[ypos[x][y]][xpos[x][y]] = vbuff[ypos[x][y]][xpos[x][y]];
                        xpos[x][y] = SIZE * x + sint[(x + y) % LEN] - OFS;
                        ypos[x][y] = SIZE * y + sint[(j + x + CURVE * y) % LEN];
                        
                        if (bitmap[x][y]) vmem[ypos[x][y]][xpos[x][y]] = bitmap[x][y];
                        else vmem[ypos[x][y]][xpos[x][y]] = vbuff[ypos[x][y]][xpos[x][y]];
                    }
                }

                j = (j + SPEED) % LEN;
                renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
                delay(FPS_90);
                readKeys();
                if (keyDown(SDL_SCANCODE_RETURN)) return;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }

            pos++;
            if (pos >= len) pos = 0;
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }

    void run()
    {
        RGB pal[256] = { 0 };
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Text-Scrolling")) return;
        if (!loadPNG(vbuff[0], pal, "assets/friend.png")) return;
        setPalette(pal);
        memcpy(vmem, vbuff, IMAGE_SIZE);
        for (uint16_t i = 0; i < LEN; i++) sint[i] = uint16_t(sin(4 * M_PI * i / LEN) * AMP + OFS);
        loadFont();
        scrollText();
    }
}

namespace fastShowBMP {
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = {0};

    void setRGB(const uint8_t* pal)
    {
        int16_t i = 0, k = 0;
        RGB rgb[256] = { 0 };
        
        for (i = 0; i < 256; i++)
        {
            k = i << 2;
            rgb[i].r = pal[k + 2];
            rgb[i].g = pal[k + 1];
            rgb[i].b = pal[k + 0];
        }
        setPalette(rgb);
    }

    int16_t loadBMP(const char* fname)
    {
        uint8_t pal[1024] = { 0 };

        FILE* fp = fopen(fname, "rb");
        if (!fp) return 0;

        fseek(fp, 54, SEEK_SET);
        fread(pal, 1024, 1, fp);
        fread(vbuff, IMAGE_SIZE, 1, fp);
        fclose(fp);
        setRGB(pal);
        return 1;
    }

    void flipScreen()
    {
        uint8_t* vmem = (uint8_t*)getDrawBuffer();
#ifdef _USE_ASM
        __asm {
            mov     edi, vmem
            lea     esi, vbuff
            add     esi, IMAGE_SIZE - IMAGE_WIDTH
            mov     ebx, IMAGE_HEIGHT
        next:
            mov     ecx, IMAGE_WIDTH >> 2
            rep	    movsd
            sub     esi, IMAGE_WIDTH << 1
            dec     ebx
            jnz     next
        }
#else
        uint8_t* pbuff = (uint8_t*)vbuff[MAX_HEIGHT];
        for (int16_t i = 0; i < IMAGE_HEIGHT; i++)
        {
            memcpy(vmem, pbuff, IMAGE_WIDTH);
            pbuff -= IMAGE_WIDTH;
            vmem += IMAGE_WIDTH;
        }
#endif
        render();
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fast BMP Show")) return;
        if (!loadBMP("assets/face.bmp")) return;
        flipScreen();
        while (!finished(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace EMS {
    uint8_t vbuff[IMAGE_SIZE] = { 0 };
    uint8_t* vmem = NULL;
    uint8_t* handle[15] = { 0 };

    void RAM2EMS(const uint8_t* buff, uint8_t* tmem)
    {
        memcpy(tmem, buff, IMAGE_SIZE);
    }

    void EMS2RAM(uint8_t* tmem, const uint8_t* buff)
    {
        memcpy(tmem, buff, IMAGE_SIZE);
    }

    void EMSFree(uint8_t* tmem)
    {
        free(tmem);
        tmem = NULL;
    }

    void run()
    {
        int16_t i = 0;
        RGB rgb[256] = { 0 };
        uint8_t pal[768] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "DOS-EMS Simulation")) return;
        FILE *fp = fopen("assets/mickey.dat", "rb");
        if (!fp) return;

        fread(pal, 768, 1, fp);
        for (i = 0; i < 15; i++)
        {
            handle[i] = (uint8_t*)calloc(IMAGE_SIZE, 1);
            if (!handle[i])
            {
                fprintf(stderr, "Not enough memory!\n");
                cleanup();
                exit(1);
            }
            fread(vbuff, 1, IMAGE_SIZE, fp);
            RAM2EMS(vbuff, handle[i]);
        }

        fclose(fp);
        convertPalette(pal, rgb);
        setPalette(rgb);

        i = 0;
        vmem = (uint8_t*)getDrawBuffer();

        while (!finished(SDL_SCANCODE_RETURN)) 
        {
            EMS2RAM(vmem, handle[i]);
            render();
            delay(FPS_90);
            if (i++ >= 14) i = 0;
        }

        cleanup();
        for (i = 0; i < 15; i++) EMSFree(handle[i]);
    }
}

namespace fillterEffect {
    RGB         pal[SIZE_256] = { 0 };
    uint8_t     gray[SIZE_256] = { 0 };
    uint8_t     dbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vbuff1[IMAGE_HEIGHT][IMAGE_HEIGHT] = { 0 };
    uint8_t     vbuff2[IMAGE_HEIGHT][IMAGE_HEIGHT] = { 0 };
    
    void drawPicture()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, dbuff
            lea     esi, vbuff2
            mov     ebx, IMAGE_HEIGHT
        again:
            mov     ecx, IMAGE_HEIGHT >> 2
            rep     movsd
            add     edi, IMAGE_WIDTH - IMAGE_HEIGHT
            dec     ebx
            jnz     again
        }
#else
        for (int16_t i = 0; i < IMAGE_HEIGHT; i++) memcpy(dbuff[i], vbuff2[i], IMAGE_HEIGHT);
#endif
    }

    void calcGray()
    {
        for (int16_t i = 0; i < 256; i++) gray[i] = uint8_t(pal[i].r * 0.299 + pal[i].g * 0.587 + pal[i].b * 0.114);
    }

    void shear()
    {
        int16_t x = 0, y = 0, r = 0;
        int16_t yt[IMAGE_HEIGHT] = { 0 };

        for (x = 0; x < IMAGE_HEIGHT; x++)
        {
            if (random(256) < 128) r--; else r++;
            yt[x] = r;
        }

        for (y = 0; y < IMAGE_HEIGHT; y++)
        {
            if (random(256) < 128) r--; else r++;
            for (x = 0; x < IMAGE_HEIGHT; x++)
            {
                const int16_t dx = x + r;
                const int16_t dy = y + yt[x];
                if (dx >= 0 && dx <= MAX_HEIGHT && dy >= 0 && dy <= MAX_HEIGHT) vbuff2[x][y] = vbuff1[dx][dy];
                else vbuff2[x][y] = 0;
            }
        }
    }

    void melting()
    {
        for (uint16_t i = 0; i < 40000; i++)
        {
            uint16_t y = random(IMAGE_HEIGHT);
            const uint16_t x = random(IMAGE_HEIGHT);
            while (y < MAX_HEIGHT && vbuff2[y][x] < vbuff2[y + 1][x])
            {
                const uint8_t val = vbuff2[y][x];
                vbuff2[y][x] = vbuff2[y + 1][x];
                vbuff2[y + 1][x] = val;
                y++;
            }
        }
    }

    void oilTransfer()
    {
        const int16_t n = 3;
        int16_t dx = 0, dy = 0, mfp = 0;
        int16_t histo[256] = { 0 };

        for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
        {
            for (int16_t x = 0; x < IMAGE_HEIGHT; x++)
            {
                memset(histo, 0, sizeof(histo));

                for (dy = y - n; dy < y + n; dy++)
                {
                    for (dx = x - n; dx < x + n; dx++) histo[vbuff1[dy + n][dx + n]]++;
                }

                dy = 0;
                for (dx = 0; dx < 256; dx++)
                {
                    if (histo[dx] > dy)
                    {
                        dy = histo[dx];
                        mfp = dx;
                    }
                }

                vbuff2[y][x] = uint8_t(mfp);
            }
        }
    }

    void translate(uint8_t type)
    {
        int16_t x = 0, y = 0;
        int16_t a11 = 0, a12 = 0, a13 = 0, a21 = 0, a22 = 0, a23 = 0;
        int16_t a31 = 0, a32 = 0, a33 = 0, fdiv = 0, bias = -1;

        //Soften (Medium)
        //a11 = 1; a12 = 3; a13 = 1;
        //a21 = 3; a22 = 9; a23 = 3;
        //a31 = 1; a32 = 3; a33 = 1; fdiv = 25;

        //Soften (A lot)
        a11 = 2; a12 = 2; a13 = 2;
        a21 = 2; a22 = 2; a23 = 2;
        a31 = 2; a32 = 2; a33 = 2; fdiv = 18;

        //Fire
        //a11 = 0; a12 = 0; a13 = 1;
        //a21 = 1; a22 = 1; a23 = 1;
        //a31 = 0; a32 = 0; a33 = 1; fdiv = 5;

        //Diagonal "Shatter"
        //a11 = 1; a12 = 0; a13 = 1;
        //a21 = 0; a22 = 0; a23 = 0;
        //a31 = 1; a32 = 0; a33 = 1; fdiv = 4;

        //Find Edge
        //a11 = -1; a12 = -1; a13 = -1;
        //a21 = -1; a22 = 4;  a23 = -1;
        //a31 = -1; a32 = -1; a33 = -1; fdiv = -6;

        switch (type)
        {
        case 255:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff1[y][x] = gray[vbuff1[y][x]];
            break;

        case 1:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = 255 - vbuff1[y][x];
            break;

        case 2:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = vbuff1[y][MAX_HEIGHT - x];
            break;

        case 3:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = vbuff1[x][MAX_HEIGHT - y];
            break;

        case 4:
            translate(2);
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = (vbuff1[y][x] + vbuff2[y][x]) >> 1;
            break;

        case 5:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = vbuff1[int16_t(y / 1.4)][int16_t(x / 1.4)];
            break;

        case 6:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 2; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = vbuff1[y][x + (x % 8) - 4];
            break;

        case 7:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = vbuff1[int16_t(sqrt(y * 199.0))][x];
            break;

        case 8:
            for (y = 0; y < IMAGE_HEIGHT; y++) for (x = 0; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = vbuff1[(y >> 2) << 2][(x >> 2) << 2];
            break;

        case 9:
            translate(0);
            for (y = 0; y < MAX_HEIGHT; y++) for (x = 1; x < IMAGE_HEIGHT; x++) vbuff2[y][x] = 255 - (vbuff2[y][x] - vbuff2[y + 1][x - 1] + 124);
            break;

        case 0:
            for (y = 1; y < MAX_HEIGHT; y++)
            {
                for (x = 1; x < MAX_HEIGHT; x++)
                {
                    vbuff2[y][x] = (
                        a11 * vbuff1[y - 1][x - 1] + a12 * vbuff1[y - 1][x] + a13 * vbuff1[y - 1][x + 1] +
                        a21 * vbuff1[y][x - 1] + a22 * vbuff1[y][x] + a23 * vbuff1[y][x + 1] +
                        a31 * vbuff1[y + 1][x - 1] + a32 * vbuff1[y + 1][x] + a33 * vbuff1[y + 1][x + 1]) / fdiv + bias;
                }
            }
            break;

        case 57:
            memcpy(vbuff2, vbuff1, 40000);
            break;

        default:
            break;
        }
    }

    void run()
    {
        RGB outPal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fillter -- Keys: A->M: switched; Spacer: restore")) return;
        if (!loadPNG(vbuff1[0], pal, "assets/dracula.png")) return;

        for (int16_t i = 0; i < 256; i++)
        {
            outPal[i].r = uint8_t(i);
            outPal[i].g = uint8_t(i);
            outPal[i].b = uint8_t(i);
        }

        calcGray();
        translate(255);
        setPalette(outPal);
        memcpy(vbuff2, vbuff1, 40000);

        do {
            readKeys();
            if (keyDown(SDL_SCANCODE_A))        translate(1);
            if (keyDown(SDL_SCANCODE_B))        translate(2);
            if (keyDown(SDL_SCANCODE_C))        translate(3);
            if (keyDown(SDL_SCANCODE_D))        translate(4);
            if (keyDown(SDL_SCANCODE_E))        translate(5);
            if (keyDown(SDL_SCANCODE_F))        translate(6);
            if (keyDown(SDL_SCANCODE_G))        translate(7);
            if (keyDown(SDL_SCANCODE_H))        translate(8);
            if (keyDown(SDL_SCANCODE_I))        translate(9);
            if (keyDown(SDL_SCANCODE_J))        translate(0);
            if (keyDown(SDL_SCANCODE_K))        shear();
            if (keyDown(SDL_SCANCODE_L))        melting();
            if (keyDown(SDL_SCANCODE_M))        oilTransfer();
            if (keyDown(SDL_SCANCODE_SPACE))    translate(57);
            if (keyDown(SDL_SCANCODE_ESCAPE))   quit();
            drawPicture();
            renderBuffer(dbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace fireworkEffect {
    #define ANGLE_SCALE         1440
    #define FMAX                320
    #define NUM_ARROWS          300
    #define ARROWS_LENGTH       10
    #define EXPLODE_FRAMES      25
    #define FADEDOWN_FRAMES     15
    #define	PIXEL_CIRCLE        10
    #define	NUM_CIRCLE          4
    #define	BRIGHT_ARROW        15
    #define PIXELS_EXPLODE      ((PIXEL_CIRCLE * (NUM_CIRCLE * NUM_CIRCLE) + PIXEL_CIRCLE * NUM_CIRCLE) >> 1)

    typedef struct {
        double x, y;
    } TPoint;

    typedef struct {
        TPoint  shape[ARROWS_LENGTH];
        int8_t  exploded;
        int16_t step;
        int16_t angle;
        int16_t angleAdd;
        uint8_t waitTime;
        int16_t xpos, ypos;
        int16_t	color[NUM_CIRCLE];
    } TArrow;

    typedef struct {
        int16_t x, y;
        uint8_t color, wait;
        uint8_t circle;
    } TExplode;

    TArrow*     arrows[NUM_ARROWS] = { 0 };
    TPoint      sincos[ANGLE_SCALE] = { 0 };
    TExplode    explodes[PIXEL_CIRCLE * NUM_CIRCLE][PIXELS_EXPLODE] = { 0 };

    uint16_t    cnt = 0;
    uint16_t    oldTime = 0;

    int32_t roundf(double x)
    {
        return int32_t(ceil(x));
    }

    void newArrow(TArrow* arrow)
    {
        arrow->exploded = 0;
        arrow->step = random(30) + 64;
        arrow->angle = random(roundf(ANGLE_SCALE / 9.0)) + roundf(ANGLE_SCALE / 5.1428);

        if (arrow->angle < (ANGLE_SCALE >> 2)) arrow->angleAdd = -1;
        else arrow->angleAdd = 1;

        arrow->shape[0].x = random(FMAX / 3) + double(FMAX / 3);
        arrow->shape[0].y = 220;

        for (int16_t i = 1; i < ARROWS_LENGTH; i++)
        {
            arrow->angle += arrow->angleAdd;
            arrow->shape[i] = arrow->shape[i - 1];
            arrow->shape[i].x += sincos[arrow->angle].x;
            arrow->shape[i].y -= sincos[arrow->angle].y;
        }
    }

    void initExplode(TArrow* arrow)
    {
        arrow->step = 0;
        arrow->waitTime = 1;
        arrow->xpos = roundf(arrow->shape[ARROWS_LENGTH - 1].x);
        arrow->ypos = roundf(arrow->shape[ARROWS_LENGTH - 1].y);

        switch (random(4))
        {
        case 0:
            arrow->color[0] = 47;
            arrow->color[1] = 79 - 16;
            arrow->color[2] = 143 - 32;
            break;

        case 1:
            arrow->color[0] = 111;
            arrow->color[1] = 79 - 16;
            arrow->color[2] = 143 - 32;
            break;

        case 2:
            arrow->color[0] = 63;
            arrow->color[1] = 47 - 16;
            arrow->color[2] = 111 - 32;
            break;

        case 3:
            arrow->color[0] = 63;
            arrow->color[1] = 127 - 16;
            arrow->color[2] = 63 - 32;
            break;

        default:
            break;
        }
    }

    void showArrow(TArrow* arrow, uint8_t from, uint8_t to, uint8_t hide)
    {
        int16_t i = 0;

        if (hide == 0)
        {
            for (i = from; i < to; i++)
            {
                if (arrow->shape[i].y < 199.5) putPixel(roundf(arrow->shape[i].x), roundf(arrow->shape[i].y), 15 + ARROWS_LENGTH - i);
            }
        }
        else
        {
            for (i = from; i < to; i++)
            {
                if (arrow->shape[i].y < 199.5) putPixel(roundf(arrow->shape[i].x), roundf(arrow->shape[i].y), 0);
            }
        }
    }

    void showExplode(const TArrow* arrow, uint8_t hide)
    {
        int16_t i = 0;

        if (hide == 0)
        {
            for (i = 0; i < PIXELS_EXPLODE; i++)
            {
                if (arrow->step >= EXPLODE_FRAMES)
                {
                    if (random(arrow->step / 10) == 0) putPixel(explodes[arrow->step][i].x + arrow->xpos, explodes[arrow->step][i].y + arrow->ypos, arrow->color[explodes[arrow->step][i].circle] + explodes[arrow->step][i].color);
                }
                else
                {
                    putPixel(explodes[arrow->step][i].x + arrow->xpos, explodes[arrow->step][i].y + arrow->ypos, arrow->color[explodes[arrow->step][i].circle] + explodes[arrow->step][i].color);
                }
            }
        }
        else
        {
            for (i = 0; i < PIXELS_EXPLODE; i++)
            {
                putPixel(explodes[arrow->step][i].x + arrow->xpos, explodes[arrow->step][i].y + arrow->ypos, 0);
            }
        }
    }

    void stepArrow(TArrow* arrow)
    {
        memcpy(&arrow->shape[0], &arrow->shape[1], (ARROWS_LENGTH - 1) * sizeof(TPoint));
        arrow->shape[ARROWS_LENGTH - 1] = arrow->shape[ARROWS_LENGTH - 2];
        arrow->angle += arrow->angleAdd;
        arrow->shape[ARROWS_LENGTH - 1].x += sincos[arrow->angle].x * ((arrow->step / 37.0) + 0.75);
        arrow->shape[ARROWS_LENGTH - 1].y -= sincos[arrow->angle].y * ((arrow->step / 37.0) + 0.75);
    }

    void handleArrow(TArrow* arrow)
    {
        int16_t i = 0;

        if (arrow->exploded == 0)
        {
            if (arrow->step > 1)
            {
                arrow->step -= 2;
                stepArrow(arrow);
                showArrow(arrow, 0, 1, 0);
                stepArrow(arrow);
                showArrow(arrow, 0, ARROWS_LENGTH, 0);
            }
            else
            {
                showArrow(arrow, 0, ARROWS_LENGTH, 1);
                initExplode(arrow);
                arrow->exploded = 1;
            }
        }
        else
        {
            showExplode(arrow, 1);
            for (i = 0; i < 2; i++)
            {
                arrow->waitTime--;
                if (arrow->step == EXPLODE_FRAMES + FADEDOWN_FRAMES - 1)
                {
                    if (cnt > 997)
                    {
                        newArrow(arrow);
                        if (cnt > 1000) cnt -= 1000;
                        else cnt = 0;
                    }
                    return;
                }

                if (arrow->waitTime == 0)
                {
                    arrow->waitTime = explodes[arrow->step][0].wait;
                    arrow->step++;
                }
            }

            showExplode(arrow, 0);
        }
    }

    void initCosSinTable()
    {
        for (int16_t i = 0; i < ANGLE_SCALE; i++)
        {
            sincos[i].x = cos((double(i) / ANGLE_SCALE) * 2 * M_PI);
            sincos[i].y = sin((double(i) / ANGLE_SCALE) * 2 * M_PI);
        }
    }

    void initExplodeTable()
    {
        double tmp1 = 0, tmp2 = 0;
        int16_t i = 0, j = 0, k = 0, l = 0, idx = 0;

        for (i = 0; i < EXPLODE_FRAMES; i++)
        {
            idx = 0;
            for (j = 1; j <= NUM_CIRCLE; j++)
            {
                for (k = 0; k < j * PIXEL_CIRCLE; k++)
                {
                    tmp1 = roundf(double(k) / (double(j) * PIXEL_CIRCLE) * ANGLE_SCALE);
                    tmp1 += (j - 1.0) * (0.1 * ANGLE_SCALE);

                    if (tmp1 > ANGLE_SCALE) tmp1 -= ANGLE_SCALE;
                    tmp1 = sincos[roundf(tmp1)].x;

                    tmp2 = 0.0;
                    for (l = 0; l <= i; l++) tmp2 += tmp1 * ((double(j) / NUM_CIRCLE) * 1.5) * (((EXPLODE_FRAMES + 1.0) - l) / (0.8 * EXPLODE_FRAMES));
                    explodes[i][idx].x = roundf(tmp2);

                    tmp1 = roundf(double(k) / (double(j) * PIXEL_CIRCLE) * ANGLE_SCALE);
                    tmp1 += (j - 1.0) * (0.1 * ANGLE_SCALE);

                    if (tmp1 > ANGLE_SCALE) tmp1 -= ANGLE_SCALE;
                    tmp1 = sincos[roundf(tmp1)].y;

                    tmp2 = 0.0;
                    for (l = 0; l <= i; l++) tmp2 += tmp1 * ((double(j) / NUM_CIRCLE) * 1.5) * (((EXPLODE_FRAMES + 1.0) - l) / (0.8 * EXPLODE_FRAMES));

                    explodes[i][idx].y = roundf(tmp2);
                    explodes[i][idx].y -= (i >> 2);
                    explodes[i][idx].color = (j - 1) * 16 + 1;
                    explodes[i][idx].wait = 1;
                    explodes[i][idx].circle = j - 1;
                    idx++;
                }
            }
        }

        for (i = EXPLODE_FRAMES; i < FADEDOWN_FRAMES + EXPLODE_FRAMES; i++)
        {
            idx = 0;
            for (j = 1; j <= NUM_CIRCLE; j++)
            {
                for (k = 0; k < j * PIXEL_CIRCLE; k++)
                {
                    explodes[i][idx].x = explodes[i - 1][idx].x;
                    explodes[i][idx].y = explodes[i - 1][idx].y + 1;
                    explodes[i][idx].color = explodes[EXPLODE_FRAMES - 1][idx].color + roundf((15.0 / FADEDOWN_FRAMES) * (double(i) - EXPLODE_FRAMES));
                    explodes[i][idx].wait = 2;
                    explodes[i][idx].circle = j - 1;
                    idx++;
                }
            }
        }
    }

    void initPalette()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };
        const double multiple = double(BRIGHT_ARROW) / (ARROWS_LENGTH - 2);

        for (i = 0; i <= ARROWS_LENGTH - 2; i++)
        {
            pal[i + 16].r = BRIGHT_ARROW - i;
            pal[i + 16].g = roundf(BRIGHT_ARROW - i * multiple);
            pal[i + 16].b = 0;
        }

        pal[16].r = roundf(double(BRIGHT_ARROW) / 45 * 63);
        pal[16].g = 0;
        pal[16].b = 0;

        pal[15 + ARROWS_LENGTH].r = 0;
        pal[15 + ARROWS_LENGTH].g = 0;
        pal[15 + ARROWS_LENGTH].b = 0;

        for (i = 0; i < 16; i++)
        {
            pal[i + 48].r = roundf(63.0 - i / 15.0 * 63);
            pal[i + 48].g = 0;
            pal[i + 48].b = 0;
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 64].r = roundf(63.0 - i / 15.0 * 63);
            pal[i + 64].g = 0;
            pal[i + 64].b = roundf(63.0 - i / 15.0 * 63);
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 80].r = roundf(63.0 - i / 15.0 * 63);
            pal[i + 80].g = roundf(63.0 - i / 15.0 * 63);
            pal[i + 80].b = 0;
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 96].r = roundf(63.0 - i / 15.0 * 63);
            pal[i + 96].g = roundf(63.0 - i / 15.0 * 63);
            pal[i + 96].b = roundf(63.0 - i / 15.0 * 63);
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 112].r = 0;
            pal[i + 112].g = roundf(63.0 - i / 15.0 * 63);
            pal[i + 112].b = 0;
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 128].r = 0;
            pal[i + 128].g = 0;
            pal[i + 128].b = roundf(63.0 - i / 15.0 * 63);
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 144].r = roundf(63.0 - i / 15.0 * 63);
            pal[i + 144].g = roundf(31.0 - i / 15.0 * 31);
            pal[i + 144].b = 0;
        }

        for (i = 0; i < 16; i++)
        {
            pal[i + 160].r = 0;
            pal[i + 160].g = roundf(31.0 - i / 15.0 * 31);
            pal[i + 160].b = 0;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void run()
    {
        int16_t i = 0;
        const int32_t arrowsPerFrame = roundf((double(NUM_ARROWS) / (80.0 + EXPLODE_FRAMES + FADEDOWN_FRAMES)) * 1000);

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Firework")) return;
        initPalette();
        initCosSinTable();
        initExplodeTable();

        for (i = 0; i < NUM_ARROWS; i++)
        {
            arrows[i] = (TArrow*)calloc(1, sizeof(TArrow));
            if (!arrows[i]) messageBox(GFX_ERROR, "Not enough memory!");
            newArrow(arrows[i]);
        }

        while (!finished(SDL_SCANCODE_RETURN)) 
        {
            cnt += arrowsPerFrame;
            for (i = 0; i < NUM_ARROWS; i++) handleArrow(arrows[i]);
            if (cnt > 1000) cnt = 0;
            render();
            delay(FPS_60);
        }

        cleanup();
        for (i = 0; i < NUM_ARROWS; i++) free(arrows[i]);
    }
}

namespace candleEffect {
    double sint[100] = { 0 };
    RGB rgb[256] = { 0 };
    uint8_t vbuff[30][100] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    int16_t roundf(double x)
    {
        if (x > 0.0) return int16_t(x + 0.5);
        return int16_t(x - 0.5);
    }

    void run()
    {
        int16_t x = 0, y = 0, i = 0, j = 0;
        
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Candle")) return;

        for (i = 0; i < 16; i++)
        {
            rgb[i].r = i << 2;
            rgb[i].g = i << 2;
            rgb[i].b = 0;
        }

        for (i = 0; i < 32; i++)
        {
            rgb[i + 16].r = 63;
            rgb[i + 16].g = 63 - (i << 1);
            rgb[i + 16].b = 0;
        }

        for (i = 0; i < 64; i++)
        {
            rgb[i + 32].r = 63;
            rgb[i + 32].g = 0;
            rgb[i + 32].b = 0;
        }

        shiftPalette(rgb);
        setPalette(rgb);

        for (i = 0; i < 100; i++) sint[i] = sin(i * M_PI / 24);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (x = 5; x < 24; x++)
            {
                if (random(2) < 1) vbuff[x][MAX_MIDY] = 200;
                else vbuff[x][MAX_MIDY] = 0;
            }

            for (y = 0; y < 98; y++)
            {
                for (x = 1; x < 28; x++) vbuff[x][y] = (vbuff[x - 1][y] + vbuff[x][y] + vbuff[x + 1][y] + vbuff[x - 1][y + 1] + vbuff[x + 1][y + 1] + vbuff[x - 1][y + 2] + vbuff[x][y + 2] + vbuff[x + 1][y + 2]) >> 3;
            }

            j++;

            if (j > MAX_MIDY) j -= MAX_MIDY;

            for (y = 0; y < IMAGE_MIDY; y++)
            {
                for (x = 0; x < 30; x++)
                {
                    vmem[y][x + 145] = vbuff[x][y];
                    i = y + j;
                    if (i > MAX_MIDY) i -= MAX_MIDY;
                    vmem[131 - (y - 2) / 3][145 + x + roundf(sint[i] * ((MAX_MIDY - y) >> 2))] = vbuff[x][y];
                }
            }

            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(1);
        }
        cleanup();
    }
}

namespace fireEffect {
    #define YSTART          100
    #define XSTART          150
    #define XEND            170
    #define FIRESTRENGTH    15

    uint8_t vbuff[IMAGE_HEIGHT + 1][IMAGE_WIDTH] = { 0 };

    void doFire()
    {
        int16_t x = 0, y = 0;

        for (x = XSTART; x < XEND; x++)
        {
            if (rand() % FIRESTRENGTH > 1) vbuff[MAX_HEIGHT - 1][x] = 255;
            else vbuff[MAX_HEIGHT - 1][x] = 0;
        }

        for (y = MAX_HEIGHT - 1; y > YSTART; y--)
        {
            for (x = XSTART; x < XEND; x++)
            {
                vbuff[y - 1][x] = (vbuff[y][x - 1] + vbuff[y][x + 1] + vbuff[y + 1][x] + vbuff[y - 1][x] + vbuff[y - 1][x - 1] + vbuff[y - 1][x + 1] + vbuff[y + 1][x - 1] + vbuff[y + 1][x + 1]) >> 3;
                if (vbuff[y][x] > 200) vbuff[y][x] -= 1;
                else if (vbuff[y][x] > 148) vbuff[y][x] -= 1;
                else if (vbuff[y][x] > 4) vbuff[y][x] -= 2;
            }
        }
    }

    void doQuit()
    {
        for (int16_t y = MAX_HEIGHT; y > YSTART; y--)
        {
            for (int16_t x = XSTART; x < XEND; x++)
            {
                vbuff[y - 1][x] = (vbuff[y][x - 1] + vbuff[y][x + 1] + vbuff[y + 1][x] + vbuff[y - 1][x] + vbuff[y - 1][x - 1] + vbuff[y - 1][x + 1] + vbuff[y + 1][x - 1] + vbuff[y + 1][x + 1]) >> 3;
                if (vbuff[y][x] > 200) vbuff[y][x] -= 2;
                else if (vbuff[y][x] > 4) vbuff[y][x] -= 1;
            }
        }
    }

    void run()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;

        for (i = 0; i < 64; i++)
        {
            pal[i].r = 0;
            pal[i].g = 0;
            pal[i].b = 0;
        }

        for (i = 64; i < 128; i++)
        {
            pal[i].r = i - 64;
            pal[i].g = 0;
            pal[i].b = 0;
        }

        for (i = 128; i < 192; i++)
        {
            pal[i].r = 63;
            pal[i].g = i - 128;
            pal[i].b = 0;
        }

        for (i = 192; i < 255; i++)
        {
            pal[i].r = 63;
            pal[i].g = 63;
            pal[i].b = i - 192;
        }

        pal[255].r = 0;
        pal[255].g = 0;
        pal[255].b = 0;

        shiftPalette(pal);
        setPalette(pal);
        memset(vbuff[MAX_HEIGHT], 255, IMAGE_WIDTH);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            doFire();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }

        memset(vbuff[MAX_HEIGHT], 0, IMAGE_WIDTH);

        for (i = 0; i < 35; i++)
        {
            doQuit();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace fireEffect2 {
    RGB         pal[256] = { 0 };
    uint16_t    index = 0;
    uint8_t     seed[SIZE_32K] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    void initSeed()
    {
        uint16_t i = 0, j = 0;

        for (i = 0; i < SIZE_32K; i++)
        {
            j = rand() % 10;
            if (j > 4) seed[i] = 150;
            if (j > 6) seed[i] = 100;
            if (j > 8) seed[i] = 50;
            else seed[i] = 255;
        }
    }

    void updateSeed()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++)
        {
            vbuff[MAX_HEIGHT][x] = seed[index++];
            if (index >= SIZE_32K) index = 0;
        }
    }

    void extendFlames()
    {
        for (int16_t i = MAX_HEIGHT; i > 1; i--)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++)
            {
                int16_t col = ((2 * vbuff[i][j] + vbuff[i][j - 1] + vbuff[i - 1][j]) >> 2) - 2;
                if (col > 240) col = 255;
                if (col < 0) col = 0;
                vbuff[i - 1][j] = uint8_t(col);
            }
        }
    }

    int16_t loadPalette(const char* fname)
    {
        uint8_t rgb[768] = { 0 };
        FILE* fp = fopen(fname, "rb");
        if (!fp) return 0;
        fread(rgb, 1, 768, fp);
        fclose(fp);
        convertPalette(rgb, pal);
        setPalette(pal);
        return 1;
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;
        if (!loadPalette("assets/fire.pal")) return;
        initSeed();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            updateSeed();
            extendFlames();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace fireEffect3 {
    uint8_t ned = 0;
    uint8_t vbuff[IMAGE_HEIGHT + 4][IMAGE_WIDTH] = { 0 };

    void initFire()
    {
        int16_t j = 0;
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;

        for (j = 180; j >= 64; j--)
        {
            pal[j].r = 63;
            pal[j].g = uint8_t(63.0 - (j - 64.0) / (180.0 - 64.0) * 63.0);
            pal[j].b = 0;
        }

        for (j = 0; j <= 53; j++)
        {
            pal[63 - j].r = 63 - j;
            pal[63 - j].g = 63 - j;
            pal[63 - j].b = uint8_t(j);
        }

        for (j = 0; j <= 10; j++)
        {
            pal[10 - j].r = 10 - j;
            pal[10 - j].g = 10 - j;
            pal[10 - j].b = 50 - 5 * j;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void doFire()
    {
        uint8_t col = 0;
        int16_t i = 0, j = 0;
        
        if (ned)
        {
            for (j = 0; j < 20; j++)
            {
                ned = 0;
                col = random(91) + 90;
                for (i = 0; i < 16; i++)
                {
                    vbuff[202][(j << 4) + i] = col;
                    vbuff[203][(j << 4) + i] = col;
                }
            }
        }
        else
        {
            for (j = 0; j < 40; j++)
            {
                ned = 1;
                col = random(91) + 90;
                for (i = 0; i < 8; i++)
                {
                    vbuff[202][(j << 3) + i] = col;
                    vbuff[203][(j << 3) + i] = col;
                }
            }
        }

#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            add     edi, 16000
            mov     ecx, 48640
        start:
            xor     ax, ax
            xor     dx, dx
            mov     ebx, edi
            add     ebx, 319
            mov     al, [ebx]
            inc     ebx
            mov     dl, [ebx]
            add     ax, dx
            inc     ebx
            mov     dl, [ebx]
            add     ax, dx
            add     ebx, MAX_WIDTH
            mov     dl, [ebx]
            add     ax, dx
            sub     ax, 4
            mov     bx, 4
            xor     dx, dx
            div     bx
            cmp     al, 180
            ja    	done
            mov     [edi], al
        done:
            inc     edi
            loop    start
        }
#else
        for (i = 1 + (IMAGE_MIDY >> 1); i < IMAGE_HEIGHT + 3; i++)
        {
            for (j = 0; j <= MAX_WIDTH; j++)
            {
                col = ((vbuff[i][j - 1] + vbuff[i][j] + vbuff[i][j + 1] + vbuff[i + 1][j]) - 4) >> 2;
                if (col <= 180) vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void run()
    {
        initFire();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            doFire();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(4);
        }

        cleanup();
    }
}

namespace fireEffect4 {
    uint16_t    delta = 0;
    uint16_t    vbuff[IMAGE_MIDY + 2][IMAGE_MIDX] = { 0 };
    uint16_t    vmem[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };

    void setFirePal()
    {
        RGB pal[256] = { 0 };

        for (uint8_t i = 0; i < 64; i++)
        {
            pal[i].r = i;
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = i;
            pal[i + 64].b = 0;
            pal[i + 128].r = 63;
            pal[i + 128].g = 63;
            pal[i + 128].b = i;
            pal[i + 192].r = 63;
            pal[i + 192].g = 63;
            pal[i + 192].b = 0;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void interpolation()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            add     edi, IMAGE_WIDTH
            mov     ecx, 16160
        lp1:
            mov     ax, [edi - 2]
            add     ax, [edi]
            add     ax, [edi + 2]
            add     ax, [edi + IMAGE_WIDTH]
            shr     ax, 2
            jz      lp2
            sub     ax, 1
        lp2:
            mov     [edi - IMAGE_WIDTH], ax
            add     edi, 2
            loop    lp1
        }
#else
        for (int16_t i = 1; i < IMAGE_MIDY + 1; i++)
        {
            for (int16_t j = 0; j < IMAGE_MIDX; j++)
            {
                uint16_t col = (vbuff[i][j - 1] + vbuff[i][j] + vbuff[i][j + 1] + vbuff[i + 1][j]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void purgeBuff()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vmem
            lea     esi, vbuff
            mov     dx, IMAGE_MIDY
        l3:
            mov     bx, 2
        l2:
            mov     cx, IMAGE_MIDX
        l1:
            mov     al, [esi]
            mov     ah, al
            stosw
            add     esi, 2
            dec     cx
            jnz     l1
            sub     esi, IMAGE_WIDTH
            dec     bx
            jnz     l2
            add     esi, IMAGE_WIDTH
            dec     dx
            jnz     l3
        }
#else
        for (int16_t i = 0; i < IMAGE_MIDY; i++)
        {
            for (int16_t k = 0; k < 2; k++)
            {
                for (int16_t j = 0; j < IMAGE_MIDX; j++)
                {
                    const uint16_t col = vbuff[i][j];
                    vmem[i][j + k * IMAGE_MIDX] = (col << 8) + (col & 0xff);
                }
            }
        }
#endif
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;
        setFirePal();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (int16_t i = 0; i < IMAGE_MIDX; i++)
            {
                if ((rand() % 10) < 5) delta = (rand() % 2) * 255;
                vbuff[100][i] = delta;
                vbuff[101][i] = delta;
            }
            interpolation();
            purgeBuff();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace fireEffect5 {
    #define BLINE 0
    #define INTEN 240

    uint16_t    vbuff[IMAGE_MIDY + 2][IMAGE_MIDX] = { 0 };
    uint16_t    vmem[IMAGE_MIDY][IMAGE_WIDTH] = { 0 };
    uint8_t     dbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     hline[5][24][IMAGE_WIDTH] = { 0 };

    void setFirePalette()
    {
        RGB pal[256] = { 0 };

        for (uint8_t i = 0; i < 64; i++)
        {
            pal[i].r = i;
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = i;
            pal[i + 64].b = 0;
            pal[i + 128].r = 63;
            pal[i + 128].g = 63;
            pal[i + 128].b = i;
            pal[i + 192].r = 63;
            pal[i + 192].g = 63;
            pal[i + 192].b = 0;
        }

        pal[247].r = 16;
        pal[247].g = 0;
        pal[247].b = 0;
        pal[248].r = 45;
        pal[248].g = 0;
        pal[248].b = 0;
        pal[249].r = 63;
        pal[249].g = 0;
        pal[249].b = 0;
        pal[250].r = 63;
        pal[250].g = 6;
        pal[250].b = 6;
        pal[251].r = 63;
        pal[251].g = 14;
        pal[251].b = 14;
        pal[252].r = 63;
        pal[252].g = 22;
        pal[252].b = 22;
        pal[253].r = 63;
        pal[253].g = 30;
        pal[253].b = 30;
        pal[254].r = 63;
        pal[254].g = 47;
        pal[254].b = 47;

        shiftPalette(pal);
        setPalette(pal);
    }

    void interpolation()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            add     edi, IMAGE_WIDTH
            mov     ecx, 16160
        lp1:
            mov     ax, [edi - 2]
            add     ax, [edi]
            add     ax, [edi + 2]
            add     ax, [edi + IMAGE_WIDTH]
            shr     ax, 2
            jz      lp2
            sub     ax, 1
        lp2:
            mov     [edi - IMAGE_WIDTH], ax
            add     edi, 2
            loop    lp1
        }
#else
        for (int16_t i = 1; i < IMAGE_MIDY + 1; i++)
        {
            for (int16_t j = 0; j < IMAGE_MIDX; j++)
            {
                uint16_t col = (vbuff[i][j - 1] + vbuff[i][j] + vbuff[i][j + 1] + vbuff[i + 1][j]) >> 2;
                if (col > 0) col--;
                vbuff[i - 1][j] = col;
            }
        }
#endif
    }

    void purgeBuff()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vmem
            lea     esi, vbuff
            mov     dx, IMAGE_MIDY
        l3:
            mov     bx, 2
        l2:
            mov     cx, IMAGE_MIDX
        l1:
            mov     al, [esi]
            mov     ah, al
            stosw
            add     esi, 2
            dec     cx
            jnz     l1
            sub     esi, IMAGE_WIDTH
            dec     bx
            jnz     l2
            add     esi, IMAGE_WIDTH
            dec     dx
            jnz     l3
        }
#else
        for (int16_t i = 0; i < IMAGE_MIDY; i++)
        {
            for (int16_t k = 0; k < 2; k++)
            {
                for (int16_t j = 0; j < IMAGE_MIDX; j++)
                {
                    const uint16_t col = vbuff[i][j];
                    vmem[i][j + k * IMAGE_MIDX] = (col << 8) + (col & 0xff);
                }
            }
        }
#endif
    }

    void makeBlood(uint8_t num)
    {
        for (int16_t i = 0; i < 24; i++)
        {
            for (int16_t j = 0; j < IMAGE_WIDTH; j++)
            {
                const uint8_t col = hline[num][i][j];
                if (col != 255) dbuff[BLINE + i][j] = col;
            }
        }
    }

    void run()
    {
        int16_t i = 0, delta = 0;
        uint8_t k = 0, kk = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;
        setFirePalette();

        FILE *fp = fopen("assets/bline.dat", "rb");
        if (!fp) exit(1);

        for (i = 0; i < 5; i++) fread(hline[i], 1, 24 * IMAGE_WIDTH, fp);
        fclose(fp);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            interpolation();

            for (i = 0; i < IMAGE_MIDX; i++)
            {
                delta = (rand() % 2) * INTEN;
                vbuff[100][i] = delta;
                vbuff[101][i] = delta;
            }

            purgeBuff();
            memcpy(dbuff, vmem, IMAGE_SIZE);
            makeBlood(k);
            renderBuffer(dbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);

            kk++;
            if (kk == 10)
            {
                k++;
                kk = 0;
            }
            if (k > 4) k = 0;
        }
        cleanup();
    }
}

namespace fireEffect6 {
    #define ROOTRAND    20
    #define DECAY       8
    #define YMIN        100
    #define SMOOTH 	    1
    #define FIREMIN     50
    #define STARTX      40
    #define ENDX        280
    #define WIDTH       (ENDX - STARTX)
    #define MAXCOL      110
    #define BARLEN      10

    uint8_t flames[WIDTH + 1] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void HSI2RGB(double H, double S, double I, RGB* pal)
    {
        const double r = 1 + S * sin(H - 2 * M_PI / 3);
        const double g = 1 + S * sin(H);
        const double b = 1 + S * sin(H + 2 * M_PI / 3);
        const double t = 63.999 * I / 2;

        pal->r = uint8_t(r * t);
        pal->g = uint8_t(g * t);
        pal->b = uint8_t(b * t);
    }

    void makePalette()
    {
        int16_t i = 0;
        RGB rgb[256] = { 0 };

        for (i = 0; i < MAXCOL; i++) HSI2RGB(4.6 - 1.5 * i / MAXCOL, double(i) / MAXCOL, double(i) / MAXCOL, &rgb[i]);

        for (i = MAXCOL; i < 256; i++)
        {
            rgb[i] = rgb[i - 1];

            if (rgb[i].r < 63) rgb[i].r++;
            if (rgb[i].r < 63) rgb[i].r++;

            if (!(i % 2) && rgb[i].g < 53) rgb[i].g++;
            if (!(i % 2) && rgb[i].b < 63) rgb[i].b++;
        }

        shiftPalette(rgb);
        setPalette(rgb);
    }

    int16_t xrand(int16_t val)
    {
        return random((val << 1) + 1) - val;
    }

    void run()
    {
        int16_t fires = 0;
        int16_t col = 0, x = 0;
        int16_t i = 0, j = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire -- Spacer: throw a match; W: water; A/S: +/- intensity")) return;
        makePalette();

        for (i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (j = 0; j < BARLEN; j++) vbuff[i][j] = uint8_t(i);
        }

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (i = 0; i < WIDTH; i++) vbuff[MAX_HEIGHT][i + STARTX] = flames[i];

            for (i = 0; i < WIDTH; i++)
            {
                for (j = YMIN; j < IMAGE_HEIGHT; j++)
                {
                    col = vbuff[j][i + STARTX];
                    if (!col || col < DECAY || i <= STARTX || i >= ENDX) vbuff[j - 1][i + STARTX] = 0;
                    else
                    {
                        x = i - (random(3) - 1) + STARTX;
                        if (x > ENDX - 1) x = ENDX - 1;
                        vbuff[j - 1][x] = col - random(DECAY);
                    }
                }
            }

            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            readKeys();

            if (!random(150) || keyDown(SDL_SCANCODE_SPACE)) memset(&flames[random(WIDTH - 10)], 255, 10);
            if (keyDown(SDL_SCANCODE_S) && fires > -2) fires--;
            if (keyDown(SDL_SCANCODE_A) && fires < 4) fires++;
            if (keyDown(SDL_SCANCODE_W)) for (i = 0; i < 10; i++) flames[random(WIDTH)] = 0;

            for (i = 0; i < WIDTH; i++)
            {
                x = flames[i];
                if (x < FIREMIN)
                {
                    if (x > 10) x += random(6);
                }
                else
                {
                    x += xrand(ROOTRAND) + fires;
                    if (x > 255) x = 255;
                    flames[i] = uint8_t(x);
                }
            }

            for (i = 0; i < WIDTH >> 3; i++)
            {
                x = random(50);
                flames[x] = 0;
                flames[WIDTH - x - 1] = 0;
            }

            for (i = SMOOTH; i <= WIDTH - SMOOTH; i++)
            {
                x = 0;
                for (j = -SMOOTH; j <= SMOOTH; j++) x += flames[i + j];
                flames[i] = x / ((SMOOTH << 1) + 1);
            }
        }
        cleanup();
    }
}

namespace fireEffect7 {
    uint8_t dbuff[IMAGE_WIDTH][80] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void interpolation()
    {
        for (int16_t x = 1; x < MAX_WIDTH; x++)
        {
            for (int16_t y = 1; y < 79; y++) dbuff[x][y] = (2 * dbuff[x][y + 1] + dbuff[x + 1][y + 1] + dbuff[x - 1][y - 1]) >> 2;
        }
    }

    void putFire()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++)
        {
            for (int16_t y = 0; y < 80; y++) vbuff[y + 120][x] = dbuff[x][y];
        }
    }

    void makePalette()
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 64; i++)
        {
            pal[i +   0].r =  i << 2; pal[i +   0].g =      0; pal[i +   0].b = 0;
            pal[i +  64].r = 63 << 2; pal[i +  64].g = i << 2; pal[i +  64].b = 0;
            pal[i + 128].r = 63 << 2; pal[i + 128].g = i << 2; pal[i + 128].b = 0;
        }

        setPalette(pal);
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;
        makePalette();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (int16_t x = 0; x < IMAGE_WIDTH; x++) dbuff[x][79] = random(100) + 40;
            interpolation();
            putFire();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace fireEffect8 {
    uint16_t rdx = 0;
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t randomize[19200] = { 0 };

    double idx = 0;
    double sinus[1000] = { 0.0 };

    int16_t roundf(double x)
    {
        return int16_t(floor(x));
    }

    void blurBuff()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            add     edi, IMAGE_WIDTH
            mov     ecx, IMAGE_SIZE - (IMAGE_WIDTH << 1)
            xor     ax, ax
            xor     bx, bx
        again:
            mov     al, [edi - 1]
            mov     bl, [edi + 1]
            add     ax, bx
            mov     bl, [edi - IMAGE_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH]
            add     ax, bx
            shr     ax, 2
            stosb
            loop    again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++) vbuff[i][j] = (vbuff[i][j - 1] + vbuff[i][j + 1] + vbuff[i - 1][j] + vbuff[i + 1][j]) >> 2;
        }
#endif
    }

    void pierra()
    {
        int16_t i = 0, j = 0;

        idx += 0.1;
        const double x = sinus[roundf(idx * 159) % 1000] * sinus[roundf(idx * 83 + 130) % 1000] * 140;
        const double y = sinus[roundf(idx * 97 + 153) % 1000] * sinus[roundf(idx * 107) % 1000] * 80;

        for (i = 1; i < MAX_HEIGHT; i++)
        {
            for (j = 1; j < MAX_WIDTH; j++)
            {
                if (rdx >= 19200) rdx = 0;
                if (randomize[rdx] == 1) vbuff[i][j] = 0;
                rdx++;
            }
        }

        for (j = 150 + roundf(-x); j <= 170 + roundf(-x); j++)
        {
            for (i = 90 + roundf(-y); i <= 110 + roundf(-y); i++) vbuff[i][j] = 255;
        }

        for (j = 150 + roundf(x); j <= 170 + roundf(x); j++)
        {
            for (i = 90 + roundf(y); i <= 110 + roundf(y); i++) vbuff[i][j] = 255;
        }

        blurBuff();
        blurBuff();
        blurBuff();
        blurBuff();
        blurBuff();
        blurBuff();
    }

    void run()
    {
        int16_t i = 0, j = 0;
        RGB pal[256] = { 0 };

        memset(vbuff, 0, IMAGE_SIZE);
        memset(sinus, 0, 1000);
        memset(randomize, 0, 19200);

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire")) return;
        
        for (i = 0; i < 1000; i++)
        {
            sinus[i] = sin(i * 0.00628318530718);
        }

        for (i = 0; i < 19200; i++)
        {
            if (random(60) == 1) randomize[i] = 1;
            else randomize[i] = 0;
        }

        for (i = 0; i < 64; i++)
        {
            pal[i].r = i << 2;
            pal[i].g = 0;
            pal[i].b = 0;
        }

        for (i = 64; i < 128; i++)
        {
            pal[i].r = 63 << 2;
            pal[i].g = i << 2;
            pal[i].b = 0;
        }

        for (i = 128; i < 192; i++)
        {
            pal[i].r = 63 << 2;
            pal[i].g = 63 << 2;
            pal[i].b = i << 2;
        }

        for (i = 192; i < 256; i++)
        {
            pal[i].r = 63 << 2;
            pal[i].g = 63 << 2;
            pal[i].b = 63 << 2;
        }

        setPalette(pal);

        for (i = 1; i < MAX_HEIGHT; i++)
        {
            for (j = 1; j < MAX_WIDTH; j++) vbuff[i][j] = 0;
        }

        while (!finished(SDL_SCANCODE_RETURN))
        {
            pierra();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace holeEffect1 {
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint16_t    circles[360][30] = { 0 };
    uint16_t    sinx[720] = { 0 };
    uint16_t    siny[720] = { 0 };
    int16_t     ypts[90][30] = { 0 };
    int16_t     xpts[90][30] = { 0 };

    void run()
    {
        double rad = 0;
        int16_t xx = 0, yy = 0, i = 0, x = 0, y = 0;
        RGB pal[256] = { 0 };

        for (i = 0; i < 30; i++)
        {
            rad = 0.0;
            for (x = 0; x < 360; x++)
            {
                rad = rad + 0.0175 * 4;
                circles[x][i] = uint16_t(sin(rad) * (5.0 + 4.0 * i) + (5.0 + 4.0 * i));
            }
        }

        rad = 0.0;
        for (x = 0; x < 720; x++)
        {
            rad += 0.0175;
            sinx[x] = uint16_t(sin(rad) * 140 + 140);
            siny[x] = uint16_t(cos(rad) * 90 + 90);
        }


        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Hole")) return;

        for (i = 0; i < 256; i++)
        {
            pal[i].r = 0;
            pal[i].g = i << 2;
            pal[i].b = i << 3;
        }

        setPalette(pal);

        i = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            if (i > 358) i = 0;
            i += 2;

            for (y = 0; y < 30; y++)
            {
                for (x = 0; x < 90; x++)
                {
                    xx = xpts[x][y];
                    yy = ypts[x][y];
                    vmem[yy][xx] = 0;

                    xx = (circles[x][y] + sinx[(y << 3) + i]) - (y << 2);
                    yy = (circles[x + 22][y] + siny[i + 89 + (y << 2)]) - (y << 2);

                    if (xx >= 0 && xx <= 319 && yy >= 0 && yy <= 199)
                    {
                        vmem[yy][xx] = uint8_t(y);
                        xpts[x][y] = xx;
                        ypts[x][y] = yy;
                    }
                }
            }

            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace holeEffect2 {
    #define INCANG  4
    #define XMOV    2
    #define YMOV    4

    int16_t     sintab[450] = { 0 };
    int16_t     sinx[256] = { 0 };
    int16_t     cosx[256] = { 0 };
    uint16_t    inc = 0;
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void myDec(uint8_t* val, uint8_t dec)
    {
        if (*val > dec) *val -= dec;
        else *val = 0;
    }

    void makeDegradated(uint8_t r, uint8_t g, uint8_t b)
    {
        RGB pal[256] = { 0 };
        for (int16_t i = 32; i >= 16; i--)
        {
            pal[i].r = r;
            pal[i].g = g;
            pal[i].b = b;
            myDec(&r, 4);
            myDec(&g, 4);
            myDec(&b, 4);
        }
        shiftPalette(pal);
        setPalette(pal);
    }

    void calcTables()
    {
        int16_t i = 0;

        for (i = 0; i < 256; i++)
        {
            sinx[i] = int16_t(sin(i * M_PI / 128) * 20);
            cosx[i] = int16_t(cos(i * M_PI / 128) * 80);
        }

        for (i = 0; i < 450; i++) sintab[i] = int16_t(sin(2 * M_PI * i / 360) * 128);
    }

    void drawPoint(int16_t xc, int16_t yc, int16_t rad, int16_t i, uint8_t col)
    {
        uint16_t x = 0, y = 0;

        x = (rad * sintab[90 + i]) >> 7;
        x += IMAGE_MIDX + xc;
        y = (rad * sintab[i]) >> 7;
        y += IMAGE_MIDY + yc;
        if (x < IMAGE_WIDTH && y < IMAGE_HEIGHT) vmem[y][x] = col;
    }

    void drawHole()
    {
        uint8_t col = 0;
        int16_t x = 0, y = 0;
        int16_t i = 0, j = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            col = 19;
            inc = 2;
            j = 10;

            do {
                i = 0;
                do {
                    drawPoint(cosx[(x + (200 - j)) & 0xff], sinx[(y + (200 - j)) & 0xff], j, i, col);
                    i += INCANG;
                } while (i < 360);

                j += inc;

                if (!(j % 3))
                {
                    inc++;
                    col++;
                    if (col > 31) col = 31;
                }
            } while (j < 220);

            x = XMOV + (x & 0xff);
            y = YMOV + (y & 0xff);

            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            memset(vmem, 0, IMAGE_SIZE);
        }
        cleanup();
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Hole")) return;
        calcTables();
        makeDegradated(50, 50, 64);
        drawHole();
    }
}

namespace holeEffect3 {
    #define MPOS    200
    #define DIVD    128
    #define STEP    15
    #define XST     2
    #define YST     1

    int16_t     sintab[450] = { 0 };
    int16_t     sinx[256] = { 0 };
    int16_t     cosx[256] = { 0 };
    int16_t     px[27][24] = { 0 };
    int16_t     py[27][24] = { 0 };
    int16_t     minx, miny, maxx, maxy;
    uint8_t     step, ring, point, art;
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void plotLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t col)
    {
        int16_t d = 0, sign = 0;
        int16_t x = 0, y = 0, dx = 0, dy = 0, dt = 0, ds = 0;

        if (x1 < 1 || y1 < 1 || x1 > MAX_WIDTH - 1 || y1 > MAX_HEIGHT - 1) return;
        if (x2 < 1 || y2 < 1 || x2 > MAX_WIDTH - 1 || y2 > MAX_HEIGHT - 1) return;

        if (abs(x2 - x1) < abs(y2 - y1))
        {
            if (y1 > y2)
            {
                swap(x1, x2);
                swap(y1, y2);
            }

            x = x1;
            y = y1;

            dx = abs(x2 - x1);
            dy = y2 - y1;

            dt = (dx - dy) << 1;
            ds = dx << 1;

            d = (dx << 1) - dy;
            sign = (x2 > x1) ? 1 : -1;

            vbuff[y][x] = col;

            for (y = y1 + 1; y <= y2; y++)
            {
                if (d >= 0)
                {
                    x += sign;
                    d += dt;
                }
                else d += ds;

                vbuff[y][x] = col;
            }
        }
        else
        {
            if (x1 > x2)
            {
                swap(x1, x2);
                swap(y1, y2);
            }

            x = x1;
            y = y1;

            dx = x2 - x1;
            dy = abs(y2 - y1);

            dt = (dy - dx) << 1;
            ds = dy << 1;

            d = (dy << 1) - dx;
            sign = (y2 > y1) ? 1 : -1;

            vbuff[y][x] = col;

            for (x = x1 + 1; x <= x2; x++)
            {
                if (d >= 0)
                {
                    y += sign;
                    d += dt;
                }
                else d += ds;

                vbuff[y][x] = col;
            }
        }
    }

    void horizLine(int16_t xb, int16_t xe, int16_t y, uint8_t col)
    {
#ifdef _USE_ASM
        __asm {
            xor     ebx, ebx
            xor     ecx, ecx
            mov     bx, xb
            mov     cx, xe
            cmp     ebx, ecx
            jb      skip
            xchg    ebx, ecx
        skip:
            sub     ecx, ebx
            inc     ecx
            lea     edi, vbuff
            xor     eax, eax
            mov     ax, y
            shl     ax, 6
            add     edi, eax
            shl     eax, 2
            add     edi, eax
            add     edi, ebx
            mov     al, col
            shr     ecx, 1
            jnc     next
            stosb
        next:
            mov     ah, al
            rep     stosw
        }
#else
        if (xe < xb) swap(xe, xb);
        memset(&vbuff[y][xb], col, intptr_t(xe) - xb + 1);
#endif
    }

    int16_t range(int16_t a, int16_t b, int16_t c)
    {
        return ((a >= b && a <= c) ? a : (a <= b) ? b : c);
    }

    void polygon1(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, int16_t c)
    {
        int16_t pos[MPOS][2] = { 0 };
        int16_t xdiv1, xdiv2, xdiv3, xdiv4;
        int16_t ydiv1, ydiv2, ydiv3, ydiv4;
        int16_t ly, gy, y, tmp, step;
        uint8_t dir1, dir2, dir3, dir4;

        ly = max(min(min(min(y1, y2), y3), y4), miny);
        gy = min(max(max(max(y1, y2), y3), y4), maxy);

        if (ly > maxy) return;
        if (gy < miny) return;

        dir1 = y1 < y2; xdiv1 = x2 - x1; ydiv1 = y2 - y1;
        dir2 = y2 < y3; xdiv2 = x3 - x2; ydiv2 = y3 - y2;
        dir3 = y3 < y4; xdiv3 = x4 - x3; ydiv3 = y4 - y3;
        dir4 = y4 < y1; xdiv4 = x1 - x4; ydiv4 = y1 - y4;

        y = y1;
        step = (dir1 << 1) - 1;

        if (y1 != y2)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv1 * (y - y1) / ydiv1 + x1;
                    pos[y][dir1] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y2 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir1] = range(x1, minx, maxx);

        y = y2;
        step = (dir2 << 1) - 1;

        if (y2 != y3)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv2 * (y - y2) / ydiv2 + x2;
                    pos[y][dir2] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y3 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir2] = range(x2, minx, maxx);

        y = y3;
        step = (dir3 << 1) - 1;

        if (y3 != y4)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv3 * (y - y3) / ydiv3 + x3;
                    pos[y][dir3] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y4 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir3] = range(x3, minx, maxx);

        y = y4;
        step = (dir4 << 1) - 1;

        if (y4 != y1)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv4 * (y - y4) / ydiv4 + x4;
                    pos[y][dir4] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y1 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir4] = range(x4, minx, maxx);

        for (y = ly; y <= gy; y++) horizLine(pos[y][0], pos[y][1], y, uint8_t(c));
    }

    void polygon2(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, int16_t x4, int16_t y4, int16_t c)
    {
        int16_t pos[MPOS][2] = { 0 };
        int16_t xdiv1, xdiv2, xdiv3, xdiv4;
        int16_t ydiv1, ydiv2, ydiv3, ydiv4;
        int16_t ly, gy, y, tmp, step;
        uint8_t dir1, dir2, dir3, dir4;

        ly = max(min(min(min(y1, y2), y3), y4), miny);
        gy = min(max(max(max(y1, y2), y3), y4), maxy);

        if (ly > maxy) return;
        if (gy < miny) return;

        dir1 = y1 < y2; xdiv1 = x2 - x1; ydiv1 = y2 - y1;
        dir2 = y2 < y3; xdiv2 = x3 - x2; ydiv2 = y3 - y2;
        dir3 = y3 < y4; xdiv3 = x4 - x3; ydiv3 = y4 - y3;
        dir4 = y4 < y1; xdiv4 = x1 - x4; ydiv4 = y1 - y4;

        y = y1;
        step = (dir1 << 1) - 1;

        if (y1 != y2)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv1 * (y - y1) / ydiv1 + x1;
                    pos[y][dir1] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y2 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir1] = range(x1, minx, maxx);

        y = y2;
        step = (dir2 << 1) - 1;

        if (y2 != y3)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv2 * (y - y2) / ydiv2 + x2;
                    pos[y][dir2] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y3 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir2] = range(x2, minx, maxx);

        y = y3;
        step = (dir3 << 1) - 1;

        if (y3 != y4)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv3 * (y - y3) / ydiv3 + x3;
                    pos[y][dir3] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y4 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir3] = range(x3, minx, maxx);

        y = y4;
        step = (dir4 << 1) - 1;

        if (y4 != y1)
        {
            do {
                if (range(y, ly, gy) == y)
                {
                    tmp = xdiv4 * (y - y4) / ydiv4 + x4;
                    pos[y][dir4] = range(tmp, minx, maxx);
                }
                y += step;
            } while (y != y1 + step);
        }
        else if (y >= ly && y <= gy) pos[y][dir4] = range(x4, minx, maxx);

        for (y = ly; y <= gy; y++)
        {
            horizLine(pos[y][0], pos[y][1], y, c >> 1);
            c++;
        }
    }

    void calcPoint(uint8_t art, int16_t xo, int16_t yo, int16_t r, int16_t a)
    {
        const uint16_t x = IMAGE_MIDX + xo + (r * sintab[a + 90]) / (DIVD - 20);
        const uint16_t y = IMAGE_MIDY + yo + (r * sintab[a]) / DIVD;

        switch (art)
        {
        case 0:
            if (x < IMAGE_WIDTH && y < IMAGE_HEIGHT)
            {
                px[ring][point] = x;
                py[ring][point] = y;
            }
            else
            {
                px[ring][point] = 0;
                py[ring][point] = 0;
            }
            break;

        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            px[ring][point] = x;
            py[ring][point] = y;
            break;
        default:
            break;
        }
    }

    void drawHole(uint8_t art)
    {
        int16_t ri = 0, po = 0;

        switch (art)
        {
        case 0:
            for (ri = 0; ri <= 26; ri++)
            {
                for (po = 0; po <= 23; po++) vbuff[py[ri][po]][px[ri][po]] = ri + 1;
            }
            vbuff[0][0] = 0;
            break;

        case 1:
            for (ri = 0; ri <= 26; ri++)
            {
                for (po = 1; po <= 23; po++) plotLine(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], ri + 1);
                plotLine(px[ri][23], py[ri][23], px[ri][0], py[ri][0], ri + 1);
            }
            break;

        case 2:
            for (po = 0; po <= 23; po++)
            {
                for (ri = 1; ri <= 26; ri++) plotLine(px[ri - 1][po], py[ri - 1][po], px[ri][po], py[ri][po], ri + 1);
                plotLine(px[26][po], py[26][po], px[0][po], py[0][po], ri + 1);
            }
            break;

        case 3:
            for (ri = 0; ri <= 26; ri++)
            {
                for (po = 1; po <= 23; po++) plotLine(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], ri + 1);
                plotLine(px[ri][23], py[ri][23], px[ri][0], py[ri][0], ri + 1);
            }
            for (po = 0; po <= 23; po++)
            {
                for (ri = 1; ri <= 26; ri++) plotLine(px[ri - 1][po], py[ri - 1][po], px[ri][po], py[ri][po], ri + 1);
                plotLine(px[26][po], py[26][po], px[0][po], py[0][po], ri + 1);
            }
            break;

        case 4:
            for (ri = 0; ri <= 25; ri++)
            {
                for (po = 1; po <= 23; po++) polygon1(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], px[ri + 1][po], py[ri + 1][po], px[ri + 1][po - 1], py[ri + 1][po - 1], ri + 1);
                polygon1(px[ri][23], py[ri][23], px[ri][0], py[ri][0], px[ri + 1][0], py[ri + 1][0], px[ri + 1][23], py[ri + 1][23], ri + 1);
            }

            for (ri = 0; ri < IMAGE_HEIGHT; ri++) vbuff[ri][0] = 0;
            for (ri = 0; ri < IMAGE_HEIGHT; ri++) vbuff[ri][MAX_WIDTH] = 0;
            break;

        case 5:
            for (ri = 0; ri <= 25; ri++)
            {
                for (po = 1; po <= 23; po++) polygon2(px[ri][po - 1], py[ri][po - 1], px[ri][po], py[ri][po], px[ri + 1][po], py[ri + 1][po], px[ri + 1][po - 1], py[ri + 1][po - 1], ri + 1);
                polygon2(px[ri][23], py[ri][23], px[ri][0], py[ri][0], px[ri + 1][0], py[ri + 1][0], px[ri + 1][23], py[ri + 1][23], ri + 1);
            }

            for (ri = 0; ri < IMAGE_HEIGHT; ri++) vbuff[ri][0] = 0;
            for (ri = 0; ri < IMAGE_HEIGHT; ri++) vbuff[ri][MAX_WIDTH] = 0;
            break;

        default:
            break;
        }
    }

    void run()
    {
        uint8_t k;
        uint16_t i, j, x, y;
        RGB pal[256] = { 0 };

        for (i = 0; i < 256; i++)
        {
            cosx[i] = int16_t(cos(i * M_PI / 128) * 60);
            sinx[i] = int16_t(sin(i * M_PI / 128) * 45);
        }

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Hole -- Keys: 1->6: switch effect")) return;

        minx = 0;
        miny = 0;
        maxx = MAX_WIDTH;
        maxy = MAX_HEIGHT;
        x = 30;
        y = 90;
        art = 0;

        for (i = 0; i < 450; i++) sintab[i] = int16_t(sin(2 * M_PI * i / 360) * DIVD);

        for (k = 1; k < 28; k++)
        {
            pal[k].r = k;
            pal[k].g = k << 1;
            pal[k].b = k + 30;
            pal[2 * 27 - k].r = k;
            pal[2 * 27 - k].g = k << 1;
            pal[2 * 27 - k].b = k + 30;
            pal[2 * 27 + k].r = k;
            pal[2 * 27 + k].g = k << 1;
            pal[2 * 27 + k].b = k + 30;
            pal[3 * 27 - k].r = k;
            pal[3 * 27 - k].g = k << 1;
            pal[3 * 27 - k].b = k + 30;
            pal[3 * 27 + k].r = k;
            pal[3 * 27 + k].g = k << 1;
            pal[3 * 27 + k].b = k + 30;
        }

        shiftPalette(pal);
        setPalette(pal);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            step = 2;
            j = 10;
            ring = 0;

            while (j < 220)
            {
                i = 0;
                point = 0;

                while (i < 360)
                {
                    calcPoint(art, cosx[(x + (IMAGE_HEIGHT - j)) & 0xff], sinx[(y + (IMAGE_HEIGHT - j)) & 0xff], j, i);
                    i += STEP;
                    point++;
                }

                j += step;
                if (!(j % 3)) step++;

                ring++;
            }

            x = XST + (x & 0xff);
            y = YST + (y & 0xff);

            memset(vbuff, 0, IMAGE_SIZE);
            drawHole(art);
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            readKeys();
            if (keyDown(SDL_SCANCODE_1)) art = 0;
            if (keyDown(SDL_SCANCODE_2)) art = 1;
            if (keyDown(SDL_SCANCODE_3)) art = 2;
            if (keyDown(SDL_SCANCODE_4)) art = 3;
            if (keyDown(SDL_SCANCODE_5)) art = 4;
            if (keyDown(SDL_SCANCODE_6)) art = 5;
        }
        cleanup();
    }
}

namespace kaleidoScope {
    #define START_COLOR     0
    #define END_COLOR       255

    void rainbowPalette(RGB* pal)
    {
        for (int16_t i = 0; i < 32; i++)
        {
            pal[i      ].r = i << 1;
            pal[63 - i ].r = i << 1;
            pal[i + 64 ].g = i << 1;
            pal[127 - i].g = i << 1;
            pal[i + 128].b = i << 1;
            pal[191 - i].b = i << 1;
            pal[i + 192].r = i << 1;
            pal[i + 192].g = i << 1;
            pal[i + 192].b = i << 1;
            pal[255 - i].r = i << 1;
            pal[255 - i].g = i << 1;
            pal[255 - i].b = i << 1;
        }
    }

    void makePalette()
    {
        RGB pal[256] = { 0 };
        rainbowPalette(pal);
        shiftPalette(pal);
        setPalette(pal);
    }

    void run()
    {
        if (!initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, "Kaleido-Scope")) return;

        const int16_t cx = getDrawBufferWidth() >> 1;
        const int16_t cy = getDrawBufferHeight() >> 1;
        const int16_t md = cy;

        int16_t hc = random(END_COLOR - START_COLOR);

        makePalette();

        do {
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

            int16_t x1 = random(md) + 1;
            int16_t x2 = random(md) + 1;
            int16_t y1 = random(x1);
            int16_t y2 = random(x2);

            while (random(130) > 5)
            {
                readKeys();
                if (keyDown(SDL_SCANCODE_RETURN)) break;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

                const int16_t xv1 = random(10) - 5;
                const int16_t xv2 = random(10) - 5;
                const int16_t yv1 = random(10) - 5;
                const int16_t yv2 = random(10) - 5;

                while (random(100) > 20)
                {
                    readKeys();
                    if (keyDown(SDL_SCANCODE_RETURN)) break;
                    if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

                    const int16_t xa = (x1 << 2) / 3;
                    const int16_t xb = (x2 << 2) / 3;
                    const int16_t ya = (y1 << 2) / 3;
                    const int16_t yb = (y2 << 2) / 3;

                    drawLine(cx + xa, cy - y1, cx + xb, cy - y2, hc);
                    drawLine(cx - ya, cy + x1, cx - yb, cy + x2, hc);
                    drawLine(cx - xa, cy - y1, cx - xb, cy - y2, hc);
                    drawLine(cx - ya, cy - x1, cx - yb, cy - x2, hc);
                    drawLine(cx - xa, cy + y1, cx - xb, cy + y2, hc);
                    drawLine(cx + ya, cy - x1, cx + yb, cy - x2, hc);
                    drawLine(cx + xa, cy + y1, cx + xb, cy + y2, hc);
                    drawLine(cx + ya, cy + x1, cx + yb, cy + x2, hc);

                    x1 = (x1 + xv1) % md;
                    y1 = (y1 + yv1) % md;
                    x2 = (x2 + xv2) % md;
                    y2 = (y2 + yv2) % md;

                    hc++;

                    if (hc > END_COLOR) hc = START_COLOR;

                    render();
                    delay(FPS_90);
                }
            }
            clearScreen();
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace kaleidoScope2 {
    #define MAX_STEP1   50
    #define MAX_STEP2   20

    void makeRainbowPalette()
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 32; i++)
        {
            pal[i].r = i << 1;
            pal[63 - i].r = i << 1;
            pal[i + 64].g = i << 1;
            pal[127 - i].g = i << 1;
            pal[i + 128].b = i << 1;
            pal[191 - i].b = i << 1;
            pal[i + 192].r = i << 1;
            pal[i + 192].g = i << 1;
            pal[i + 192].b = i << 1;
            pal[255 - i].r = i << 1;
            pal[255 - i].g = i << 1;
            pal[255 - i].b = i << 1;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void makeFunkyPalette()
    {
        RGB pal[256] = { 0 };

        int16_t r = 0, g = 0, b = 0;
        int16_t ry = 1, gy = 1, by = 1;

        int16_t rx = (rand() % 5) + 1;
        int16_t gx = (rand() % 5) + 1;
        int16_t bx = (rand() % 5) + 1;

        for (int16_t i = 0; i < 256; i++)
        {
            pal[i].r = uint8_t(r);
            pal[i].g = uint8_t(g);
            pal[i].b = uint8_t(b);

            if (ry) r += rx; else r -= rx;
            if (gy) g += gx; else g -= gx;
            if (by) b += bx; else b -= bx;

            if ((r + rx > 63) || (r - rx < 0))
            {
                ry = !ry;
                rx = (rand() % 5) + 1;
            }

            if ((g + gx > 63) || (g - gx < 0))
            {
                gy = !gy;
                gx = (rand() % 5) + 1;
            }

            if ((b + bx > 63) || (b - bx < 0))
            {
                by = !by;
                bx = (rand() % 5) + 1;
            }
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void scrollPalette(int16_t from, int16_t to, int16_t step)
    {
        RGB tmp = { 0 };
        RGB pal[256] = { 0 };

        getPalette(pal);

        while (step--)
        {
            memcpy(&tmp, &pal[from], sizeof(tmp));
            memcpy(&pal[from], &pal[from + 1], (intptr_t(to) - from) * sizeof(RGB));
            memcpy(&pal[to], &tmp, sizeof(tmp));
        }

        setPalette(pal);
    }

    void run(uint8_t mode)
    {
        if (!initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, "Kaleido-Scope")) return;

        if (mode) makeRainbowPalette();

        const int32_t cx = getCenterX();
        const int32_t cy = getCenterY();
        const int32_t md = cy;

        do {
            if (!mode) makeFunkyPalette();

            uint32_t step1 = 0;
            int32_t x1 = random(md) + 1;
            int32_t x2 = random(md) + 1;
            int32_t y1 = random(x1);
            int32_t y2 = random(x2);

            while (step1 < MAX_STEP1)
            {
                uint32_t step2 = 0;
                const int32_t xv1 = random(5) - 2;
                const int32_t xv2 = random(5) - 2;
                const int32_t yv1 = random(5) - 2;
                const int32_t yv2 = random(5) - 2;

                while (step2 < MAX_STEP2)
                {
                    const int32_t xa = (x1 << 2) / 3;
                    const int32_t xb = (x2 << 2) / 3;
                    const int32_t ya = (y1 << 2) / 3;
                    const int32_t yb = (y2 << 2) / 3;

                    drawLineBob(cx + xa, cy - y1, cx + xb, cy - y2);
                    drawLineBob(cx - ya, cy + x1, cx - yb, cy + x2);
                    drawLineBob(cx - xa, cy - y1, cx - xb, cy - y2);
                    drawLineBob(cx - ya, cy - x1, cx - yb, cy - x2);
                    drawLineBob(cx - xa, cy + y1, cx - xb, cy + y2);
                    drawLineBob(cx + ya, cy - x1, cx + yb, cy - x2);
                    drawLineBob(cx + xa, cy + y1, cx + xb, cy + y2);
                    drawLineBob(cx + ya, cy + x1, cx + yb, cy + x2);

                    render();
                    delay(FPS_90);

                    x1 = (x1 + xv1) % md;
                    y1 = (y1 + yv1) % md;
                    x2 = (x2 + xv2) % md;
                    y2 = (y2 + yv2) % md;

                    step2++;
                    readKeys();
                    if (keyDown(SDL_SCANCODE_RETURN)) break;
                    if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
                }

                step1++;
                readKeys();
                if (keyDown(SDL_SCANCODE_RETURN)) break;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }

            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            clearScreen();
            if (mode) scrollPalette(0, 255, 64);
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace fastCircleFill {
    #define MAX_RAD 64

    RGB pal[256] = { 0 };

    void makePalette(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
    {
        int16_t i = 0;
        int16_t white = 10;

        for (i = 0; i <= 63 - white; i++)
        {
            pal[n + i].r = ((r * i) / (63 - white)) << 2;
            pal[n + i].g = ((g * i) / (63 - white)) << 2;
            pal[n + i].b = ((b * i) / (63 - white)) << 2;
        }

        for (i = 0; i <= white; i++)
        {
            pal[n + i + 63 - white].r = (r + (63 - r) * i / white) << 2;
            pal[n + i + 63 - white].g = (g + (63 - g) * i / white) << 2;
            pal[n + i + 63 - white].b = (b + (63 - b) * i / white) << 2;
        }
    }

    void run()
    {
        if (!initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, "Fast Filled-Circle")) return;
        makePalette(0, 63, 32, 16);
        makePalette(64, 32, 63, 16);
        makePalette(128, 16, 16, 63);
        makePalette(128 + 64, 63, 16, 16);
        setPalette(pal);
        
        const int32_t cmx = getMaxX();
        const int32_t cmy = getMaxY();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            const uint32_t x = rand() % cmx;
            const uint32_t y = rand() % cmy;
            const uint32_t col = (rand() % 4) * MAX_RAD;
            for (uint32_t i = 0; i < MAX_RAD; i++) fillCircle(x + ((MAX_RAD - i) >> 1), y + ((MAX_RAD - i) >> 1), (MAX_RAD - i) << 1, i + col);
            render();
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace flagsEffect {
    #define AMPLI   6
    #define FSPEED  4

    uint16_t index = 0;
    uint16_t costab[256] = { 0 };
    uint8_t vbuff1[IMAGE_MIDY][IMAGE_MIDX] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void createCosTable()
    {
        for (int16_t i = 0; i < 256; i++) costab[i] = uint16_t(cos(i * M_PI / 64) * AMPLI);
    }

    int16_t initTexture()
    {
        RGB rgb[256] = { 0 };
        if (!loadPNG(vbuff1[0], rgb, "assets/skull.png")) return 0;
        setPalette(rgb);
        return 1;
    }

    void putImage()
    {
        for (int16_t y = 0; y < IMAGE_MIDY; y++)
        {
            for (int16_t x = 0; x < IMAGE_MIDX; x++)
            {
                if (vbuff1[y][x] != 1) vbuff2[y][x] = vbuff1[y][x];
            }
        }
    }

    void displayMap()
    {
        for (int16_t y = 0; y < IMAGE_MIDY; y++)
        {
            for (int16_t x = 0; x < IMAGE_MIDX; x++)
            {
                const int16_t xpos = (x << 1) + costab[(index + 125 * (x + y)) & 0xff];
                const int16_t ypos = (y << 1) + costab[(index + 3 * x + 125 * y) & 0xff];
                if (xpos >= 0 && xpos <= MAX_WIDTH && ypos >= 0 && ypos <= MAX_HEIGHT) vbuff2[ypos][xpos] = vbuff1[y][x];
            }
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Flags")) return;
        if (!initTexture()) return;
        
        createCosTable();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            displayMap();
            putImage();
            renderBuffer(vbuff2, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            memset(vbuff2, 0, IMAGE_SIZE);
            index += FSPEED;
        }
        cleanup();
    }
}

namespace flagsEffect2 {
    #define FX 3
    #define FY 3
    #define CE 191

    int16_t sintab[256] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void run()
    {
        int16_t i = 0, j = 0;
        uint16_t wave = 0;
        uint8_t col = 0;

        for (i = 0; i < 256; i++) sintab[i] = int16_t(sin(8 * M_PI * i / 255) * 10);

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Flags")) return;
        makeRainbowPalette();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            wave++;

            for (i = 0; i <= 280 / FX; i++)
            {
                for (j = 0; j <= IMAGE_MIDX / FY; j++)
                {
                    //Deutschland
                    //if (j * FY < 53) col = 8;
                    //else if (j * FY < 106) col = 4; else col = 14;

                    //French
                    //if (j * FY < 53) col = 4;
                    //else if (j * FY < 106) col = 15; else col = 9;

                    //Schweden
                    //if (i * FX > 80 && i * FX < 100 && j * FY >= 0 &&
                    //	j * FY < 160 || j * FY > 70 && j * FY < 90 &&
                    //	i * FX >= 0 && i * FX < 280) col = 15; else col = 4;

                    //Schweiz
                    //if (i * FX > 120 && i * FX < 160 && j * FY > 20 &&
                    //	j * FY < 140 || j * FY > 60 && j * FY < 100 &&
                    //	i * FX > 60 && i * FX < 220) col = 15; else col = 4;

                    //USA
                    if (i * FX < 120 && j * FY < 85)
                    {
                        if (((j % 3) == 2 && (i % 7) == 2 && (j % 6) == 2) || (((3 + i) % 7) == 2 && ((j + 3) % 6) == 2)) col = 220;
                        else col = 150;
                    }
                    else
                    {
                        if ((j * FY) % 25 < 12) col = 40;
                        else col = 220;
                    }

                    const int16_t x = 20 + sintab[(wave + (j + i) * CE) & 0xff] + i * FX;
                    const int16_t y = 20 + sintab[(wave + j + i * CE) & 0xff] + j * FY;
                    if (x >=0 && x <= MAX_WIDTH && y >= 0 && y <= MAX_HEIGHT) vmem[y][x] = col;
                    
                }
            }

            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            memset(vmem, 0, IMAGE_SIZE);
        }
        cleanup();
    }
}

namespace lakeEffect {
    #define STARTY  72
    #define HEIGHT  2 * STARTY

    uint8_t sintab[400] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t dbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t bitmap[STARTY][IMAGE_WIDTH] = { 0 };
    uint8_t water[STARTY][IMAGE_WIDTH] = { 0 };
    
    void calcWater()
    {
        int16_t i = 0, val = 0;

        for (i = 1; i <= STARTY; i++)
        {
            val = sintab[i];
            memcpy(&water[STARTY - i][val], &bitmap[i - 1][0], intptr_t(IMAGE_WIDTH) - val);
        }

        val = sintab[0];
        memcpy(&sintab[0], &sintab[1], HEIGHT - 1);
        sintab[HEIGHT - 2] = uint8_t(val);
    }

    void run()
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 400; i++) sintab[i] = uint8_t(sin(5 * M_PI * i / 100) * 3 + 3);

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Lake")) return;
        if (!loadPNG(vbuff[0], pal, "assets/palio.png")) return;
        setPalette(pal);

        memcpy(dbuff, vbuff, IMAGE_SIZE);
        memcpy(bitmap, &vbuff[IMAGE_HEIGHT - HEIGHT][0], STARTY * IMAGE_WIDTH);
        
        while (!finished(SDL_SCANCODE_RETURN))
        {
            calcWater();
            memcpy(&dbuff[IMAGE_HEIGHT - STARTY][0], water, STARTY * IMAGE_WIDTH);
            renderBuffer(dbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace landScapeGeneration {
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    uint8_t getColor(int16_t mc, int16_t n, int16_t dvd)
    {
        if (dvd == 0) return 0;
        return (mc + n - random(2 * n)) / dvd;
    }

    void calcPlasma(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
    {
        if (x2 - x1 < 2 && y2 - y1 < 2) return;

        const int16_t p1 = vbuff[y1][x1];
        const int16_t p2 = vbuff[y2][x1];
        const int16_t p3 = vbuff[y1][x2];
        const int16_t p4 = vbuff[y2][x2];

        const int16_t xn = (x2 + x1) >> 1;
        const int16_t yn = (y2 + y1) >> 1;
        const int16_t dxy = (x2 - x1 + y2 - y1) * 5 / 3;

        if (!vbuff[y1][xn]) vbuff[y1][xn] = getColor(p1 + p3, dxy, 2);
        if (!vbuff[yn][x1]) vbuff[yn][x1] = getColor(p1 + p2, dxy, 2);
        if (!vbuff[yn][x2]) vbuff[yn][x2] = getColor(p3 + p4, dxy, 2);
        if (!vbuff[y2][xn]) vbuff[y2][xn] = getColor(p2 + p4, dxy, 2);

        vbuff[yn][xn] = getColor(p1 + p2 + p3 + p4, dxy, 4);

        calcPlasma(x1, y1, xn, yn);
        calcPlasma(xn, y1, x2, yn);
        calcPlasma(x1, yn, xn, y2);
        calcPlasma(xn, yn, x2, y2);
    }

    void run()
    {
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "LandScape-Generation")) return;

        for (uint8_t i = 0; i < 64; i++)
        {
            pal[i].r = 0;
            pal[i].g = 0;
            pal[i].b = i;
            pal[i + 63].r = 63 - (i >> 2);
            pal[i + 63].g = 63 - (i >> 1);
            pal[i + 63].b = 0;
            pal[i + 127].r = 0;
            pal[i + 127].g = 63 - (i >> 1);
            pal[i + 127].b = 0;
            pal[i + 192].r = i;
            pal[i + 192].g = i;
            pal[i + 192].b = i;
        }

        shiftPalette(pal);
        setPalette(pal);
        calcPlasma(0, 0, MAX_WIDTH, MAX_HEIGHT);
        renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
        while (!finished(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace landScapeEffect {
    #define DENT    3
    #define ROUGH   26
    #define HMAX    128
    #define XMAX    (320 / DENT)
    #define YMAX    (120 / DENT)
    #define MPOSX   (IMAGE_WIDTH - XMAX)
    #define MPOSY   (IMAGE_HEIGHT - YMAX)

    uint8_t dbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    void setPalette()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };

        for (i = 0; i <= 15; i++)
        {
            pal[i].r = 0;
            pal[i].g = 0;
            pal[i].b = i << 2;
        }

        for (i = 0; i <= 31; i++)
        {
            pal[i + 16].r = 63 - (i >> 1);
            pal[i + 16].g = 63 - i;
            pal[i + 16].b = 0;
        }

        for (i = 0; i <= 31; i++)
        {
            pal[i + 48].r = 0;
            pal[i + 48].g = 63 - i;
            pal[i + 48].b = 0;
        }

        for (i = 0; i <= 48; i++)
        {
            pal[i + 80].r = i + 16;
            pal[i + 80].g = i + 16;
            pal[i + 80].b = i + 16;
        }

        pal[255].r = 63;
        pal[255].g = 0;
        pal[255].b = 0;

        shiftPalette(pal);
        setPalette(pal);
    }

    void adjust(int16_t xa, int16_t ya, int16_t x, int16_t y, int16_t xb, int16_t yb)
    {
        if (dbuff[y][x]) return;

        const int16_t dist = abs(xa - xb) + abs(ya - yb);
        uint8_t col = (50 * (dbuff[ya][xa] + dbuff[yb][xb]) + (10 * (1 + random(10)) / 11 - 5) * dist * ROUGH) / 100;
        if (col > HMAX) col = HMAX;
        dbuff[y][x] = col;
    }

    void subDivide(int16_t l, int16_t t, int16_t r, int16_t b)
    {
        if (r - l < 2 && b - t < 2) return;

        const int16_t x = (l + r) >> 1;
        const int16_t y = (t + b) >> 1;

        adjust(l, t, x, t, r, t);
        adjust(r, t, r, y, r, b);
        adjust(l, b, x, b, r, b);
        adjust(l, t, l, y, l, b);

        if (!dbuff[y][x])
        {
            uint8_t col = (dbuff[t][l] + dbuff[t][r] + dbuff[b][l] + dbuff[b][r]) >> 2;
            if (col > HMAX) col = HMAX;
            dbuff[y][x] = col;
        }

        subDivide(l, t, x, y);
        subDivide(x, t, r, y);
        subDivide(l, y, x, b);
        subDivide(x, y, r, b);
    }

    void generateLandScape()
    {
        FILE* fp = fopen("assets/land.map", "rb");

        if (fp)
        {
            fread(dbuff, IMAGE_SIZE, 1, fp);
            fclose(fp);
        }
        else
        {
            subDivide(0, 0, MAX_WIDTH, MAX_HEIGHT);
            fp = fopen("assets/land.map", "wb");
            if (fp)
            {
                fwrite(dbuff, IMAGE_SIZE, 1, fp);
                fclose(fp);
            }
        }
        
        memcpy(vbuff, dbuff, IMAGE_SIZE);

        for (int16_t i = 0; i < IMAGE_HEIGHT; i++)
        {
            for (int16_t j = 0; j < IMAGE_WIDTH; j++)
            {
                vbuff[i][j] = (vbuff[i][j] >> 1) + 110;
                if (vbuff[i][j] < 115) vbuff[i][j] = 115;
            }
        }

        memset(dbuff, 0, IMAGE_SIZE);
    }

    void showLandscape()
    {
        int32_t i = 0, j = 0, lmb = 0;
        
        showMouseCursor(SDL_DISABLE);
        setMousePosition(IMAGE_WIDTH, IMAGE_HEIGHT);

        do {
            memset(dbuff, 0, IMAGE_SIZE);
            getMouseState(&i, &j, &lmb);

            i >>= 1;
            j >>= 1;

            if (i > MPOSX) i = MPOSX;
            if (j > MPOSY) j = MPOSY;
            
            for (int16_t n = 0; n < XMAX * YMAX; n++)
            {
                const int16_t x = -(DENT * (n % XMAX - (XMAX >> 1) - 1) * 45) / (n / XMAX - 45) - 153;
                if (x > -317 && x < -3)
                {
                    const uint8_t col = vbuff[n / XMAX + j][n % XMAX + i];
                    dbuff[DENT * (n / XMAX) - col + 198][x] = col - 100;
                }
            }

            renderBuffer(dbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        } while (!finished(SDL_SCANCODE_RETURN) && !lmb);
        showMouseCursor(SDL_ENABLE);
        cleanup();
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "LandScape -- Move your mouse, left click to exit...")) return;
        setPalette();
        generateLandScape();
        setMousePosition(IMAGE_WIDTH, IMAGE_HEIGHT);
        showLandscape();
    }
}

namespace lensEffect {
    #define RDS 30

    uint8_t     vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void stretch2Lines(int16_t x1, int16_t x2, int16_t y1, int16_t y2, int16_t yr1, int16_t yw1, int16_t yr2, int16_t yw2)
    {
        const int16_t dx = abs(x2 - x1);
        const int16_t dy = abs(y2 - y1);

        const int16_t sx = sign(x2 - x1);
        const int16_t sy = sign(y2 - y1);

        int16_t err = (dy << 1) - dx;

        const int16_t dx2 = dx << 1;
        const int16_t dy2 = dy << 1;

        for (int16_t d = 0; d <= dx; d++)
        {
            vbuff1[yw1][x1] = vbuff2[yr1][y1];
            vbuff1[yw2][x1] = vbuff2[yr2][y1];

            while (err >= 0)
            {
                y1 += sy;
                err -= dx2;
            }

            x1 += sx;
            err += dy2;
        }
    }

    void circleStretch(int16_t x0, int16_t y0, int16_t xc, int16_t yc, int16_t r)
    {
        int16_t p = 3 - (r << 1);
        int16_t x = 0;
        int16_t y = r;

        while (x < y)
        {
            stretch2Lines(xc - y, xc + y, x0, x0 + (r << 1), r - x + y0, yc - x, r + x + y0, yc + x);
            if (p < 0) p = p + (x << 2) + 6;
            else
            {
                stretch2Lines(xc - x, xc + x, x0, x0 + (r << 1), r - y + y0, yc - y, r + y + y0, yc + y);
                p = p + ((x - y) << 2) + 10;
                y--;
            }
            x++;
        }

        if (x == y) stretch2Lines(xc - x, xc + x, x0, x0 + (r << 1), r - y + y0, yc - y, r + y + y0, yc + y);
    }

    void run()
    {
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Lens")) return;
        if (!loadPNG(vbuff2[0], pal, "assets/sunflow.png")) return;
        setPalette(pal);

        int16_t x = IMAGE_MIDX;
        int16_t y = IMAGE_MIDY;

        int16_t xadd = 1;
        int16_t yadd = 1;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            memcpy(vbuff1, vbuff2, IMAGE_SIZE);
            circleStretch(x - RDS, y - RDS, x, y, RDS);
            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);

            x += xadd;
            y += yadd;

            if (x < 32) xadd = -xadd;
            if (x > MAX_WIDTH - 32) xadd = -xadd;
            if (y < 32) yadd = -yadd;
            if (y > MAX_HEIGHT - 32) yadd = -yadd;
        }
        cleanup();
    }
}

namespace zoomInEffect {
    #define ZR 40
    #define ZH 18
    #define ZD 80

    const uint8_t hand0[][16] = {
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
        {0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0},
        {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1},
        {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1},
        {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
    };

    const uint8_t hand1[][16] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0},
        {0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1},
        {0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0}
    };

    int16_t lens[ZD][ZD] = { 0 };
    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void calcMask()
    {
        for (int16_t y = 0; y < ZD; y++)
        {
            for (int16_t x = 0; x < ZD; x++)
            {
                const int16_t ux = x - (ZD >> 1);
                const int16_t uy = y - (ZD >> 1);

                if (ux * ux + uy * uy < ZR * ZR)
                {
                    const double z = sqrt(double(ZR) * ZR - double(ux) * ux - double(uy) * uy);
                    const int16_t sx = int16_t((ZH - z) * (ux / z));
                    const int16_t sy = int16_t((ZH - z) * (uy / z));
                    lens[y][x] = sy * IMAGE_WIDTH + sx;
                }
                else lens[y][x] = 0;
            }
        }
    }

    void putHand(uint8_t n, uint16_t x, uint16_t y)
    {
        for (int16_t i = 0; i < 17; i++)
        {
            for (int16_t j = 0; j < 16; j++)
            {
                switch (n)
                {
                case 0: if (hand0[i][j]) vbuff2[y + i + ZR - 8][x + j + ZR - 8] = 255; break;
                case 1: if (hand1[i][j]) vbuff2[y + i + ZR - 8][x + j + ZR - 8] = 255; break;
                default: break;
                }
            }
        }
    }

    void construct(uint8_t n, uint16_t xp, uint16_t yp)
    {
        memcpy(vbuff2, vbuff1, IMAGE_SIZE);

        for (int16_t y = 0; y < ZD; y++)
        {
            for (int16_t x = 0; x < ZD; x++)
            {
                const int16_t ux = x - (ZD >> 1);
                const int16_t uy = y - (ZD >> 1);
                const int16_t vp = y + yp + lens[y][x] / IMAGE_WIDTH;
                const int16_t hp = x + xp + lens[y][x] % IMAGE_WIDTH;
                if (vp < IMAGE_HEIGHT && vp > 0 && xp < IMAGE_WIDTH && xp > 0 && (ZR - 1) * (ZR - 1) > ux * ux + uy * uy) vbuff2[y + yp][x + xp] = vbuff1[vp][hp];
            }
        }

        putHand(n, xp, yp);
    }

    void run()
    {
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Len Zoom-In -- Move your mouse, left click to exit...")) return;
        if (!loadPNG(vbuff1[0], pal, "assets/insect.png")) return;
        setPalette(pal);

        calcMask();
        setMousePosition(56 << 1, 32 << 1);
        showMouseCursor(SDL_DISABLE);
        
        int32_t n = 0, m = 0;
        int32_t x = 0, y = 0, lmb = 0;

        do {
            n++;
            getMouseState(&x, &y, &lmb);
            x >>= 1;
            y >>= 1;

            if (x < 5) x = 5;
            if (y < 5) y = 5;
            if (x > IMAGE_WIDTH - ZD - 5) x = IMAGE_WIDTH - ZD - 5;
            if (y > IMAGE_HEIGHT - ZD) y = IMAGE_HEIGHT - ZD;

            if (!(n % 20))
            {
                n = 0;
                m = (!m) ? 1 : 0;
            }

            construct(m, x, y);
            renderBuffer(vbuff2, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        } while (!finished(SDL_SCANCODE_RETURN) && !lmb);
        showMouseCursor(SDL_ENABLE);
        cleanup();
    }
}

namespace zoomOutEffect {
    int16_t ztab[64] = { 0 };
    int16_t xtab[64][64] = { 0 };
    int16_t ytab[64][64] = { 0 };
    int16_t xpostab[SIZE_256] = { 0 };
    int16_t ypostab[SIZE_256] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void calcMatrix()
    {
        for (int16_t y = -32; y < 32; y++)
        {
            for (int16_t x = -32; x < 32; x++)
            {
                int16_t tz = int16_t(sqrt(4.0 * x * x + 4.0 * y * y));
                tz = ztab[tz >> 1];
                const int16_t tx = x * tz / 2300;
                const int16_t ty = y * tz / 2300;
                xtab[y + 32][x + 32] = tx;
                ytab[y + 32][x + 32] = ty;
            }
        }
    }

    void preCalculate()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };

        double v = 0.0;
        const double vadd = M_PI / 128;

        for (i = 0; i < 256; i++)
        {
            xpostab[i] = int16_t(sin(v) * 95 + IMAGE_MIDX);
            ypostab[i] = int16_t(sin(v) * 35 + IMAGE_MIDY);
            v += vadd;
        }

        v = M_PI / 2;
        for (i = 0; i < 64; i++) ztab[i] = int16_t(sin(v + i * vadd) * 2500);
        calcMatrix();

        if (!loadPNG(vmem[0], pal, "assets/drunken.png")) return;
        setPalette(pal);
        memcpy(vbuff, vmem, IMAGE_SIZE);
    }

    void copyFromBuffer(int16_t x, int16_t y)
    {
        if (!x && !y) return;

#ifdef _USE_ASM
        __asm {
            lea     edi, vmem
            lea     esi, vbuff
            xor     eax, eax
            mov     ax, y
            mov     dx, IMAGE_WIDTH
            mul     dx
            add     ax, x
            add     edi, eax
            add     esi, eax
            sub     esi, 10272
            sub     edi, 10272
            mov     edx, 64
        again:
            mov     ecx, 16
            rep     movsd
            add     esi, SIZE_256
            add     edi, SIZE_256
            dec     edx
            jnz     again
        }
#else
        int16_t u = y - 32, v = x - 32;
        for (int16_t i = 0; i < 64; i++) memcpy(&vmem[u + i][v], &vbuff[u + i][v], 64);
#endif
    }

    void printGlass(int16_t x, int16_t y)
    {
#ifdef _USE_ASM
        __asm {
            xor     esi, esi
            xor     edi, edi
            xor     eax, eax
            mov     ax, y
            mov     dx, IMAGE_WIDTH
            mul     dx
            add     ax, x
            add     edi, eax
            sub     edi, 20544
            mov     bx, ax
            mov     ch, 64
        again:
            mov     cl, 64
        next:
            mov     ax, ytab[esi]
            mov     dx, IMAGE_WIDTH
            mul     dx
            add     ax, xtab[esi]
            add     ax, bx
            mov     dl, vbuff[edi]
            mov     vmem[eax], dl
            add     edi, 2
            add     esi, 2
            dec     cl
            jnz     next
            add     edi, SIZE_512
            dec     ch
            jnz     again
        }
#else
        int16_t u = 0, v = 0;
        int16_t m = y - 64, n = x - 64;

        for (int16_t i = 0; i < 64; i++, v += 2, u = 0)
        {
            for (int16_t j = 0; j < 64; j++)
            {
                vmem[y + ytab[i][j]][x + xtab[i][j]] = vbuff[m + v][n + u];
                u += 2;
            }
        }
#endif
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Len Zoom-Out")) return;
        preCalculate();

        int16_t oldx = 0;
        int16_t oldy = 0;
        uint16_t xpos = 40;
        uint16_t ypos = 60;
        const uint16_t xadd = 2;
        const uint16_t yadd = 1;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            copyFromBuffer(oldx, oldy);
            const int16_t x = xpostab[xpos & 0xff];
            const int16_t y = ypostab[ypos & 0xff];
            printGlass(x, y);
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            oldx = x;
            oldy = y;
            xpos += xadd;
            ypos += yadd;
        }
        cleanup();
    }
}

namespace lineBobEffect {

    void checkBounds(int32_t a, int32_t* b, int32_t c)
    {
        if (a >= c) *b = -1;
        else if (a <= 0) *b = 1;
    }

    void lineBob(uint32_t cnt)
    {
        const int32_t cmx = getMaxX();
        const int32_t cmy = getMaxY();
        const int32_t cwidth = getDrawBufferWidth();
        const int32_t cheight = getDrawBufferHeight();

        int32_t x1 = rand() % cwidth;
        int32_t x2 = rand() % cwidth;
        int32_t y1 = rand() % cheight;
        int32_t y2 = rand() % cheight;

        int32_t dx1 = 1;
        int32_t dx2 = -1;
        int32_t dy1 = 1;
        int32_t dy2 = -1;

        while (!keyDown(SDL_SCANCODE_RETURN) && cnt < 2000U + random(4000))
        {
            x1 += dx1;
            x2 += dx2;
            y1 += dy1;
            y2 += dy2;

            checkBounds(x1, &dx1, cmx);
            checkBounds(x2, &dx2, cmx);
            checkBounds(y1, &dy1, cmy);
            checkBounds(y2, &dy2, cmy);

            if ((cnt % 10) == 0) delay(10);

            drawLineBob(x1, y1, x2, y2);
            render();
            readKeys();
            cnt++;
        }
    }

    void makeFunkyPalette()
    {
        RGB pal[256] = { 0 };

        int16_t r = 0, g = 0, b = 0;
        int16_t ry = 1, gy = 1, by = 1;

        uint8_t rx = (rand() % 5) + 1;
        uint8_t gx = (rand() % 5) + 1;
        uint8_t bx = (rand() % 5) + 1;

        for (int16_t i = 0; i < 256; i++)
        {
            pal[i].r = uint8_t(r);
            pal[i].g = uint8_t(g);
            pal[i].b = uint8_t(b);

            if (ry) r += rx; else r -= rx;
            if (gy) g += gx; else g -= gx;
            if (by) b += bx; else b -= bx;

            if ((r + rx > 63) || (r - rx < 0))
            {
                ry = !ry;
                rx = (rand() % 5) + 1;
            }

            if ((g + gx > 63) || (g - gx < 0))
            {
                gy = !gy;
                gx = (rand() % 5) + 1;
            }

            if ((b + bx > 63) || (b - bx < 0))
            {
                by = !by;
                bx = (rand() % 5) + 1;
            }
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void run()
    {
        uint32_t cnt = 0;
        if (!initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, "Line-Bob")) return;

        do {
            cnt = 0;
            readKeys();
            makeFunkyPalette();
            lineBob(cnt);
            clearScreen();
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace lineBlurEffect {
    #define LSPEED   2
    #define PLUS    1
    #define MINUS   0

    typedef struct {
        int16_t x0, y0, x1, y1;
        int16_t dirx0, diry0;
        int16_t dirx1, diry1;
    } TPoint;

    TPoint      points;

    RGB         curpal[1024] = { 0 };
    RGB         oldpal[6][1024] = { 0 };
    int16_t     flag = 1;
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void setColors()
    {
        int16_t i = 0, j = 0;

        for (i = 0, j = 1; j < 256; i += 4, j++)
        {
            curpal[i + 0].r = uint8_t(j);
            curpal[i + 1].r = uint8_t(j);
            curpal[i + 2].r = uint8_t(j);
            curpal[i + 3].r = uint8_t(j);

            curpal[i + 0].g = 0;
            curpal[i + 1].g = 0;
            curpal[i + 2].g = 0;
            curpal[i + 3].g = 0;

            curpal[i + 0].b = 0;
            curpal[i + 1].b = 0;
            curpal[i + 2].b = 0;
            curpal[i + 3].b = 0;
        }

        for (i = 0, j = 1; j < 256; i += 4, j++)
        {
            oldpal[0][i + 0].r = 0;
            oldpal[0][i + 1].r = 0;
            oldpal[0][i + 2].r = 0;
            oldpal[0][i + 3].r = 0;

            oldpal[0][i + 0].g = uint8_t(j);
            oldpal[0][i + 1].g = uint8_t(j);
            oldpal[0][i + 2].g = uint8_t(j);
            oldpal[0][i + 3].g = uint8_t(j);

            oldpal[0][i + 0].b = 0;
            oldpal[0][i + 1].b = 0;
            oldpal[0][i + 2].b = 0;
            oldpal[0][i + 3].b = 0;
        }

        for (i = 0, j = 1; j < 256; i += 4, j++)
        {
            oldpal[1][i + 0].r = 0;
            oldpal[1][i + 1].r = 0;
            oldpal[1][i + 2].r = 0;
            oldpal[1][i + 3].r = 0;

            oldpal[1][i + 0].g = 0;
            oldpal[1][i + 1].g = 0;
            oldpal[1][i + 2].g = 0;
            oldpal[1][i + 3].g = 0;

            oldpal[1][i + 0].b = uint8_t(j);
            oldpal[1][i + 1].b = uint8_t(j);
            oldpal[1][i + 2].b = uint8_t(j);
            oldpal[1][i + 3].b = uint8_t(j);
        }

        for (i = 0, j = 1; j < 256; i += 4, j++)
        {
            oldpal[2][i + 0].r = uint8_t(j);
            oldpal[2][i + 1].r = uint8_t(j);
            oldpal[2][i + 2].r = uint8_t(j);
            oldpal[2][i + 3].r = uint8_t(j);

            oldpal[2][i + 0].g = uint8_t(j);
            oldpal[2][i + 1].g = uint8_t(j);
            oldpal[2][i + 2].g = uint8_t(j);
            oldpal[2][i + 3].g = uint8_t(j);

            oldpal[2][i + 0].b = uint8_t(j);
            oldpal[2][i + 1].b = uint8_t(j);
            oldpal[2][i + 2].b = uint8_t(j);
            oldpal[2][i + 3].b = uint8_t(j);
        }

        for (i = 0, j = 1; j < 256; i += 4, j++)
        {
            oldpal[3][i + 0].r = uint8_t(j);
            oldpal[3][i + 1].r = uint8_t(j);
            oldpal[3][i + 2].r = uint8_t(j);
            oldpal[3][i + 3].r = uint8_t(j);

            oldpal[3][i + 0].g = uint8_t(j);
            oldpal[3][i + 1].g = uint8_t(j);
            oldpal[3][i + 2].g = uint8_t(j);
            oldpal[3][i + 3].g = uint8_t(j);

            oldpal[3][i + 0].b = 0;
            oldpal[3][i + 1].b = 0;
            oldpal[3][i + 2].b = 0;
            oldpal[3][i + 3].b = 0;
        }

        for (i = 0, j = 1; j < 64; i += 4, j++)
        {
            oldpal[4][i + 0].r = uint8_t(j);
            oldpal[4][i + 1].r = uint8_t(j);
            oldpal[4][i + 2].r = uint8_t(j);
            oldpal[4][i + 3].r = uint8_t(j);

            oldpal[4][i + 0].g = 0;
            oldpal[4][i + 1].g = 0;
            oldpal[4][i + 2].g = 0;
            oldpal[4][i + 3].g = 0;

            oldpal[4][i + 0].b = uint8_t(j);
            oldpal[4][i + 1].b = uint8_t(j);
            oldpal[4][i + 2].b = uint8_t(j);
            oldpal[4][i + 3].b = uint8_t(j);
        }

        for (i = 0, j = 1; j < 64; i += 4, j++)
        {
            oldpal[5][i + 0].r = 0;
            oldpal[5][i + 1].r = 0;
            oldpal[5][i + 2].r = 0;
            oldpal[5][i + 3].r = 0;

            oldpal[5][i + 0].g = uint8_t(j);
            oldpal[5][i + 1].g = uint8_t(j);
            oldpal[5][i + 2].g = uint8_t(j);
            oldpal[5][i + 3].g = uint8_t(j);

            oldpal[5][i + 0].b = uint8_t(j);
            oldpal[5][i + 1].b = uint8_t(j);
            oldpal[5][i + 2].b = uint8_t(j);
            oldpal[5][i + 3].b = uint8_t(j);
        }
    }

    void changeColor(int16_t k)
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 256; i++)
        {
            if (curpal[i].r < oldpal[k][i].r) curpal[i].r++;
            if (curpal[i].r > oldpal[k][i].r) curpal[i].r--;
            if (curpal[i].g < oldpal[k][i].g) curpal[i].g++;
            if (curpal[i].g > oldpal[k][i].g) curpal[i].g--;
            if (curpal[i].b < oldpal[k][i].b) curpal[i].b++;
            if (curpal[i].b > oldpal[k][i].b) curpal[i].b--;
            flag = (curpal[i].r == oldpal[k][i].r) && (curpal[i].g == oldpal[k][i].g) && (curpal[i].b == oldpal[k][i].b) ? 0 : 1;
        }

        memcpy(pal, curpal, sizeof(pal));
        shiftPalette(pal);
        setPalette(pal);
    }

    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t col)
    {
        const int16_t dx = abs(x2 - x1);
        const int16_t dy = abs(y2 - y1);
        int16_t x = x1;
        int16_t y = y1;
        int16_t xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0;
        int16_t den = 0, num = 0, numadd = 0, numpixels = 0, curpixel = 0;

        if (x2 >= x1)
        {
            xinc1 = 1;
            xinc2 = 1;
        }
        else
        {
            xinc1 = -1;
            xinc2 = -1;
        }

        if (y2 >= y1)
        {
            yinc1 = 1;
            yinc2 = 1;
        }
        else
        {
            yinc1 = -1;
            yinc2 = -1;
        }

        if (dx >= dy)
        {
            xinc1 = 0;
            yinc2 = 0;
            den = dx;
            num = dx >> 1;
            numadd = dy;
            numpixels = dx;
        }
        else
        {
            xinc2 = 0;
            yinc1 = 0;
            den = dy;
            num = dy >> 1;
            numadd = dx;
            numpixels = dy;
        }

        for (curpixel = 0; curpixel < numpixels; curpixel++)
        {
            vbuff[y][x] = col;
            num += numadd;
            if (num >= den)
            {
                num -= den;
                x += xinc1;
                y += yinc1;
            }
            x += xinc2;
            y += yinc2;
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Line-Blur")) return;
        setColors();

        memset(vbuff, 0, IMAGE_SIZE);
        memset(&points, 0, sizeof(TPoint));
        
        points.x0 = (rand() % (IMAGE_MIDX - 2)) + 1;
        points.y0 = (rand() % (IMAGE_MIDY - 2)) + 1;
        points.x1 = (rand() % (IMAGE_MIDX - 2)) + 1;
        points.y1 = (rand() % (IMAGE_MIDY - 2)) + 1;

        int16_t var = 0;
        int16_t rd1 = (rand() % 4) + 1;
        int16_t rd2 = (rand() % 1) + 1;
        int16_t rd3 = (rand() % 2) + 1;
        int16_t rd4 = (rand() % 3) + 1;
        int16_t rd5 = (rand() % 3) + 1;
        int16_t rd6 = (rand() % 4) + 1;
        int16_t rd7 = (rand() % 2) + 1;
        int16_t rd8 = (rand() % 3) + 1;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            if (!flag) var = rand() % 6;
            changeColor(var);

            if (points.x0 > MAX_WIDTH)
            {
                points.dirx0 = MINUS;
                rd1 = (rand() % LSPEED) + 1;
            }

            if (points.x0 < 1)
            {
                points.dirx0 = PLUS;
                rd2 = (rand() % LSPEED) + 1;
            }

            if (points.y0 > MAX_HEIGHT)
            {
                points.diry0 = MINUS;
                rd3 = (rand() % LSPEED) + 1;
            }

            if (points.y0 < 1)
            {
                points.diry0 = PLUS;
                rd4 = (rand() % LSPEED) + 1;
            }

            if (points.x1 > MAX_WIDTH)
            {
                points.dirx1 = MINUS;
                rd5 = (rand() % LSPEED) + 1;
            }

            if (points.x1 < 1)
            {
                points.dirx1 = PLUS;
                rd6 = (rand() % LSPEED) + 1;
            }

            if (points.y1 > MAX_HEIGHT)
            {
                points.diry1 = MINUS;
                rd7 = (rand() % LSPEED) + 1;
            }

            if (points.y1 < 1)
            {
                points.diry1 = PLUS;
                rd8 = (rand() % LSPEED) + 1;
            }

            if (points.dirx0 == PLUS)   points.x0 += rd1;
            if (points.dirx0 == MINUS)  points.x0 -= rd2;
            if (points.diry0 == PLUS)   points.y0 += rd3;
            if (points.diry0 == MINUS)  points.y0 -= rd4;
            if (points.dirx1 == PLUS)   points.x1 += rd5;
            if (points.dirx1 == MINUS)  points.x1 -= rd6;
            if (points.diry1 == PLUS)   points.y1 += rd7;
            if (points.diry1 == MINUS)  points.y1 -= rd8;

            if (points.x0 < MAX_WIDTH - 1 && points.x0 > 1 && points.y0 > 1 && points.y0 < MAX_HEIGHT - 1 && points.x1 < MAX_WIDTH - 1 && points.x1 > 1 && points.y1 > 1 && points.y1 < MAX_HEIGHT - 1)
            {
                drawLine(points.x0, points.y0, points.x1, points.y1, 245);
            }

            for (int16_t y = 1; y < MAX_HEIGHT; y++)
            {
                for (int16_t x = 0; x <= MAX_WIDTH; x++) vbuff[y][x] = (vbuff[y][x - 1] + vbuff[y][x + 1] + vbuff[y - 1][x] + vbuff[y + 1][x]) >> 2;
            }

            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace mazeGeneration {
    #define X1  0
    #define Y1  0
    #define X2  31
    #define Y2  31
    #define COL 15

    typedef struct {
        uint8_t x, y;
    } TStack;

    TStack stack[2000] = { 0 };

    uint8_t maze[32][32] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void clearMaze()
    {
        memset(maze[0], 0, sizeof(maze));
    }

    void writeBorder()
    {
        int16_t z = 0;

        for (z = Y1; z <= Y2; z++)
        {
            maze[z][X1] = 1;
            maze[z][X2] = 1;
        }

        for (z = X1; z <= X2; z++)
        {
            maze[Y1][z] = 1;
            maze[Y2][z] = 1;
        }
    }

    uint8_t getPixel(uint8_t xc, uint8_t yc, int16_t xn, int16_t yn, int16_t xp, int16_t yp)
    {
        uint8_t a = 0;

        for (int16_t y = yc + yn; y <= yc + yp; y++)
        {
            for (int16_t x = xc + xn; x <= xc + xp; x++)
            {
                if (x >= X1 && x <= X2 && y >= Y1 && y <= Y2)
                {
                    if (maze[y][x]) a++;
                }
                else
                {
                    a++;
                }
            }
        }

        return a;
    }

    void generateMaze(uint8_t x, uint8_t y, uint8_t w, uint8_t wiggle)
    {
        uint8_t r = 0;
        uint8_t d[4] = { 0 };
        uint16_t sp = 0;

        maze[y][x] = 1;
        stack[0].x = x;
        stack[0].y = y;

        do {
            memset(d, 0, sizeof(d));

            do {
                if (random(100) > wiggle) r = random(4) + 1;

                if (r > 0 && d[r - 1]) r = 0;
                else
                {
                    switch (r)
                    {
                    case 1:
                        if (getPixel(x, y - 1, -w, -w, w, 0) > 0)
                        {
                            r = 0;
                            d[0] = 1;
                        }
                        break;

                    case 2:
                        if (getPixel(x, y + 1, -w, 0, w, w) > 0)
                        {
                            r = 0;
                            d[1] = 1;
                        }
                        break;

                    case 3:
                        if (getPixel(x - 1, y, -w, -w, 0, w) > 0)
                        {
                            r = 0;
                            d[2] = 1;
                        }
                        break;

                    case 4:
                        if (getPixel(x + 1, y, 0, -w, w, w) > 0)
                        {
                            r = 0;
                            d[3] = 1;
                        }
                        break;

                    default:
                        break;
                    }
                }
            } while (!r && !(d[0] && d[1] && d[2] && d[3]));

            if (!r)
            {
                x = stack[sp].x;
                y = stack[sp].y;
                if (sp > 0) sp--;
            }
            else
            {
                switch (r)
                {
                case 1: y--; break;
                case 2: y++; break;
                case 3: x--; break;
                case 4: x++; break;
                default: break;
                }

                maze[y][x] = 1;
                stack[sp].x = x;
                stack[sp].y = y;
                if (sp < 2000) sp++;
            }
        } while (sp);
    }

    void drawMaze()
    {
        for (int16_t i = 0; i < 32; i++)
        {
            for (int16_t j = 0; j < 32; j++)
            {
                if (maze[i][j])
                {
                    vmem[i * 3 + 52 + 0][j * 3 + 112 + 0] = COL;
                    vmem[i * 3 + 52 + 0][j * 3 + 112 + 1] = COL;
                    vmem[i * 3 + 52 + 0][j * 3 + 112 + 2] = COL;
                    vmem[i * 3 + 52 + 1][j * 3 + 112 + 0] = COL;
                    vmem[i * 3 + 52 + 1][j * 3 + 112 + 1] = COL;
                    vmem[i * 3 + 52 + 1][j * 3 + 112 + 2] = COL;
                    vmem[i * 3 + 52 + 2][j * 3 + 112 + 0] = COL;
                    vmem[i * 3 + 52 + 2][j * 3 + 112 + 1] = COL;
                    vmem[i * 3 + 52 + 2][j * 3 + 112 + 2] = COL;
                }
            }
        }
    }

    void run()
    {
        int32_t keyCode = 0;

        if (!loadFont("assets/sysfont.xfn", 0)) return;
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Maze-Generation -- Keys: spacer change maze; enter save maze")) return;

        do {
            clearMaze();
            memset(vmem, 0, IMAGE_SIZE);
            generateMaze(X2 >> 1, Y2 >> 1, 2, 16);
            writeBorder();
            drawMaze();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            keyCode = waitUserInput();
        } while (keyCode != SDL_SCANCODE_RETURN);

        changeDrawBuffer(vmem, IMAGE_WIDTH, IMAGE_HEIGHT);
        writeText(20, 15, 28, 0, "Write maze to maze.dat file (Y/N)?");
        restoreDrawBuffer();
        renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);

        if (waitUserInput() == SDL_SCANCODE_Y)
        {
            FILE *fp = fopen("assets/maze.dat", "wb");
            if (fp)
            {
                fwrite(maze[0], 1, sizeof(maze), fp);
                fclose(fp);
                memset(vmem, 0, 320 * 30);
                changeDrawBuffer(vmem, IMAGE_WIDTH, IMAGE_HEIGHT);
                writeText(5, 17, 28, 0, "Writting complete. Any key to exit ...");
                restoreDrawBuffer();
                renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
                waitUserInput();
            }
        }

        freeFont(0);
        cleanup();
    }
}

namespace pixelMorphing {
    #define JUMP    64
    #define SJUMP   6

    typedef struct {
        int16_t     herex, herey;
        int16_t     targetx, targety;
        int16_t     dx, dy;
        uint8_t     active;
        uint8_t     color;
        uint16_t    numof;
    } TPart;

    TPart       nextrow[4096] = { 0 };
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void moveEmOut(int16_t num)
    {
        for (int16_t i = 0; i < num; i++)
        {
            if (nextrow[i].active)
            {
                int16_t x = nextrow[i].herex >> SJUMP;
                int16_t y = nextrow[i].herey >> SJUMP;
                vmem[y][x] = vbuff[y][x];

                nextrow[i].herex -= nextrow[i].dy;
                nextrow[i].herey -= nextrow[i].dx;

                x = nextrow[i].herex >> SJUMP;
                y = nextrow[i].herey >> SJUMP;

                vmem[y][x] = nextrow[i].color;
                nextrow[i].numof--;

                if (!nextrow[i].numof)
                {
                    nextrow[i].active = 0;
                    vbuff[y][x] = nextrow[i].color;
                }
            }
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }

        renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
        delay(FPS_90);
    }

    void doPicture()
    {
        uint16_t k = 0;
        uint8_t dbuff[80][80] = { 0 };
        memset(nextrow, 0, sizeof(nextrow));
        if (!loadPNG(dbuff[0], NULL, "assets/boss.png")) return;

        for (int16_t j = 0; j < 80; j++)
        {
            for (int16_t i = 0; i < 80; i++)
            {
                if (dbuff[j][i])
                {
                    nextrow[k].herex = random(IMAGE_WIDTH) << SJUMP;
                    nextrow[k].herey = random(IMAGE_HEIGHT) << SJUMP;
                    nextrow[k].targetx = (i + 120) << SJUMP;
                    nextrow[k].targety = (j + 60) << SJUMP;
                    nextrow[k].dy = (nextrow[k].herex - nextrow[k].targetx) / JUMP;
                    nextrow[k].dx = (nextrow[k].herey - nextrow[k].targety) / JUMP;
                    nextrow[k].color = dbuff[j][i];
                    nextrow[k].active = 1;
                    nextrow[k].numof = JUMP;
                    k++;
                }
            }
        }

        for (int16_t i = 0; i < JUMP; i++) moveEmOut(k);
    }

    void run()
    {
        RGB pal[256] = { 0 };
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Pixel-Morphing")) return;
        if (!loadPNG(vbuff[0], pal, "assets/duke.png")) return;
        setPalette(pal);
        memcpy(vmem, vbuff, IMAGE_SIZE);
        doPicture();
        if (waitUserInput() == SDL_SCANCODE_ESCAPE) quit();
        cleanup();
    }
}

namespace pierraEffect {
    double idx, sinus[1000];
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    int16_t roundf(double x)
    {
        if (x >= 0) return int16_t(x + 0.5);
        return int16_t(x - 0.5);
    }

    void motionBlur()
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vmem
            add     edi, IMAGE_WIDTH
            mov     ecx, IMAGE_SIZE - (IMAGE_WIDTH << 1)
            xor     bx, bx
        again:
            xor     ax, ax
            mov     bl, [edi - 1]
            add     ax, bx
            mov     bl, [edi + 1]
            add     ax, bx
            mov     bl, [edi - IMAGE_WIDTH]
            add     ax, bx
            mov     bl, [edi + IMAGE_WIDTH]
            add     ax, bx
            shr     ax, 2
            stosb
            loop    again
        }
#else
        for (int16_t i = 1; i < MAX_HEIGHT; i++)
        {
            for (int16_t j = 0; j <= MAX_WIDTH; j++) vmem[i][j] = (vmem[i][j - 1] + vmem[i][j + 1] + vmem[i - 1][j] + vmem[i + 1][j]) >> 2;
        }
#endif
    }

    void plotLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t col)
    {
        const int16_t dx = abs(x2 - x1);
        const int16_t dy = abs(y2 - y1);
        int16_t x = x1;
        int16_t y = y1;
        int16_t xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0;
        int16_t den = 0, num = 0, numadd = 0, numpixels = 0, curpixel = 0;

        if (x2 >= x1)
        {
            xinc1 = 1;
            xinc2 = 1;
        }
        else
        {
            xinc1 = -1;
            xinc2 = -1;
        }

        if (y2 >= y1)
        {
            yinc1 = 1;
            yinc2 = 1;
        }
        else
        {
            yinc1 = -1;
            yinc2 = -1;
        }

        if (dx >= dy)
        {
            xinc1 = 0;
            yinc2 = 0;
            den = dx;
            num = dx >> 1;
            numadd = dy;
            numpixels = dx;
        }
        else
        {
            xinc2 = 0;
            yinc1 = 0;
            den = dy;
            num = dy >> 1;
            numadd = dx;
            numpixels = dy;
        }

        for (curpixel = 0; curpixel < numpixels; curpixel++)
        {
            vmem[y][x] = col;
            num += numadd;
            if (num >= den)
            {
                num -= den;
                x += xinc1;
                y += yinc1;
            }
            x += xinc2;
            y += yinc2;
        }
    }

    void pierra(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t num, int16_t ko)
    {
        int16_t i = 0, j = 0, h = 0, k = 0, m = 0;

        double x = (double)x1;
        double y = (double)y1;

        const double dx = (double(x2) - x1) / num;
        const double dy = (double(y2) - y1) / num;

        for (m = 1; m <= num; m++)
        {
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

            for (h = roundf(y - ko); h <= roundf(y + ko); h++)
            {
                for (k = roundf(x - ko); k <= roundf(x + ko); k++) vmem[h][k] = 255;
                if (keyDown(SDL_SCANCODE_RETURN)) break;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }

            x += dx;
            y += dy;
            idx += 0.02;

            const double xx = sinus[roundf(idx * 159) % 1000] * sinus[roundf(idx * 83 + 130) % 1000] * 140;
            const double yy = sinus[roundf(idx * 97 + 153) % 1000] * sinus[roundf(idx * 107) % 1000] * 80;

            for (i = 158 + roundf(-xx); i <= 162 + roundf(-xx); i++)
            {
                for (j = 98 + roundf(-yy); j <= 102 + roundf(-yy); j++) vmem[j][i] = 155;
                if (keyDown(SDL_SCANCODE_RETURN)) break;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }

            for (i = 158 + roundf(xx); i <= 162 + roundf(xx); i++)
            {
                for (j = 98 + roundf(yy); j <= 102 + roundf(yy); j++) vmem[j][i] = 155;
                if (keyDown(SDL_SCANCODE_RETURN)) break;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }

            plotLine(30, 30, 110, 30, 55);
            plotLine(70, 30, 70, 140, 55);
            plotLine(130, 110, 160, 90, 55);
            plotLine(160, 90, 140, 70, 55);
            plotLine(140, 70, 120, 90, 55);
            plotLine(120, 90, 140, 140, 55);
            plotLine(140, 140, 170, 130, 55);
            plotLine(200, 140, 200, 70, 55);
            plotLine(200, 70, 230, 80, 55);
            plotLine(230, 80, 210, 100, 55);
            plotLine(210, 100, 230, 140, 55);
            plotLine(270, 70, 290, 100, 55);
            plotLine(290, 100, 270, 140, 55);
            plotLine(270, 140, 250, 100, 55);
            plotLine(250, 100, 270, 70, 55);
            plotLine(20, 170, 300, 170, 55);
            motionBlur();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(2);
        }
    }

    void run()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };

        idx = 0.0;
        for (i = 0; i < 1000; i++)
        {
            sinus[i] = sin(i * 0.00628318530718);
        }

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fire-Effect")) return;

        for (i = 0; i < 64; i++)
        {
            pal[i].r = uint8_t(i);
            pal[i].g = 0;
            pal[i].b = 0;
        }

        for (i = 64; i < 128; i++)
        {
            pal[i].r = 63;
            pal[i].g = uint8_t(i);
            pal[i].b = 0;
        }

        for (i = 128; i < 192; i++)
        {
            pal[i].r = 63;
            pal[i].g = 63;
            pal[i].b = uint8_t(i);
        }

        for (i = 192; i < 256; i++)
        {
            pal[i].r = 63;
            pal[i].g = 63;
            pal[i].b = 63;
        }

        shiftPalette(pal);
        setPalette(pal);

        do {
            pierra(30, 30, 110, 30, 40, 5);
            pierra(70, 30, 70, 140, 30, 5);
            pierra(130, 110, 160, 90, 20, 5);
            pierra(160, 90, 140, 70, 20, 5);
            pierra(140, 70, 120, 90, 20, 5);
            pierra(120, 90, 140, 140, 20, 5);
            pierra(140, 140, 170, 130, 20, 5);
            pierra(200, 140, 200, 70, 20, 5);
            pierra(200, 70, 230, 80, 20, 5);
            pierra(230, 80, 210, 100, 20, 5);
            pierra(210, 100, 230, 140, 20, 5);
            pierra(270, 70, 290, 100, 20, 5);
            pierra(290, 100, 270, 140, 20, 5);
            pierra(270, 140, 250, 100, 20, 5);
            pierra(250, 100, 270, 70, 20, 5);
            pierra(20, 170, 300, 170, 25, 5);
            pierra(300, 170, 20, 170, 25, 5);
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace plasmaEffect1 {
    #define RAD 0.017453293

    uint8_t angle = 0;
    uint8_t wave[256] = { 0 };
    uint8_t sintab[256] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void initPlasma()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };

        for (i = 0; i <= 63; i++)
        {
            pal[i].r = uint8_t(i);
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = uint8_t(i);
            pal[i + 64].b = 0;
            pal[i + 128].r = 63;
            pal[i + 128].g = uint8_t(63 - i);
            pal[i + 128].b = 0;
            pal[i + 192].r = uint8_t(63 - i);
            pal[i + 192].g = 0;
            pal[i + 192].b = 0;
        }

        shiftPalette(pal);
        setPalette(pal);

        for (i = 0; i < 256; i++) sintab[i] = uint8_t(sin(i * RAD * 360.0 / 256.0) * 32.0);
    }

    void drawPlasma()
    {
        int16_t x = 0, y = 0;

        for (x = 0; x < 256; x++)
        {
            wave[x] = sintab[uint8_t(angle + x)] + sintab[uint8_t((angle + x) << 1)] + sintab[uint8_t((angle - x) << 2)];
        }

        for (y = 0; y < IMAGE_HEIGHT; y++)
        {
            for (x = 0; x < IMAGE_WIDTH; x++) vmem[y][x] = wave[x & 0xff] + wave[y] + 64;
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Plasma-Effect")) return;
        initPlasma();

        angle = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            drawPlasma();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            angle = (angle + 1) & 0xff;
        }
        cleanup();
    }
}

namespace plasmaEffect2 {
    RGB pal[256] = { 0 };
    uint8_t fx[IMAGE_WIDTH] = { 0 };
    uint8_t fy[IMAGE_HEIGHT] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    void setColors()
    {
        for (int16_t x = 0; x < 64; x++)
        {
            pal[x      ].r = 63 << 2;
            pal[x      ].g = (63 - x) << 2;
            pal[x      ].b = 0;
            pal[x + 64 ].r = (63 - x) << 2;
            pal[x + 64 ].g = x << 2;
            pal[x + 64 ].b = 0;
            pal[x + 128].r = 0;
            pal[x + 128].g = (63 - x) << 2;
            pal[x + 128].b = x << 2;
            pal[x + 192].r = x << 2;
            pal[x + 192].g = x << 2;
            pal[x + 192].b = (63 - x) << 2;
        }
        setPalette(pal);
    }

    void pixelBob(int16_t* bobx, int16_t* boby)
    {
        int16_t x = rand() % 2;
        int16_t y = rand() % 2;

        if (!x) (*bobx)++; else (*bobx)--;
        if (!y) (*boby)++; else (*boby)--;

        for (x = *bobx; x <= *bobx + 10; x++)
        {
            y = *boby;
            for (int16_t i = 0; i < 10; i++)
            {
                y++;
                vmem[y][x]++;
            }
        }
    }

    void cyclePallete()
    {
        RGB tmp = { 0 };
        memcpy(&tmp, &pal[0], sizeof(RGB));
        memcpy(&pal[0], &pal[1], 255 * sizeof(RGB));
        memcpy(&pal[255], &tmp, sizeof(RGB));
        setPalette(pal);
    }

    void run()
    {
        int16_t x = 0, y = 0;
        int16_t bob1x = 200;
        int16_t bob1y = 70;
        int16_t bob2x = 230;
        int16_t bob2y = 130;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Plasma-Effect")) return;

        for (x = 0; x < IMAGE_WIDTH; x++) fx[x] = uint8_t((cos(M_PI * x / IMAGE_MIDX) + 1) * 127.5);
        for (x = 0; x < IMAGE_HEIGHT; x++) fy[x] = uint8_t((sin(M_PI * x / IMAGE_MIDY) + 1) * 127.5);
        for (y = 0; y < IMAGE_HEIGHT; y++)
        {
            for (x = 0; x < IMAGE_WIDTH; x++) vmem[y][x] = fx[x] + fy[y];
            if (y + 2 <= MAX_HEIGHT) for (x = 0; x <= MAX_WIDTH; x++) vmem[y + 2][x] = uint8_t(y);
        }

        setColors();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            cyclePallete();
            pixelBob(&bob1x, &bob1y);
            pixelBob(&bob2x, &bob2y);
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace plasmaEffect3 {
    RGB pal[256] = { 0 };

    void plasmaPalette()
    {
        for (int16_t i = 0; i < 256; i++)
        {
            pal[i].r = uint8_t(32 + 30 * sin(i * M_PI * 2 / 255));
            pal[i].g = uint8_t(32 + 30 * cos(i * M_PI * 2 / 255));
            pal[i].b = uint8_t(32 + 30 * sin(-i * M_PI * 2 / 255));
        }
        shiftPalette(pal);
        setPalette(pal);
    }

    double FN(double x, double y)
    {
        return 128 + 128 * (cos(x * x * y / 1000000.0) + sin(y * y * x / 1000000.0));
    }

    void cyclePallete()
    {
        RGB tmp = { 0 };

        while (!finished(SDL_SCANCODE_RETURN))
        {
            memcpy(&tmp, &pal[0], sizeof(RGB));
            memcpy(&pal[0], &pal[1], 255 * sizeof(RGB));
            memcpy(&pal[255], &tmp, sizeof(RGB));
            setPalette(pal);
            delay(FPS_90);
        }
        cleanup();
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Plasma")) return;
        plasmaPalette();

        for (int16_t i = 0; i < IMAGE_WIDTH; i++)
        {
            for (int16_t j = 0; j < IMAGE_HEIGHT; j++) putPixel(i, j, uint32_t(FN(i, j)));
        }

        cyclePallete();
    }
}

namespace plasmaEffect4 {
    uint8_t p1, p2;
    uint8_t sintab[256] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void setupSinus()
    {
        const double v = 0.0;
        const double vadd = 2 * M_PI / 256;
        for (int16_t i = 0; i < 256; i++) sintab[i] = uint8_t(sin(v + i * vadd) * 63 + 64);
    }

    void setColors()
    {
        RGB pal[256] = { 0 };

        for (uint8_t i = 0; i < 64; i++)
        {
            pal[i].r = i;
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 64].r = 63 - i;
            pal[i + 64].g = 0;
            pal[i + 64].b = 0;
            pal[i + 128].r = i;
            pal[i + 128].g = 0;
            pal[i + 128].b = i;
            pal[i + 192].r = 63 - i;
            pal[i + 192].g = 0;
            pal[i + 192].b = 63 - i;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void showPlasma()
    {
        uint8_t v1 = p1;
        uint8_t v2 = p2;

        for (int16_t i = 0; i < IMAGE_HEIGHT; i++)
        {
            uint8_t dl = sintab[v1];
            uint8_t dh = sintab[v2];
            for (int16_t j = 0; j < IMAGE_WIDTH; j++)
            {
                vmem[i][j] = sintab[dl] + sintab[dh];
                dl += 2;
                dh--;
            }
            v1 -= 2;
            v2++;
        }
    }

    void changeAngle()
    {
        p1++;
        p2 -= 3;
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Plasma")) return;
        setupSinus();
        setColors();

        p1 = 187;
        p2 = 230;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            showPlasma();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            changeAngle();
        }
        cleanup();
    }
}

namespace plasmaEffect5 {
    #define KX (2 * M_PI / IMAGE_WIDTH)
    #define KY (2 * M_PI / IMAGE_HEIGHT)

    RGB pal[512] = { 0 };

    void makeCoolPalette()
    {
        int16_t i = 0;
        int16_t r = 0, g = 0, b = 0;

        for (i = 0; i < 64; i++)
        {
            pal[i].r = r << 2;
            pal[i].g = g << 2;
            pal[i].b = b << 2;
            if (r < 63) r++;
            if (g < 63) g++;
            if (b < 63) b++;
        }

        for (i = 0; i < 64; i++)
        {
            pal[i + 64].r = r << 2;
            pal[i + 64].g = g << 2;
            pal[i + 64].b = b << 2;
            if (b > 0) b--;
        }

        for (i = 0; i < 64; i++)
        {
            pal[i + 128].r = r << 2;
            pal[i + 128].g = g << 2;
            pal[i + 128].b = b << 2;
            if (r > 0) r--;
            if (g > 0) g--;
            if (b < 63 && !(i % 2)) b++;
        }

        for (i = 0; i < 64; i++)
        {
            pal[i + 192].r = r << 2;
            pal[i + 192].g = g << 2;
            pal[i + 192].b = b << 2;
            if (g < 63) g++;
            if (b > 0 && !(i % 2)) b--;
        }

        for (i = 0; i < 64; i++)
        {
            pal[256 + i].r = r << 2;
            pal[256 + i].g = g << 2;
            pal[256 + i].b = b << 2;
            if (r < 63 && !(i % 2)) r++;
            if (g > 0) g--;
        }

        for (i = 0; i < 64; i++)
        {
            pal[256 + i + 64].r = r << 2;
            pal[256 + i + 64].g = g << 2;
            pal[256 + i + 64].b = b << 2;
            if (r < 63 && !(i % 2)) r++;
            if (b < 63 && !(i % 2)) b++;
        }

        for (i = 0; i < 64; i++)
        {
            pal[256 + i + 128].r = r << 2;
            pal[256 + i + 128].g = g << 2;
            pal[256 + i + 128].b = b << 2;
            if (b < 63 && !(i % 2)) b++;
            if (g < 63 && !(i % 2)) g++;
            if (r > 0) r--;
        }

        for (i = 0; i < 64; i++)
        {
            pal[256 + i + 192].r = r << 2;
            pal[256 + i + 192].g = g << 2;
            pal[256 + i + 192].b = b << 2;
            if (b > 0) b--;
            if (g > 0 && !(i % 2)) g--;
        }

        setPalette(pal);
    }

    void rotatePalette(int16_t x, int16_t y)
    {
        RGB tmp = { 0 };
        do {
            memcpy(&tmp, &pal[x], sizeof(RGB));
            memcpy(&pal[x], &pal[x + 1], y * sizeof(RGB));
            memcpy(&pal[y], &tmp, sizeof(RGB));
            setPalette(pal);
            delay(FPS_90);
        } while (!finished(SDL_SCANCODE_RETURN));
        cleanup();
    }

    void drawSinCos()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++)
        {
            for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
            {
                const uint8_t col = uint8_t((sin(x * KX * 0.5) * sin(y * KY * 0.5)) * (127.0 - 20.0) + 128);
                putPixel(x, y, col);
            }
        }
        render();
    }

    void filterSinCos()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++)
        {
            for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
            {
                uint8_t col = getPixel(x, y);
                col += uint8_t((sin(x * KX * 10) * sin(y * KY * 10)) * 20);
                putPixel(x, y, col);
            }
        }
        render();
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Plasma")) return;
        makeCoolPalette();
        drawSinCos();
        while (!finished(SDL_SCANCODE_RETURN));
        filterSinCos();
        while (!finished(SDL_SCANCODE_RETURN));
        rotatePalette(1, 512);
    }
}

namespace rippleEffect {
    #define AMPL 8
    #define FREQ 15

    uint16_t angle = 0;
    uint8_t dbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t sqrtab[SIZE_32K] = { 0 };
    int16_t wave[IMAGE_HEIGHT] = { 0 };

    int16_t initTexture()
    {
        RGB pal[256] = { 0 };
        if (!loadPNG(vbuff[0], pal, "assets/pig.png")) return 0;
        setPalette(pal);
        return 1;
    }

    void preCalculate()
    {
        for (int16_t y = 0; y < IMAGE_MIDY; y++)
        {
            for (int16_t x = 0; x < IMAGE_MIDX; x++) sqrtab[(y * 160 + x) << 1] = uint8_t(sqrt(double(x) * x + double(y) * y));
        }
    }

    void updateWave()
    {
        angle++;
        for (int16_t k = 0; k < IMAGE_HEIGHT; k++) wave[k] = int16_t(AMPL * sin(M_PI * FREQ * (double(k) - angle) / 180.0));
    }

    void drawRipples()
    {
        for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
        {
            for (int16_t x = 0; x < IMAGE_WIDTH; x++)
            {
                int16_t xx = abs(x - IMAGE_MIDX);
                int16_t yy = abs(y - IMAGE_MIDY);
                const int16_t dist = sqrtab[(yy * 160 + xx) << 1];
                const int16_t alt = wave[dist];

                xx = x + alt;
                yy = y + alt;

                if (yy > MAX_HEIGHT) yy -= IMAGE_HEIGHT;
                if (yy < 0) yy += IMAGE_HEIGHT;
                dbuff[y][x] = vbuff[yy][xx];
            }
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Ripple")) return;
        if (!initTexture()) return;
        preCalculate();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            updateWave();
            drawRipples();
            renderBuffer(dbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace rotateMap {
    #define SCALING_FACTOR1     10
    #define SCALING_FACTOR2     50
    #define XOFFSET             160
    #define YOFFSET             100
    #define ZOFFSET             550
    #define VERTICES            (SCALING_FACTOR1 * SCALING_FACTOR2)
    #define PALSIZE             63
    #define RADIUS1             25
    #define RADIUS2             50
    #define RADIUS3             25

    typedef struct {
        int16_t x, y;
    } TPoint2D;

    typedef struct {
        int16_t x, y, z;
    } TPoint3D;

    TPoint3D shape[VERTICES] = { 0 };
    TPoint3D rotatedShape[VERTICES] = { 0 };

    TPoint2D shape2d[VERTICES] = { 0 };
    TPoint2D face2d[3] = { 0 };

    uint8_t colors[3] = { 0 };

    int16_t vertices = 0, vertexCount, zvalue;
    int16_t drawOrder[VERTICES] = { 0 };
    int16_t maxz = RADIUS1 + RADIUS2 + RADIUS3;
    int16_t polyCoords[VERTICES][4] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    double phi = 0, theta = 0;

    void gouraudShader(const TPoint2D* vertices, const uint8_t* colors)
    {
        uint8_t startCol = 0, endCol = 0;
        int16_t miny = 0, maxy = 0, i = 0, incSignCol = 0, incCountCol = 0, diffCol = 0;
        int16_t y = 0, lineWidth = 0, endVertex1 = 0, endVertex2 = 0, startVertex1 = 0, startVertex2 = 0;
        int16_t xdiff1 = 0, xdiff2 = 0, ydiff1 = 0, ydiff2 = 0, x1 = 0, x2 = 0, diff1 = 0, diff2 = 0;
        int16_t xcal1 = 0, xcal2 = 0, cal1 = 0, cal2 = 0;

        for (i = 0; i < 3; i++)
        {
            if (vertices[i].y < vertices[miny].y) miny = i;
            else if (vertices[i].y > vertices[maxy].y) maxy = i;
        }

        startVertex1 = startVertex2 = miny;

        endVertex1 = miny + 2;
        if (endVertex1 >= 3) endVertex1 -= 3;

        endVertex2 = miny + 1;
        if (endVertex2 >= 3) endVertex2 -= 3;

        xdiff1 = vertices[endVertex1].x - vertices[startVertex1].x;
        ydiff1 = vertices[endVertex1].y - vertices[startVertex1].y;
        xdiff2 = vertices[endVertex2].x - vertices[startVertex1].x;
        ydiff2 = vertices[endVertex2].y - vertices[startVertex1].y;

        diff1 = colors[endVertex1] - colors[startVertex1];
        diff2 = colors[endVertex2] - colors[startVertex2];

        if (ydiff1 == 0) ydiff1 = 1;
        if (ydiff2 == 0) ydiff2 = 1;

        for (y = vertices[miny].y; y <= vertices[maxy].y; y++)
        {
            x2 = vertices[startVertex1].x + xcal1 / ydiff1;
            xcal1 += xdiff1;

            x1 = vertices[startVertex2].x + xcal2 / ydiff2;
            xcal2 += xdiff2;

            endCol = colors[startVertex1] + cal1 / ydiff1;
            cal1 += diff1;
            startCol = colors[startVertex2] + cal2 / ydiff2;
            cal2 += diff2;

            if (endCol > startCol) incSignCol = 1;
            else incSignCol = -1;

            diffCol = abs(startCol - endCol);
            lineWidth = x2 - x1;
            incCountCol = diffCol - (lineWidth >> 1);

            for (i = x1; i < x2; i++)
            {
                vbuff[y][i] = startCol;

                while (incCountCol >= 0)
                {
                    startCol += incSignCol;
                    incCountCol -= lineWidth;
                }

                incCountCol += diffCol;
            }

            if (y == vertices[endVertex1].y)
            {
                startVertex1 = endVertex1;
                endVertex1 = endVertex2;

                xdiff1 = vertices[endVertex1].x - vertices[startVertex1].x;
                ydiff1 = vertices[endVertex1].y - vertices[startVertex1].y;

                diff1 = colors[endVertex1] - colors[startVertex1];

                if (ydiff1 == 0) ydiff1 = 1;

                xcal1 = xdiff1;
                cal1 = diff1;
            }

            if (y == vertices[endVertex2].y)
            {
                startVertex2 = endVertex2;
                endVertex2 = endVertex1;

                xdiff2 = vertices[endVertex2].x - vertices[startVertex2].x;
                ydiff2 = vertices[endVertex2].y - vertices[startVertex2].y;

                diff2 = colors[endVertex2] - colors[startVertex2];

                if (ydiff2 == 0) ydiff2 = 1;

                xcal2 = xdiff2;
                cal2 = diff2;
            }
        }
    }

    void rotatePoint(int16_t x, int16_t y, int16_t xc, int16_t yc, double sina, double cosa, int16_t* endx, int16_t* endy)
    {
        *endx = int16_t((x - xc) * cosa - (double(y) - yc) * sina + xc);
        *endy = int16_t((y - yc) * cosa + (double(x) - xc) * sina + yc);
    }

    int32_t compare(const void* a, const void* b)
    {
        const int16_t va = *(int16_t*)a;
        const int16_t vb = *(int16_t*)b;
        return rotatedShape[va].z - rotatedShape[vb].z;
    }

    void rotateShape()
    {
        const double sinPhi = sin(phi);
        const double cosPhi = cos(phi);
        const double sinTheta = sin(theta);
        const double cosTheta = cos(theta);

        for (vertexCount = 0; vertexCount < vertices; vertexCount++)
        {
            rotatePoint(shape[vertexCount].y, shape[vertexCount].z, 0, 0, sinPhi, cosPhi, &rotatedShape[vertexCount].y, &rotatedShape[vertexCount].z);
            rotatePoint(shape[vertexCount].x, rotatedShape[vertexCount].z, 0, 0, sinTheta, cosTheta, &rotatedShape[vertexCount].x, &rotatedShape[vertexCount].z);
        }

        phi += 0.05;
        theta += 0.04;
    }

    void drawFace(int16_t index1, int16_t index2, int16_t index3)
    {
        if ((rotatedShape[index2].x - rotatedShape[index1].x) *
            (rotatedShape[index1].y - rotatedShape[index3].y) <
            (rotatedShape[index2].y - rotatedShape[index1].y) *
            (rotatedShape[index1].x - rotatedShape[index3].x)) return;

        face2d[0].x = shape2d[index1].x;
        face2d[0].y = shape2d[index1].y;

        face2d[1].x = shape2d[index2].x;
        face2d[1].y = shape2d[index2].y;

        face2d[2].x = shape2d[index3].x;
        face2d[2].y = shape2d[index3].y;

        colors[0] = 1 + (PALSIZE - 1) * (rotatedShape[index1].z + maxz) / (maxz << 1);
        colors[1] = 1 + (PALSIZE - 1) * (rotatedShape[index2].z + maxz) / (maxz << 1);
        colors[2] = 1 + (PALSIZE - 1) * (rotatedShape[index3].z + maxz) / (maxz << 1);

        if (polyCoords[drawOrder[vertexCount]][0] & 2)
        {
            colors[0] += PALSIZE;
            colors[1] += PALSIZE;
            colors[2] += PALSIZE;
        }

        gouraudShader(face2d, colors);
    }

    void drawFaces()
    {
        for (vertexCount = 0; vertexCount < vertices; vertexCount++)
        {
            zvalue = (ZOFFSET + rotatedShape[vertexCount].z) >> 2;
            shape2d[vertexCount].x = XOFFSET + ((rotatedShape[vertexCount].x) << 7) / zvalue;
            shape2d[vertexCount].y = YOFFSET + ((rotatedShape[vertexCount].y) << 7) / zvalue;
        }

        for (vertexCount = 0; vertexCount < vertices; vertexCount++)
        {
            drawFace(polyCoords[drawOrder[vertexCount]][0], polyCoords[drawOrder[vertexCount]][1], polyCoords[drawOrder[vertexCount]][2]);
            drawFace(polyCoords[drawOrder[vertexCount]][0], polyCoords[drawOrder[vertexCount]][2], polyCoords[drawOrder[vertexCount]][3]);
        }
    }

    void setupData()
    {
        int16_t i = 0, j = 0;
        RGB pal[256] = { 0 };

        double alpha = 0.0;
        for (j = 0; j < SCALING_FACTOR2; j++, alpha += 2 * M_PI / SCALING_FACTOR2)
        {
            const double x = RADIUS2 * cos(2 * alpha) + RADIUS1 * sin(alpha);
            const double y = RADIUS2 * sin(2 * alpha) + RADIUS1 * cos(alpha);
            const double z = RADIUS2 * cos(3 * alpha);

            const double dx = -2.0 * RADIUS2 * sin(2 * alpha) + RADIUS1 * cos(alpha);
            const double dy = 2.0 * RADIUS2 * cos(2 * alpha) - RADIUS1 * sin(alpha);
            const double dz = -3.0 * RADIUS2 * sin(3 * alpha);

            const double value = sqrt(dx * dx + dz * dz);
            const double modulus = sqrt(dx * dx + dy * dy + dz * dz);

            double beta = 0.0;
            for (i = 0; i < SCALING_FACTOR1; i++, beta += 2 * M_PI / SCALING_FACTOR1)
            {
                shape[vertices].x = int16_t(x - RADIUS3 * (cos(beta) * dz - sin(beta) * dx * dy / modulus) / value);
                shape[vertices].y = int16_t(y - RADIUS3 * sin(beta) * value / modulus);
                shape[vertices].z = int16_t(z + RADIUS3 * (cos(beta) * dx + sin(beta) * dy * dz / modulus) / value);
                vertices++;
            }
        }

        for (i = 0; i < SCALING_FACTOR2; i++)
        {
            int16_t idx1 = i * SCALING_FACTOR1;
            int16_t idx2 = idx1 + SCALING_FACTOR1;

            idx2 %= vertices;

            int16_t rotation = 0;
            double minDist = (double(shape[idx1].x) - shape[idx2].x) * (double(shape[idx1].x) - shape[idx2].x) + (double(shape[idx1].y) - shape[idx2].y) * (double(shape[idx1].y) - shape[idx2].y) + (double(shape[idx1].z) - shape[idx2].z) * (double(shape[idx1].z) - shape[idx2].z);

            for (j = 1; j < SCALING_FACTOR1; j++)
            {
                idx2 = j + idx1 + SCALING_FACTOR1;
                if (i == SCALING_FACTOR2 - 1) idx2 = j;

                const double dist = (double(shape[idx1].x) - shape[idx2].x) * (double(shape[idx1].x) - shape[idx2].x) + (double(shape[idx1].y) - shape[idx2].y) * (double(shape[idx1].y) - shape[idx2].y) + (double(shape[idx1].z) - shape[idx2].z) * (double(shape[idx1].z) - shape[idx2].z);

                if (dist < minDist)
                {
                    minDist = dist;
                    rotation = j;
                }
            }

            for (j = 0; j < SCALING_FACTOR1; j++)
            {
                polyCoords[idx1 + j][0] = idx1 + j;
                idx2 = j + 1;
                idx2 %= SCALING_FACTOR1;
                polyCoords[idx1 + j][1] = idx1 + idx2;
                idx2 = j + rotation + 1;
                idx2 %= SCALING_FACTOR1;
                polyCoords[idx1 + j][2] = (idx1 + idx2 + SCALING_FACTOR1) % vertices;
                idx2 = j + rotation;
                idx2 %= SCALING_FACTOR1;
                polyCoords[idx1 + j][3] = (idx1 + idx2 + SCALING_FACTOR1) % vertices;
            }
        }

        for (i = 0; i < vertices; i++) drawOrder[i] = i;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Rotate-Map")) return;

        for (i = 1; i <= PALSIZE; i++)
        {
            pal[i].r = 63 - i;
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 63].r = 0;
            pal[i + 63].g = 0;
            pal[i + 63].b = 63 - i;
        }
        shiftPalette(pal);
        setPalette(pal);
    }

    void run()
    {
        setupData();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            rotateShape();
            qsort(drawOrder, vertices, 2, compare);
            drawFaces();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            memset(vbuff, 0, IMAGE_SIZE);
        }
        cleanup();
    }
}

namespace scaleMap {
    int16_t costab[256] = { 0 };
    int16_t sintab[256] = { 0 };
    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void stretch(int16_t x1, int16_t x2, int16_t y1, int16_t y2, int16_t yr, int16_t yw)
    {
        const int16_t dx = abs(x2 - x1);
        const int16_t dy = abs(y2 - y1);

        const int16_t sx = sign(x2 - x1);
        const int16_t sy = sign(y2 - y1);

        int16_t err = (dy << 1) - dx;
        const int16_t dx2 = dx << 1;
        const int16_t dy2 = dy << 1;

        for (int16_t d = 0; d < dx; d++)
        {
            vbuff1[yw][x1] = vbuff2[yr][y1];
            while (err >= 0)
            {
                y1 += sy;
                err -= dx2;
            }

            x1 += sx;
            err += dy2;
        }
    }

    void rectStretch(int16_t xs1, int16_t ys1, int16_t xs2, int16_t ys2, int16_t xd1, int16_t yd1, int16_t xd2, int16_t yd2)
    {
        const int16_t dx = abs(yd2 - yd1);
        const int16_t dy = abs(ys2 - ys1);

        const int16_t sx = sign(yd2 - yd1);
        const int16_t sy = sign(ys2 - ys1);

        int16_t err = (dy << 1) - dx;
        const int16_t dx2 = dx << 1;
        const int16_t dy2 = dy << 1;

        for (int16_t d = 0; d < dx; d++)
        {
            stretch(xd1, xd2, xs1, xs2, ys1, yd1);

            while (err >= 0)
            {
                ys1 += sy;
                err -= dx2;
            }

            yd1 += sx;
            err += dy2;
        }
    }

    void runStretch1()
    {
        int16_t z = 114;
        int16_t zadd = -2;

        do {
            z += zadd;
            const int16_t x = (16 << 8) / z;
            const int16_t y = (10 << 8) / z;

            memcpy(vbuff1, vbuff2, IMAGE_SIZE);
            rectStretch(0, 0, MAX_WIDTH, MAX_HEIGHT, IMAGE_MIDX - (x >> 1), IMAGE_MIDY - (y >> 1), IMAGE_MIDX - (x >> 1) + x, IMAGE_MIDY - (y >> 1) + y);
            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            if (z < 16 || z > 114) zadd = -zadd;
            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        } while (!keyDown(SDL_SCANCODE_RETURN) && !keyDown(SDL_SCANCODE_2) && !keyDown(SDL_SCANCODE_3));
    }

    void runStretch2()
    {
        int16_t i = 128;
        int16_t dir = 3;
        int16_t vx1 = 10;

        do {
            memcpy(vbuff1, vbuff2, IMAGE_SIZE);
            if (vx1 < -50 || vx1 > 260) dir = -dir;

            const int16_t vy1 = 25 + sintab[i & 0xff];
            const int16_t vy2 = vy1 + IMAGE_MIDY;
            i += 4;

            if (vy2 < IMAGE_HEIGHT) vx1 += dir;

            const int16_t vx2 = vx1 + IMAGE_MIDY;
            int16_t x1 = vx1;
            if (x1 < 0) x1 = 0;

            int16_t y1 = vy1;
            if (y1 < 0) y1 = 0;

            int16_t x2 = vx2;
            if (x2 > MAX_WIDTH) x2 = MAX_WIDTH;

            int16_t y2 = vy2;
            if (y2 > MAX_HEIGHT) y2 = MAX_HEIGHT;

            rectStretch(0, 0, MAX_WIDTH, MAX_HEIGHT, x1, y1, x2, y2);
            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        } while (!keyDown(SDL_SCANCODE_RETURN) && !keyDown(SDL_SCANCODE_1) && !keyDown(SDL_SCANCODE_3));
    }

    void runStretch3()
    {
        int16_t i = 0;

        do {
            memcpy(vbuff1, vbuff2, IMAGE_SIZE);
            rectStretch(0, 0, MAX_WIDTH, MAX_HEIGHT, 40, IMAGE_MIDY - costab[i], 140, IMAGE_MIDY + costab[i]);
            rectStretch(0, 0, MAX_WIDTH, MAX_HEIGHT, 230 - costab[i], 50, 230 + costab[i], 150);
            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            i = (i + 6) % 256;
            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        } while (!keyDown(SDL_SCANCODE_RETURN) && !keyDown(SDL_SCANCODE_1) && !keyDown(SDL_SCANCODE_2));
    }

    void run()
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 256; i++)
        {
            costab[i] = int16_t(cos(2 * M_PI * i / 255) * 50);
            sintab[i] = int16_t(sin(2 * M_PI * i / 255) * 60 + 80);
        }

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Scaler -- Keys: 1->3: change mode")) return;
        if (!loadPNG(vbuff2[0], pal, "assets/face.png")) return;
        setPalette(pal);
        runStretch1();

        do {
            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            if (keyDown(SDL_SCANCODE_1)) runStretch1();
            if (keyDown(SDL_SCANCODE_2)) runStretch2();
            if (keyDown(SDL_SCANCODE_3)) runStretch3();
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace shadeBob {
    int16_t gw = 0, gh = 0, col = 0;
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void makeShadeBob(int16_t x, int16_t y)
    {
#ifdef _USE_ASM
        __asm {
            xor     eax, eax
            xor     edx, edx
            mov     dx, IMAGE_WIDTH
            sub     dx, gw
            lea     edi, vmem
            mov     ax, y
            shl     ax, 6
            add     ah, byte ptr y
            add     ax, x
            add     edi, eax
            mov     al, byte ptr col
            mov     bx, gh
        again:
            mov     cx, gw
        next:
            add     [edi], al
            inc     edi
            dec     cx
            jnz     next
            add     edi, edx
            dec     bx
            jnz     again
        }
#else
        for (int16_t a = 0; a < gh; a++)
        {
            for (int16_t b = 0; b < gw; b++) vmem[y + a][x + b] += col;
        }
#endif
    }

    void makePalette()
    {
        RGB pal[256] = { 0 };

        for (int16_t i = 0; i < 64; i++)
        {
            pal[i      ].r = i << 2;
            pal[32 + i ].g = i << 2;
            pal[64 + i ].r = (63 - i) << 2;
            pal[64 + i ].b = i << 2;
            pal[128 + i].r = i << 2;
            pal[128 + i].b = (63 - i) << 2;
            pal[96 + i ].g = (63 - i) << 2;
            pal[192 + i].r = (63 - i) << 2;
        }
        setPalette(pal);
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Shade-Bob")) return;
        if (!loadPNG(vbuff[0], NULL, "assets/killer.png")) return;
        makePalette();

        const int16_t an = 400;
        gh = random(60) + 40;
        gw = random(120) + 40;
        const int16_t w = IMAGE_WIDTH - gw;
        const int16_t h = IMAGE_HEIGHT - gh;

        do {
            memcpy(vmem, vbuff, IMAGE_SIZE);

            int16_t xr = 0, yr = 0;
            int16_t xp = random(w - 4) + 2;
            int16_t yp = random(h - 4) + 2;

            do
            {
                xr = random(5) - 2;
                yr = random(5) - 2;
                col = random(5) - 2;
            } while (!xr && !yr && !col);

            for (int16_t i = 0; i < an; i++)
            {
                xp += xr;
                yp += yr;

                if (xp < 2 || xp >= w - 2) xr = -xr;
                if (yp < 2 || yp >= h - 2) yr = -yr;

                makeShadeBob(xp, yp);
                makeShadeBob(w - xp, yp);
                makeShadeBob(xp, h - yp);
                makeShadeBob(w - xp, h - yp);
                renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
                delay(FPS_90);
                readKeys();
                if (keyDown(SDL_SCANCODE_RETURN)) break;
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            }
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace shadePattern {
    typedef struct {
        uint16_t x, y;
    } TPoint;

    TPoint points[16000] = { 0 };

    uint8_t brush[30][30] = { 0 };
    uint16_t arrpos, arrmax;
    int32_t newx, newy, oldx, oldy;
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void initBrush()
    {
        for (int16_t c = 0; c < 30; c++)
            for (int16_t b = c; b < 30 - c; b++)
                for (int16_t a = c; a < 30 - c; a++) brush[a][b] = c << 2;
    }

    void generatePalette()
    {
        RGB pal[256] = { 0 };

        for (uint8_t i = 0; i < 64; i++)
        {
            pal[i].r = i;
            pal[i].g = 0;
            pal[i].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = i;
            pal[i + 64].b = 0;
            pal[i + 128].r = 63;
            pal[i + 128].g = 63;
            pal[i + 128].b = 0;
            pal[i + 192].r = 63;
            pal[i + 192].g = 63;
            pal[i + 192].b = i;
        }

        shiftPalette(pal);
        setPalette(pal);
    }

    void drawShadeBob(uint16_t xp, uint16_t yp)
    {
        for (int16_t b = 0; b < 30; b++)
        {
            for (int16_t a = 0; a < 30; a++)
            {
                int16_t c = vbuff[yp + b - 10][xp + a - 10] + brush[a][b] + (vmem[yp + b - 10][xp + a - 10] >> 2);
                if (c > 255) c = 255;
                vmem[yp + b - 10][xp + a - 10] = uint8_t(c);
            }
        }
    }

    void initParameter()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Shade-Bob")) return;
        initBrush();
        if (!loadPNG(vbuff[0], NULL, "assets/killer.png")) return;
        generatePalette();
        arrpos = 0;
    }

    void editShadeBob()
    {
        setMousePosition(IMAGE_WIDTH, IMAGE_HEIGHT);
        getMouseState(&oldx, &oldy);
        oldx >>= 1;
        oldy >>= 1;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            getMouseState(&newx, &newy);
            newx >>= 1;
            newy >>= 1;
            if (newx + 19 > MAX_WIDTH) newx = MAX_WIDTH - 19;
            if (newy + 19 > MAX_HEIGHT) newy = MAX_HEIGHT - 19;
            if (newx < 10) newx = 10;
            if (newy < 10) newy = 10;
            if (newx != oldx || newy != oldy)
            {
                points[arrpos].x = newx;
                points[arrpos].y = newy;
                drawShadeBob(newx, newy);
                renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
                delay(FPS_90);
                oldx = newx;
                oldy = newy;
                arrpos++;
                if (arrpos >= 16000) break;
            }
        }

        FILE* fp = fopen("assets/path.dat", "wb");
        if (fp)
        {
            fwrite(&arrpos, sizeof(arrpos), 1, fp);
            fwrite(points, sizeof(TPoint), arrpos, fp);
            fclose(fp);
        }
    }

    void playShadeBob()
    {
        FILE* fp = fopen("assets/path.dat", "rb");
        if (!fp) return;

        fread(&arrmax, sizeof(arrmax), 1, fp);
        if (arrmax >= 16000)
        {
            fclose(fp);
            return;
        }

        fread(points, sizeof(TPoint), arrmax, fp);
        fclose(fp);

        do {
            drawShadeBob(points[arrpos].x, points[arrpos].y);
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            arrpos++;
            if (arrpos >= arrmax) arrpos = 0;
        } while (!finished(SDL_SCANCODE_RETURN));
        cleanup();
    }

    void run(int16_t argc)
    {
        initParameter();
        if (argc == 1) editShadeBob();
        else playShadeBob();
    }
}

namespace shadeBobSin {
    double alpha, rx, ry;

    int16_t clear, reg;
    int16_t orgx, orgy;
    int16_t ux[1001] = { 0 };
    int16_t uy[1001] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    inline int16_t range(int16_t x, int16_t y)
    {
        return (x >= 0 && x <= MAX_WIDTH && y >= 0 && y <= MAX_HEIGHT);
    }

    void outDraw()
    {
        const double sinx = sin(alpha);
        const double cosx = cos(alpha);

        const int16_t x = int16_t(orgx + rx * pow(cosx, 3) * pow(sinx, 2));
        const int16_t y = int16_t(orgy + ry * pow(sinx, 3) * pow(cosx, 2));

        ux[reg] = x;
        uy[reg] = y;

        for (int16_t i = y - 5; i <= y + 5; i++)
        {
            for (int16_t j = x - 5; j <= x + 5; j++)
            {
                if (range(j, i))
                {
                    const uint8_t p = vbuff[i][j];
                    if (p < 250) vbuff[i][j] = p + 4;
                }
            }
        }
    }

    void inDraw()
    {
        int16_t x = 0, y = 0;

        if (reg >= 999)
        {
            x = ux[0];
            y = uy[0];
        }
        else
        {
            x = ux[reg + 1];
            y = uy[reg + 1];
        }

        if (!x && !y) return;

        for (int16_t i = y - 2; i <= y + 2; i++)
        {
            for (int16_t j = x - 2; j <= x + 2; j++)
            {
                if (range(j, i))
                {
                    const uint8_t p = vbuff[i][j];
                    if (p > 10 && p < 254) vbuff[i][j] = p - 10;
                }
            }
        }
    }

    void run()
    {
        int16_t x = 0, y = 0;
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Shade-Bob")) return;

        for (x = 0; x < 64; x++)
        {
            pal[x].r = uint8_t(x);
            pal[x].g = 0;
            pal[x].b = 0;
            pal[x + 64].r = 63;
            pal[x + 64].g = uint8_t(x);
            pal[x + 64].b = 0;
            pal[x + 128].r = 63;
            pal[x + 128].g = 63;
            pal[x + 128].b = 0;
            pal[x + 192].r = 63;
            pal[x + 192].g = 63;
            pal[x + 192].b = uint8_t(x);
        }

        shiftPalette(pal);
        setPalette(pal);
        if (!loadPNG(vbuff[0], NULL, "assets/killer.png")) return;

        alpha = 0;
        rx = 150;
        ry = 90;
        orgx = IMAGE_MIDX;
        orgy = IMAGE_MIDY;
        clear = 17400;
        reg = 0;

        do {
            outDraw();
            inDraw();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);

            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

            reg++;
            if (reg > 999) reg = 0;

            alpha += 0.001;
            rx += 0.05;
            ry += rx * 0.0001;
            clear--;
            
        } while (clear);

        for (y = 0; y < IMAGE_MIDY; y++)
        {
            for (x = 0; x < IMAGE_WIDTH; x++) vbuff[y][x] = 0;
            for (x = 0; x < IMAGE_WIDTH; x++) vbuff[MAX_HEIGHT - y][x] = 0;
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }
        cleanup();
    }
}

namespace snowFall {
    int16_t     actflk = 0;
    int16_t     xflake[IMAGE_WIDTH] = { 0 };
    int16_t     yflake[IMAGE_WIDTH] = { 0 };
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void updateFlakes()
    {
        for (int16_t i = 0; i < actflk; i++)
        {
            int16_t x = xflake[i];
            int16_t y = yflake[i];

            if (!vmem[y][x - 1] || !vmem[y][x] || !vmem[y][x + 1])
            {
                vmem[y - 1][x] = 0;

                if (rand() % 2 == 1)
                {
                    if (!vmem[y][x - 1])
                    {
                        y++;
                        x--;
                    }
                    else if (!vmem[y][x + 1])
                    {
                        y++;
                        x++;
                    }
                }
                else
                {
                    if (!vmem[y][x + 1])
                    {
                        y++;
                        x++;
                    }
                    else if (!vmem[y][x - 1])
                    {
                        y++;
                        x--;
                    }
                }

                vmem[y - 1][x] = 255;
                xflake[i] = x;
                yflake[i] = y;
            }
            else
            {
                if (vmem[y][x - 1] == 255 || vmem[y][x] == 255 || vmem[y][x + 1] == 255)
                {
                    xflake[i] = x;
                    yflake[i] = y;
                }
                else
                {
                    vmem[y - 1][x] = 36;
                    xflake[i] = rand() % IMAGE_WIDTH;
                    yflake[i] = 1;
                }
            }
        }
    }

    void initSnow()
    {
        RGB rgb[256] = { 0 };
        if (!loadPNG(vmem[0], rgb, "assets/flake.png")) return;
        rgb[255].r = 255;
        rgb[255].g = 255;
        rgb[255].b = 255;
        setPalette(rgb);
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Snow-Fall")) return;

        actflk = 0;
        initSnow();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            if (actflk < IMAGE_WIDTH)
            {
                xflake[actflk] = rand() % IMAGE_WIDTH;
                yflake[actflk] = 1;
                actflk++;
            }

            updateFlakes();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace softFire {
    uint16_t    incx = 48;
    uint8_t     dark[194][2] = { 0 };
    uint8_t     flames[122][IMAGE_HEIGHT] = { 0 };

    const uint8_t text[][200] = {
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,3,2,2,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0},{0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,
        1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,1,1,1,1,
        1,1,1,1,1,1,1,1,2,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,2,3},{0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},{0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
        2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,
        2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},{0,0,0,0,0,0,0,3,2,2,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,2,3},{0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},{0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},{0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,2,3},{0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},{0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
        3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3},{0,0,3,2,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,2,3},{0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,
        0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,
        0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,
        2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0},{0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,3,3,3,3,
        2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,3,3,3,3,3,3,3,3,2,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
        3,3,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,2,1,1,1,1,1,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0},{3,2,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,
        3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,0,0,0,0,0,0,0,0,0,0,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,
        1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,
        1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,
        1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,
        3,3,3,3,3,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,
        1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,
        1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,
        1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,
        1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,3,2,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,
        2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0},{0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,
        2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,3,2,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0},{0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,3,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,
        0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,
        3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,
        0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,
        1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,2,2,2,2,2,2,
        2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,
        1,1,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,
        1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,
        1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,
        1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,
        1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,
        1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,
        1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,
        1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,
        2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,3,3,3,3,3,3,3,3,3,
        3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0},{0,3,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,
        1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,
        0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,
        1,1,1,1,1,1,1,1,1,1,2,3,3,0,0,0,0,0,0,0,0,0,0,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,
        1,1,1,1,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,
        3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
        3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,3,
        3,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,3,3,3,3,3,3,3,
        3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,3,
        2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,
        1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0},{0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,
        1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,3,3,2,2,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,
        3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,3,3,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
        3,3,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,3,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,3,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,3,3,0,0,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,3,2,2,2,2,
        2,3,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,3,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
        3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,1,1,2,2,3,3,0,0,0,0,0,3,
        2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,2,3,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,
        2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,
        3,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,2,2,2,2,2,
        2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,2,2,3,3,3,3,3,3,2,2,1,1,1,
        1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,
        2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,1,2,3,3,0,0,0,0,0,0,3,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,
        1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,3,2,1,
        1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        3,2,2,2,2,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,2,
        3,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,
        1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,
        0,3,2,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,2,
        2,2,2,2,2,2,2,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,
        1,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,2,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,
        0,0,0,0,0,3,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,
        0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,
        0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,2,1,1,1,1,1,2,3,0,0,0,
        0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,2,1,1,
        1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,
        1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,2,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,
        3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,
        0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,2,3,2,1,1,1,1,1,2,
        3,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,
        0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,2,3,
        0,0,0,0,0,3,2,1,1,1,1,1,2,3,2,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,1,1,1,1,1,2,3,3,
        2,1,1,1,1,1,1,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,
        0,0,3,2,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,2,3,2,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,
        1,1,2,3,0,0,0,3,2,1,1,1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,
        0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,2,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,2,1,1,1,1,
        1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,
        1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,3,3,3,0,0,0,0,0,3,2,1,1,1,1,
        1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,
        2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,
        2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,1,2,
        3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,
        1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,
        1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,
        3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,
        1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,
        3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,1,2,2,
        2,2,2,2,2,1,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,2,3,3,0,0,0,0,0,0,3,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,
        3,2,1,1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,1,1,2,2,3,3,3,3,3,3,2,2,1,1,1,
        1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,
        2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,0,0,0,0,
        3,2,1,1,1,1,1,1,1,1,2,2,2,2,2,2,1,1,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,
        1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,
        3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,
        0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,
        0,0,0,3,2,1,1,1,1,1,2,3,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        1,1,1,1,2,3,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,
        0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,1,2,3,
        0,0,0,0,0,0,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,
        1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,
        1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,2,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,
        1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,3,3,2,2,1,1,1,1,1,1,
        1,1,2,2,3,3,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,0,0,0,3,2,1,1,1,1,1,2,3,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,2,3,0,0,
        0,0,0,0,0,3,2,1,1,1,1,1,2,3,3,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,3,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,0,0,3,2,2,2,2,
        2,3,0,0,0,0,0,0,0,0,0,0,0,3,3,2,2,2,2,2,2,2,2,3,3,0,0,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,0,0,
        0,0,3,2,2,2,2,2,3,0,0,3,2,2,2,2,2,3,0,0,0,0,0,0,0,0,0,3,2,2,2,2,2,3,0,0,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,3,
        3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,
        0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,0,0,0,3,3,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    };

    void run()
    {
        RGB pal[256] = { 0 };
        uint8_t i = 0, j = 0, k = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Soft-Fire -- Keys: A/S: +/- fires")) return;

        for (i = 0; i <= 7; i++)
        {
            pal[i].r = i;
            pal[i].g = i;
            pal[i].b = i;
            pal[i + 8].r = 8 - i;
            pal[i + 8].g = 8 - i;
            pal[i + 8].b = 8 - i;
            pal[i + 16].r = i;
            pal[i + 16].g = i;
            pal[i + 16].b = i;
            pal[i + 24].r = 8 - i;
            pal[i + 24].g = 8 - i;
            pal[i + 24].b = 8 - i;
        }

        for (i = 0; i <= 31; i++)
        {
            pal[i + 32].r = i << 1;
            pal[i + 32].g = 0;
            pal[i + 32].b = 0;
            pal[i + 64].r = 63;
            pal[i + 64].g = i;
            pal[i + 64].b = 0;
            pal[i + 96].r = 63;
            pal[i + 96].g = 32 + i;
            pal[i + 96].b = i;
            pal[i + 128].r = 63;
            pal[i + 128].g = 63;
            pal[i + 128].b = i + 32;
            pal[i + 160].r = 63 - i;
            pal[i + 160].g = 63;
            pal[i + 160].b = 63;
        }

        for (i = 0; i <= 31; i++)
        {
            pal[i + 192].r = 0;
            pal[i + 192].g = 0;
            pal[i + 192].b = i;
        }

        shiftPalette(pal);
        setPalette(pal);

        for (i = 0; i <= 36; i++) dark[i][0] = 0;
        for (i = 0; i <= 39; i++) dark[i][1] = 0;
        for (i = 40; i <= 192; i++) dark[i][0] = (i * 3) >> 2;
        for (i = 40; i <= 192; i++) dark[i][1] = i >> 1;

        for (j = 0; j <= 31; j++)
        {
            for (i = 0; i <= 7; i++)
            {
                for (k = 0; k <= 1; k++)
                {
                    horizLine(i * 40 + 10, j * 2 + k, 21, j + 192);
                    horizLine(i * 40 + 10, j * 2 + k + 64, 21, 223 - j);
                    horizLine(i * 40 + 10, j * 2 + k + 128, 21, j + 192);
                    horizLine(i * 40 + 30, j * 2 + k, 21, 223 - j);
                    horizLine(i * 40 + 30, j * 2 + k + 64, 21, 192 + j);
                    horizLine(i * 40 + 30, j * 2 + k + 128, 21, 223 - j);
                }
            }
        }

        for (j = 0; j <= 115; j++) for (i = 0; i < IMAGE_HEIGHT; i++) flames[j][i] = 0;

        do {
            for (j = 0; j <= 118; j++) for (i = 1; i < MAX_HEIGHT; i++)
            {
                k = (flames[j + 1][i] + flames[j + 2][i] + flames[j + 1][i - 1] + flames[j + 1][i + 1]) >> 2;
                if (k > 0) k--;
                flames[j][i] = k;
            }

            for (j = 119; j <= 120; j++)
            {
                for (i = 0; i < IMAGE_HEIGHT; i++) flames[j][i] = IMAGE_MIDX * (rand() % 2) + incx;
            }

            for (j = 0; j <= 112; j++)
            {
                for (i = 0; i < IMAGE_HEIGHT; i++)
                {
                    switch (text[j][i])
                    {
                    case 1: putPixel(i + 60, j + 50, flames[j][i]); break;
                    case 2: putPixel(i + 60, j + 50, dark[flames[j][i]][0]); break;
                    case 3: putPixel(i + 60, j + 50, dark[flames[j][i]][1]); break;
                    }
                }
            }

            render();
            delay(FPS_90);
            readKeys();
            if (keyDown(SDL_SCANCODE_A) && incx < 74) incx += 4;
            if (keyDown(SDL_SCANCODE_S) && incx > 4) incx -= 4;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace spriteEffect {
    #define SW 34
    #define SH 63
    #define KC 15

    typedef uint8_t TFrame[SH][SW];

    const uint8_t palbubmio[][3] = {
        {0,	24,	0}, {6,	35, 10}, {12, 47, 20}, {26, 16,	0},
        {37, 26, 0}, {29, 28, 24}, {47, 41, 0}, {20, 59, 30},
        {47, 45, 47}, {53, 53, 2}, {57, 24, 24}, {61, 61, 61},
        {63, 33, 33}, {63, 43, 43}, {63, 63, 49}, {63, 63, 63}
    };

    typedef struct {
        int16_t x, y;
        int16_t speed, frame;
        uint8_t active;
    } TSprite;

    TSprite bubmio[10] = { 0 };
    TFrame frames[7] = { 0 };
    uint8_t vbuff1[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t vbuff2[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void putFrame(int16_t x, int16_t y, int16_t k)
    {
#ifdef _USE_ASM
        uint8_t* pdata = (uint8_t*)frames[k];

        __asm {
            mov     esi, pdata
            lea     edi, vbuff1
            xor     ebx, ebx
            mov     bx, y
            shl     bx, 6
            add     bh, byte ptr y
            add     bx, x
            add     edi, ebx
            mov     dl, SH
            mov     ecx, SW
            mov     eax, ecx
        draw:
            push    edi
            mov     ecx, eax
        line:
            mov     bl, [esi]
            cmp     bl, KC
            jnz     store
            inc     esi
            inc     edi
            loop    line
            jmp     next
        store:
            movsb
            loop    line
        next:
            pop     edi
            dec     dl
            jz      done
            add     edi, IMAGE_WIDTH
            jmp     draw
        done:
        }
#else
        for (int16_t i = 0; i < SH; i++)
        {
            for (int16_t j = 0; j < SW; j++)
            {
                const uint8_t col = frames[k][i][j];
                if (col != KC) vbuff1[y + i][x + j] = frames[k][i][j];
            }
        }
#endif
    }

    void copyBlock(int16_t x, int16_t y)
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff1
            lea     esi, vbuff2
            xor     ebx, ebx
            mov     bx, y
            shl     bx, 6
            add     bh, byte ptr y
            add     bx, x
            add     edi, ebx
            add     esi, ebx
            mov     al, 60
            mov     bx, SH
        plot:
            mov     ecx, SW / 2
            rep     movsw
            add     edi, IMAGE_WIDTH - SW
            add     esi, IMAGE_WIDTH - SW
            dec     bx
            jnz     plot
        }
#else
        for (int16_t h = 0; h < SH; h++) memcpy(&vbuff1[y + h][x], &vbuff2[y + h][x], SW);
#endif
    }

    void newBubmio()
    {
        int16_t i = 0;
        while (i < 10)
        {
            if (!bubmio[i].active)
            {
                bubmio[i].x = SW + rand() % 100;
                bubmio[i].y = 0;
                bubmio[i].active = 1;
                bubmio[i].frame = 0;
                bubmio[i].speed = rand() % 6 + 1;
                break;
            }
            i++;
        }
    }

    void removeBubmio()
    {
        int16_t i = 0;
        while (i < 10)
        {
            if (bubmio[i].active)
            {
                copyBlock(bubmio[i].x, bubmio[i].y);
                bubmio[i].active = 0;
                break;
            }
            i++;
        }
    }

    void jumpBubmio()
    {
        int16_t i = 0;

        newBubmio();

        do {
            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            if (keyPressed(SDL_SCANCODE_A)) newBubmio();
            if (keyPressed(SDL_SCANCODE_S)) removeBubmio();
            
            for (i = 0; i < 10; i++)
            {
                if (bubmio[i].active)
                {
                    copyBlock(bubmio[i].x, bubmio[i].y);
                    bubmio[i].x += bubmio[i].speed;
                    bubmio[i].y += bubmio[i].speed;

                    if (bubmio[i].x >= IMAGE_WIDTH - SW || bubmio[i].y >= IMAGE_HEIGHT - SH)
                    {
                        bubmio[i].active = 0;
                        newBubmio();
                    }
                }
            }

            for (i = 0; i < 10; i++)
            {
                if (bubmio[i].active)
                {
                    putFrame(bubmio[i].x, bubmio[i].y, bubmio[i].frame);
                    bubmio[i].frame++;
                    if (bubmio[i].frame == 7) bubmio[i].frame = 0;
                }
            }

            renderBuffer(vbuff1, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_30);
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }

    void run()
    {
        int16_t i = 0;
        RGB pal[256] = { 0 };
        RGB tmp[256] = { 0 };
        
        FILE* fp = fopen("assets/bubmio.dat", "rb");
        if (!fp) exit(1);
        for (i = 0; i < 7; i++) fread(frames[i], SW * SH, 1, fp);
        fclose(fp);

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Sprites -- Keys: A/S: +/- bubmio!")) return;

        for (i = 0; i < 16; i++)
        {
            pal[i].r = palbubmio[i][0] << 2;
            pal[i].g = palbubmio[i][1] << 2;
            pal[i].b = palbubmio[i][2] << 2;
        }

        if (!loadPNG(vbuff1[0], tmp, "assets/fun.png")) return;

        for (i = 16; i < 256; i++)
        {
            pal[i].r = tmp[i].r;
            pal[i].g = tmp[i].g;
            pal[i].b = tmp[i].b;
        }

        setPalette(pal);

        memcpy(getDrawBuffer(), vbuff1, IMAGE_SIZE);
        memcpy(vbuff2, vbuff1, IMAGE_SIZE);

        jumpBubmio();
    }
}

namespace starEffect {
    #define MAXPOINT    50

    typedef struct {
        int16_t x, y;
        int16_t state, color;
        int16_t delta;
    } TStar;

    TStar stars[MAXPOINT] = { 0 };

    void drawPoint(TStar p, int16_t state)
    {
        const int16_t color = state ? p.color : 0;

        switch (p.state)
        {
        case 0:
            putPixel(p.x, p.y, color);
            break;

        case 1:
            drawLine(p.x - 1, p.y, p.x + 1, p.y, color);
            drawLine(p.x, p.y - 1, p.x, p.y + 1, color);
            break;

        case 2:
            drawLine(p.x - 2, p.y, p.x + 2, p.y, color);
            drawLine(p.x, p.y - 2, p.x, p.y + 2, color);
            break;

        case 3:
            drawLine(p.x - 4, p.y, p.x + 4, p.y, color);
            drawLine(p.x, p.y - 4, p.x, p.y + 4, color);
            drawRect(p.x - 1, p.y - 1, 2, 2, color);
            break;

        default:
            break;
        }
    }

    void run()
    {
        int16_t i = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Stars")) return;
        
        for (i = 0; i < MAXPOINT; i++)
        {
            stars[i].x = random(IMAGE_WIDTH);
            stars[i].y = random(IMAGE_HEIGHT);
            stars[i].state = random(4);
            stars[i].color = 35 + random(69);
            stars[i].delta = random(2);
        }

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (i = 0; i < MAXPOINT; i++) drawPoint(stars[i], 1);
            render();
            delay(FPS_30);

            for (i = 0; i < MAXPOINT; i++) drawPoint(stars[i], 0);

            for (i = 0; i < MAXPOINT; i++)
            {
                if (stars[i].delta)
                {
                    stars[i].state++;
                    if (stars[i].state >= 3) stars[i].delta = 0;
                }
                else
                {
                    stars[i].state--;
                    if (stars[i].state <= 0) stars[i].delta = 1;
                }
            }
        }
        cleanup();
    }
}

namespace star2dEffect {
    #define STARCOL     140
    #define MAXSTARS    200
    #define MAXSPEED    4

    int16_t    starx[MAXSTARS] = { 0 };
    int16_t    stary[MAXSTARS] = { 0 };
    int16_t    speed[MAXSTARS] = { 0 };
    uint8_t    vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void createStar()
    {
        for (int16_t i = 0; i < MAXSTARS; i++)
        {
            starx[i] = random(IMAGE_WIDTH);
            stary[i] = random(IMAGE_HEIGHT);
            speed[i] = random(MAXSPEED) + 1;
            if (!vmem[stary[i]][starx[i]]) vmem[stary[i]][starx[i]] = STARCOL;
        }
        makeLinearPalette();
    }

    void moveStar()
    {
        for (int16_t i = 0; i < MAXSTARS; i++)
        {
            if (vmem[stary[i]][starx[i]] == STARCOL) vmem[stary[i]][starx[i]] = 0;
            else
            {
                starx[i] += speed[i];
                if (starx[i] >= IMAGE_WIDTH) starx[i] -= IMAGE_WIDTH;
                else if (!vmem[stary[i]][starx[i]]) vmem[stary[i]][starx[i]] = STARCOL;
            }
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "2D-Stars")) return;
        createStar();

        while (!finished(SDL_SCANCODE_RETURN))
        {
            moveStar();
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace star3dEffect {
    typedef struct {
        int16_t x, y, z;
    } TPoint3D;

    typedef struct {
        int16_t x, y;
    } TPoint2D;

    TPoint3D stars[1000] = { 0 };
    TPoint2D pos[1000] = { 0 };
    uint8_t density, direction;
    uint8_t bounce, bx, by;
    int16_t speed, xofs, yofs;

    void initStars()
    {
        for (int16_t i = 0; i < 1000; i++)
        {
            do
            {
                stars[i].x = random(IMAGE_WIDTH) - IMAGE_MIDX;
                stars[i].y = random(IMAGE_HEIGHT) - IMAGE_MIDY;
                stars[i].z = i + 1;
            } while (!stars[i].x || !stars[i].y);
        }
    }

    void calcStars()
    {
        for (int16_t i = 0; i < 1000; i++)
        {
            pos[i].x = ((stars[i].x << density) / stars[i].z) + xofs;
            pos[i].y = ((stars[i].y << density) / stars[i].z) + yofs;
        }
    }

    void drawStars()
    {
        for (int16_t i = 0; i < 1000; i++)
        {
            if (pos[i].x > 0 && pos[i].x < IMAGE_WIDTH && pos[i].y > 0 && pos[i].y < IMAGE_HEIGHT)
            {
                if (stars[i].z < 100) putPixel(pos[i].x, pos[i].y, 4);
                else if (stars[i].z < 200) putPixel(pos[i].x, pos[i].y, 3);
                else if (stars[i].z < 300) putPixel(pos[i].x, pos[i].y, 2);
                else if (stars[i].z < 400) putPixel(pos[i].x, pos[i].y, 1);
            }
        }
    }

    void clearStars()
    {
        for (int16_t i = 0; i < 1000; i++)
        {
            if (pos[i].x > 0 && pos[i].x < IMAGE_WIDTH && pos[i].y > 0 && pos[i].y < IMAGE_HEIGHT) putPixel(pos[i].x, pos[i].y, 0);
        }
    }

    void moveStars(uint8_t dir)
    {
        for (int16_t i = 0; i < 1000; i++)
        {
            if (dir)
            {
                stars[i].z -= speed;
                if (stars[i].z < 1) stars[i].z = 1000 - 1;
            }
            else
            {
                stars[i].z += speed;
                if (stars[i].z > 1000 - 1) stars[i].z = 1;
            }
        }
    }

    void bounceStars()
    {
        if (bx) xofs += speed; else xofs -= speed;
        if (by) yofs += speed; else yofs -= speed;
        if (xofs < 1 || xofs > MAX_WIDTH) bx = !bx;
        if (yofs < 1 || yofs > MAX_HEIGHT) by = !by;
    }

    void run()
    {
        RGB pal[256] = { 0 };
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Star-Field -- Keys: A/S: +/- speed; Z/X: +/- density; Arrows: move; B: toggles; Spacer: direction")) return;
        initStars();

        pal[1].r = 0;
        pal[1].g = 0;
        pal[1].b = 30;
        pal[2].r = 10;
        pal[2].g = 10;
        pal[2].b = 40;
        pal[3].r = 20;
        pal[3].g = 20;
        pal[3].b = 50;
        pal[4].r = 30;
        pal[4].g = 30;
        pal[4].b = 60;
        shiftPalette(pal);
        setPalette(pal);
        clearScreen();

        direction = 1;
        speed = 2;
        density = 7;
        xofs = IMAGE_MIDX;
        yofs = IMAGE_MIDY;
        bounce = 0;
        bx = 1;
        by = 0;

        do {
            calcStars();
            drawStars();
            render();
            delay(FPS_90);
            clearStars();
            moveStars(direction);
            readKeys();

            if (bounce) bounceStars();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            if (keyPressed(SDL_SCANCODE_A)) if (speed < 30) speed++;
            if (keyPressed(SDL_SCANCODE_S)) if (speed > 0) speed--;
            if (keyPressed(SDL_SCANCODE_Z)) if (density < 9) density++;
            if (keyPressed(SDL_SCANCODE_X)) if (density > 6) density--;
            if (keyDown(SDL_SCANCODE_LEFT)) if (xofs > 0) xofs--;
            if (keyDown(SDL_SCANCODE_RIGHT)) if (xofs < MAX_WIDTH) xofs++;
            if (keyDown(SDL_SCANCODE_UP)) if (yofs > 0) yofs--;
            if (keyDown(SDL_SCANCODE_DOWN)) if (yofs < MAX_HEIGHT) yofs++;
            if (keyPressed(SDL_SCANCODE_B)) bounce = !bounce;
            if (keyPressed(SDL_SCANCODE_SPACE)) direction = !direction;
        } while (!keyDown(SDL_SCANCODE_RETURN));

        freeFont(0);
        cleanup();
    }
}

namespace fontEffect1 {
    uint8_t*    chars[64] = { 0 };
    uint8_t     chrinfo[64][3] = { 0 };
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    int16_t loadFont(const char* fname)
    {
        int8_t ch = 0;
        uint8_t tx = 0, ty = 0;
        uint8_t pal[768] = { 0 };
        RGB rgb[256] = { 0 };

        FILE* fp = fopen(fname, "rb");
        if (!fp) return 0;

        fread(pal, 768, 1, fp);

        while (!feof(fp))
        {
            fread(&ch, 1, 1, fp);
            fread(&tx, 1, 1, fp);
            fread(&ty, 1, 1, fp);

            const uint16_t size = tx * ty;

            chars[ch - 32] = (uint8_t*)calloc(size, 1);
            if (!chars[ch - 32]) exit(1);

            chrinfo[ch - 32][0] = tx;
            chrinfo[ch - 32][1] = ty;
            chrinfo[ch - 32][2] = 1;
            fread(chars[ch - 32], size, 1, fp);
        }

        fclose(fp);
        convertPalette(pal, rgb);
        setPalette(rgb);
        return 1;
    }

    void putChar(int16_t dx, int16_t dy, int8_t ch)
    {
        const uint8_t x = chrinfo[ch - 32][0];
        const uint8_t y = chrinfo[ch - 32][1];

        for (uint8_t j = 0; j < y; j++)
        {
            for (uint8_t i = 0; i < x; i++)
            {
                const uint8_t c = *(chars[ch - 32] + intptr_t(j) * x + i);
                if (c > 0) vmem[dy + j][dx + i] = c;
            }
        }
    }

    void writeXY(int16_t x, int16_t y, const char* str)
    {
        for (int16_t i = 0; i < int16_t(strlen(str)); i++)
        {
            if (str[i] >= 32 && str[i] <= 93)
            {
                if (str[i] == 32) x += chrinfo['H' - 32][0] / 2;
                else
                {
                    putChar(x, y, str[i]);
                    x += chrinfo[str[i] - 32][0] + 1;
                }
            }
        }
    }

    void freeMem()
    {
        for (int16_t i = 0; i < 64; i++)
        {
            if (chrinfo[i][2])
            {
                free(chars[i]);
                chrinfo[i][2] = 0;
            }
        }
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Fonts")) return;
        if (!loadFont("assets/font001.fnt")) return;
        writeXY(1, 2, "I LOVE ...");
        writeXY(1, 38, "DEMOS!");
        writeXY(1, 74, "GRAPHICS!");
        writeXY(1, 110, "BITMAP!");
        writeXY(1, 146, "SUPER VESA!");
        renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
        while (!finished(SDL_SCANCODE_RETURN));
        freeMem();
        cleanup();
    }
}

namespace fontEffect2 {
    uint8_t* chars[64] = { 0 };
    uint16_t ypos[IMAGE_WIDTH] = { 0 };
    uint8_t chrinfo[64][3] = { 0 };
    uint8_t bitmap[60][IMAGE_WIDTH] = { 0 };
    uint8_t sint1[230] = { 0 };
    uint8_t sint2[115] = { 0 };
    uint8_t row, newrow, index;
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    const char text[] = "ILOVE...DEMO!WATCOMC!GRAPHICS!BITMAP!SUPER VESA!";

    void calcPos()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++) ypos[x] = sint1[(x + index) % 230] + sint2[x % 115];
    }

    void calcSin()
    {
        int16_t i = 0;
        for (i = 0; i < 230; i++) sint1[i] = uint8_t(sin(2 * M_PI * i / 230) * 20 + 20);
        for (i = 0; i < 115; i++) sint2[i] = uint8_t(sin(2 * M_PI * i / 115) * 3 + 3);
    }

    int16_t loadFont(const char* fname)
    {
        int8_t chr = 0;
        uint8_t w = 0, h = 0;
        uint8_t pal[768] = { 0 };
        RGB rgb[256] = { 0 };

        FILE* fp = fopen(fname, "rb");
        if (!fp) return 0;

        fread(pal, 768, 1, fp);

        while (!feof(fp))
        {
            fread(&chr, 1, 1, fp);
            fread(&w, 1, 1, fp);
            fread(&h, 1, 1, fp);
            const uint16_t size = w * h;

            chars[chr - 32] = (uint8_t*)calloc(size, 1);
            if (!chars[chr - 32]) exit(1);

            chrinfo[chr - 32][0] = w;
            chrinfo[chr - 32][1] = h;
            chrinfo[chr - 32][2] = 1;
            fread(chars[chr - 32], size, 1, fp);
        }

        fclose(fp);
        convertPalette(pal, rgb);
        setPalette(rgb);
        return 1;
    }

    void drawBitmap(uint8_t* map)
    {
        for (int16_t i = 0; i < IMAGE_WIDTH; i++)
        {
            uint16_t y = 0;
            for (int16_t j = 0; j < 60; j++)
            {
                vmem[ypos[i] + j][i] = map[y + i];
                y += IMAGE_WIDTH;
            }
        }
    }

    void freeMem()
    {
        for (int16_t i = 0; i < 64; i++)
        {
            if (chrinfo[i][2])
            {
                free(chars[i]);
                chrinfo[i][2] = 0;
            }
        }
    }

    void newRows(int8_t chr, uint8_t row, int16_t pos)
    {
        int16_t x = 0, y = 0;

        if (chrinfo[chr - 32][2] != 1) return;

        for (y = 0; y < 60; y++) bitmap[y][pos] = 0;

        x = (60 - chrinfo[chr - 32][1]) >> 1;

        if (row == chrinfo[chr - 32][0] + 1)
        {
            for (y = 0; y < chrinfo[chr - 32][1]; y++) bitmap[y + x][pos - 1] = 0;
        }
        else
        {
            for (y = 0; y < chrinfo[chr - 32][1]; y++) bitmap[y + x][pos] = *(chars[chr - 32] + intptr_t(y) * chrinfo[chr - 32][0] + row);
        }
    }

    void updateRows()
    {
        row++;
        if (row >= chrinfo[text[newrow] - 32][0])
        {
            row = 0;
            newrow++;
            if (newrow >= uint8_t(strlen(text))) newrow = 0;
        }
    }

    void run()
    {
        memset(bitmap, 0, sizeof(bitmap));
        calcSin();
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Text")) return;
        if (!loadFont("assets/font003.fnt")) return;

        row = 0;
        newrow = 0;
        index = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            memcpy(bitmap, &bitmap[0][2], sizeof(bitmap));
            updateRows();
            newRows(text[newrow], row, IMAGE_WIDTH - 2);
            updateRows();
            newRows(text[newrow], row, MAX_WIDTH);
            calcPos();
            drawBitmap(bitmap[0]);
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            index = (index + 4) % 230;
        }

        freeMem();
        cleanup();
    }
}

namespace fontEffect3 {
    uint8_t* chars[64] = { 0 };
    uint8_t chrinfo[64][3] = { 0 };
    uint8_t bitmap[30][IMAGE_WIDTH] = { 0 };
    uint8_t sintab[256] = { 0 };
    uint8_t row, newrow, index;
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    const char text[] = "ILOVE...DEMO!WATCOMC!GRAPHIC!BITMAP!SUPER VESA!";

    void calcSin()
    {
        for (int16_t i = 0; i < 256; i++) sintab[i] = uint8_t(sin(15 * M_PI * i / 755) * 24 + 70);
    }

    int16_t loadFont(const char* fname)
    {
        uint8_t width = 0, height = 0, chr = 0;
        uint8_t pal[768] = { 0 };
        RGB rgb[256] = { 0 };

        FILE* fp = fopen(fname, "rb");
        if (!fp) return 0;
        fread(pal, 768, 1, fp);

        while (!feof(fp))
        {
            fread(&chr, 1, 1, fp);
            fread(&width, 1, 1, fp);
            fread(&height, 1, 1, fp);
            const uint16_t size = width * height;

            chars[chr - 32] = (uint8_t*)calloc(size, 1);
            if (!chars[chr - 32]) return 0;

            chrinfo[chr - 32][0] = width;
            chrinfo[chr - 32][1] = height;
            chrinfo[chr - 32][2] = 1;
            fread(chars[chr - 32], size, 1, fp);
        }

        fclose(fp);
        convertPalette(pal, rgb);
        setPalette(rgb);
        return 1;
    }

    void freeMem()
    {
        for (int16_t i = 0; i < 64; i++)
        {
            if (chrinfo[i][2])
            {
                free(chars[i]);
                chrinfo[i][2] = 0;
            }
        }
    }

    void newRows(uint8_t chr, uint8_t row, int16_t pos)
    {
        int16_t height = 0, i = 0;
        if (chrinfo[chr - 32][2] != 1) return;

        i = (30 - chrinfo[chr - 32][1]) >> 1;

        for (height = 0; height < 30; height++) bitmap[height][pos] = 0;

        if (row == chrinfo[chr - 32][0] + 1)
        {
            for (height = 0; height < chrinfo[chr - 32][1]; height++) bitmap[height + i][pos - 1] = 0;
        }
        else
        {
            for (height = 0; height < chrinfo[chr - 32][1]; height++) bitmap[height + i][pos] = *(chars[chr - 32] + intptr_t(height) * chrinfo[chr - 32][0] + row);
        }
    }

    void updateRows()
    {
        row++;
        if (row >= chrinfo[text[newrow] - 32][0])
        {
            row = 0;
            newrow++;
            if (newrow >= uint8_t(strlen(text))) newrow = 0;
        }
    }

    void run()
    {
        memset(bitmap, 0, sizeof(bitmap));
        calcSin();
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Text-Scrolling")) return;
        if (!loadFont("assets/font002.fnt")) return;

        row = 0;
        newrow = 0;
        index = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            memcpy(bitmap, &bitmap[0][2], 9600);
            updateRows();
            newRows(text[newrow], row, IMAGE_WIDTH - 2);
            updateRows();
            newRows(text[newrow], row, MAX_WIDTH);
            memcpy(&vmem[sintab[index]][0], bitmap, 9600);
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
            index = (index + 2) % IMAGE_MIDY;
        }

        freeMem();
        cleanup();
    }
}

namespace thunderBoltEffect {
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void addItem(int16_t x, int16_t y, uint8_t item)
    {
        const uint8_t col = vmem[y][x];
        if (col < item) vmem[y][x] = item;
    }

    void processItem(int16_t x, int16_t y, int16_t ints, int16_t dx)
    {
        int16_t part = 0, dy = 0;

        while (ints > 0 && y < IMAGE_HEIGHT - 10 && x > 10 && x < IMAGE_WIDTH - 10)
        {
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

            addItem(x + 1, y, (ints - 3) * 20 + 50);
            addItem(x, y, (ints * 20) + 50);
            addItem(x - 1, y, (ints - 2) * 20 + 50);

            if ((!random(4) && (y > 170)) || (!random(4) && abs(dx) > 5) || !random(20)) ints--;

            y++;

            if (dx > 0) x -= random(dx);
            else x += random(-dx);

            if (abs(dx) < 2)
            {
                x -= random(5) + 1;
                dx = -2;
            }

            if (abs(dx) > 5)
            {
                x += random(5) + 1;
                dx = 2;
            }

            if ((y > 33 && !random(3)) || ints < 4)
            {
                part = random(10);

                do {
                    dy = random(10);
                } while (!dy);

                processItem(x, y, (ints * part / 10) - (y / 20), -random(dy));
                processItem(x, y, (ints * (10 - part) / 10) - (y / 20), random(dy));
            }
        }
    }

    void flashPalette(RGB* pal)
    {
        int16_t i = 0, j = 0;
        RGB flash[256] = { 0 };

        memcpy(flash, pal, 1024);
        flash[0].r = 255;
        flash[0].g = 255;
        flash[0].b = 255;
        setPalette(flash);
        delay(15);

        for (j = 63; j >= 0; j--)
        {
            for (i = 0; i <= 255; i++)
            {
                if (pal[i].r > 4) pal[i].r -= 4;
                if (pal[i].g > 4) pal[i].g -= 4;
                if (pal[i].b > 4) pal[i].b -= 4;
            }
            setPalette(pal);
            delay(15);
            readKeys();
            if (keyDown(SDL_SCANCODE_RETURN)) break;
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
        }
    }

    void thunderBolt(const RGB* src, RGB* dst)
    {
        int16_t dx = 0;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            do {
                dx = random(6) - 3;
            } while (abs(dx) <= 1);

            setPalette(dst);
            processItem(random(100) + 110, 0, 10, dx);
            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            flashPalette(dst);
            memcpy(dst, src, 1024);
            memset(vmem, 0, IMAGE_SIZE);
        }
        cleanup();
    }

    void run()
    {
        RGB dst[256] = { 0 };
        RGB src[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Thunder-Bold")) return;

        for (uint8_t i = 0; i < 64; i++)
        {
            dst[i].r = 0;
            dst[i].g = 0;
            dst[i].b = i;
            dst[i + 64].r = 0;
            dst[i + 64].g = i;
            dst[i + 64].b = 63;
            dst[i + 128].r = i;
            dst[i + 128].g = 63;
            dst[i + 128].b = 63;
            dst[i + 192].r = 63;
            dst[i + 192].g = 63;
            dst[i + 192].b = 63;
        }

        shiftPalette(dst);
        memcpy(src, dst, 256 * sizeof(RGB));
        thunderBolt(src, dst);
    }
}

namespace scrollingEffect {
    typedef uint8_t TCHAR[16];

    const uint8_t bitMask[] = { 128, 64, 32, 16, 8, 4, 2, 1 };
    const char* scrolledText = "I LOVE DEMO, GRAPHICS, BITMAP, SUPER VESA ! ";

    uint16_t    sintab[IMAGE_WIDTH] = { 0 };
    uint8_t     chrtab[IMAGE_WIDTH] = { 0 };
    uint8_t     coltab[IMAGE_WIDTH] = { 0 };
    TCHAR       tbuff[SIZE_256] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    int16_t loadFont()
    {
        FILE* fp = fopen("assets/font8x16.fnt", "rb");
        if (!fp) return 0;
        fread(tbuff, 1, sizeof(tbuff), fp);
        fclose(fp);
        return 1;
    }

    void calcSin()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++)
        {
            sintab[x] = uint16_t(sin(5 * M_PI * x / IMAGE_WIDTH) * 22 + 22);
            if (cos(5 * M_PI * x / IMAGE_WIDTH) <= 0) coltab[x] = 15;
            else coltab[x] = 14;
        }
    }

    void scrollText()
    {
        uint16_t pos = 0;
        const uint16_t len = uint16_t(strlen(scrolledText));

        do {
            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
            if (keyDown(SDL_SCANCODE_RETURN)) break;

            const uint8_t chr = scrolledText[pos];
            for (int16_t i = 0; i < 16; i++)
            {
                readKeys();
                if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
                if (keyDown(SDL_SCANCODE_RETURN)) break;

                for (int16_t x = 0; x < MAX_WIDTH; x++) chrtab[x] = chrtab[x + 1];
                chrtab[MAX_WIDTH] = tbuff[chr][i];

                for (int16_t x = 0; x < IMAGE_WIDTH - 8; x++)
                {
                    for (int16_t k = 0; k < 8; k++)
                    {
                        if (chrtab[x] & bitMask[k]) vmem[80 - sintab[x]][x + k] = coltab[x];
                        else vmem[80 - sintab[x]][x + k] = vbuff[80 - sintab[x]][x + k];
                    }
                }

                renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
                delay(FPS_90);
            }

            pos++;
            if (pos >= len) pos = 0;
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }

    void run()
    {
        RGB pal[256] = { 0 };

        if (!loadFont()) return;
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Text-Scrolling")) return;
        if (!loadPNG(vbuff[0], pal, "assets/friend.png")) return;
        setPalette(pal);
        memcpy(vmem, vbuff, IMAGE_SIZE);
        renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
        calcSin();
        scrollText();
    }
}

namespace voxelEffect {
    uint8_t cmap[SIZE_256][SIZE_256] = { 0 };
    uint8_t hmap[SIZE_256][SIZE_256] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t sky[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    uint16_t x, y, height;
    int16_t angle, dst;
    int16_t costab[2048] = { 0 };
    int16_t sintab[2048] = { 0 };
    int16_t dcomp[127] = { 0 };

    void vertLine(uint16_t x, uint16_t y, uint16_t len, uint8_t col)
    {
#ifdef _USE_ASM
        __asm {
            lea     edi, vbuff
            xor     ebx, ebx
            mov     bx, y
            mov     cx, bx
            shl     bx, 8
            shl     cx, 6
            add     bx, cx
            add     bx, x
            add     edi, ebx
            mov     cx, len
            mov     al, col
        next:
            stosb
            add     edi, MAX_WIDTH
            dec     cx
            jnz     next
        }
#else
        for (uint16_t i = 0; i < len; i++) vbuff[y + i][x] = col;
#endif
    }

    void ray(uint16_t a, uint16_t dx, uint16_t dy, uint16_t sx)
    {
        const uint16_t delx = costab[a];
        const uint16_t dely = sintab[a];

        uint16_t dt = 1;
        uint16_t miny = MAX_HEIGHT;

        while (dt < 127)
        {
            dx += delx;
            dy += dely;

            const int32_t h = hmap[dy >> 8][dx >> 8] - height;
            const uint16_t y = dcomp[dt - 1] - (h << 5) / dt + dst;

            if (y <= miny)
            {
                vertLine(sx, y, miny - y + 1, cmap[dy >> 8][dx >> 8]);
                miny = y;
            }

            dt++;
        }
    }

    void drawView()
    {
        for (int16_t i = 0; i < IMAGE_WIDTH; i++)
        {
            const int16_t a = (angle + i + 1888) & 0x7ff;
            ray(a, x, y, i);
        }
    }

    int16_t initMap()
    {
        RGB rgb[256] = { 0 };
        if (!loadPNG(cmap[0], rgb, "assets/ground.png")) return 0;
        if (!loadPNG(hmap[0], NULL, "assets/height.png")) return 0;
        if (!loadPNG(sky[0], NULL, "assets/sky.png")) return 0;
        setPalette(rgb);
        return 1;
    }

    void initTables()
    {
        int16_t i = 0;
        for (i = 0; i < 2048; i++)
        {
            costab[i] = int16_t(cos(i * M_PI / 1024) * 256);
            sintab[i] = int16_t(sin(i * M_PI / 1024) * 256);
        }

        for (i = 1; i < 128; i++) dcomp[i - 1] = 2000 / i + IMAGE_MIDY;

        x = 32767;
        y = 32767;
        dst = 0;
        angle = 600;
    }

    void run()
    {
        initTables();

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Voxel -- Keys: A/S: change height; Arrows: move")) return;
        if (!initMap()) return;

        do {
            height = hmap[y >> 8][x >> 8];
            memcpy(vbuff, sky, IMAGE_SIZE);
            drawView();
            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);

            readKeys();
            if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

            if (keyDown(SDL_SCANCODE_UP))
            {
                x += costab[angle] << 1;
                y += sintab[angle] << 1;
            }

            if (keyDown(SDL_SCANCODE_DOWN))
            {
                x -= costab[angle] << 1;
                y -= 2 * sintab[angle] << 1;
            }

            if (keyDown(SDL_SCANCODE_LEFT)) angle = (angle + 4064) & 0x7ff;
            if (keyDown(SDL_SCANCODE_RIGHT)) angle = (angle + 32) & 0x7ff;
            if (keyDown(SDL_SCANCODE_A)) dst = 80;
            else if (keyDown(SDL_SCANCODE_S)) dst = -100;
            else dst = 0;
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace rippleEffect2 {
    #define S31	    961.0    
    #define S159    25281.0
    
    uint8_t wb[64][128] = { 0 };
    uint8_t lb[32][IMAGE_MIDX] = { 0 };
    uint8_t heighest[IMAGE_MIDX] = { 0 };

    void generateLand()
    {
        for (int16_t i = 0; i < 32; i++)
        {
            for (int16_t j = 0; j < IMAGE_MIDX; j++)
            {
                lb[i][j] = uint8_t(sqrt((double(i) * i / S31) + (double(j) * j / S159)) * 127);
                if (lb[i][j] > 127) lb[i][j] = 127;
            }
        }
    }

    void generateWav()
    {
        for (int16_t j = 0; j < 64; j++)
        {
            for (int16_t i = 0; i < 128; i++)
            {
                const double r = 5 * M_PI * (i + 1.0) / IMAGE_WIDTH;
                const double d = j * M_PI / 32;
                const double v = ((sin(r - d) - sin(-d)) / r) - (cos(r - d) / 2);
                const double a = cos(i * M_PI / 256);
                wb[j][i] = uint8_t(IMAGE_MIDY + IMAGE_MIDY * a * a * v);
            }
        }
    }

    void run()
    {
        uint16_t n = 0;
        int16_t i = 0, j = 0;
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Ripple")) return;
        generateLand();
        generateWav();

        for (i = 0; i < 32; i++)
        {
            pal[i].r = i << 1;
            pal[i].g = 0;
            pal[i].b = 0;
        }

        for (i = 0; i < 32; i++)
        {
            pal[i + 32].r = 63;
            pal[i + 32].g = i << 1;
            pal[i + 32].b = 0;
        }

        shiftPalette(pal);
        setPalette(pal);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            n = (n + 1) % 64;
            for (j = 0; j < IMAGE_MIDX; j++)
            {
                uint16_t limit = 0;
                uint16_t c = 63;
                uint16_t lasth = 199;
                for (i = 31; i >= 0; i--)
                {
                    limit = (56 + i) + wb[n][lb[i][j]];
                    while (lasth > limit)
                    {
                        putPixel(j + IMAGE_MIDX, lasth, c);
                        putPixel(IMAGE_MIDX - j - 1, lasth, c);
                        lasth--;
                    }
                    c--;
                }
                for (i = 0; i < 32; i++)
                {
                    limit = (56 - i) + wb[n][lb[i][j]];
                    while (lasth > limit)
                    {
                        putPixel(j + IMAGE_MIDX, lasth, c);
                        putPixel(IMAGE_MIDX - j - 1, lasth, c);
                        lasth--;
                    }
                    c--;
                }
                for (i = lasth; i >= heighest[j]; i--)
                {
                    putPixel(j + IMAGE_MIDX, i, 0);
                    putPixel(IMAGE_MIDX - j - 1, i, 0);
                }
                heighest[j] = uint8_t(lasth);
            }
            render();
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace waterFall {

    void makeDrip()
    {
        for (int16_t x = 0; x < IMAGE_WIDTH; x++)
        {
            uint16_t r = random(255) << 8;
            const uint16_t s = (128 + random(128)) << 3;
            for (int16_t y = 0; y < IMAGE_WIDTH; y++)
            {
                if (!(r & 0xff00)) putPixel(x, y, 1);
                else putPixel(x, y, r >> 8);
                r += s;
            }
        }
    }

    void rotatePalette()
    {
        int16_t i = 0, j = 0;
        uint16_t ofs = 0;
        RGB pal[256] = { 0 };

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (i = 0; i < 256; i++)
            {
                j = (i + ofs) & 0xff;
                if (j)
                {
                    pal[j].r = i >> 3;
                    pal[j].g = i >> 3;
                    pal[j].b = (i >> 3) + 32;
                }
            }
            shiftPalette(pal);
            setPalette(pal);
            delay(FPS_90);
            ofs++;
        }
        cleanup();
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Water-Fall")) return;
        makeDrip();
        rotatePalette();
    }
}

namespace winterEffect {
    #define FLAKES  1000
    #define FASTEST 360

    typedef struct {
        int16_t x, y;
        int16_t w, h;
    } TFlakes;

    TFlakes     flakes[FLAKES] = { 0 };

    uint16_t    rad = 0;
    uint8_t     vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    uint8_t     vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    void perturb()
    {
#ifdef _USE_ASM
        __asm {
            mov     dx, rad
            xor     dx, 0xAA55
            shl     dx, 1
            adc     dx, 0x0118
            mov     rad, dx
        }
#else
        rad = (((rad ^ 0xAA55) << 1) % 0xFFFF) + 0x0118;
#endif
    }

    void run()
    {
        int16_t i = 0;
        uint16_t pos = 0;
        RGB pal[256] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Winter")) return;
        if (!loadPNG(vbuff[0], pal, "assets/winter.png")) return;
        setPalette(pal);
        memcpy(vmem, vbuff, IMAGE_SIZE);
        memset(flakes, 0, sizeof(flakes));

        for (i = 0; i < FLAKES; i++)
        {
            perturb();
            pos += rad;
            flakes[i].y = (((rad & 0xff) * FASTEST) & 0xff) + 5;
            flakes[i].x = (((rad & 0x0f) * flakes[i].y) & 0xff) + 1;
            flakes[i].w = pos % IMAGE_WIDTH;
            flakes[i].h = pos / IMAGE_HEIGHT;
        }

        while (!finished(SDL_SCANCODE_RETURN))
        {
            for (i = 0; i < FLAKES; i++)
            {
                perturb();
                vmem[flakes[i].h % IMAGE_HEIGHT][flakes[i].w % IMAGE_WIDTH] = vbuff[flakes[i].h % IMAGE_HEIGHT][flakes[i].w % IMAGE_WIDTH];
                if (flakes[i].x >= (rad & 0x0f)) flakes[i].w++;
                if (flakes[i].y >= (rad & 0xff)) flakes[i].h++;
                vmem[flakes[i].h % IMAGE_HEIGHT][flakes[i].w % IMAGE_WIDTH] = (flakes[i].y >> 5) + 240;
            }

            renderBuffer(vmem, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace wormEffect {
    void run()
    {
        RGB pal[256] = { 0 };
        RGB tmp[16] = { 0 };

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Worm")) return;

        for (int16_t i = 1; i < 16; i++)
        {
            for (int16_t j = 0; j < 16; j++)
            {
                pal[(i << 4) + j].r = i << 2;
                pal[(i << 4) + j].g = j << 1;
                pal[(i << 4) + j].b = 63;
            }
        }

        shiftPalette(pal);
        setPalette(pal);
        if (!loadPNG((uint8_t*)getDrawBuffer(), NULL, "assets/worm.png")) return;

        while (!finished(SDL_SCANCODE_RETURN))
        {
            memcpy(tmp, &pal[16], sizeof(tmp));
            for (int16_t i = 1; i < 16; i++) memcpy(&pal[(i - 1) << 4], &pal[i << 4], sizeof(tmp));
            memcpy(&pal[240], tmp, sizeof(tmp));
            setPalette(pal);
            delay(FPS_90);
        }
        cleanup();
    }
}

namespace rayCastingEffect {
    #define CLR     111

    double  sinx = 0, cosx = 0;
    double	head = 0, turn = 0, step = 0;
    double  px = 0, py = 0, newpx = 0, newpy = 0;
    
    int32_t verts = 0, pass = 0, magic = 0, cosy = 0, siny = 0;
    int32_t hph = 0, hmh = 0, horiz = 0, px128 = 0, py128 = 0;

    RGB pal[SIZE_256] = { 0 };

    uint8_t showMaze = 1;
    uint8_t maze[32][32] = { 0 };
    uint8_t shade[16][SIZE_256] = { 0 };
    uint8_t walls[SIZE_128][SIZE_128] = { 0 };
    uint8_t floors[SIZE_128][SIZE_128] = { 0 };
    uint8_t ceils[SIZE_128][SIZE_128] = { 0 };
    uint8_t vbuff[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void drawWallFloor(int32_t vofs, int32_t dark)
    {
#ifdef _USE_ASM
        __asm {
            shl     dark, 8
            mov     ebx, verts
            xor     edi, edi
            mov     edi, vofs
            mov     eax, hmh
            mov     ecx, hph
            sub     ecx, eax
            jc      done
            shl     eax, 6
            add     edi, eax
            shl     eax, 2
            add     edi, eax
            mov     esi, horiz
            and     esi, 7Fh
            mov     edx, esi
            xor     eax, eax
        cycl0:
            push    ebx
            shr     ebx, 16
            shl     ebx, 7
            mov     esi, edx
            add     esi, ebx
            mov     al, walls[esi]
            mov     esi, eax
            add     esi, dark
            mov     al, shade[esi]
            mov     vbuff[edi], al
            add     edi, IMAGE_WIDTH
            pop     ebx
            add     ebx, pass
            dec     ecx
            jnz     cycl0
            mov     eax, hph
            cmp     eax, IMAGE_HEIGHT
            jae     done
            mov     edi, vofs
            push    di
            shl     eax, 6
            add     edi, eax
            shl     eax, 2
            add     edi, eax
            rol     edi, 16
            pop     di
            mov     eax, hmh
            shl     eax, 6
            add     edi, eax
            shl     eax, 2
            add     edi, eax
            sub     edi, IMAGE_WIDTH
            ror     edi, 16
            mov     ebx, hph
            sub     ebx, IMAGE_MIDY
        cycl1:
            xor     edx, edx
            mov     eax, magic
            div     ebx
            mov     ecx, eax
            mov     eax, siny
            mul     ecx
            shr     eax, 20
            add     eax, px128
            and     eax, 7Fh
            mov     esi, eax
            mov     eax, cosy
            mul     ecx
            shr     eax, 20
            add     eax, py128
            and     eax, 7Fh
            shl     eax, 7
            add     esi, eax
            shr     ecx, 10
            test    ecx, ecx
            jnz     nozero
        nozero:
            cmp     ecx, 15
            jbe     no15
            mov     ecx, 15
        no15:
            shl     ecx, 8
            xor     eax, eax
            mov     al, floors[esi]
            add     eax, ecx
            mov     al, shade[eax]
            mov     dx, di
            mov     vbuff[edx], al
            add     edi, IMAGE_WIDTH
            rol     edi, 16
            xor     eax, eax
            mov     al, ceils[esi]
            add     eax, ecx
            mov     al, shade[eax]
            mov     dx, di
            mov     vbuff[edx], al
            sub     edi, IMAGE_WIDTH
            ror     edi, 16
            inc     ebx
            cmp     ebx, IMAGE_MIDY
            jnz     cycl1
        done:
        }
#else
        int32_t bx, cx;
        uint16_t ax, py;

        int32_t cy = verts;
        uint16_t px = horiz & 0x7f;

        for (cx = hmh; cx < hph; cx++)
        {
            py = (cy >> 16) & 0x7f;
            bx = walls[py][px];
            vbuff[cx][vofs] = shade[dark][bx];
            cy += pass;
        }

        for (bx = hph; bx < IMAGE_HEIGHT; bx++)
        {
            cx = magic / (bx - IMAGE_MIDY);
            py = (((cx * siny) >> 20) + px128) & 0x7f;
            px = (((cx * cosy) >> 20) + py128) & 0x7f;
            cx >>= 10;
            if (cx > 15) cx = 15;
            ax = floors[py][px];
            vbuff[bx][vofs] = shade[cx][ax];
            ax = ceils[py][px];
            vbuff[MAX_HEIGHT - bx][vofs] = shade[cx][ax];
        }
#endif
    }

    int16_t loadMaze(const char* fname)
    {
        FILE* fp = fopen(fname, "rb");
        if (!fp) return 0;

        for (int16_t y = 0; y < 32; y++) fread(maze[y], 1, 32, fp);
        fclose(fp);
       
        px = py = 1.5;

        return 1;
    }

    void drawMaze()
    {
        for (int16_t i = 0; i < 32; i++)
        {
            for (int16_t j = 0; j < 32; j++)
            {
                if (maze[i][j])
                {
                    vbuff[i * 3 + 0][j * 3 + 0] = CLR;
                    vbuff[i * 3 + 0][j * 3 + 1] = CLR;
                    vbuff[i * 3 + 0][j * 3 + 2] = CLR;
                    vbuff[i * 3 + 1][j * 3 + 0] = CLR;
                    vbuff[i * 3 + 1][j * 3 + 1] = CLR;
                    vbuff[i * 3 + 1][j * 3 + 2] = CLR;
                    vbuff[i * 3 + 2][j * 3 + 0] = CLR;
                    vbuff[i * 3 + 2][j * 3 + 1] = CLR;
                    vbuff[i * 3 + 2][j * 3 + 2] = CLR;
                }
            }
        }
    }

    void setShade()
    {
        for (int16_t i = 0; i < 256; i++) shade[0][i] = uint8_t(i);

        for (int16_t k = 1; k < 16; k++)
        {
            for (int16_t i = 0; i < 256; i++)
            {
                const int16_t r = (pal[i].r >> 2) * (16 - k);
                const int16_t g = (pal[i].g >> 2) * (16 - k);
                const int16_t b = (pal[i].b >> 2) * (16 - k);
                int16_t diff1 = 1000;

                for (int16_t j = 0; j < 256; j++)
                {
                    const int16_t diff2 = abs((pal[j].r << 2) - r) + abs((pal[j].g << 2) - g) + abs((pal[j].b << 2) - b);
                    if (diff1 > diff2)
                    {
                        diff1 = diff2;
                        shade[k][i] = uint8_t(j);
                    }
                }
            }
        }
    }

    uint8_t range(int16_t x, int16_t y)
    {
        return (x < 0 || y < 0 || x > 27 || y > 27 || maze[y][x] > 0);
    }

    void computeView()
    {
        double x1 = 0, x2 = 0;
        double y1 = 0, y2 = 0;
        double slope = 0;
        double angle = head + 32;

        px128 = int32_t(px * 128);
        py128 = int32_t(py * 128);

        for (uint16_t vofs = 0; vofs < IMAGE_WIDTH; vofs++)
        {
            angle -= 0.2;
            magic = int32_t(128.0 * 1024.0 / cos((angle - head) * RAD));
            siny = int32_t(sin(angle * RAD) * 128 * 1024);
            cosy = int32_t(cos(angle * RAD) * 128 * 1024);

            if (angle != 180 && angle != 0)
            {
                if (siny > 0)
                {
                    slope = tan((90 - angle) * RAD);
                    x1 = floor(px) + 1.0;
                    y1 = py + (x1 - px) * slope;
                    while (!range(int16_t(x1), int16_t(y1)))
                    {
                        x1++;
                        y1 += slope;
                    }
                }
                else
                {
                    slope = tan((angle - 270) * RAD);
                    x1 = floor(px) - 0.000000001;
                    y1 = py + (px - x1) * slope;
                    while (!range(int16_t(x1), int16_t(y1)))
                    {
                        x1--;
                        y1 += slope;
                    }
                }
            }

            if (angle != 270 && angle != 90)
            {
                if (cosy > 0)
                {
                    slope = tan(angle * RAD);
                    y2 = floor(py) + 1.0;
                    x2 = px + (y2 - py) * slope;
                    while (!range(int16_t(x2), int16_t(y2)))
                    {
                        y2++;
                        x2 += slope;
                    }
                }
                else
                {
                    slope = tan((angle - 180) * RAD);
                    y2 = floor(py) - 0.000000001;
                    x2 = px - (py - y2) * slope;
                    while (!range(int16_t(x2), int16_t(y2)))
                    {
                        y2--;
                        x2 -= slope;
                    }
                }
            }

            double spacer = 0;
            const double dist1 = (px - x1) * (px - x1) + (py - y1) * (py - y1);
            const double dist2 = (px - x2) * (px - x2) + (py - y2) * (py - y2);

            if (dist1 > dist2)
            {
                spacer = sqrt(dist2) * 1024;
                horiz = int32_t(fabs(128 - 128 * x2));
            }
            else
            {
                spacer = sqrt(dist1) * 1024;
                horiz = int32_t(fabs(128 - 128 * y1));
            }

            const int32_t height = int32_t(magic / spacer);
            pass = (64 << 16) / height;
            hmh = IMAGE_MIDY - height;
            hph = IMAGE_MIDY + height;

            uint16_t darker = int32_t(spacer / 1024);
            if (darker > 15) darker = 15;

            if (hmh < 0)
            {
                verts = -(hmh * pass);
                hmh = 0;
                hph = IMAGE_HEIGHT;
            }
            else verts = 0;

            drawWallFloor(vofs, darker);
        }
    }

    void checkEvalKeys()
    {
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

        if (keyPressed(SDL_SCANCODE_TAB)) showMaze = !showMaze;

        if (keyDown(SDL_SCANCODE_LEFT))
        {
            if (turn < 1.0) turn = 1.0;
            else if (turn < 8.0) turn *= 2.0;
        }
        else if (keyDown(SDL_SCANCODE_RIGHT))
        {
            if (turn > -1.0) turn = -1.0;
            else if (turn > -8.0) turn *= 2.0;
        }
        else turn *= 0.6;

        if (keyDown(SDL_SCANCODE_DOWN))
        {
            if (step > -0.1) step = -0.1;
            else if (step > -0.25) step *= 1.1;
        }
        else if (keyDown(SDL_SCANCODE_UP))
        {
            if (step < 0.1) step = 0.1;
            else if (step < 0.25) step *= 1.1;
        }
        else step *= 0.8;
    }

    void run()
    {
        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Ray-Casting -- Keys: Arrows: move; TAB: show/hide maze")) return;
        if (!loadMaze("assets/maze.dat")) return;
        if (!loadPNG(walls[0], pal, "assets/wallr.png")) return;
        if (!loadPNG(floors[0], pal, "assets/floor.png")) return;
        if (!loadPNG(ceils[0], pal, "assets/ceil.png")) return;

        setPalette(pal);
        setShade();

        do {
            readKeys();
            checkEvalKeys();
            head += turn;

            if (head > 360.0) head = 0.0;
            else if (head < 0.0) head = 360.0;

            if (step > 0.0)
            {
                sinx = sin(head * RAD);
                cosx = cos(head * RAD);
                newpx = px + sinx * (step + 0.5);
                newpy = py + cosx * (step + 0.5);
            }
            else if (step < 0.0)
            {
                sinx = sin(head * RAD);
                cosx = cos(head * RAD);
                newpx = px + sinx * (step - 0.5);
                newpy = py + cosx * (step - 0.5);
            }

            if (!maze[int16_t(py + 0.1)][int16_t(newpx)] && !maze[int16_t(py - 0.1)][int16_t(newpx)]) px += sinx * step;
            if (!maze[int16_t(newpy)][int16_t(px + 0.1)] && !maze[int16_t(newpy)][int16_t(px - 0.1)]) py += cosx * step;

            computeView();
            if (showMaze) drawMaze();

            renderBuffer(vbuff, SCREEN_MIDX, SCREEN_MIDY);
            delay(FPS_60);
        } while (!keyDown(SDL_SCANCODE_RETURN));
        cleanup();
    }
}

namespace chain4Effect {

    RGB pal[256] = { 0 };
    uint8_t vbuff[400][640] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };
    
    void initChain4()
    {
        if (!loadPNG(vbuff[0], pal, "assets/chen.png")) return;
        setPalette(pal);
        for (int16_t y = 0; y < IMAGE_HEIGHT; y++) memcpy(vmem[y], vbuff[y], IMAGE_WIDTH);
    }

    void moveTo(uint16_t x, uint16_t y)
    {
        if (x > IMAGE_WIDTH || y > IMAGE_HEIGHT) return;
        for (int16_t i = 0; i < IMAGE_HEIGHT; i++) memcpy(vmem[i], &vbuff[i + y][x], IMAGE_WIDTH);
    }

    void run()
    {
        int32_t x = 0, y = 0, lmb = 0;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Chain4 Simulation -- Move your mouse!")) return;
        initChain4();
        setMousePosition(0, 0);
        showMouseCursor(SDL_DISABLE);

        do {
            getMouseState(&x, &y, &lmb);
            moveTo(x >> 1, y >> 1);
            renderBuffer(vmem[0], IMAGE_WIDTH, IMAGE_HEIGHT);
            delay(FPS_90);
        } while (!finished(SDL_SCANCODE_RETURN) && !lmb);
        showMouseCursor(SDL_ENABLE);
        cleanup();
    }
}

namespace hardwareScroll {
    RGB pal[256] = { 0 };
    uint8_t vsave[IMAGE_WIDTH] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void scroll(int16_t dir)
    {
        if (dir > 0)
        {
            for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
            {
                uint8_t swp = vmem[y][0];
                memcpy(&vmem[y][0], &vmem[y][1], IMAGE_WIDTH);
                vmem[y][MAX_WIDTH] = swp;
            }
        }
        else
        {
            for (int16_t y = 0; y < IMAGE_HEIGHT; y++)
            {
                uint8_t swp = vmem[y][MAX_WIDTH];
                for (int16_t i = MAX_WIDTH; i > 0; i--) vmem[y][i] = vmem[y][i - 1];
                vmem[y][0] = swp;
            }
        }
    }

    void wobble(int16_t dir)
    {
        if (dir)
        {
            memcpy(vsave, vmem[0], IMAGE_WIDTH);
            for (int16_t y = 1; y < MAX_HEIGHT; y++) memcpy(vmem[y], vmem[y + 1], IMAGE_WIDTH);
            memset(vmem[MAX_HEIGHT], 0, IMAGE_WIDTH);
        }
        else
        {
            memcpy(vsave, vmem[MAX_HEIGHT], IMAGE_WIDTH);
            for (int16_t y = MAX_HEIGHT; y > 0; y--) memcpy(vmem[y], vmem[y - 1], IMAGE_WIDTH);
            memset(vmem[0], 0, IMAGE_WIDTH);
        }

        renderBuffer(vmem[0], IMAGE_WIDTH, IMAGE_HEIGHT);
        delay(FPS_90);
    }

    void quit()
    {
        int16_t i, j;
        for (j = 0; j < 10; j++)
        {
            for (i = 0; i < 5; i++) wobble(0);
            for (i = 0; i < 5; i++) wobble(1);
        }
    }

    void run()
    {
        int16_t x = 0, addx = 1;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Hardware Scrolling")) return;
        if (!loadPNG(vmem[0], pal, "assets/fear.png")) return;
        setPalette(pal);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            scroll(addx);
            renderBuffer(vmem[0], IMAGE_WIDTH, IMAGE_HEIGHT);
            delay(3);
            x += addx;
            if (x == 0 || x == IMAGE_WIDTH) addx = -addx;
        }

        quit();
        cleanup();
    }
}

namespace copper3Effect {
    RGB pal[256] = { 0 };
    uint8_t vmem[IMAGE_HEIGHT][IMAGE_WIDTH] = { 0 };

    void scroll(int16_t dir)
    {
    }

    void run()
    {
        int16_t x = 0, addx = 1;

        if (!initScreen(IMAGE_WIDTH, IMAGE_HEIGHT, 8, 1, "Copper3 Simulation")) return;
        setPalette(pal);

        while (!finished(SDL_SCANCODE_RETURN))
        {
            scroll(addx);
            renderBuffer(vmem[0], IMAGE_WIDTH, IMAGE_HEIGHT);
            delay(3);
            x += addx;
            if (x == 0 || x == IMAGE_WIDTH) addx = -addx;
        }
        cleanup();
    }
}

void gfxEffectsMix()
{
    mazeGeneration::run();
    starEffect::run();
    flagsEffect2::run();
    star2dEffect::run();
    flagsEffect::run();
    star3dEffect::run();
    plasmaEffect5::run();
    fadePalette::run();
    juliaSet::run();
    bumpMap::run();
    fireBump::run();
    circleEffect::run();
    crossFade::run();
    skyEffect::run();
    phongShader::run();
    plasmaTexture::run();
    fireDown::run();
    fireTexture::run();
    fireTexture2::run();
    fireTexture3::run();
    tunnelEffect::run();
    textureMappingEffect::run();
    bitmapRotate::run();
    intro16k::run();
    textScrolling::run();
    fastShowBMP::run();
    EMS::run();
    fillterEffect::run();
    fireworkEffect::run();
    candleEffect::run();
    fireEffect::run();
    fireEffect2::run();
    fireEffect3::run();
    fireEffect4::run();
    fireEffect5::run();
    fireEffect6::run();
    fireEffect7::run();
    fireEffect8::run();
    holeEffect1::run();
    holeEffect2::run();
    holeEffect3::run();
    kaleidoScope::run();
    kaleidoScope2::run(1);
    kaleidoScope2::run(0);
    fastCircleFill::run();
    lakeEffect::run();
    landScapeGeneration::run();
    landScapeEffect::run();
    lensEffect::run();
    zoomInEffect::run();
    zoomOutEffect::run();
    lineBobEffect::run();
    lineBlurEffect::run();
    pixelMorphing::run();
    pierraEffect::run();
    plasmaEffect1::run();
    plasmaEffect2::run();
    plasmaEffect3::run();
    plasmaEffect4::run();
    rippleEffect::run();
    rotateMap::run();
    rayCastingEffect::run();
    scaleMap::run();
    shadeBob::run();
    shadePattern::run(1);
    shadePattern::run(0);
    shadeBobSin::run();
    snowFall::run();
    softFire::run();
    spriteEffect::run();
    fontEffect1::run();
    fontEffect2::run();
    fontEffect3::run();
    thunderBoltEffect::run();
    scrollingEffect::run();
    voxelEffect::run();
    rippleEffect2::run();
    waterFall::run();
    winterEffect::run();
    wormEffect::run();
    waterEffect::run();
    rainEffect::run();
    chain4Effect::run();
    hardwareScroll::run();
    copper3Effect::run();
}
