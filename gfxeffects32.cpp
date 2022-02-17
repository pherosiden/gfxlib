#include <vector>
#include <algorithm>
#include "gfxlib.h"

#define SCR_WIDTH	640
#define SCR_HEIGHT	480 

void juliaSet()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Julia-Set")) return;

    int32_t i = 0;

    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels) return;
    
    pixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    const int32_t iterations = 255;
    const double cre = -0.7, cim = 0.27015;
    
    const double xscale = 3.0 / SCR_WIDTH;
    const double yscale = 2.0 / SCR_HEIGHT;

    const double scale = max(xscale, yscale);
    const double mx = -0.5 * SCR_WIDTH * scale;
    const double my = -0.5 * SCR_HEIGHT * scale;

    /*==================================== use FMA version below ==============================
    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        const double y0 = y * scale + my;
        for (int32_t x = 0; x < SCR_WIDTH; x++)
        {
            const double x0 = x * scale + mx;
            double x1 = x0;
            double y1 = y0;
            for (i = 0; i < iterations; i++)
            {
                const double x2 = x1 * x1;
                const double y2 = y1 * y1;
                if (x2 + y2 >= 4.0) break;
                y1 = 2 * x1 * y1 + cim;
                x1 = x2 - y2 + cre;
            }

            //use color model conversion to get rainbow palette
            pixels[y][x] = hsv2rgb(0xff * i / iterations, 0xff, (i < iterations) ? 0xff : 0);
        }
    }
    ====================AVX-512 version support INTEL 11th later===============================

    const __m512d xim = _mm512_set1_pd(cim);
    const __m512d xre = _mm512_set1_pd(cre);

    const __m512d dd = _mm512_set1_pd(scale);
    const __m512d tx = _mm512_set1_pd(mx);

    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        const __m512d y0 = _mm512_set1_pd(y * scale + my);
        for (int32_t x = 0; x < SCR_WIDTH; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m512d x0 = _mm512_fmadd_pd(dd, _mm512_cvtepi32_pd(ind), tx);
            __m512d x1 = x0;
            __m512d y1 = y0;
            __m512i iters = _mm512_setzero_si512();
            __m512i masks = _mm512_setzero_si512();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m512d x2 = _mm512_mul_pd(x1, x1);
                const __m512d y2 = _mm512_mul_pd(y1, y1);
                const __m512d abs = _mm512_add_pd(x2, y2);
                const __m512i cmp = _mm512_movm_epi64(_mm512_cmp_pd_mask(abs, _mm512_set1_pd(4), _CMP_GE_OS));

                masks = _mm512_or_epi32(cmp, masks);
                if (_mm512_test_epi64_mask(masks, masks) == 1) break;

                iters = _mm512_add_epi32(iters, _mm512_andnot_epi32(masks, _mm512_set1_epi32(1)));

                const __m512d t = _mm512_add_pd(x1, x1);
                y1 = _mm512_fmadd_pd(t, y1, xim);
                x1 = _mm512_add_pd(_mm512_sub_pd(x2, y2), xre);
            }

            //extract iteration position for each pixel
            alignas(64) int32_t ipos[16] = { 0 };
            _mm512_stream_si512(it, iters);

            //use HSV convert to get full rainbow palette
            uint32_t* pdst = &pixels[y][x];
            *pdst++ = hsv2rgb(255 * ipos[0]  / iterations, 255, (ipos[0]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[2]  / iterations, 255, (ipos[2]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[4]  / iterations, 255, (ipos[4]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[6]  / iterations, 255, (ipos[6]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[8]  / iterations, 255, (ipos[8]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[10] / iterations, 255, (ipos[10] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[12] / iterations, 255, (ipos[12] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[14] / iterations, 255, (ipos[14] < iterations) ? 255 : 0);
        }
    }
    ============================================================================================*/
    
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < SCR_WIDTH; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m256d x0 = _mm256_fmadd_pd(dd, _mm256_cvtepi32_pd(ind), tx);
            __m256d x1 = x0;
            __m256d y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x1, x1);
                const __m256d y2 = _mm256_mul_pd(y1, y1);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;

                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                const __m256d t = _mm256_add_pd(x1, x1);
                y1 = _mm256_fmadd_pd(t, y1, xim);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            //extract iteration position for each pixel
            alignas(32) int32_t it[8] = { 0 };
            _mm256_stream_si256((__m256i*)it, iters);

            //use HSV convert to get full rainbow palette
            uint32_t* pdst = &pixels[y][x];
            *pdst++ = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
        }
    }

    render();
    while (!finished(SDL_SCANCODE_RETURN));
    free(pixels);
    cleanup();
}

static const uint32_t firePalette[256] = {
    //Jare's original FirePal.
    #define C(r,g,b) ((((r) * 4) << 16) | ((g) * 4 << 8) | ((b) * 4))
    C(0,    0,   0), C(0,    1,   1), C(0,    4,   5), C(0,    7,   9),
    C(0,    8,  11), C(0,    9,  12), C(15,   6,   8), C(25,   4,   4),
    C(33,   3,   3), C(40,   2,   2), C(48,   2,   2), C(55,   1,   1),
    C(63,   0,   0), C(63,   0,   0), C(63,   3,   0), C(63,   7,   0),
    C(63,  10,   0), C(63,  13,   0), C(63,  16,   0), C(63,  20,   0),
    C(63,  23,   0), C(63,  26,   0), C(63,  29,   0), C(63,  33,   0),
    C(63,  36,   0), C(63,  39,   0), C(63,  39,   0), C(63,  40,   0),
    C(63,  40,   0), C(63,  41,   0), C(63,  42,   0), C(63,  42,   0),
    C(63,  43,   0), C(63,  44,   0), C(63,  44,   0), C(63,  45,   0),
    C(63,  45,   0), C(63,  46,   0), C(63,  47,   0), C(63,  47,   0),
    C(63,  48,   0), C(63,  49,   0), C(63,  49,   0), C(63,  50,   0),
    C(63,  51,   0), C(63,  51,   0), C(63,  52,   0), C(63,  53,   0),
    C(63,  53,   0), C(63,  54,   0), C(63,  55,   0), C(63,  55,   0),
    C(63,  56,   0), C(63,  57,   0), C(63,  57,   0), C(63,  58,   0),
    C(63,  58,   0), C(63,  59,   0), C(63,  60,   0), C(63,  60,   0),
    C(63,  61,   0), C(63,  62,   0), C(63,  62,   0), C(63,  63,   0),
    //Followed by "white heat".

    #define W C(63,63,63)
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W,
    W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W
    #undef W
    #undef C
};

static uint8_t dstBuff[SCR_WIDTH * SCR_HEIGHT] = { 0 };
static uint8_t prevBuff[SCR_WIDTH * SCR_HEIGHT] = { 0 };

void fireDemo1()
{
    uint8_t avg = 0;
    int32_t i = 0, sum = 0;
    
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Fire")) return;

    //get drawing buffer
    uint32_t* frameBuff = (uint32_t*)getDrawBuffer();

    while (!finished(SDL_SCANCODE_RETURN))
    {
        for (i = SCR_WIDTH + 1; i < (SCR_HEIGHT - 1) * SCR_WIDTH - 1; i++)
        {
            //Average the eight neighbours.
            sum =
                prevBuff[i - SCR_WIDTH - 1] +
                prevBuff[i - SCR_WIDTH    ] +
                prevBuff[i - SCR_WIDTH + 1] +
                prevBuff[i - 1] +
                prevBuff[i + 1] +
                prevBuff[i + SCR_WIDTH - 1] +
                prevBuff[i + SCR_WIDTH    ] +
                prevBuff[i + SCR_WIDTH + 1];

            avg = uint8_t(sum >> 3);

            //"Cool" the pixel if the two bottom bits of the
            // sum are clear (somewhat random). For the bottom
            // rows, cooling can overflow, causing "sparks".
            if (!(sum & 3) && (avg > 0 || i >= (SCR_HEIGHT - 4) * SCR_WIDTH)) avg--;
            dstBuff[i] = avg;
        }

        //Copy back and scroll up one row.
        //The bottom row is all zeros, so it can be skipped.
        for (i = 0; i < (SCR_HEIGHT - 2) * SCR_WIDTH; i++) prevBuff[i] = dstBuff[i + SCR_WIDTH];

        //Remove dark pixels from the bottom rows (except again the
        // bottom row which is all zeros).
        for (i = (SCR_HEIGHT - 7) * SCR_WIDTH; i < (SCR_HEIGHT - 1) * SCR_WIDTH; i++)
        {
            if (dstBuff[i] < 15) dstBuff[i] = 22 - dstBuff[i];
        }

        //Copy to frame buffer and map to RGBA, scrolling up one row.
        for (i = 0; i < (SCR_HEIGHT - 2) * SCR_WIDTH; i++)
        {
            frameBuff[i] = firePalette[dstBuff[i + SCR_WIDTH]];
        }

        //Update the texture and render it.
        render();
        delay(FPS_90);
    }

    cleanup();
}

static uint32_t palette[SIZE_256] = { 0 };
static uint32_t fires[SCR_HEIGHT][SCR_WIDTH] = { 0 };

void fireDemo2()
{
    //set up the screen
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Fire")) return;

    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels) return;

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (int32_t i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    //make sure the fire buffer is zero in the beginning
    memset(fires, 0, sizeof(fires));

    //generate the palette
    for (int32_t x = 0; x < 256; x++)
    {
        //HSL2RGB is used to generate colors:
        //hue goes from 0 to 85: red to yellow
        //saturation is always the maximum: 255
        //lightness is 0..255 for x=0..128, and 255 for x=128..255
        //set the palette to the calculated RGB value
        palette[x] = hsl2rgb(x / 3, 255, min(255, x << 1));
    }

    //start the loop (one frame per loop)
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //randomize the bottom row of the fire buffer
        for (int32_t x = 0; x < SCR_WIDTH; x++) fires[SCR_HEIGHT - 1][x] = abs(32768 + rand()) & 0xff;

        //do the fire calculations for every pixel, from top to bottom
        for (int32_t y = 0; y < SCR_HEIGHT - 1; y++)
        {
            for (int32_t x = 0; x < SCR_WIDTH; x++)
            {
                fires[y][x] = ((
                    fires[(y + 1) % SCR_HEIGHT][(x - 1 + SCR_WIDTH) % SCR_WIDTH] +
                    fires[(y + 1) % SCR_HEIGHT][(x                ) % SCR_WIDTH] +
                    fires[(y + 1) % SCR_HEIGHT][(x + 1            ) % SCR_WIDTH] +
                    fires[(y + 2) % SCR_HEIGHT][(x                ) % SCR_WIDTH]) * 32 / 129) & 0xff;
            }
        }

        //set the drawing buffer to the fire buffer, using the palette colors
        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            for (int32_t x = 0; x < SCR_WIDTH; x++) pixels[y][x] = palette[fires[y][x]];
        }

        //draw the buffer
        render();
        delay(FPS_90);
    }

    free(pixels);
    cleanup();
}

/*=============================RAY CASTING===================================*/

#define TEXTURE_WIDTH		64
#define TEXTURE_HEIGHT		64
#define TEXTURE_COUNT		11
#define MAP_WIDTH			24
#define MAP_HEIGHT			24
#define NUM_SPRITES			19

static int32_t miniMap[MAP_WIDTH][MAP_HEIGHT] =
{
    {8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
    {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
    {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
    {8,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
    {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
    {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6},
    {8,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6},
    {7,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6},
    {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6},
    {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4},
    {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,6,0,6,0,6},
    {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,4,6,0,6,6,6},
    {7,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,3,3,0,3,3,3},
    {2,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3},
    {2,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
    {2,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
    {1,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3},
    {2,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5},
    {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
    {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
    {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
    {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
    {2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
};

typedef struct
{
    double  x;
    double  y;
    int32_t data;
} SPRITE;

static SPRITE sprite[NUM_SPRITES] =
{
    //green light in front of player start
    {20.5, 11.5, 10},

    //green lights in every room
    {18.5,4.5, 10},
    {10.0,4.5, 10},
    {10.0,12.5,10},
    {3.5, 6.5, 10},
    {3.5, 20.5,10},
    {3.5, 14.5,10},
    {14.5,20.5,10},

    //row of pillars in front of wall: fish eye test
    {18.5, 10.5, 9},
    {18.5, 11.5, 9},
    {18.5, 12.5, 9},

    //some barrels around the map
    {21.5, 1.5, 8},
    {15.5, 1.5, 8},
    {16.0, 1.8, 8},
    {16.2, 1.2, 8},
    {3.5,  2.5, 8},
    {9.5, 15.5, 8},
    {10.0, 15.1,8},
    {10.5, 15.8,8},
};

//1D buffer
static double zBuffer[SCR_WIDTH] = { 0 };

//arrays used to sort the sprites
static int32_t spriteOrder[NUM_SPRITES] = { 0 };
static double spriteDistance[NUM_SPRITES] = { 0 };

//function used to sort the sprites
//sort the sprites based on distance
void sortSprites(int32_t* order, double* dist, int32_t amount)
{
    std::vector<std::pair<double, int32_t>> sprites(amount);
    for (int32_t i = 0; i < amount; i++)
    {
        sprites[i].first = dist[i];
        sprites[i].second = order[i];
    }

    std::sort(sprites.begin(), sprites.end());

    //restore in reverse order to go from farthest to nearest
    for (int32_t i = 0; i < amount; i++)
    {
        dist[i] = sprites[intptr_t(amount) - i - 1].first;
        order[i] = sprites[intptr_t(amount) - i - 1].second;
    }
}

void rayCasting()
{
    double posX = 22.0, posY = 11.5; //x and y start position
    double dirX = -1.0, dirY = 0.0; //initial direction vector
    double planeX = 0.0, planeY = 0.66; //the 2d ray caster version of camera plane
    
    uint32_t time = 0, oldTime = 0;

    int32_t tw = 0, th = 0, i = 0;
    uint32_t* pbuffs[11] = { 0 };

    const char* fname[TEXTURE_COUNT] = {
        "assets/eagle.png",
        "assets/redbrick.png",
        "assets/purplestone.png",
        "assets/greystone.png",
        "assets/bluestone.png",
        "assets/mossy.png",
        "assets/wood.png",
        "assets/colorstone.png",
        "assets/barrel.png",
        "assets/pillar.png",
        "assets/greenlight.png"
    };

    //init screen mode
    if (!loadFont("assets/sysfont.xfn", 0)) return;
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Ray-casting [Fast version] -- Keys: Use arrows to move your works!")) return;

    //matrix textures
    uint32_t** textures[TEXTURE_COUNT] = { 0 };

    //load some textures
    for (i = 0; i < TEXTURE_COUNT; i++)
    {
        if (!loadTexture(&pbuffs[i], &tw, &th, fname[i])) return;
        textures[i] = (uint32_t**)calloc(th, sizeof(uint32_t*));
        if (!textures[i]) return;

        textures[i][0] = pbuffs[i];
        for (int32_t j = 1; j < th; j++) textures[i][j] = &textures[i][0][j * tw];
    }

    uint32_t** renderBuff = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!renderBuff) return;

    renderBuff[0] = (uint32_t*)getDrawBuffer();
    for (i = 0; i < SCR_HEIGHT; i++) renderBuff[i] = &renderBuff[0][i * SCR_WIDTH];

    const int32_t mheight = SCR_HEIGHT >> 1;

    //start the main loop
    do {
        //FLOOR CASTING
        for (int32_t y = mheight + 1; y < SCR_HEIGHT; y++)
        {
            //rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
            const double rayDirX0 = dirX - planeX;
            const double rayDirY0 = dirY - planeY;
            const double rayDirX1 = dirX + planeX;
            const double rayDirY1 = dirY + planeY;

            //current y position compared to the center of the screen (the horizon)
            const int32_t p = y - mheight;

            //vertical position of the camera.
            const double posZ = 0.5 * SCR_HEIGHT;

            //horizontal distance from the camera to the floor for the current row.
            //0.5 is the z position exactly in the middle between floor and ceiling.
            const double rowDistance = posZ / p;

            //calculate the real world step vector we have to add for each x (parallel to camera plane)
            //adding step by step avoids multiplications with a weight in the inner loop
            const double floorStepX = rowDistance * (rayDirX1 - rayDirX0) / SCR_WIDTH;
            const double floorStepY = rowDistance * (rayDirY1 - rayDirY0) / SCR_WIDTH;

            //real world coordinates of the leftmost column. This will be updated as we step to the right.
            double floorX = posX + rowDistance * rayDirX0;
            double floorY = posY + rowDistance * rayDirY0;

            for (int32_t x = 0; x < SCR_WIDTH; x++)
            {
                //the cell coordinator is simply got from the integer parts of floorX and floorY
                const int32_t cellX = int32_t(floorX);
                const int32_t cellY = int32_t(floorY);

                //get the texture coordinate from the fractional part
                const int32_t tx = int32_t(TEXTURE_WIDTH * (floorX - cellX)) & (TEXTURE_WIDTH - 1);
                const int32_t ty = int32_t(TEXTURE_HEIGHT * (floorY - cellY)) & (TEXTURE_HEIGHT - 1);

                floorX += floorStepX;
                floorY += floorStepY;

                //choose texture and draw the pixel
                uint32_t color = 0;
                int32_t floorTexture = 0;
                const int32_t ceilingTexture = 6;
                
                const int32_t checkerBoardPattern = int32_t(cellX + cellY) & 1;
                if (checkerBoardPattern == 0) floorTexture = 3;
                else floorTexture = 4;

                //floor
                color = textures[floorTexture][ty][tx];
                renderBuff[y][x] = (color >> 1) & 8355711; //make a bit darker

                //ceiling (symmetrical, at screenHeight - y - 1 instead of y)
                color = textures[ceilingTexture][ty][tx];
                renderBuff[SCR_HEIGHT - y - 1][x] = (color >> 1) & 8355711; //make a bit darker
            }
        }

        //WALL CASTING
        for (int32_t x = 0; x < SCR_WIDTH; x++)
        {
            //calculate ray position and direction
            const double cameraX = 2.0 * x / intmax_t(SCR_WIDTH) - 1; //x-coordinate in camera space
            const double rayDirX = dirX + planeX * cameraX;
            const double rayDirY = dirY + planeY * cameraX;

            //which box of the map we're in
            int32_t mapX = int32_t(posX);
            int32_t mapY = int32_t(posY);

            //length of ray from current position to next x or y-side
            double sideDistX = 0.0;
            double sideDistY = 0.0;

            //length of ray from one x or y-side to next x or y-side
            double perpWallDist = 0.0;
            const double deltaDistX = abs(1 / rayDirX);
            const double deltaDistY = abs(1 / rayDirY);

            //what direction to step in x or y-direction (either +1 or -1)
            int32_t stepX = 0;
            int32_t stepY = 0;

            int32_t hit = 0; //was there a wall hit?
            int32_t side = 0; //was a NS or a EW wall hit?

            //calculate step and initial sideDist
            if (rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            if (rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }

            //perform DDA
            while (hit == 0)
            {
                //jump to next map square, OR in x-direction, OR in y-direction
                if (sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }

                //check if ray has hit a wall
                if (miniMap[mapX][mapY] > 0) hit = 1;
            }

            //calculate distance of perpendicular ray (Euclidean distance will give fisheye effect!)
            if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
            else perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;

            //calculate height of line to draw on screen
            const int32_t lineHeight = int32_t(SCR_HEIGHT / perpWallDist);
            const int32_t mlineHeight = lineHeight >> 1;

            //calculate lowest and highest pixel to fill in current stripe
            int32_t drawStart = -mlineHeight + mheight;
            if (drawStart < 0) drawStart = 0;
            int32_t drawEnd = mlineHeight + mheight;
            if (drawEnd > SCR_HEIGHT) drawEnd = SCR_HEIGHT;

            //texturing calculations, 1 subtracted from it so that texture 0 can be used!
            const int32_t texNum = (miniMap[mapX][mapY] - 1) % TEXTURE_COUNT;

            //calculate value of wallX
            double wallX = 0;
            if (side == 0) wallX = posY + perpWallDist * rayDirY;
            else wallX = posX + perpWallDist * rayDirX;

            wallX -= floor((wallX));

            //x coordinate on the texture
            int32_t texX = int32_t(wallX * TEXTURE_WIDTH);
            if (side == 0 && rayDirX > 0) texX = TEXTURE_WIDTH - texX - 1;
            if (side == 1 && rayDirY < 0) texX = TEXTURE_WIDTH - texX - 1;

            //how much to increase the texture coordinate per screen pixel
            const double step = double(TEXTURE_HEIGHT) / lineHeight;

            //starting texture coordinate
            double texPos = (intmax_t(drawStart) - mheight + mlineHeight) * step;

            for (int32_t y = drawStart; y < drawEnd; y++)
            {
                //cast the texture coordinate to integer, and mask with (SCR_HEIGHT - 1) in case of overflow
                const int32_t texY = int32_t(texPos) & (TEXTURE_HEIGHT - 1);
                texPos += step;

                //lookup texture color
                uint32_t color = textures[texNum][texY][texX];

                //make color darker for y-sides: R, G and B byte each divided through two with a 'shift' and an 'and'
                if (side == 1) color = (color >> 1) & 8355711;
                renderBuff[y][x] = color;
            }

            //SET THE ZBUFFER FOR THE SPRITE CASTING
            zBuffer[x] = perpWallDist; //perpendicular distance is used
        }

        //SPRITE CASTING
        //sort sprites from far to close
        for (int32_t i = 0; i < NUM_SPRITES; i++)
        {
            spriteOrder[i] = i;
            spriteDistance[i] = (sqr(posX - sprite[i].x) + sqr(posY - sprite[i].y)); //sqrt not taken, unneeded
        }

        sortSprites(spriteOrder, spriteDistance, NUM_SPRITES);

        //after sorting the sprites, do the projection and draw them
        for (int32_t i = 0; i < NUM_SPRITES; i++)
        {
            //translate sprite position to relative to camera
            const double spriteX = sprite[spriteOrder[i]].x - posX;
            const double spriteY = sprite[spriteOrder[i]].y - posY;

            //transform sprite with the inverse camera matrix
            //[ planeX   dirX ] -1                                       [ dirY      -dirX ]
            //[               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
            //[ planeY   dirY ]                                          [ -planeY  planeX ]

            const double invDet = 1.0 / (planeX * dirY - dirX * planeY);
            const double transformX = invDet * (dirY * spriteX - dirX * spriteY);
            const double transformY = invDet * (-planeY * spriteX + planeX * spriteY);

            const int32_t spriteScreenX = int32_t((SCR_WIDTH >> 1) * (1 + transformX / transformY));

            //calculate height of the sprite on screen
            const int32_t spriteHeight = abs(int32_t(SCR_HEIGHT / transformY));
            const int32_t mspriteHeight = spriteHeight >> 1;

            //calculate lowest and highest pixel to fill in current stripe
            int32_t drawStartY = -mspriteHeight + mheight;
            if (drawStartY < 0) drawStartY = 0;
            int32_t drawEndY = mspriteHeight + mheight;
            if (drawEndY > SCR_HEIGHT) drawEndY = SCR_HEIGHT;

            //calculate width of the sprite
            const int32_t spriteWidth = abs(int32_t(SCR_HEIGHT / transformY));
            const int32_t mspriteWidth = spriteWidth >> 1;

            int32_t drawStartX = -mspriteWidth + spriteScreenX;
            if (drawStartX < 0) drawStartX = 0;
            int32_t drawEndX = mspriteWidth + spriteScreenX;
            if (drawEndX > SCR_WIDTH) drawEndX = SCR_WIDTH;

            //loop through every vertical stripe of the sprite on screen
            for (int32_t stripe = drawStartX; stripe < drawEndX; stripe++)
            {
                const int32_t texX = (((stripe - (-mspriteWidth + spriteScreenX)) << 8) * TEXTURE_WIDTH / spriteWidth) >> 8;
                //the conditions in the if are:
                //1) it's in front of camera plane so you don't see things behind you
                //2) it's on the screen (left)
                //3) it's on the screen (right)
                //4) ZBuffer, with perpendicular distance
                if (transformY > 0 && stripe > 0 && stripe < SCR_WIDTH && transformY < zBuffer[stripe])
                {
                    //for every pixel of the current stripe
                    for (int32_t y = drawStartY; y < drawEndY; y++)
                    {
                        const int32_t d = (y << 8) - (SCR_HEIGHT << 7) + (spriteHeight << 7);
                        const int32_t texY = ((d * TEXTURE_HEIGHT) / spriteHeight) >> 8;
                        const uint32_t color = textures[sprite[spriteOrder[i]].data][texY][texX];
                        if (color & 0x00ffffff) renderBuff[y][stripe] = color;
                    }
                }
            }
        }

        //timing for input and FPS counter
        oldTime = time;
        time = getTime();
        const double frameTime = (time - oldTime) / 1000.0;
        writeText(1, 1, RGB_WHITE, 0, "FPS:%.f", 1.0 / frameTime);
        render();

        //clear current render buffer
        memset(renderBuff[0], 0, sizeof(uint32_t)* SCR_WIDTH* SCR_HEIGHT);

        //fetch user input
        readKeys();
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

        //speed modifiers
        const double moveSpeed = frameTime * 3.0; //the constant value is in squares/second
        const double rotSpeed = frameTime * 2.0; //the constant value is in radians/second
        
        //move forward if no wall in front of you
        if (keyDown(SDL_SCANCODE_UP))
        {
            if (miniMap[int32_t(posX + dirX * moveSpeed)][int32_t(posY)] == false) posX += dirX * moveSpeed;
            if (miniMap[int32_t(posX)][int32_t(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
        }
        //move backwards if no wall behind you
        if (keyDown(SDL_SCANCODE_DOWN))
        {
            if (miniMap[int32_t(posX - dirX * moveSpeed)][int32_t(posY)] == false) posX -= dirX * moveSpeed;
            if (miniMap[int32_t(posX)][int32_t(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
        }
        //rotate to the right
        if (keyDown(SDL_SCANCODE_RIGHT))
        {
            //both camera direction and camera plane must be rotated
            const double oldDirX = dirX;
            dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
            dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);

            const double oldPlaneX = planeX;
            planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
            planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        }
        //rotate to the left
        if (keyDown(SDL_SCANCODE_LEFT))
        {
            //both camera direction and camera plane must be rotated
            const double oldDirX = dirX;
            dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
            dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);

            const double oldPlaneX = planeX;
            planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
            planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        }

        //correct frames rate
        delay(FPS_60);
    } while (!keyDown(SDL_SCANCODE_RETURN));

    //cleanup...
    for (int32_t i = 0; i < 11; i++)
    {
        free(textures[i][0]);
        free(textures[i]);
    }

    free(renderBuff);
    freeFont(0);
    cleanup();
}

void basicDrawing()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "2D primitives")) return;

    //this is outsize screen
    int32_t x1 = -50, y1 = -20, x2 = 1000, y2 = 1200;

    //the new line represents the part of the old line that is visible on screen
    clipLine(&x1, &y1, &x2, &y2);

    //the newline is drawn as a red line with smooth pixel
    drawLine(x1, y1, x2, y2, RGB_RED, BLEND_MODE_ANTIALIASED);
    drawCircle(100, 100, 30, RGB_GREEN, BLEND_MODE_ANTIALIASED);
    fillCircle(200, 100, 40, RGB_YELLOW);
    drawEllipse(200, 200, 50, 100, RGB_BLUE, BLEND_MODE_ANTIALIASED);
    fillEllipse(300, 300, 50, 100, RGB_MAGENTA);
    drawRect(150, 150, 200, 200, RGB_CYAN);
    drawRoundRect(400, 20, 200, 80, 50, RGB_CYAN);
    vertLine(320, 100, 300, RGB_WHITE);
    horizLine(50, 20, 200, RGB_PURPLE);
    drawBox(400, 350, 100, 100, 50, 50, RGB_GREEN);
    drawRoundBox(400, 150, 200, 100, 50, RGB_RED);
    drawRotatedEllipse(90, 380, 100, 50, 40, rgb(255, 255, 0), BLEND_MODE_ANTIALIASED);

    //make all visible on screen
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    cleanup();
}

void imageArithmetic()
{
    int32_t w = 0, h = 0;

    //declare image buffers
    uint32_t *image1 = NULL, *image2 = NULL;

    //load the images into the buffers. This assumes all have the same size.
    if (!loadTexture(&image1, &w, &h, "assets/photo1.png")) return;
    if (!loadTexture(&image2, &w, &h, "assets/photo2.png")) return;

    //set up the screen
    if (!initScreen(w, h, 32, 0, "Image Arithmetic")) return;

    const int32_t size = w * h;
    uint32_t* result = (uint32_t*)getDrawBuffer();
    
    uint32_t* itdst = result;
    uint32_t* itimg1 = image1;
    uint32_t* itimg2 = image2;

    for (int32_t i = 0; i < size; i++)
    {
        ARGB* pdst = (ARGB*)itdst++;
        const ARGB* pimg1 = (const ARGB*)itimg1++;
        const ARGB* pimg2 = (const ARGB*)itimg2++;

        //average
        pdst->r = (pimg2->r + pimg1->r) >> 1;
        pdst->g = (pimg2->g + pimg1->g) >> 1;
        pdst->b = (pimg2->b + pimg1->b) >> 1;

        //adding
        //pdst->r = min(pimg2->r + pimg1->r, 255);
        //pdst->g = min(pimg2->g + pimg1->g, 255);
        //pdst->b = min(pimg2->b + pimg1->b, 255);

        //subtract
        //pdst->r = min(pimg2->r - pimg1->r, 0);
        //pdst->g = min(pimg2->g - pimg1->g, 0);
        //pdst->b = min(pimg2->b - pimg1->b, 0);

        //multiply
        //pdst->r = uint8_t(255 * (pimg2->r / 255.0 * pimg1->r / 255.0));
        //pdst->g = uint8_t(255 * (pimg2->g / 255.0 * pimg1->g / 255.0));
        //pdst->b = uint8_t(255 * (pimg2->b / 255.0 * pimg1->b / 255.0));

        //difference
        //pdst->r = abs(pimg1->r - pimg2->r);
        //pdst->g = abs(pimg1->g - pimg2->g);
        //pdst->b = abs(pimg1->b - pimg2->b);

        //min
        //pdst->r = min(pimg1->r, pimg2->r);
        //pdst->g = min(pimg1->g, pimg2->g);
        //pdst->b = min(pimg1->b, pimg2->b);

        //max
        //pdst->r = max(pimg1->r, pimg2->r);
        //pdst->g = max(pimg1->g, pimg2->g);
        //pdst->b = max(pimg1->b, pimg2->b);

        //amplitude
        //pdst->r = uint8_t(sqrt(double(pimg1->r) * pimg1->r + double(pimg2->r) * pimg2->r) / sqrt(2.0));
        //pdst->g = uint8_t(sqrt(double(pimg1->g) * pimg1->g + double(pimg2->g) * pimg2->g) / sqrt(2.0));
        //pdst->b = uint8_t(sqrt(double(pimg1->b) * pimg1->b + double(pimg2->b) * pimg2->b) / sqrt(2.0));

        //and
        //pdst->r = pimg1->r & pimg2->r;
        //pdst->g = pimg1->g & pimg2->g;
        //pdst->b = pimg1->b & pimg2->b;

        //or
        //pdst->r = pimg1->r | pimg2->r;
        //pdst->g = pimg1->g | pimg2->g;
        //pdst->b = pimg1->b | pimg2->b;

        //xor
        //pdst->r = pimg1->r ^ pimg2->r;
        //pdst->g = pimg1->g ^ pimg2->g;
        //pdst->b = pimg1->b ^ pimg2->b;
    }

    //redraw & sleep
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    free(image1);
    free(image2);
    cleanup();
}

void crossFading()
{
    int32_t w = 0, h = 0;

    //declare image buffers
    uint32_t *image1 = NULL, *image2 = NULL;

    //load the images into the buffers. This assumes all have the same size.
    if (!loadTexture(&image1, &w, &h, "assets/photo1.png")) return;
    if (!loadTexture(&image2, &w, &h, "assets/photo2.png")) return;
    
    //set up the screen
    if (!initScreen(w, h, 32, 0, "Cross-Fading")) return;
    
    const int32_t size = w * h;
    uint32_t* result = (uint32_t*)getDrawBuffer();

    while (!finished(SDL_SCANCODE_RETURN))
    {
        ARGB* pdst = (ARGB*)result;
        const ARGB* pimg1 = (const ARGB*)image1;
        const ARGB* pimg2 = (const ARGB*)image2;

        const double weight = (1.0 + cos(getTime() / 1000.0)) / 2.0;

        //do the blending pixels
        for (int32_t i = 0; i < size; i++)
        {
            pdst->r = uint8_t(pimg1->r * weight + pimg2->r * (1 - weight));
            pdst->g = uint8_t(pimg1->g * weight + pimg2->g * (1 - weight));
            pdst->b = uint8_t(pimg1->b * weight + pimg2->b * (1 - weight));
            pdst++;
            pimg1++;
            pimg2++;
        }

        //render
        render();
    }

    free(image1);
    free(image2);
    cleanup();
}

void juliaExplorer()
{
    if (!loadFont("assets/sysfont.xfn", 0)) return;
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Julia-Explorer")) return;

    //windows title buffer (add FPS)
    char sbuff[200] = { 0 };

    //use to show/hide text
    int32_t showText = 0, i = 0;

    //current and old time, and their difference (for input)
    uint32_t time = 0, oldTime = 0, frameTime = 0;
        
    //make memory access pixels
    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels) return;

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    //user input key
    int32_t input = 0;

    //iterations
    int32_t iterations = 255;
    double cre = -0.7, cim = 0.27015;

    //scale unit
    const double xscale = 3.0 / SCR_WIDTH;
    const double yscale = 2.0 / SCR_HEIGHT;

    //calculate scale and current position
    double scale = max(xscale, yscale);
    double mx = -0.5 * SCR_WIDTH * scale;
    double my = -0.5 * SCR_HEIGHT * scale;

    do
    {
        /*=================== use fma version below ===============
        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            //scan-x
            const double y0 = y * scale + my;
            for (int32_t x = 0; x < SCR_WIDTH; x++)
            {
                const double x0 = x * scale + mx;
                double x1 = x0;
                double y1 = y0;
                for (i = 0; i < iterations; i++)
                {
                    const double x2 = x1 * x1;
                    const double y2 = y1 * y1;
                    if (x2 + y2 >= 4.0) break;
                    y1 = 2 * x1 * y1 + cim;
                    x1 = x2 - y2 + cre;
                }

                //use color model conversion to get rainbow palette
                pixels[y][x] = hsv2rgb(0xff * i / iterations, 0xff, (i < iterations) ? 0xff : 0);
            }
        }
        ===========================================================
        ==========AVX - 512 version support INTEL 11th later=======

        const __m512d xim = _mm512_set1_pd(cim);
        const __m512d xre = _mm512_set1_pd(cre);

        const __m512d dd = _mm512_set1_pd(scale);
        const __m512d tx = _mm512_set1_pd(mx);

        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            const __m512d y0 = _mm512_set1_pd(y * scale + my);
            for (int32_t x = 0; x < SCR_WIDTH; x += 8)
            {
                const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
                const __m512d x0 = _mm512_fmadd_pd(dd, _mm512_cvtepi32_pd(ind), tx);
                __m512d x1 = x0;
                __m512d y1 = y0;
                __m512i iters = _mm512_setzero_si512();
                __m512i masks = _mm512_setzero_si512();

                for (int32_t n = 0; n < iterations; n++)
                {
                    const __m512d x2 = _mm512_mul_pd(x1, x1);
                    const __m512d y2 = _mm512_mul_pd(y1, y1);
                    const __m512d abs = _mm512_add_pd(x2, y2);
                    const __m512i cmp = _mm512_movm_epi64(_mm512_cmp_pd_mask(abs, _mm512_set1_pd(4), _CMP_GE_OS));

                    masks = _mm512_or_epi32(cmp, masks);
                    if (_mm512_test_epi64_mask(masks, masks) == 1) break;

                    iters = _mm512_add_epi32(iters, _mm512_andnot_epi32(masks, _mm512_set1_epi32(1)));

                    const __m512d t = _mm512_add_pd(x1, x1);
                    y1 = _mm512_fmadd_pd(t, y1, xim);
                    x1 = _mm512_add_pd(_mm512_sub_pd(x2, y2), xre);
                }

                //extract iteration position for each pixel
                alignas(64) int32_t ipos[16] = { 0 };
                _mm512_stream_si512(it, iters);

                //use HSV convert to get full rainbow palette
                uint32_t* pdst = &pixels[y][x];
                *pdst++ = hsv2rgb(255 * ipos[0]  / iterations, 255, (ipos[0]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[2]  / iterations, 255, (ipos[2]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[4]  / iterations, 255, (ipos[4]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[6]  / iterations, 255, (ipos[6]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[8]  / iterations, 255, (ipos[8]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[10] / iterations, 255, (ipos[10] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[12] / iterations, 255, (ipos[12] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[14] / iterations, 255, (ipos[14] < iterations) ? 255 : 0);
            }
        }
        ==========================================================================================*/

        const __m256d xim = _mm256_set1_pd(cim);
        const __m256d xre = _mm256_set1_pd(cre);

        const __m256d dd = _mm256_set1_pd(scale);
        const __m256d tx = _mm256_set1_pd(mx);

        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            const __m256d y0 = _mm256_set1_pd(y * scale + my);
            for (int32_t x = 0; x < SCR_WIDTH; x += 4)
            {
                const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
                const __m256d x0 = _mm256_fmadd_pd(dd, _mm256_cvtepi32_pd(ind), tx);
                __m256d x1 = x0;
                __m256d y1 = y0;
                __m256i iters = _mm256_setzero_si256();
                __m256i masks = _mm256_setzero_si256();

                for (int32_t n = 0; n < iterations; n++)
                {
                    const __m256d x2 = _mm256_mul_pd(x1, x1);
                    const __m256d y2 = _mm256_mul_pd(y1, y1);
                    const __m256d abs = _mm256_add_pd(x2, y2);
                    const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), _CMP_GE_OS));

                    masks = _mm256_or_si256(cmp, masks);
                    if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;

                    iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                    const __m256d t = _mm256_add_pd(x1, x1);
                    y1 = _mm256_fmadd_pd(t, y1, xim);
                    x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
                }

                //extract iteration position for each pixel
                alignas(32) int32_t it[8] = { 0 };
                _mm256_stream_si256((__m256i*)it, iters);

                //use HSV convert to get full rainbow palette
                uint32_t* pdst = &pixels[y][x];
                *pdst++ = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
            }
        }

        //print the values of all variables on screen if that option is enabled
        if (showText <= 1)
        {
            writeText(1,  1, RGB_WHITE, 0, "X:%g", mx);
            writeText(1, 11, RGB_WHITE, 0, "Y:%g", my);
            writeText(1, 21, RGB_WHITE, 0, "Z:%g", scale);
            writeText(1, 31, RGB_WHITE, 0, "R:%g", cre);
            writeText(1, 41, RGB_WHITE, 0, "I:%g", cim);
            writeText(1, 51, RGB_WHITE, 0, "N:%d", iterations);
        }

        //print the help text on screen if that option is enabled
        if (showText == 0)
        {
            writeText(1, SCR_HEIGHT - 41, RGB_WHITE, 0, "Arrows move, I/O zooms");
            writeText(1, SCR_HEIGHT - 31, RGB_WHITE, 0, "1,2,3,4 change shape");
            writeText(1, SCR_HEIGHT - 21, RGB_WHITE, 0, "z,x changes iterations");
            writeText(1, SCR_HEIGHT - 11, RGB_WHITE, 0, "h cycle texts");
        }

        render();

        //get the time and old time for time dependent input
        oldTime = time;
        time = getTime();
        frameTime = (time - oldTime);
        sprintf(sbuff, "Julia-Explorer [FPS: %.2f]", 1000.0 / frameTime);
        setWindowTitle(sbuff);

        //read user input key
        input = waitUserInput();
        
        //ZOOM keys
        if (input == SDL_SCANCODE_I)
        {
            const double newScale = scale / 1.08;
            mx += SCR_WIDTH * (scale - newScale) * 0.5;
            my += SCR_HEIGHT * (scale - newScale) * 0.5;
            scale = newScale;
        }
        
        if (input == SDL_SCANCODE_O)
        {
            const double newScale = scale * 1.08;
            mx += SCR_WIDTH * (scale - newScale) * 0.5;
            my += SCR_HEIGHT * (scale - newScale) * 0.5;
            scale = newScale;
        }
        
        //MOVE keys
        if (input == SDL_SCANCODE_UP)
        {
            const double sy = -(SCR_HEIGHT / 100.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_DOWN)
        {
            const double sy = (SCR_HEIGHT / 100.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_LEFT)
        {
            const double sx = -(SCR_WIDTH / 100.0);
            mx += sx * scale;
        }

        if (input == SDL_SCANCODE_RIGHT)
        {
            const double sx = (SCR_WIDTH / 100.0);
            mx += sx * scale;
        }
        
        //CHANGE SHAPE keys
        if (input == SDL_SCANCODE_1) { cim += 0.0002; }
        if (input == SDL_SCANCODE_2) { cim -= 0.0002; }
        if (input == SDL_SCANCODE_3) { cre += 0.0002; }
        if (input == SDL_SCANCODE_4) { cre -= 0.0002; }

        //keys to change number of iterations
        if (input == SDL_SCANCODE_Z) { iterations <<= 1; }
        if (input == SDL_SCANCODE_X) { if (iterations > 2) iterations >>= 1; }

        //key to change the text options
        if (input == SDL_SCANCODE_H) { showText++; showText %= 3; }
        if (input == SDL_SCANCODE_ESCAPE) quit();
    } while (input != SDL_SCANCODE_RETURN);

    freeFont(0);
    free(pixels);
    cleanup();
}

void mandelbrotSet()
{
    //make larger to see more detail!
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Mandelbrot-Set")) return;

    int32_t i = 0;

    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels) return;

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    const int32_t iterations = 255;
    const double xscale = 3.0 / SCR_WIDTH;
    const double yscale = 2.0 / SCR_HEIGHT;

    const double scale = max(xscale, yscale);
    const double mx = -0.5 * SCR_WIDTH * scale - 0.5;
    const double my = -0.5 * SCR_HEIGHT * scale;

    /*=================== use fma version below ========================
    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        const double y0 = y * scale + my;
        for (int32_t x = 0; x < SCR_WIDTH; x++)
        {
            const double x0 = x * scale + mx;
            double x1 = x0;
            double y1 = y0;
            for (i = 0; i < iterations; i++)
            {
                const double x2 = x1 * x1;
                const double y2 = y1 * y1;
                if (x2 + y2 >= 4.0) break;
                y1 = 2 * x1 * y1 + y0;
                x1 = x2 - y2 + x0;
            }

            //use color model conversion to get rainbow palette
            pixels[y][x] = hsv2rgb(0xff * i / iterations, 0xff, (i < iterations) ? 0xff : 0);
        }
    }
    ======================================================================
    ============AVX - 512 version support INTEL 11th later================

    const __m512d dd = _mm512_set1_pd(scale);
    const __m512d tx = _mm512_set1_pd(mx);

    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        const __m512d y0 = _mm512_set1_pd(y * scale + my);
        for (int32_t x = 0; x < SCR_WIDTH; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m512d x0 = _mm512_fmadd_pd(dd, _mm512_cvtepi32_pd(ind), tx);
            __m512d x1 = x0;
            __m512d y1 = y0;
            __m512i iters = _mm512_setzero_si512();
            __m512i masks = _mm512_setzero_si512();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m512d x2 = _mm512_mul_pd(x1, x1);
                const __m512d y2 = _mm512_mul_pd(y1, y1);
                const __m512d abs = _mm512_add_pd(x2, y2);
                const __m512i cmp = _mm512_movm_epi64(_mm512_cmp_pd_mask(abs, _mm512_set1_pd(4), _CMP_GE_OS));

                masks = _mm512_or_epi32(cmp, masks);
                if (_mm512_test_epi64_mask(masks, masks) == 1) break;

                iters = _mm512_add_epi32(iters, _mm512_andnot_epi32(masks, _mm512_set1_epi32(1)));

                const __m512d t = _mm512_add_pd(x1, x1);
                y1 = _mm512_fmadd_pd(t, y1, y0);
                x1 = _mm512_add_pd(_mm512_sub_pd(x2, y2), x0);
            }

            //extract iteration position for each pixel
            alignas(64) int32_t ipos[16] = { 0 };
            _mm512_stream_si512(it, iters);

            //use HSV convert to get full rainbow palette
            uint32_t* pdst = &pixels[y][x];
            *pdst++ = hsv2rgb(255 * ipos[0]  / iterations, 255, (ipos[0]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[2]  / iterations, 255, (ipos[2]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[4]  / iterations, 255, (ipos[4]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[6]  / iterations, 255, (ipos[6]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[8]  / iterations, 255, (ipos[8]  < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[10] / iterations, 255, (ipos[10] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[12] / iterations, 255, (ipos[12] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * ipos[14] / iterations, 255, (ipos[14] < iterations) ? 255 : 0);
        }
    }
    ==========================================================================================*/

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < SCR_WIDTH; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m256d x0 = _mm256_fmadd_pd(dd, _mm256_cvtepi32_pd(ind), tx);
            __m256d x1 = x0;
            __m256d y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x1, x1);
                const __m256d y2 = _mm256_mul_pd(y1, y1);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                const __m256d t = _mm256_add_pd(x1, x1);
                y1 = _mm256_fmadd_pd(t, y1, y0);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            //extract iteration position for each pixel
            alignas(32) int32_t it[8] = { 0 };
            _mm256_stream_si256((__m256i*)it, iters);

            //use HSV convert to get full rainbow palette
            uint32_t* pdst = &pixels[y][x];
            *pdst++ = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
            *pdst++ = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
        }
    }

    //make the Mandelbrot Set visible and wait to exit
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    free(pixels);
    cleanup();
}

void mandelbrotExporer()
{
    if (!loadFont("assets/sysfont.xfn", 0)) return;
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Mandelbrot-Explorer")) return;

    //windows title buffer (add FPS)
    char sbuff[200] = { 0 };

    //show hint text
    int32_t showText = 0, i = 0;

    //current and old time, and their difference (for input)
    uint32_t time = 0, oldTime = 0, frameTime = 0;
    
    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels) return;

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    //user input key
    int32_t input = 0;

    //iterations
    int32_t iterations = 255;

    //scale unit
    const double xscale = 3.0 / SCR_WIDTH;
    const double yscale = 2.0 / SCR_HEIGHT;

    //calculate scale and current position
    double scale = max(xscale, yscale);
    double mx = -0.5 * SCR_WIDTH * scale - 0.5;
    double my = -0.5 * SCR_HEIGHT * scale;

    //begin main program loop
    do
    {
        /*======================== use fma version below =====================
        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            //scan-x
            const double y0 = y * scale + my;
            for (int32_t x = 0; x < SCR_WIDTH; x++)
            {
                const double x0 = x * scale + mx;
                double x1 = x0;
                double y1 = y0;
                for (i = 0; i < iterations; i++)
                {
                    const double x2 = x1 * x1;
                    const double y2 = y1 * y1;
                    if (x2 + y2 >= 4.0) break;
                    y1 = 2 * x1 * y1 + y0;
                    x1 = x2 - y2 + x0;
                }

                //use color model conversion to get rainbow palette
                pixels[y][x] = hsv2rgb(0xff * i / iterations, 0xff, (i < iterations) ? 0xff : 0);
            }
        }
        ========================================================================
        ============AVX - 512 version support INTEL 11th later==================

        const __m512d dd = _mm512_set1_pd(scale);
        const __m512d tx = _mm512_set1_pd(mx);

        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            const __m512d y0 = _mm512_set1_pd(y * scale + my);
            for (int32_t x = 0; x < SCR_WIDTH; x += 8)
            {
                const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
                const __m512d x0 = _mm512_fmadd_pd(dd, _mm512_cvtepi32_pd(ind), tx);
                __m512d x1 = x0;
                __m512d y1 = y0;
                __m512i iters = _mm512_setzero_si512();
                __m512i masks = _mm512_setzero_si512();

                for (int32_t n = 0; n < iterations; n++)
                {
                    const __m512d x2 = _mm512_mul_pd(x1, x1);
                    const __m512d y2 = _mm512_mul_pd(y1, y1);
                    const __m512d abs = _mm512_add_pd(x2, y2);
                    const __m512i cmp = _mm512_movm_epi64(_mm512_cmp_pd_mask(abs, _mm512_set1_pd(4), _CMP_GE_OS));

                    masks = _mm512_or_epi32(cmp, masks);
                    if (_mm512_test_epi64_mask(masks, masks) == 1) break;

                    iters = _mm512_add_epi32(iters, _mm512_andnot_epi32(masks, _mm512_set1_epi32(1)));

                    const __m512d t = _mm512_add_pd(x1, x1);
                    y1 = _mm512_fmadd_pd(t, y1, y0);
                    x1 = _mm512_add_pd(_mm512_sub_pd(x2, y2), x0);
                }

                //extract iteration position for each pixel
                alignas(64) int32_t ipos[16] = { 0 };
                _mm512_stream_si512(it, iters);

                //use HSV convert to get full rainbow palette
                uint32_t* pdst = &pixels[y][x];
                *pdst++ = hsv2rgb(255 * ipos[0]  / iterations, 255, (ipos[0]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[2]  / iterations, 255, (ipos[2]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[4]  / iterations, 255, (ipos[4]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[6]  / iterations, 255, (ipos[6]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[8]  / iterations, 255, (ipos[8]  < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[10] / iterations, 255, (ipos[10] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[12] / iterations, 255, (ipos[12] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * ipos[14] / iterations, 255, (ipos[14] < iterations) ? 255 : 0);
            }
        }
        ==========================================================================================*/
        const __m256d dd = _mm256_set1_pd(scale);
        const __m256d tx = _mm256_set1_pd(mx);

        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            const __m256d y0 = _mm256_set1_pd(y * scale + my);
            for (int32_t x = 0; x < SCR_WIDTH; x += 4)
            {
                const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
                const __m256d x0 = _mm256_fmadd_pd(dd, _mm256_cvtepi32_pd(ind), tx);
                __m256d x1 = x0;
                __m256d y1 = y0;
                __m256i iters = _mm256_setzero_si256();
                __m256i masks = _mm256_setzero_si256();

                for (int32_t n = 0; n < iterations; n++)
                {
                    const __m256d x2 = _mm256_mul_pd(x1, x1);
                    const __m256d y2 = _mm256_mul_pd(y1, y1);
                    const __m256d abs = _mm256_add_pd(x2, y2);
                    const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), _CMP_GE_OS));

                    masks = _mm256_or_si256(cmp, masks);
                    if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;

                    iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                    const __m256d t = _mm256_add_pd(x1, x1);
                    y1 = _mm256_fmadd_pd(t, y1, y0);
                    x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
                }

                //extract iteration position for each pixel
                alignas(32) int32_t it[8] = { 0 };
                _mm256_stream_si256((__m256i*)it, iters);

                //use HSV convert to get full rainbow palette
                uint32_t* pdst = &pixels[y][x];
                *pdst++ = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
                *pdst++ = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
            }
        }

        //print the values of all variables on screen if that option is enabled
        if (showText <= 1)
        {
            writeText(1,  1, RGB_WHITE, 0, "X:%g", mx);
            writeText(1, 11, RGB_WHITE, 0, "Y:%g", my);
            writeText(1, 21, RGB_WHITE, 0, "Z:%g", scale);
            writeText(1, 31, RGB_WHITE, 0, "N:%d", iterations);
        }

        //print the help text on screen if that option is enabled
        if (showText == 0)
        {
            writeText(1, SCR_HEIGHT - 31, RGB_WHITE, 0, "Arrows move, I/O zooms");
            writeText(1, SCR_HEIGHT - 21, RGB_WHITE, 0, "z,x changes iterations");
            writeText(1, SCR_HEIGHT - 11, RGB_WHITE, 0, "h cycle texts");
        }

        render();

        //get the time and old time for time dependent input
        oldTime = time;
        time = getTime();
        frameTime = (time - oldTime);
        sprintf(sbuff, "Mandelbrot-Explorer [FPS: %.2f]", 1000.0 / frameTime);
        setWindowTitle(sbuff);

        //read user input key
        input = waitUserInput();

        //ZOOM keys
        if (input == SDL_SCANCODE_I)
        {
            const double newScale = scale / 1.1;
            mx += SCR_WIDTH * (scale - newScale) * 0.5;
            my += SCR_HEIGHT * (scale - newScale) * 0.5;
            scale = newScale;
        }

        if (input == SDL_SCANCODE_O)
        {
            const double newScale = scale * 1.1;
            mx += SCR_WIDTH * (scale - newScale) * 0.5;
            my += SCR_HEIGHT * (scale - newScale) * 0.5;
            scale = newScale;
        }

        //MOVE keys
        if (input == SDL_SCANCODE_UP)
        {
            const double sy = -(SCR_HEIGHT / 10.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_DOWN)
        {
            const double sy = (SCR_HEIGHT / 10.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_LEFT)
        {
            const double sx = -(SCR_WIDTH / 10.0);
            mx += sx * scale;
        }

        if (input == SDL_SCANCODE_RIGHT)
        {
            const double sx = (SCR_WIDTH / 10.0);
            mx += sx * scale;
        }

        //keys to change number of iterations
        if (input == SDL_SCANCODE_Z) { iterations <<= 1; }
        if (input == SDL_SCANCODE_X) { if (iterations > 2) iterations >>= 1; }

        //key to change the text options
        if (input == SDL_SCANCODE_H) { showText++; showText %= 3; }
        if (input == SDL_SCANCODE_ESCAPE) quit();
    } while (input != SDL_SCANCODE_RETURN);

    freeFont(0);
    free(pixels);
    cleanup();
}

static uint32_t colors[SIZE_256] = { 0 };
static uint32_t plasma[SCR_HEIGHT][SCR_WIDTH] = { 0 };

void plasmaDemo()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Plasma")) return;

    int32_t paletteShift = 0;
    
    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels) return;

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (int32_t i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    //use HSV2RGB to vary the Hue of the color through the palette
    for (int32_t x = 0; x < 256; x++) colors[x] = hsv2rgb(x, 255, 255);

    const int32_t mwidth = SCR_WIDTH >> 1;
    const int32_t mheight = SCR_HEIGHT >> 1;

    //generate the plasma once
    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        for (int32_t x = 0; x < SCR_WIDTH; x++)
        {
            //the plasma buffer is a sum of sines
            //const uint32_t color = uint32_t(
            //      128.0 + (128.0 * sin(x / 16.0))
            //    + 128.0 + (128.0 * sin(y / 16.0))) >> 1;
            //plasma[y][x] = color;

            //rolling
            //const uint32_t color = uint32_t(
            //      128.0 + (128.0 * sin(x / 16.0))
            //    + 128.0 + (128.0 * sin(y / 8.0))
            //    + 128.0 + (128.0 * sin((double(x) + y) / 16.0))
            //    + 128.0 + (128.0 * sin(sqrt(double(x) * x + double(y) * y) / 8.0))) >> 2;
            //plasma[y][x] = color;

            //sin
            const uint32_t color = uint32_t(
                  128.0 + (128.0 * sin(x / 16.0))
                + 128.0 + (128.0 * sin(y / 32.0))
                + 128.0 + (128.0 * sin(sqrt((double(x) - mwidth) * (double(x) - mwidth) + (double(y) - mheight) * (double(y) - mheight)) / 8.0))
                + 128.0 + (128.0 * sin(sqrt(double(x) * x + double(y) * y) / 8.0))) >> 2;
            plasma[y][x] = color;
        }
    }

    //start the animation loop, it rotates the palette
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //the parameter to shift the palette varies with time
        paletteShift = int32_t(getTime() / 10.0);

        //draw every pixel again, with the shifted palette color
        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            for (int32_t x = 0; x < SCR_WIDTH; x++) pixels[y][x] = colors[(plasma[y][x] + paletteShift) % 256];
        }

        //make everything visible
        render();
    }

    free(pixels);
    cleanup();
}

static int32_t distBuff[SCR_HEIGHT][SCR_WIDTH] = { 0 };
static int32_t angleBuff[SCR_HEIGHT][SCR_WIDTH] = { 0 };

void tunnelDemo()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Tunnel")) return;

    int32_t tw = 0, th = 0, i = 0;
    uint32_t* ptext = NULL;

    //load tunnel texture
    if (!loadTexture(&ptext, &tw, &th, "assets/map03.png")) return;

    //make matrix array for easy pixel accessing
    uint32_t** texture = (uint32_t**)calloc(th, sizeof(uint32_t*));
    if (!texture) return;

    texture[0] = ptext;
    for (i = 1; i < th; i++) texture[i] = &texture[0][i * tw];

    uint32_t** pixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!pixels)
    {
        free(texture);
        return;
    }

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < SCR_HEIGHT; i++) pixels[i] = &pixels[0][i * SCR_WIDTH];

    const double ratio = 128;
    const double scale = 1.5;

    const int32_t mwidth = SCR_WIDTH >> 1;
    const int32_t mheight = SCR_HEIGHT >> 1;

    //generate non-linear transformation table
    for (int32_t y = 0; y < SCR_HEIGHT; y++)
    {
        for (int32_t x = 0; x < SCR_WIDTH; x++)
        {
            distBuff[y][x] = int32_t(ratio * th / sqrt(sqr(double(x) - mwidth) + sqr(double(y) - mheight)));
            angleBuff[y][x] = int32_t(scale * tw * atan2(double(y) - mheight, double(x) - mwidth) / M_PI);
        }
    }

    //begin the loop
    while (!finished(SDL_SCANCODE_RETURN))
    {
        const double animation = getTime() / 1000.0;

        //calculate the shift values out of the animation value
        const int32_t shiftX = int32_t(tw * animation * 0.3);
        const int32_t shiftY = int32_t(th * animation * 0.5);

        for (int32_t y = 0; y < SCR_HEIGHT; y++)
        {
            for (int32_t x = 0; x < SCR_WIDTH; x++)
            {
                //get the offset from the texture by using the tables, shifted with the animation values
                const int32_t oy = uint32_t(distBuff[y][x] + shiftX) % th;
                const int32_t ox = uint32_t(angleBuff[y][x] + shiftY) % tw;
                pixels[y][x] = texture[oy][ox];
            }
        }

        delay(FPS_30);
        render();
    }

    free(pixels);
    free(texture[0]);
    free(texture);
    cleanup();
}

/*
//blur
#define FILTER_WIDTH    5
#define FILTER_HEIGHT   5

static const double filter[][FILTER_WIDTH] =
{
    {0, 0, 1, 0, 0},
    {0, 1, 1, 1, 0},
    {1, 1, 1, 1, 1},
    {0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0},
};

static const double bias = 0.0;
static const double factor = 1.0 / 13.0;
*/

/*//Gaussian Blur (3 x 3)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

static const double filter[][FILTER_WIDTH] =
{
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1},
};

static const double bias = 0.0;
static const double factor = 1.0 / 16.0;
*/

/*//Gaussian Blur (5 x 5)
#define FILTER_WIDTH    5
#define FILTER_HEIGHT   5

static const double filter[][FILTER_WIDTH] =
{
    {1,  4,  6,  4,  1},
    {4, 16, 24, 16,  4},
    {6, 24, 36, 24,  6},
    {4, 16, 24, 16,  4},
    {1,  4,  6,  4,  1},
};

static const double bias = 0.0;
static const double factor = 1.0 / 256.0;
*/

/*//Gaussian Blur (3f x 3f)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

static const double filter[][FILTER_WIDTH] =
{
    {0.077847, 0.123317, 0.077847},
    {0.123317, 0.195346, 0.123317},
    {0.077847, 0.123317, 0.077847},
};

static const double bias = 0.0;
static const double factor = 1.0;
*/

/*//Motion Blur
#define FILTER_WIDTH    9
#define FILTER_HEIGHT   9

static const double filter[][FILTER_WIDTH] =
{
    {1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1},
};

static const double bias = 0.0;
static const double factor = 1.0 / 9.0;
*/
/*
//Find Edges
#define FILTER_WIDTH    5
#define FILTER_HEIGHT   5

static const double filter[][FILTER_WIDTH] =
{
    {-1,  0,  0,  0,  0},
    { 0, -2,  0,  0,  0},
    { 0,  0,  6,  0,  0},
    { 0,  0,  0, -2,  0},
    { 0,  0,  0,  0, -1},
};

static const double bias = 0.0;
static const double factor = 1.0;
*/

/*
//Sharpen
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

static const double filter[][FILTER_WIDTH] =
{
    {-1, -1, -1},
    {-1,  9, -1},
    {-1, -1, -1},
};

static const double bias = 0.0;
static const double factor = 1.0;
*/

//Emboss (3 x 3)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

static const double filter[][FILTER_WIDTH] = {
    {-1, -1,  0},
    {-1,  0,  1},
    { 0,  1,  1},
};

static const double factor = 1.0;
static const double bias = 128.0;

void imageFillter()
{
    //load demo image
    int32_t i = 0;
    uint32_t* ptex = NULL;
    int32_t tw = 0, th = 0;

    if (!loadTexture(&ptex, &tw, &th, "assets/photo3.png")) return;

    //set up the screen
    if (!initScreen(tw, th, 32, 0, "Filters")) return;

    uint32_t** image = (uint32_t**)calloc(th, sizeof(uint32_t*));
    if (!image) return;

    image[0] = ptex;
    for (i = 1; i < th; i++) image[i] = &image[0][i * tw];

    uint32_t** pixels = (uint32_t**)calloc(th, sizeof(uint32_t*));
    if (!pixels)
    {
        free(image);
        return;
    }

    pixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < th; i++) pixels[i] = &pixels[0][i * tw];

    //apply the filter
    for (int32_t y = 0; y < th; y++)
    {
        for (int32_t x = 0; x < tw; x++)
        {
            double red = 0.0, green = 0.0, blue = 0.0;

            //multiply every value of the filter with corresponding image pixel
            for (int32_t fy = 0; fy < FILTER_HEIGHT; fy++)
            {
                for (int32_t fx = 0; fx < FILTER_WIDTH; fx++)
                {
                    const int32_t dx = (x - FILTER_WIDTH / 2 + fx + tw) % tw;
                    const int32_t dy = (y - FILTER_HEIGHT / 2 + fy + th) % th;
                    const ARGB* pixel = (const ARGB*)&image[dy][dx];
                    red   += pixel->r * filter[fy][fx];
                    green += pixel->g * filter[fy][fx];
                    blue  += pixel->b * filter[fy][fx];
                }
            }

            //make target pixel
            ARGB* pdst = (ARGB*)&pixels[y][x];

            //truncate values smaller than zero and larger than 255
            pdst->r = clamp(int32_t(factor * red + bias), 0, 255);
            pdst->g = clamp(int32_t(factor * green + bias), 0, 255);
            pdst->b = clamp(int32_t(factor * blue + bias), 0, 255);

            //make gray
            pdst->r = pdst->g = pdst->b = uint8_t(0.2126 * pdst->r + 0.7152 * pdst->g + 0.0722 * pdst->b);

            //take absolute value and truncate to 255
            //pdst->r = min(abs(int32_t(fact * red + bias)), 255);
            //pdst->g = min(abs(int32_t(fact * green + bias)), 255);
            //pdst->b = min(abs(int32_t(fact * blue + bias)), 255);
        }
    }

    //redraw & sleep
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    free(image[0]);
    free(image);
    free(pixels);
    cleanup();
}

/*=================================================================================*/
/*                     RAY CASTING WITH SHADER EFFECT                              */
/* Reference: https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/  */
/* Rewrite to C/C++ by pherosiden@gmail.com                                        */
/*=================================================================================*/

//size of tile (wall height)
#define TILE_SIZE               128
#define WALL_HEIGHT             128

//world map width
#define MINI_MAP_WIDTH          5

//brightness value
#define BASE_LIGHT_VALUE        150

//MIN distance to wall
#define MIN_DISTANCE_TO_WALL    60

//Player speed
#define PLAYER_SPEED            32

//2D map
#define WORLD_MAP_WIDTH         20
#define WORLD_MAP_HEIGHT        20

//Remember that PROJECTIONPLANE = screen size
#define PROJECTION_PLANE_WIDTH  640
#define PROJECTION_PLANE_HEIGHT 480

//Player distance to projection plan = PROJECTION_PLANE_WIDTH / 2 / tan(30)
#define PLAYER_PROJECTION_PLAN  554

//We use FOV of 60 degrees.  So we use this FOV basis of the table, taking into account
//that we need to cast 320 rays (PROJECTIONPLANEWIDTH) within that 60 degree FOV.
#define ANGLE60                 PROJECTION_PLANE_WIDTH

//You must make sure these values are integers because we're using lookup tables.
#define ANGLE0                  0
#define ANGLE30                 (ANGLE60 / 2)
#define ANGLE90                 (ANGLE30 * 3)
#define ANGLE180                (ANGLE90 * 2)
#define ANGLE270                (ANGLE90 * 3)
#define ANGLE360                (ANGLE60 * 6)
#define ANGLE5                  (ANGLE30 / 6)

//trigonometric tables (the ones with "I" such as ISiTable are "Inverse" table)
static double sinTable[ANGLE360 + 1]   = { 0 };
static double isinTable[ANGLE360 + 1]  = { 0 };
static double cosTable[ANGLE360 + 1]   = { 0 };
static double icosTable[ANGLE360 + 1]  = { 0 };
static double tanTable[ANGLE360 + 1]   = { 0 };
static double itanTable[ANGLE360 + 1]  = { 0 };
static double fishTable[ANGLE360 + 1]  = { 0 };
static double stepTableX[ANGLE360 + 1] = { 0 };
static double stepTableY[ANGLE360 + 1] = { 0 };

//player's attributes
static int32_t playerX                 = PROJECTION_PLANE_WIDTH >> 1;
static int32_t playerY                 = PROJECTION_PLANE_HEIGHT >> 1;
static int32_t playerArc               = ANGLE60;
static int32_t playerHeight            = WALL_HEIGHT >> 1;

//half of the screen height
static int32_t projectionPlaneCenterY  = PROJECTION_PLANE_HEIGHT >> 1;

//the following variables are used to keep the player coordinate in the overhead map
static int32_t playerMapX              = 0;
static int32_t playerMapY              = 0;

//build-in world map
static uint8_t worldMap[WORLD_MAP_HEIGHT][WORLD_MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1},
    {1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,1,1,0,1},
    {1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,0,1,0,1},
    {1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1},
    {1,0,0,0,1,0,1,0,1,0,0,1,0,0,0,0,0,1,0,1},
    {1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1},
    {1,0,0,0,1,1,1,0,1,0,0,1,0,0,1,1,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,1},
    {1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,1},
    {1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1},
    {1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

//some textures raw pixels data
static uint32_t**   rawPixels = NULL;
static uint32_t**   wallTexture = NULL;
static uint32_t**   floorTexture = NULL;
static uint32_t**   ceilingTexture = NULL;

//some textures size
static int32_t      wallWidth = 0, wallHeight = 0;
static int32_t      floorWidth = 0, floorHeight = 0;
static int32_t      ceilingWidth = 0, ceilingHeight = 0;

//show/hide maze
static bool         showMaze = true;

//Bresenham draw line with buffer
void drawLineBuffer(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    //validate range
    if (x1 < 0 || x1 >= SCR_WIDTH || x2 < 0 || x2 >= SCR_WIDTH || y1 < 0 || y1 >= SCR_HEIGHT || y2 < 0 || y2 >= SCR_HEIGHT) return;

    const int32_t dx = abs(x2 - x1); //the difference between the x's
    const int32_t dy = abs(y2 - y1); //the difference between the y's

    int32_t x = x1; //start x off at the first pixel
    int32_t y = y1; //start y off at the first pixel

    int32_t addx1 = 0, addx2 = 0, addy1 = 0, addy2 = 0;
    int32_t den = 0, num = 0, addNum = 0, numPixels = 0;

    if (x2 >= x1) //the x-values are increasing
    {
        addx1 = 1;
        addx2 = 1;
    }
    else //the x-values are decreasing
    {
        addx1 = -1;
        addx2 = -1;
    }
    if (y2 >= y1) //the y-values are increasing
    {
        addy1 = 1;
        addy2 = 1;
    }
    else //the y-values are decreasing
    {
        addy1 = -1;
        addy2 = -1;
    }
    if (dx >= dy) //there is at least one x-value for every y-value
    {
        addx1 = 0; //don't change the x when numerator >= denominator
        addy2 = 0; //don't change the y for every iteration
        den = dx;
        num = dx / 2;
        addNum = dy;
        numPixels = dx; //there are more x-values than y-values
    }
    else //there is at least one y-value for every x-value
    {
        addx2 = 0; //don't change the x for every iteration
        addy1 = 0; //don't change the y when numerator >= denominator
        den = dy;
        num = dy / 2;
        addNum = dx;
        numPixels = dy; //there are more y-values than x-values
    }

    for (int32_t currPixel = 0; currPixel < numPixels; currPixel++)
    {
        //draw the current pixel to screen buffer
        rawPixels[y][x] = color;
        num += addNum;  //increase the numerator by the top of the fraction

        if (num >= den) //check if numerator >= denominator
        {
            num -= den; //calculate the new numerator value
            x += addx1; //change the x as appropriate
            y += addy1; //change the y as appropriate
        }

        x += addx2; //change the x as appropriate
        y += addy2; //change the y as appropriate
    }
}

//draw walls slice tinted
void drawWallSliceRectangleTinted(int32_t x, int32_t y, int32_t height, int32_t offset, double brightnessLevel)
{
    //range check
    if (x >= SCR_WIDTH) x = SCR_WIDTH - 1;
    if (y >= SCR_HEIGHT) y = SCR_HEIGHT - 1;
    if (brightnessLevel > 1) brightnessLevel = 1;

    int32_t heightToDraw = height;
    
    //clip bottom
    if (y + heightToDraw > SCR_HEIGHT) heightToDraw = SCR_HEIGHT - y;

    //we need to check this, otherwise, program might crash when trying
    //to fetch the shade if this condition is true (possible if height is 0)
    if (heightToDraw < 0) return;

    //loop error
    int32_t error = 0;

    //index texture offset
    int32_t offsetY = offset / TILE_SIZE;
    const int32_t offsetX = offset % TILE_SIZE;
    
    //we're going to draw the first row, then move down and draw the next row
    //and so on we can use the original x destination to find out
    //the x position of the next row 
    //Remeber that the source bitmap is rotated, so the width is actually the
    //height
    while (true)
    {
        //if error < actualHeight, this will cause row to be skipped until
        //this addition sums to scaledHeight
        //if error > actualHeight, this ill cause row to be drawn repeatedly until
        //this addition becomes smaller than actualHeight
        //1) Think the image height as 100, if percent is >= 100, we'll need to
        //copy the same pixel over and over while decrementing the percentage.  
        //2) Similarly, if percent is <100, we skip a pixel while incrementing
        //and do 1) when the percentage we're adding has reached >=100
        error += height;

        //dereference for faster access (especially useful when the same bit
        //will be copied more than once)

        //cheap shading trick by using brightnessLevel (which doesn't really have to correspond to "brightness") 
        //to alter colors.  You can use logarithmic falloff or linear falloff to produce some interesting effect
        //const uint8_t* color = (const uint8_t*)&wallTexture[offsetX];
        const ARGB* color = (const ARGB*)&wallTexture[offsetY][offsetX];

        //while there's a row to draw & not end of drawing area
        while (error >= wallWidth)
        {
            error -= wallWidth;
            if (y >= 0)
            {
                //modify the pixel
                ARGB* pixel = (ARGB*)&rawPixels[y][x];
                pixel->r = uint8_t(color->r * brightnessLevel);
                pixel->g = uint8_t(color->g * brightnessLevel);
                pixel->b = uint8_t(color->b * brightnessLevel);
            }
            y++;

            //clip bottom (just return if we reach bottom)
            heightToDraw--;
            if (heightToDraw <= 0) return;
        }

        //goto next line
        offsetY++;
        if (offsetY >= wallHeight) offsetY = wallHeight - 1;
    }
}

//filled rectangel with color
void drawFillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
    for (int32_t h = 0; h < height; h++)
    {
        for (int32_t w = 0; w < width; w++) rawPixels[y + h][x + w] = color;
    }
}

//initialize some texture data
int32_t initData()
{
    int32_t i = 0;
    double radian = 0;

    //load texture data
    uint32_t* pWall = NULL, *pFloor = NULL, *pCeiling = NULL;
    if (!loadTexture(&pWall, &wallWidth, &wallHeight, "assets/wallr.png")) return 0;
    if (!loadTexture(&pFloor, &floorWidth, &floorHeight, "assets/floor.png")) return 0;
    if (!loadTexture(&pCeiling, &ceilingWidth, &ceilingHeight, "assets/ceil.png")) return 0;

    //setup draw buffer as matrix to easy access data
    rawPixels = (uint32_t**)calloc(SCR_HEIGHT, sizeof(uint32_t*));
    if (!rawPixels) return 0;

    //assign offset data
    rawPixels[0] = (uint32_t*)getDrawBuffer();
    for (i = 1; i < SCR_HEIGHT; i++) rawPixels[i] = &rawPixels[0][i * SCR_WIDTH];

    //setup texture data as matrix to easy access data
    wallTexture = (uint32_t**)calloc(wallHeight, sizeof(uint32_t*));
    if (!wallTexture)
    {
        free(rawPixels);
        return 0;
    }

    wallTexture[0] = pWall;
    for (i = 1; i < wallHeight; i++) wallTexture[i] = &wallTexture[0][i * wallWidth];

    //floor texture data
    floorTexture = (uint32_t**)calloc(floorHeight, sizeof(uint32_t*));
    if (!floorTexture)
    {
        free(rawPixels);
        free(wallTexture);
        return 0;
    }

    floorTexture[0] = pFloor;
    for (i = 1; i < floorHeight; i++) floorTexture[i] = &floorTexture[0][i * floorWidth];

    //ceiling texture data
    ceilingTexture = (uint32_t**)calloc(ceilingHeight, sizeof(uint32_t*));
    if (!ceilingTexture)
    {
        free(rawPixels);
        free(wallTexture);
        free(floorTexture);
        return 0;
    }

    ceilingTexture[0] = pCeiling;
    for (i = 1; i < ceilingHeight; i++) ceilingTexture[i] = &ceilingTexture[0][i * ceilingWidth];

    //setup lookup table
    for (i = 0; i <= ANGLE360; i++)
    {
        //populate tables with their radian values.
        //(the addition of 0.0001 is a kludge to avoid divisions by 0. Removing it will produce unwanted holes in the wall when a ray is at 0, 90, 180, or 270 degree angles)
        radian = (i * M_PI / (ANGLE60 * 3.0)) + 0.0001;
        sinTable[i] = sin(radian);
        isinTable[i] = 1.0 / sinTable[i];
        cosTable[i] = cos(radian);
        icosTable[i] = 1.0 / cosTable[i];
        tanTable[i] = tan(radian);
        itanTable[i] = 1.0 / tanTable[i];

        //next we crate a table to speed up wall lookups.
        //
        // You can see that the distance between walls are the same
        // if we know the angle
        // _____|_/next xi______________
        //      |
        // ____/|next xi_________   slope = tan = height / dist between xi's
        //    / |
        // __/__|_________  dist between xi = height/tan where height=tile size
        //old xi|
        //                 distance between xi = x_step[view_angle];

        //facing LEFT
        if (i >= ANGLE90 && i < ANGLE270)
        {
            stepTableX[i] = TILE_SIZE / tanTable[i];
            if (stepTableX[i] > 0) stepTableX[i] = -stepTableX[i];
        }
        //facing RIGHT
        else
        {
            stepTableX[i] = (TILE_SIZE / tanTable[i]);
            if (stepTableX[i] < 0) stepTableX[i] = -stepTableX[i];
        }

        //FACING DOWN
        if (i >= ANGLE0 && i < ANGLE180)
        {
            stepTableY[i] = TILE_SIZE * tanTable[i];
            if (stepTableY[i] < 0) stepTableY[i] = -stepTableY[i];
        }
        //FACING UP
        else
        {
            stepTableY[i] = TILE_SIZE * tanTable[i];
            if (stepTableY[i] > 0) stepTableY[i] = -stepTableY[i];
        }
    }

    //create table for fixing FISHBOWL distortion
    for (i = -ANGLE30; i <= ANGLE30; i++)
    {
        //we don't have negative angle, so make it start at 0
        //this will give range from column 0 to 319 (PROJECTONPLANEWIDTH) since we only will need to use those range
        radian = i * M_PI / (ANGLE60 * 3.0);
        fishTable[i + ANGLE30] = 1.0 / cos(radian);
    }

    return 1;
}

//draw world mini map
void drawOverheadMap()
{
    if (!showMaze) return;

    //draw block of world mini map
    for (int32_t row = 0; row < WORLD_MAP_HEIGHT; row++)
    {
        for (int32_t col = 0; col < WORLD_MAP_WIDTH; col++)
        {
            if (worldMap[row][col]) drawFillRectangle(col * MINI_MAP_WIDTH, row * MINI_MAP_WIDTH, MINI_MAP_WIDTH, MINI_MAP_WIDTH, RGB_DARK_GREY);
        }
    }
}

//draw ray on the overhead map
void drawRayOnOverheadMap(int32_t x, int32_t y)
{
    if (!showMaze) return;

    //draw line from the player position to the position where the ray intersect with wall
    drawLineBuffer(playerMapX, playerMapY, x * MINI_MAP_WIDTH / TILE_SIZE, y * MINI_MAP_WIDTH / TILE_SIZE, RGB_GREEN);
}

//draw player POV on the overhead map
void drawPlayerPOVOnOverheadMap()
{
    if (!showMaze) return;

    //draw a red line indication the player's direction
    drawLineBuffer(playerMapX, playerMapY, int32_t(playerMapX + cosTable[playerArc] * 10), int32_t(playerMapY + sinTable[playerArc] * 10), RGB_RED);
}

//start ray casting
void doRayCasting()
{
    //horizontal or vertical coordinate of intersection theoretically, this will be multiple of TILE_SIZE
    //but some trick did here might cause the values off by 1
    int32_t verticalGrid = 0, horizontalGrid = 0;

    //how far to the next bound (this is multiple of tile size)
    int32_t distToNextVerticalGrid = 0, distToNextHorizontalGrid = 0;

    //x, y intersections
    double intersectionX = 0.0, intersectionY = 0.0;
    double distToNextIntersectionX = 0.0, distToNextIntersectionY = 0.0;

    //the current cell that the ray is in
    int32_t gridIndexX = 0, gridIndexY = 0;

    //the distance of the x and y ray intersections from the viewpoint
    double distToVerticalGridBeingHit = 0.0;
    double distToHorizontalGridBeingHit = 0.0;

    int32_t castColumn = 0;
    int32_t castArc = playerArc;

    //field of view is 60 degree with the point of view (player's direction in the middle)
    //30  30
    //   ^
    // \ | /
    //  \|/
    //   v
    //we will trace the rays starting from the leftmost ray
    castArc -= ANGLE30;

    //wrap around if necessary
    if (castArc < ANGLE0) castArc += ANGLE360;

    for (castColumn = 0; castColumn < PROJECTION_PLANE_WIDTH; castColumn++)
    {
        //ray is between 0 to 180 degree (1st and 2nd quadrant)
        double tmpX = 0.0, tmpY = 0.0;

        //ray is facing down
        if (castArc > ANGLE0 && castArc < ANGLE180)
        {
            //truncate then add to get the coordinate of the FIRST grid (horizontal wall) that is in front of the player (this is in pixel unit)
            horizontalGrid = (playerY / TILE_SIZE) * TILE_SIZE + TILE_SIZE;

            //compute distance to the next horizontal wall
            distToNextHorizontalGrid = TILE_SIZE;

            tmpX = itanTable[castArc] * (intmax_t(horizontalGrid) - playerY);
            //we can get the vertical distance to that wall by
            //(horizontalGrid-playerY)
            //we can get the horizontal distance to that wall by
            //1/tan(arc)*verticalDistance
            //find the x interception to that wall
            intersectionX = tmpX + playerX;
        }
        //else, the ray is facing up
        else
        {
            horizontalGrid = (playerY / TILE_SIZE) * TILE_SIZE;
            distToNextHorizontalGrid = -TILE_SIZE;
            tmpX = itanTable[castArc] * (intmax_t(horizontalGrid) - playerY);
            intersectionX = tmpX + playerX;
            horizontalGrid--;
        }

        //LOOK FOR HORIZONTAL WALL
        //if ray is directly facing right or left, then ignore it 
        if (castArc == ANGLE0 || castArc == ANGLE180)
        {
            distToHorizontalGridBeingHit = DBL_MAX;
        }
        //else, move the ray until it hits a horizontal wall
        else
        {
            distToNextIntersectionX = stepTableX[castArc];
            while (true)
            {
                gridIndexX = int32_t(intersectionX / TILE_SIZE);
                gridIndexY = horizontalGrid / TILE_SIZE;

                //if we've looked as far as outside the map range, then bail out
                if (gridIndexX >= WORLD_MAP_WIDTH || gridIndexY >= WORLD_MAP_HEIGHT || gridIndexX < 0 || gridIndexY < 0)
                {
                    distToHorizontalGridBeingHit = DBL_MAX;
                    break;
                }
                //if the grid is not an Opening, then stop
                else if (worldMap[gridIndexY][gridIndexX])
                {
                    distToHorizontalGridBeingHit = (intersectionX - playerX) * icosTable[castArc];
                    break;
                }
                //else, keep looking. At this point, the ray is not blocked, extend the ray to the next grid
                else
                {
                    intersectionX += distToNextIntersectionX;
                    horizontalGrid += distToNextHorizontalGrid;
                }
            }
        }

        //FOLLOW X RAY
        if (castArc < ANGLE90 || castArc > ANGLE270)
        {
            verticalGrid = (playerX / TILE_SIZE) * TILE_SIZE + TILE_SIZE;
            distToNextVerticalGrid = TILE_SIZE;
            tmpY = tanTable[castArc] * (intmax_t(verticalGrid) - playerX);
            intersectionY = tmpY + playerY;
        }
        //RAY FACING LEFT
        else
        {
            verticalGrid = (playerX / TILE_SIZE) * TILE_SIZE;
            distToNextVerticalGrid = -TILE_SIZE;
            tmpY = tanTable[castArc] * (intmax_t(verticalGrid) - playerX);
            intersectionY = tmpY + playerY;
            verticalGrid--;
        }

        //LOOK FOR VERTICAL WALL
        if (castArc == ANGLE90 || castArc == ANGLE270)
        {
            distToVerticalGridBeingHit = DBL_MAX;
        }
        else
        {
            distToNextIntersectionY = stepTableY[castArc];
            while (true)
            {
                //compute current map position to inspect
                gridIndexX = verticalGrid / TILE_SIZE;
                gridIndexY = int32_t(intersectionY / TILE_SIZE);

                if (gridIndexX >= WORLD_MAP_WIDTH || gridIndexY >= WORLD_MAP_HEIGHT || gridIndexX < 0 || gridIndexY < 0)
                {
                    distToVerticalGridBeingHit = DBL_MAX;
                    break;
                }
                else if (worldMap[gridIndexY][gridIndexX])
                {
                    distToVerticalGridBeingHit = (intersectionY - playerY) * isinTable[castArc];
                    break;
                }
                else
                {
                    intersectionY += distToNextIntersectionY;
                    verticalGrid += distToNextVerticalGrid;
                }
            }
        }

        //DRAW THE WALL SLICE
        double ratio = 0.0;
        double scale = 0.0;
        double distance = 0.0;
        
        int32_t offset = 0;         //offset of drawing texture
        int32_t topOfWall = 0;		//used to compute the top and bottom of the sliver that
        int32_t bottomOfWall = 0;	//will be the staring point of floor and ceiling

        bool isVerticalHit = false; //vertical ray hit

        //determine which ray strikes a closer wall
        //if yray distance to the wall is closer, the yDistance will be shorter than the xDistance
        if (distToHorizontalGridBeingHit < distToVerticalGridBeingHit)
        {
            drawRayOnOverheadMap(int32_t(intersectionX), horizontalGrid);
            distance = distToHorizontalGridBeingHit / fishTable[castColumn];
            ratio = PLAYER_PROJECTION_PLAN / distance;
            bottomOfWall = int32_t(ratio * playerHeight + projectionPlaneCenterY);
            scale = intmax_t(PLAYER_PROJECTION_PLAN) * WALL_HEIGHT / distance;
            topOfWall = bottomOfWall - int32_t(scale);
            offset = int32_t(intersectionX) % TILE_SIZE;
        }
        //else, we use xray instead (meaning the vertical wall is closer than the horizontal wall)
        else
        {
            isVerticalHit = true;
            drawRayOnOverheadMap(verticalGrid, int32_t(intersectionY));
            distance = distToVerticalGridBeingHit / fishTable[castColumn];
            ratio = PLAYER_PROJECTION_PLAN / distance;
            bottomOfWall = int32_t(ratio * playerHeight + projectionPlaneCenterY);
            scale = intmax_t(PLAYER_PROJECTION_PLAN) * WALL_HEIGHT / distance;
            topOfWall = bottomOfWall - int32_t(scale);
            offset = int32_t(intersectionY) % TILE_SIZE;
        }

        //range check
        if (bottomOfWall < 0) bottomOfWall = 0;
        if (topOfWall >= SCR_HEIGHT) topOfWall = SCR_HEIGHT - 1;

        //add simple shading so that farther wall slices appear darker use arbitrary value of the farthest distance
        //trick to give different shades between vertical and horizontal (you could also use different textures for each if you wish to)
        if (isVerticalHit) drawWallSliceRectangleTinted(castColumn, topOfWall, (bottomOfWall - topOfWall) + 1, offset, BASE_LIGHT_VALUE / floor(distance));
        else drawWallSliceRectangleTinted(castColumn, topOfWall, (bottomOfWall - topOfWall) + 1, offset, (BASE_LIGHT_VALUE - 50.0) / floor(distance));

        //FLOOR CASTING at the simplest! Try to find ways to optimize this, you can do it!
        if (floorTexture)
        {
            //find the first bit so we can just add the width to get the next row (of the same column)
            for (int32_t row = bottomOfWall; row < PROJECTION_PLANE_HEIGHT; row++)
            {
                const double straightDistance = double(playerHeight) / (intmax_t(row) - projectionPlaneCenterY) * PLAYER_PROJECTION_PLAN;
                const double actualDistance = straightDistance * fishTable[castColumn];

                //translate relative to viewer coordinates
                const int32_t endX = int32_t(actualDistance * cosTable[castArc]) + playerX;
                const int32_t endY = int32_t(actualDistance * sinTable[castArc]) + playerY;

                //get the tile intersected by ray
                const int32_t cellX = endX / TILE_SIZE;
                const int32_t cellY = endY / TILE_SIZE;

                //make sure the tile is within our map
                if (cellX < WORLD_MAP_WIDTH && cellY < WORLD_MAP_HEIGHT && cellX > 0 && cellY > 0 && endX > 0 && endY > 0)
                {
                    //cheap shading trick
                    const double brightnessLevel = clamp(BASE_LIGHT_VALUE / actualDistance, 0, 1);

                    //find offset of tile and column in texture                    
                    const ARGB* color = (ARGB*)&floorTexture[endY % TILE_SIZE][endX % TILE_SIZE];

                    //make target pixel and color
                    ARGB* pixels = (ARGB*)&rawPixels[row][castColumn];

                    //draw the pixels
                    pixels->r = uint8_t(color->r * brightnessLevel);
                    pixels->g = uint8_t(color->g * brightnessLevel);
                    pixels->b = uint8_t(color->b * brightnessLevel);
                }
            }
        }
        
        //CEILING CASTING at the simplest! Try to find ways to optimize this, you can do it!
        if (ceilingTexture)
        {
            //find the first bit so we can just add the width to get the next row (of the same column)
            for (int32_t row = topOfWall; row >= 0; row--)
            {
                const double zoom = (double(WALL_HEIGHT) - playerHeight) / (double(projectionPlaneCenterY) - row);
                const double diagonalDistance = zoom * PLAYER_PROJECTION_PLAN * fishTable[castColumn];

                //translate relative to viewer coordinates
                const int32_t endX = int32_t(diagonalDistance * cosTable[castArc]) + playerX;
                const int32_t endY = int32_t(diagonalDistance * sinTable[castArc]) + playerY;

                //Get the tile intersected by ray
                const int32_t cellX = endX / TILE_SIZE;
                const int32_t cellY = endY / TILE_SIZE;

                //make sure the tile is within our map
                if (cellX < WORLD_MAP_WIDTH && cellY < WORLD_MAP_HEIGHT && cellX > 0 && cellY > 0 && endX > 0 && endY > 0)
                {
                    //cheap shading trick
                    const double brightnessLevel = clamp((BASE_LIGHT_VALUE - 50.0) / diagonalDistance, 0, 1);

                    //find offset of tile and column in texture
                    const ARGB* color = (ARGB*)&ceilingTexture[endY % TILE_SIZE][endX % TILE_SIZE];

                    //make target pixel and color
                    ARGB* pixels = (ARGB*)&rawPixels[row][castColumn];

                    //draw the pixels
                    pixels->r = uint8_t(color->r * brightnessLevel);
                    pixels->g = uint8_t(color->g * brightnessLevel);
                    pixels->b = uint8_t(color->b * brightnessLevel);
                }
            }
        }

        //TRACE THE NEXT RAY
        castArc++;
        if (castArc >= ANGLE360) castArc -= ANGLE360;
    }
}

void runRayCasting()
{
    //time for record FPS
    uint32_t time = 0, oldTime = 0;

    if (!loadFont("assets/sysfont.xfn", 0)) return;
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Raycasting [Shader] -- Arrows: move; Q/Z: lookup; E/C: fly/crouch; Tab:show/hide maze")) return;
    if (!initData()) return;

    //start the main loop
    do {
        //determinate player position on the overhead map
        playerMapX = playerX / TILE_SIZE * MINI_MAP_WIDTH;
        playerMapY = playerY / TILE_SIZE * MINI_MAP_WIDTH;
        
        //start raycasting and draw the ray, world mini map, player
        doRayCasting();
        drawOverheadMap();
        drawPlayerPOVOnOverheadMap();
        
        //timing for input and FPS counter
        oldTime = time;
        time = getTime();

        //frame time is the time this frame has taken, in seconds
        const double frameTime = (time - oldTime) / 1000.0;
        
        //report FPS counter
        writeText(SCR_WIDTH - 50, 2, RGB_WHITE, 0, "FPS:%.f", 1.0 / frameTime);
        render();
        
        //clear background
        memset(rawPixels[0], RGB_WHITE, sizeof(uint32_t) * SCR_WIDTH * SCR_HEIGHT);
        
        //fetch user input
        readKeys();
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

        //rotate left
        if (keyDown(SDL_SCANCODE_LEFT))
        {
            playerArc -= ANGLE5;
            if (playerArc <= ANGLE0) playerArc += ANGLE360;
        }
        //rotate right
        else if (keyDown(SDL_SCANCODE_RIGHT))
        {
            playerArc += ANGLE5;
            if (playerArc >= ANGLE360) playerArc -= ANGLE360;
        }

        // _____     _
        //|\ arc     |
        //|  \       y
        //|    \     |
        //           -
        //|--x--|  
        //
        // sin(arc)=y/diagonal
        // cos(arc)=x/diagonal where diagonal=speed
        const double playerXDir = cosTable[playerArc];
        const double playerYDir = sinTable[playerArc];

        int32_t dx = 0;
        int32_t dy = 0;

        //move forward
        if (keyDown(SDL_SCANCODE_UP))
        {
            dx = int32_t(playerXDir * PLAYER_SPEED);
            dy = int32_t(playerYDir * PLAYER_SPEED);
        }
        //move backward
        else if (keyDown(SDL_SCANCODE_DOWN))
        {
            dx = -int32_t(playerXDir * PLAYER_SPEED);
            dy = -int32_t(playerYDir * PLAYER_SPEED);
        }

        playerX += dx;
        playerY += dy;

        //compute cell position
        const int32_t playerCellX = playerX / TILE_SIZE;
        const int32_t playerCellY = playerY / TILE_SIZE;

        //compute position relative to cell (ie: how many pixel from edge of cell)
        const int32_t playerCellOffsetX = playerX % TILE_SIZE;
        const int32_t playerCellOffsetY = playerY % TILE_SIZE;

        //make sure the player don't bump into walls
        if (dx > 0)
        {
            //moving right
            if (worldMap[playerCellY][playerCellX + 1] && (playerCellOffsetX > (TILE_SIZE - MIN_DISTANCE_TO_WALL)))
            {
                //back player up
                playerX -= (playerCellOffsetX - (TILE_SIZE - MIN_DISTANCE_TO_WALL));
            }
        }
        else
        {
            //moving left
            if (worldMap[playerCellY][playerCellX - 1] && (playerCellOffsetX < MIN_DISTANCE_TO_WALL))
            {
                //back player up
                playerX += (MIN_DISTANCE_TO_WALL - playerCellOffsetX);
            }
        }

        if (dy < 0)
        {
            //moving up
            if (worldMap[playerCellY - 1][playerCellX] && (playerCellOffsetY < MIN_DISTANCE_TO_WALL))
            {
                //back player up 
                playerY += (MIN_DISTANCE_TO_WALL - playerCellOffsetY);
            }
        }
        else
        {
            //moving down                                  
            if (worldMap[playerCellY + 1][playerCellX] && (playerCellOffsetY > (TILE_SIZE - MIN_DISTANCE_TO_WALL)))
            {
                //back player up 
                playerY -= (playerCellOffsetY - (TILE_SIZE - MIN_DISTANCE_TO_WALL));
            }
        }

        //vertical lookup
        if (keyDown(SDL_SCANCODE_Q)) projectionPlaneCenterY += 15;
        else if (keyDown(SDL_SCANCODE_Z)) projectionPlaneCenterY -= 15;

        if (projectionPlaneCenterY < -PROJECTION_PLANE_HEIGHT >> 1) projectionPlaneCenterY = -PROJECTION_PLANE_HEIGHT >> 1;
        else if (projectionPlaneCenterY >= int32_t(PROJECTION_PLANE_HEIGHT * 1.5)) projectionPlaneCenterY = int32_t(PROJECTION_PLANE_HEIGHT * 1.5);

        //fly and crouch
        if (keyDown(SDL_SCANCODE_E)) playerHeight++;
        else if (keyDown(SDL_SCANCODE_C)) playerHeight--;

        if (playerHeight < 1) playerHeight = 1;
        else if (playerHeight >= WALL_HEIGHT - 5) playerHeight = WALL_HEIGHT - 5;

        //show/hide maze
        if (keyPressed(SDL_SCANCODE_TAB)) showMaze = !showMaze;

        //correct frames rate
        delay(FPS_30);
    } while (!keyDown(SDL_SCANCODE_RETURN));

    //cleanup...
    free(rawPixels);
    free(wallTexture[0]);
    free(floorTexture[0]);
    free(ceilingTexture[0]);
    free(wallTexture);
    free(floorTexture);
    free(ceilingTexture);
    freeFont(0);
    cleanup();
}

void gfxEffects()
{
    juliaSet();
    mandelbrotSet();
    juliaExplorer();
    mandelbrotExporer();
    fireDemo1();
    fireDemo2();
    plasmaDemo();
    tunnelDemo();
    basicDrawing();
    imageArithmetic();
    imageFillter();
    crossFading();
    rayCasting();
    runRayCasting();
}
