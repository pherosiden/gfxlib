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
    const double gamma = 0.85;
    const double brightness = 1.6;
    
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

            //calculate smooth colors
            const double s0 = i + 1 - log(log(sqrt(x1 * x1 + y1 * y1))) / log(2.0);
            const double t0 = s0 / iterations;
            const double rf = pow(9 * (1 - t0) * t0 * t0 * t0 * brightness, gamma);
            const double gf = pow(15 * (1 - t0) * (1 - t0) * t0 * t0 * brightness, gamma);
            const double bf = pow(8.5 * (1 - t0) * (1 - t0) * (1 - t0) * t0 * brightness, gamma);
            const uint8_t ru = uint8_t(clamp(rf * 255, 0, 255));
            const uint8_t gu = uint8_t(clamp(gf * 255, 0, 255));
            const uint8_t bu = uint8_t(clamp(bf * 255, 0, 255));
            pixels[y][x] = rgb(ru, gu, bu);
        }
    }

    /*====================AVX-512 version support INTEL 11th later===============================
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
            _mm512_store_si512(it, iters);

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
            alignas(32) uint32_t it[8] = { 0 };
            _mm256_store_si256((__m256i*)it, iters);

            //use HSV convert to get full rainbow palette
            uint32_t* pdst = &pixels[y][x];
            pdst[0] = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
            pdst[1] = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
            pdst[2] = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
            pdst[3] = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
        }
    }

    render();
    waitKeyPressed(SDL_SCANCODE_RETURN);
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

        //Copy back and scroll up one row. The bottom row is all zeros, so it can be skipped.
        for (i = 0; i < (SCR_HEIGHT - 2) * SCR_WIDTH; i++) prevBuff[i] = dstBuff[i + SCR_WIDTH];

        //Remove dark pixels from the bottom rows (except again the bottom row which is all zeros).
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

    //initialize random number seed
    srand(uint32_t(time(NULL)));

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
        dist[i] = sprites[amount - i - 1].first;
        order[i] = sprites[amount - i - 1].second;
    }
}

void rayCasting()
{
    double posX = 22.0, posY = 11.5; //x and y start position
    double dirX = -1.0, dirY = 0.0; //initial direction vector
    double planeX = 0.0, planeY = 0.66; //the 2d ray caster version of camera plane
    
    uint64_t time = 0, oldTime = 0;

    int32_t tw = 0, th = 0, i = 0;
    uint32_t* pbuffs[11] = { 0 };

    const char* const fname[TEXTURE_COUNT] = {
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
        for (i = 0; i < NUM_SPRITES; i++)
        {
            spriteOrder[i] = i;
            spriteDistance[i] = (sqr(posX - sprite[i].x) + sqr(posY - sprite[i].y)); //sqrt not taken, unneeded
        }

        sortSprites(spriteOrder, spriteDistance, NUM_SPRITES);

        //after sorting the sprites, do the projection and draw them
        for (i = 0; i < NUM_SPRITES; i++)
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
        memset(renderBuff[0], 0, sizeof(uint32_t) * SCR_WIDTH * SCR_HEIGHT);

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
    for (int32_t k = 0; k < 11; k++)
    {
        free(textures[k][0]);
        free(textures[k]);
    }

    free(renderBuff);
    freeFont(0);
    cleanup();
}

void basicDrawing()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "2D Primitives")) return;

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
    waitKeyPressed(SDL_SCANCODE_RETURN);
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
    waitKeyPressed(SDL_SCANCODE_RETURN);
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
    uint64_t time = 0, oldTime = 0;
        
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
        /*=================== use FMA version below ===============
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
        ==========AVX-512 version support INTEL 11th later=========

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
                _mm512_store_si512(it, iters);

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
                _mm256_store_si256((__m256i*)it, iters);

                //use HSV convert to get full rainbow palette
                uint32_t* pdst = &pixels[y][x];
                pdst[0] = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
                pdst[1] = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
                pdst[2] = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
                pdst[3] = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
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
        snprintf(sbuff, sizeof(sbuff), "Julia-Explorer [FPS: %.2f]", 1000.0 / (time - oldTime));
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

    /*=================== use FMA version below ========================
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
    ============AVX-512 version support INTEL 11th later==================

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
            _mm512_store_si512(it, iters);

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
            _mm256_store_si256((__m256i*)it, iters);

            //use HSV convert to get full rainbow palette
            uint32_t* pdst = &pixels[y][x];
            pdst[0] = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
            pdst[1] = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
            pdst[2] = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
            pdst[3] = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
        }
    }

    //make the Mandelbrot Set visible and wait to exit
    render();
    waitKeyPressed(SDL_SCANCODE_RETURN);
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
    uint64_t time = 0, oldTime = 0;
    
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
        /*======================== use FMA version below =====================
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
        ============AVX-512 version support INTEL 11th later====================

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
                _mm512_store_si512(it, iters);

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
                _mm256_store_si256((__m256i*)it, iters);

                //use HSV convert to get full rainbow palette
                uint32_t* pdst = &pixels[y][x];
                pdst[0] = hsv2rgb(255 * it[0] / iterations, 255, (it[0] < iterations) ? 255 : 0);
                pdst[1] = hsv2rgb(255 * it[2] / iterations, 255, (it[2] < iterations) ? 255 : 0);
                pdst[2] = hsv2rgb(255 * it[4] / iterations, 255, (it[4] < iterations) ? 255 : 0);
                pdst[3] = hsv2rgb(255 * it[6] / iterations, 255, (it[6] < iterations) ? 255 : 0);
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
        snprintf(sbuff, sizeof(sbuff), "Mandelbrot-Explorer [FPS: %.2f]", 1000.0 / (time - oldTime));
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

    free(texture[0]);
    free(texture);
    free(pixels);
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
    waitKeyPressed(SDL_SCANCODE_RETURN);
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
    uint64_t time = 0, oldTime = 0;

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

        //report FPS counter
        writeText(SCR_WIDTH - 50, 2, RGB_WHITE, 0, "FPS:%.f", 1000.0 / (time - oldTime));
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

namespace
{
    constexpr int32_t MAX_FIREWORK_COUNT = 10;
    constexpr int32_t MAX_PARTICLE_COUNT = 800;
    constexpr int32_t MAX_TRAIL_LENGTH = 18;
    constexpr int32_t BURST_COUNT = 22;
    constexpr int32_t RANDOM_BURST = BURST_COUNT;
    constexpr int32_t DENSE_COUNT = 2;
    constexpr int32_t COLOR_SHELL_STATE2_AGE = 22;
    constexpr int32_t COLOR_SHELL_STATE3_AGE = 44;

    enum FIREWORK_STATE
    {
        FIREWORK_WAITING,
        FIREWORK_LAUNCHING,
        FIREWORK_EXPLODING
    };

    struct FIREWORK_VECTOR
    {
        double x, y;
    };

    struct FIREWORK_PARTICLE
    {
        FIREWORK_VECTOR position;
        FIREWORK_VECTOR velocity;
        FIREWORK_VECTOR trail[MAX_TRAIL_LENGTH];
        double alpha;
        double alphaRate;
        double sparkle;
        double sparkleRate;
        double trailStrength;
        double headCoreStrength;
        double headHeatStrength;
        double headGlowStrength;
        double trailFadePower;
        double waveAmplitude;
        uint32_t color;
        uint32_t tipColor;
        uint32_t headGlowColor;
        int32_t size;
        int32_t trailLength;
        int32_t trailWidth;
        int32_t trailEndWidth;
        bool taperedHead;
        bool roundedHead;
        bool solidHead;
        bool wavyTrail;
        bool strobe;
    };

    struct FIREWORK
    {
        FIREWORK_PARTICLE rocket;
        FIREWORK_PARTICLE particles[MAX_PARTICLE_COUNT];
        uint32_t primaryColor;
        uint32_t secondaryColor;
        FIREWORK_STATE state;
        int32_t waitFrames;
        int32_t age;
        int32_t burstType;
        int32_t particleCount;
        double burstRotation;
        double burstSize;
        double gravity;
        double drag;
        double scale;
    };

    FIREWORK fireworkList[MAX_FIREWORK_COUNT];
    int32_t burstSelection = RANDOM_BURST;
    int32_t activeCount = MAX_FIREWORK_COUNT;

    const uint32_t palette[] = {
        0xff304f, 0xff7a18, 0xffd43b, 0xf8ff72,
        0x5dff8b, 0x36e7ff, 0x4385ff, 0xa968ff,
        0xff54d7, 0xffffff
    };
    
    constexpr int32_t FIREWORK_COLOR_COUNT = sizeof(palette) / sizeof(palette[0]);

    //Firework geometry is accumulated on CPU, but rasterization and additive
    //blending are performed by SDL's accelerated renderer in one draw call.
    std::vector<SDL_Vertex> fireworkVertices;
    std::vector<int> fireworkIndices;

    SDL_FColor gpuColor(uint32_t color)
    {
        return {
            float((color >> 16) & 0xff) / 255.0f,
            float((color >> 8) & 0xff) / 255.0f,
            float(color & 0xff) / 255.0f,
            1.0f
        };
    }

    int addVertex(float x, float y, const SDL_FColor& color)
    {
        fireworkVertices.push_back({ { x, y }, color, { 0.0f, 0.0f } });
        return int(fireworkVertices.size()) - 1;
    }

    void addTriangle(float x0, float y0, float x1, float y1, float x2, float y2,
                     const SDL_FColor& c0, const SDL_FColor& c1, const SDL_FColor& c2)
    {
        fireworkIndices.push_back(addVertex(x0, y0, c0));
        fireworkIndices.push_back(addVertex(x1, y1, c1));
        fireworkIndices.push_back(addVertex(x2, y2, c2));
    }

    void addGlowEllipse(double centerX, double centerY, double directionX, double directionY,
                        double halfLength, double halfWidth, uint32_t color, int32_t segments = 10,
                        bool solid = false)
    {
        const SDL_FColor centerColor = gpuColor(color);
        const SDL_FColor edgeColor = solid ? centerColor : SDL_FColor{ 0.0f, 0.0f, 0.0f, 1.0f };
        const double normalX = -directionY;
        const double normalY = directionX;

        const int center = addVertex(float(centerX), float(centerY), centerColor);
        const int edgeStart = int(fireworkVertices.size());
        for (int32_t i = 0; i < segments; i++)
        {
            const double angle = M_PI * 2.0 * i / segments;
            addVertex(float(centerX + directionX * cos(angle) * halfLength + normalX * sin(angle) * halfWidth),
                      float(centerY + directionY * cos(angle) * halfLength + normalY * sin(angle) * halfWidth),
                      edgeColor);
        }
        for (int32_t i = 0; i < segments; i++)
        {
            fireworkIndices.push_back(center);
            fireworkIndices.push_back(edgeStart + i);
            fireworkIndices.push_back(edgeStart + (i + 1) % segments);
        }
    }

    void addTrailSegment(double x0, double y0, double x1, double y1, double width, uint32_t color)
    {
        const double dx = x1 - x0;
        const double dy = y1 - y0;
        const double length = sqrt(dx * dx + dy * dy);
        if (length < 0.01)
        {
            addGlowEllipse(x0, y0, 1.0, 0.0, width * 0.55, width * 0.55, color, 6);
            return;
        }

        const float nx = float(-dy / length * width * 0.5);
        const float ny = float(dx / length * width * 0.5);
        const SDL_FColor c = gpuColor(color);
        const int base = int(fireworkVertices.size());
        addVertex(float(x0) + nx, float(y0) + ny, c);
        addVertex(float(x0) - nx, float(y0) - ny, c);
        addVertex(float(x1) - nx, float(y1) - ny, c);
        addVertex(float(x1) + nx, float(y1) + ny, c);
        const int quad[] = { base, base + 1, base + 2, base, base + 2, base + 3 };
        fireworkIndices.insert(fireworkIndices.end(), quad, quad + 6);
    }

    void addGlowRing(double centerX, double centerY, double radius, double width, uint32_t color)
    {
        const SDL_FColor c = gpuColor(color);
        constexpr int32_t segments = 32;
        const double inner = max(0.0, radius - width * 0.5);
        const double outer = radius + width * 0.5;
        for (int32_t i = 0; i < segments; i++)
        {
            const double a0 = M_PI * 2.0 * i / segments;
            const double a1 = M_PI * 2.0 * (i + 1) / segments;
            const float ix0 = float(centerX + cos(a0) * inner), iy0 = float(centerY + sin(a0) * inner);
            const float ox0 = float(centerX + cos(a0) * outer), oy0 = float(centerY + sin(a0) * outer);
            const float ix1 = float(centerX + cos(a1) * inner), iy1 = float(centerY + sin(a1) * inner);
            const float ox1 = float(centerX + cos(a1) * outer), oy1 = float(centerY + sin(a1) * outer);
            addTriangle(ix0, iy0, ox0, oy0, ox1, oy1, c, c, c);
            addTriangle(ix0, iy0, ox1, oy1, ix1, iy1, c, c, c);
        }
    }

    int32_t scaledRayCount(const FIREWORK& firework, int32_t baseCount)
    {
        return max(baseCount, int32_t(baseCount * firework.scale + 0.5));
    }

    double calcRayDistance(int32_t index, int32_t particleCount, int32_t rayCount)
    {
        const int32_t layerCount = (particleCount + rayCount - 1) / rayCount;
        return double(1.0 * index / rayCount) / max(layerCount - 1, 1);
    }

    void resetTrails(FIREWORK_PARTICLE& particle)
    {
        for (int32_t i = 0; i < particle.trailLength; i++) particle.trail[i] = particle.position;
    }

    void moveTrails(FIREWORK_PARTICLE& particle, const FIREWORK_VECTOR& previous)
    {
        FIREWORK_VECTOR position = previous;
        for (int32_t i = 0; i < particle.trailLength; i++)
        {
            const FIREWORK_VECTOR next = particle.trail[i];
            particle.trail[i] = position;
            position = next;
        }
    }

    uint32_t mixColor(uint32_t first, uint32_t second, double amount)
    {
        amount = clamp(amount, 0.0, 1.0);
        const double inverse = 1.0 - amount;
        const uint32_t red = uint32_t(((first >> 16) & 0xff) * inverse + ((second >> 16) & 0xff) * amount);
        const uint32_t green = uint32_t(((first >> 8) & 0xff) * inverse + ((second >> 8) & 0xff) * amount);
        const uint32_t blue = uint32_t((first & 0xff) * inverse + (second & 0xff) * amount);
        return (red << 16) | (green << 8) | blue;
    }

    uint32_t fadeColor(uint32_t color, double alpha)
    {
        const uint32_t amount = uint32_t(clamp(alpha, 0.0, 255.0));
        const uint32_t red = ((color >> 16) & 0xff) * amount / 255;
        const uint32_t green = ((color >> 8) & 0xff) * amount / 255;
        const uint32_t blue = (color & 0xff) * amount / 255;
        return (red << 16) | (green << 8) | blue;
    }

    uint32_t boostColor(uint32_t color, double gain)
    {
        const uint32_t red = min(255, uint32_t(((color >> 16) & 0xff) * gain));
        const uint32_t green = min(255, uint32_t(((color >> 8) & 0xff) * gain));
        const uint32_t blue = min(255, uint32_t((color & 0xff) * gain));
        return (red << 16) | (green << 8) | blue;
    }

    void scheduleFirework(int32_t index, int32_t waitFrames)
    {
        FIREWORK& firework = fireworkList[index];
        firework.state = FIREWORK_WAITING;
        firework.waitFrames = waitFrames;
        firework.age = 0;
        firework.rocket.alpha = 0.0;
    }

    void selectBurst()
    {
        for (int32_t i = 0; i <= RANDOM_BURST; i++)
        {
            if (!keyPressed(SDL_SCANCODE_A + i)) continue;
            burstSelection = i;
            activeCount = (i >= 13 && i <= 21) ? DENSE_COUNT : MAX_FIREWORK_COUNT;
            const int32_t launchSpacing = (activeCount == DENSE_COUNT) ? 70 : 4;
            for (int32_t j = 0; j < activeCount; j++) scheduleFirework(j, 1 + j * launchSpacing);
            return;
        }
    }

    void launchFirework(FIREWORK& firework, int32_t width, int32_t height)
    {
        firework.age = 0;
        firework.state = FIREWORK_LAUNCHING;
        
        if (burstSelection == RANDOM_BURST) firework.burstType = rand() % BURST_COUNT;
        else firework.burstType = burstSelection;

        //Burst types 5 and 7 were removed; compact the keyboard selection
        //while preserving the internal ids of all remaining styles.
        if (firework.burstType >= 5) firework.burstType++;
        if (firework.burstType >= 7) firework.burstType++;

        const double screenScale = clamp(min(width / 640.0, height / 480.0), 1.0, 3.0);
        firework.scale = 1.0 + (screenScale - 1.0) * 0.82;

        if (firework.burstType == 15) firework.scale *= 1.05;
        else if (firework.burstType == 16) firework.scale *= 1.15;
        else if (firework.burstType == 17) firework.scale *= 1.12;
        else if (firework.burstType == 18) firework.scale *= 1.02;
        else if (firework.burstType == 19) firework.scale *= 1.04;
        else if (firework.burstType == 20) firework.scale *= 1.08;
        else if (firework.burstType == 21) firework.scale *= 1.04;
        else if (firework.burstType == 22) firework.scale *= 1.08;
        else if (firework.burstType == 23) firework.scale *= 1.08;
        
        const double densityScale = pow(screenScale, 0.58);
        if (firework.burstType == 15) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 46) * 6);
        else if (firework.burstType == 16) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 44) * 6);
        else if (firework.burstType == 17) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 42) * 6);
        else if (firework.burstType == 18) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 210));
        else if (firework.burstType == 19) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 72) * 3);
        else if (firework.burstType == 20) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 68) * 4);
        else if (firework.burstType == 21) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 128));
        else if (firework.burstType == 22) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 150));
        else if (firework.burstType == 23) firework.particleCount = min(MAX_PARTICLE_COUNT, scaledRayCount(firework, 88) * 5);
        else firework.particleCount = min(MAX_PARTICLE_COUNT, int32_t(180 * densityScale + 0.5));

        firework.primaryColor = palette[rand() % FIREWORK_COLOR_COUNT];
        firework.secondaryColor = palette[rand() % FIREWORK_COLOR_COUNT];
        firework.gravity = (firework.burstType == 3) ? 0.055 : (firework.burstType >= 4) ? 0.078 : 0.105;
        firework.drag = (firework.burstType == 3) ? 0.986 : (firework.burstType >= 4) ? 0.981 : 0.974;

        //Display-shell styles inspired by the reference photographs.
        if (firework.burstType == 8)
        {
            firework.primaryColor = 0xe8f4ff;     //silver palm
            firework.secondaryColor = 0x8ebcff;
            firework.gravity = 0.065;
            firework.drag = 0.988;
        }
        else if (firework.burstType == 9)
        {
            firework.primaryColor = 0xffd45a;     //gold chrysanthemum
            firework.secondaryColor = 0xffffff;
            firework.gravity = 0.075;
            firework.drag = 0.984;
        }
        else if (firework.burstType == 10)
        {
            firework.gravity = 0.09;              //rainbow peony
            firework.drag = 0.98;
        }
        else if (firework.burstType == 11)
        {
            firework.primaryColor = 0x7fc8ff;     //blue-white dahlia
            firework.secondaryColor = RGB_WHITE;
            firework.gravity = 0.085;
            firework.drag = 0.979;
        }
        else if (firework.burstType == 12)
        {
            firework.primaryColor = 0xffb52e;     //long golden spider
            firework.secondaryColor = 0xffee9a;
            firework.gravity = 0.052;
            firework.drag = 0.99;
        }
        else if (firework.burstType == 13)
        {
            firework.primaryColor = 0xffcf45;     //crackling rain
            firework.secondaryColor = 0xff7330;
            firework.gravity = 0.14;
            firework.drag = 0.983;
        }
        else if (firework.burstType == 14)
        {
            firework.primaryColor = palette[rand() % (FIREWORK_COLOR_COUNT - 1)]; //double shell
            firework.secondaryColor = RGB_WHITE;
            firework.gravity = 0.08;
            firework.drag = 0.981;
        }
        else if (firework.burstType == 15)
        {
            firework.primaryColor = 0xff3348;     //red-gold weeping chrysanthemum
            firework.secondaryColor = 0xffdc72;
            firework.gravity = 0.072;
            firework.drag = 0.994;
        }
        else if (firework.burstType == 16)
        {
            firework.primaryColor = 0xff571c;     //orange tips
            firework.secondaryColor = 0x8668ff;   //electric violet core
            firework.gravity = 0.055;
            firework.drag = 0.989;
        }
        else if (firework.burstType == 17)
        {
            firework.primaryColor = RGB_WHITE;    //white comet dahlia
            firework.secondaryColor = 0xb8c4d0;
            firework.gravity = 0.038;
            firework.drag = 0.989;
        }
        else if (firework.burstType == 18)
        {
            firework.primaryColor = 0xffb83e;     //golden chrysanthemum
            firework.secondaryColor = 0xffe6a0;
            firework.gravity = 0.074;
            firework.drag = 0.987;
        }
        else if (firework.burstType == 19)
        {
            firework.primaryColor = 0x355dcb;     //deep blue middle crown
            firework.secondaryColor = 0xff58b8;   //pink luminous heart
            firework.gravity = 0.062;
            firework.drag = 0.988;
        }
        else if (firework.burstType == 20)
        {
            firework.primaryColor = 0xff182d;     //dense red inner stars
            firework.secondaryColor = 0x9b55ff;   //fine violet outer filaments
            firework.gravity = 0.048;
            firework.drag = 0.989;
        }
        else if (firework.burstType == 21)
        {
            firework.primaryColor = 0xffc34a;     //thick luminous gold body
            firework.secondaryColor = 0xff8a24;
            firework.gravity = 0.064;
            firework.drag = 0.986;
        }
        else if (firework.burstType == 22)
        {
            firework.primaryColor = 0xffcf54;     //solid gold comet heads and bodies
            firework.secondaryColor = 0xff9c24;
            firework.gravity = 0.058;
            firework.drag = 0.988;
        }
        else if (firework.burstType == 23)
        {
            firework.primaryColor = 0x55e84a;     //green outer rays in state 1
            firework.secondaryColor = 0xffdf38;   //yellow dotted ring
            firework.gravity = 0.018;
            firework.drag = 0.996;
        }

        firework.gravity *= firework.scale;

        const int32_t waveStyle = rand() % 3;
        const double launchMargin = firework.burstType >= 4 ? 0.22 : 0.08;
        firework.rocket.position = { frand(width * launchMargin, width * (1.0 - launchMargin)), double(height + 4) };
        firework.rocket.velocity = { frand(-0.65, 0.65), -frand(firework.burstType >= 4 ? 10.2 : 8.2, (firework.burstType >= 4) ? 12.0 : 11.5) };
        firework.rocket.velocity.x *= firework.scale;
        firework.rocket.velocity.y *= firework.scale;
        firework.rocket.alpha = 255.0;
        firework.rocket.alphaRate = 0.0;
        firework.rocket.sparkle = frand(0.0, M_PI * 2.0);
        firework.rocket.sparkleRate = 0.72;
        firework.rocket.trailStrength = 1.0;
        firework.rocket.headCoreStrength = 0.7;
        firework.rocket.headHeatStrength = 0.88;
        firework.rocket.headGlowStrength = 0.0;
        firework.rocket.trailFadePower = 2.0;
        firework.rocket.waveAmplitude = (waveStyle == 0) ? 2.4 : (waveStyle == 1) ? 3.3 : 4.3;
        firework.rocket.color = 0xffd080;
        firework.rocket.tipColor = 0;
        firework.rocket.headGlowColor = 0;
        firework.rocket.size = 2;
        firework.rocket.trailLength = 10;
        firework.rocket.trailWidth = 2;
        firework.rocket.trailEndWidth = 1;
        firework.rocket.taperedHead = true;
        firework.rocket.roundedHead = true;
        firework.rocket.solidHead = false;
        firework.rocket.wavyTrail = true;
        firework.rocket.strobe = false;
        resetTrails(firework.rocket);
    }

    void explodeFirework(FIREWORK& firework)
    {
        firework.age = 0;
        firework.state = FIREWORK_EXPLODING;
        firework.rocket.alpha = 0.0;
        firework.burstRotation = frand(0.0, M_PI * 2.0);
        firework.burstSize = firework.burstType >= 15 ? frand(0.94, 1.06) : frand(0.88, 1.12);

        for (int32_t i = 0; i < firework.particleCount; i++)
        {
            FIREWORK_PARTICLE& particle = firework.particles[i];
            double speed = 0.0;
            double angle = frand(0.0, M_PI * 2.0);
            
            if (firework.burstType == 1)
            {
                //A clean ring with just enough jitter to avoid looking mechanical.
                angle = M_PI * 2.0 * i / firework.particleCount + frand(-0.018, 0.018);
                speed = frand(4.6, 5.3);
            }
            else if (firework.burstType == 2)
            {
                //Two interleaved shells create a dense chrysanthemum bloom.
                angle = M_PI * 2.0 * i / firework.particleCount + frand(-0.035, 0.035);
                speed = (i & 1) ? frand(2.6, 3.6) : frand(5.0, 6.2);
            }
            else if (firework.burstType == 3)
            {
                //Long-lived, slower sparks fall into a willow shape.
                speed = 2.0 + pow(frand(0.0, 1.0), 0.42) * 4.2;
            }
            else if (firework.burstType == 4)
            {
                //Three very large rainbow shells expand through each other.
                const int32_t layer = i % 3;
                angle = M_PI * 2.0 * i / firework.particleCount + frand(-0.025, 0.025);
                speed = 3.7 + layer * 1.65 + frand(-0.25, 0.25);
            }
            else if (firework.burstType == 6)
            {
                //A wide crown made from three concentric, hard-strobing shells.
                const int32_t layer = i % 3;
                angle = M_PI * 2.0 * i / firework.particleCount + frand(-0.02, 0.02);
                speed = 3.2 + layer * 1.85 + frand(-0.2, 0.2);
            }
            else if (firework.burstType == 8)
            {
                //Particles sharing a ray create the fine, comb-like branches of a silver palm.
                const int32_t rayCount = scaledRayCount(firework, 28);
                const int32_t ray = i % rayCount;
                const double distance = calcRayDistance(i, firework.particleCount, rayCount);
                angle = ray * M_PI * 2.0 / rayCount + frand(-0.012, 0.012);
                speed = 1.8 + distance * 6.2;
            }
            else if (firework.burstType == 9)
            {
                //A dense gold flower built from many bright radial streaks.
                const int32_t rayCount = scaledRayCount(firework, 32);
                const int32_t ray = i % rayCount;
                const double distance = calcRayDistance(i, firework.particleCount, rayCount);
                angle = ray * M_PI * 2.0 / rayCount + frand(-0.02, 0.02);
                speed = 1.2 + distance * 6.4;
            }
            else if (firework.burstType == 10)
            {
                //A broad multicolor peony with organic gaps between clusters.
                angle = frand(0.0, M_PI * 2.0);
                speed = 1.8 + pow(frand(0.0, 1.0), 0.38) * 5.8;
            }
            else if (firework.burstType == 11)
            {
                //Two cool-white layers reproduce the compact dahlia at the lower-left.
                angle = M_PI * 2.0 * i / firework.particleCount + frand(-0.03, 0.03);
                speed = (i & 1) ? frand(3.5, 4.4) : frand(5.7, 6.5);
            }
            else if (firework.burstType == 12)
            {
                //Long, sparse golden spokes with heavy luminous tails.
                const int32_t rayCount = scaledRayCount(firework, 24);
                const int32_t ray = i % rayCount;
                const double distance = calcRayDistance(i, firework.particleCount, rayCount);
                angle = ray * M_PI * 2.0 / rayCount + frand(-0.01, 0.01);
                speed = 2.1 + distance * 6.8;
            }
            else if (firework.burstType == 13)
            {
                //A loose shell quickly bends down into a field of crackling embers.
                angle = frand(0.0, M_PI * 2.0);
                speed = frand(2.0, 5.4);
            }
            else if (firework.burstType == 14)
            {
                //Colored outer ring surrounding a chaotic silver inner core.
                if (i < firework.particleCount / 2)
                {
                    angle = M_PI * 4.0 * i / firework.particleCount + frand(-0.018, 0.018);
                    speed = frand(6.0, 6.5);
                }
                else speed = frand(1.5, 4.7);
            }
            else if (firework.burstType == 15)
            {
                const double sector = M_PI * 2.0 / firework.particleCount;
                angle = firework.burstRotation + i * sector + frand(-0.42, 0.42) * sector;
                const double radial = 0.28 + 0.72 * pow(frand(0.0, 1.0), 0.58);
                speed = frand(6.5, 7.1) * radial;
            }
            else if (firework.burstType == 16)
            {
                const double sector = M_PI * 2.0 / firework.particleCount;
                angle = firework.burstRotation + i * sector + frand(-0.42, 0.42) * sector;
                const double radial = 0.31 + 0.69 * pow(frand(0.0, 1.0), 0.6);
                speed = frand(6.1, 6.8) * radial;
            }
            else if (firework.burstType == 17)
            {
                //Stratified random rays: one ray per angular sector keeps the global
                //silhouette round, while independent speed removes concentric shells.
                const double sector = M_PI * 2.0 / firework.particleCount;
                angle = firework.burstRotation + i * sector + frand(-0.42, 0.42) * sector;
                const double radial = 0.30 + 0.70 * pow(frand(0.0, 1.0), 0.58);
                speed = frand(7.1, 7.6) * radial;
            }
            else if (firework.burstType == 18)
            {
                //One continuous crown: evenly distributed directions preserve the
                //sphere while varied ray lengths avoid visible concentric rings.
                angle = firework.burstRotation + i * M_PI * 2.0 / firework.particleCount + frand(-0.012, 0.012);
                speed = 2.15 + pow(frand(0.0, 1.0), 0.82) * 5.65;
            }
            else if (firework.burstType == 19)
            {
                //Overlapping pink, blue and silver crowns. Their speed ranges overlap
                //so the color transition stays soft instead of forming hard rings.
                const int32_t layerCount = 3;
                const int32_t rayCount = firework.particleCount / layerCount;
                const int32_t layer = i / rayCount;
                const int32_t ray = i % rayCount;
                angle = firework.burstRotation + ray * M_PI * 2.0 / rayCount + layer * M_PI * 0.72 / rayCount + frand(-0.018, 0.018);
                if (layer == 0) speed = frand(2.0, 4.65);
                else if (layer == 1) speed = frand(3.85, 6.35);
                else speed = frand(5.55, 7.75);
            }
            else if (firework.burstType == 20)
            {
                //Four particles share each ray: a heavy red star near the core,
                //a hot orange bridge, then two violet-white outer filaments.
                const int32_t layerCount = 4;
                const int32_t rayCount = firework.particleCount / layerCount;
                const int32_t layer = i / rayCount;
                const int32_t ray = i % rayCount;
                angle = firework.burstRotation + ray * M_PI * 2.0 / rayCount
                      + layer * M_PI * 0.46 / rayCount + frand(-0.012, 0.012);
                if (layer == 0) speed = frand(3.0, 5.35);
                else if (layer == 1) speed = frand(4.8, 6.95);
                else if (layer == 2) speed = frand(6.65, 8.75);
                else speed = frand(8.35, 10.15);
            }
            else if (firework.burstType == 21)
            {
                //Sparse radial comets with varied lengths keep the shell open.
                const double sector = M_PI * 2.0 / firework.particleCount;
                angle = firework.burstRotation + i * sector + frand(-0.38, 0.38) * sector;
                const double radial = 0.38 + 0.62 * pow(frand(0.0, 1.0), 0.55);
                speed = frand(5.2, 7.8) * radial;
            }
            else if (firework.burstType == 22)
            {
                //Four sparse radial layers retain black gaps between the large heads.
                const int32_t innerCount = max(1, firework.particleCount / 5);
                const int32_t middleCount = max(1, firework.particleCount / 4);
                const int32_t outerMiddleCount = max(1, firework.particleCount / 4);
                const int32_t layer = i < innerCount ? 0 : i < innerCount + middleCount ? 1
                                    : i < innerCount + middleCount + outerMiddleCount ? 2 : 3;
                const int32_t layerStart = layer == 0 ? 0 : layer == 1 ? innerCount
                                         : layer == 2 ? innerCount + middleCount
                                                      : innerCount + middleCount + outerMiddleCount;
                const int32_t rayCount = layer == 0 ? innerCount : layer == 1 ? middleCount
                                       : layer == 2 ? outerMiddleCount : firework.particleCount - layerStart;
                const int32_t ray = i - layerStart;
                angle = firework.burstRotation + ray * M_PI * 2.0 / rayCount
                      + layer * M_PI * 0.64 / rayCount + frand(-0.005, 0.005);
                speed = layer == 0 ? frand(2.4, 3.1)
                      : layer == 1 ? frand(3.85, 4.75)
                      : layer == 2 ? frand(5.35, 6.35)
                                   : frand(7.0, 8.45);
            }
            else if (firework.burstType == 23)
            {
                //One dotted ring plus four staggered ray layers. Every ray keeps a
                //mostly straight axis, while uneven angles and lengths fill a sphere.
                const int32_t rayCount = firework.particleCount / 5;
                const bool dot = i < rayCount;
                const int32_t layer = dot ? -1 : (i - rayCount) / rayCount;
                const int32_t ray = dot ? i : (i - rayCount) % rayCount;
                const double rayJitter = sin((ray + 1) * 12.9898) * 0.019
                                       + sin((ray + 1) * 4.1414) * 0.008;
                const double depthWave = 0.5 + 0.5 * sin(ray * 2.399963 + 1.7);
                const double sphereDepth = 0.78 + 0.30 * sqrt(max(0.0, depthWave));
                angle = firework.burstRotation + ray * M_PI * 2.0 / rayCount
                      + rayJitter + (dot ? M_PI * 0.55 / rayCount : (layer - 1.5) * 0.004)
                      + frand(-0.003, 0.003);
                //A layer has one coherent speed band. The non-linear base speeds
                //separate colors radially; shared depth variation shapes the sphere.
                const double layerSpeed = dot ? 4.85 : layer == 0 ? 3.0 : layer == 1 ? 4.4
                                        : layer == 2 ? 6.25 : 8.7;
                speed = layerSpeed * sphereDepth * frand(0.97, 1.03);
            }
            else
            {
                //Bias speed outwards for a full but naturally uneven peony.
                speed = 1.3 + pow(frand(0.0, 1.0), 0.48) * 5.8;
            }

            if (firework.burstType <= 15)
            {
                const double sector = M_PI * 2.0 / firework.particleCount;
                angle = firework.burstRotation + i * sector + frand(-0.42, 0.42) * sector;
                const double radial = 0.26 + 0.74 * pow(frand(0.0, 1.0), 0.58);
                speed = frand(6.5, 7.2) * radial;
            }

            speed *= firework.scale * firework.burstSize;
            particle.position = firework.rocket.position;
            particle.velocity = { cos(angle) * speed, sin(angle) * speed };
            particle.alpha = 255.0;
            particle.alphaRate = (firework.burstType == 3) ? frand(1.8, 2.8) : (firework.burstType >= 4) ? frand(2.0, 3.5) : frand(2.8, 4.8);
            particle.sparkle = frand(0.0, M_PI * 2.0);
            particle.sparkleRate = frand(0.48, 1.15);
            particle.trailStrength = 0.64;
            particle.headCoreStrength = 0.7;
            particle.headHeatStrength = 0.88;
            particle.headGlowStrength = 0.0;
            particle.trailFadePower = 2.0;
            particle.waveAmplitude = 0.0;
            particle.tipColor = 0;
            particle.headGlowColor = 0;
            particle.trailLength = 9 + rand() % 4;
            particle.trailWidth = 1;
            particle.trailEndWidth = 1;
            particle.taperedHead = false;
            particle.roundedHead = false;
            particle.solidHead = false;
            particle.wavyTrail = false;
            particle.color = (i & 1) ? firework.primaryColor : firework.secondaryColor;

            if (firework.burstType == 4) particle.color = palette[(i / 7 + rand() % 3) % FIREWORK_COLOR_COUNT];
            else if (firework.burstType == 6 && i % 3 == 0) particle.color = RGB_WHITE;

            if (firework.burstType == 8)
            {
                particle.color = (i % 6 == 0) ? firework.secondaryColor : firework.primaryColor;
                particle.trailStrength = 1.15;
                particle.alphaRate = frand(1.8, 2.7);
            }
            else if (firework.burstType == 9)
            {
                particle.color = (i % 7 == 0) ? RGB_WHITE : firework.primaryColor;
                particle.trailStrength = 0.95;
                particle.alphaRate = frand(2.0, 3.0);
            }
            else if (firework.burstType == 10)
            {
                particle.color = palette[(i / 5 + rand() % 4) % (FIREWORK_COLOR_COUNT - 1)];
                particle.trailStrength = 0.48;
            }
            else if (firework.burstType == 11)
            {
                particle.color = (i & 1) ? firework.primaryColor : firework.secondaryColor;
                particle.trailStrength = 0.58;
            }
            else if (firework.burstType == 12)
            {
                particle.color = (i % 8 == 0) ? firework.secondaryColor : firework.primaryColor;
                particle.trailStrength = 1.3;
                particle.alphaRate = frand(1.6, 2.5);
            }
            else if (firework.burstType == 13)
            {
                particle.color = (i & 1) ? firework.primaryColor : firework.secondaryColor;
                particle.trailStrength = 0.34;
                particle.alphaRate = frand(1.9, 3.2);
            }
            else if (firework.burstType == 14)
            {
                particle.color = i < firework.particleCount / 2 ? firework.primaryColor : RGB_WHITE;
                particle.trailStrength = i < firework.particleCount / 2 ? 0.5 : 0.9;
            }
            else if (firework.burstType == 15)
            {
                const int32_t colorChance = rand() % 100;
                particle.color = colorChance < 12 ? RGB_WHITE : colorChance < 50 ? firework.secondaryColor : firework.primaryColor;
                particle.trailStrength = frand(0.72, 1.15);
                particle.alphaRate = frand(1.06, 1.74);
                particle.sparkleRate = frand(0.12, 0.3);
                particle.trailLength = 5 + rand() % 6;
                particle.trailWidth = rand() % 100 < 25 ? 2 : 3;
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 16)
            {
                const int32_t colorChance = rand() % 100;
                particle.color = colorChance < 10 ? RGB_WHITE : colorChance < 45 ? firework.secondaryColor : colorChance < 75 ? mixColor(firework.secondaryColor, firework.primaryColor, 0.5) : firework.primaryColor;
                particle.trailStrength = frand(0.72, 1.18);
                particle.alphaRate = frand(1.62, 2.52);
                particle.sparkleRate = frand(0.28, 0.62);
                particle.trailLength = 5 + rand() % 6;
                particle.trailWidth = rand() % 100 < 28 ? 2 : 3;
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 17)
            {
                particle.color = mixColor(0x87939e, RGB_WHITE, frand(0.25, 0.95));
                particle.trailStrength = frand(0.72, 1.18);
                particle.alphaRate = frand(1.51, 2.30);
                particle.sparkleRate = frand(0.18, 0.42);
                particle.trailLength = 6 + rand() % 5;
                particle.trailWidth = rand() % 100 < 30 ? 2 : 3;
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 18)
            {
                const int32_t colorChance = rand() % 100;
                particle.color = colorChance < 10 ? 0xfff0bd : colorChance < 68 ? firework.primaryColor : firework.secondaryColor;
                particle.tipColor = RGB_WHITE;
                particle.trailStrength = frand(0.94, 1.12);
                particle.alphaRate = frand(1.45, 2.15);
                particle.headCoreStrength = 0.42;
                particle.sparkleRate = frand(0.2, 0.48);

                //Long radial tails are the signature of this chrysanthemum. The
                //quadratic trail fade makes their old end dissolve into the dark.
                particle.trailLength = min(MAX_TRAIL_LENGTH, 13 + rand() % 6);
                particle.trailWidth = 4;
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 19)
            {
                const int32_t rayCount = firework.particleCount / 3;
                const int32_t layer = i / rayCount;
                const int32_t colorChance = rand() % 100;
                if (layer == 0) particle.color = colorChance < 22 ? 0xffd2ed : colorChance < 66 ? 0xff4fac : 0xd746c9;
                else if (layer == 1) particle.color = colorChance < 18 ? 0x9db8ff : colorChance < 65 ? 0x3157bd : 0x536ee0;
                else particle.color = colorChance < 25 ? RGB_WHITE : colorChance < 62 ? 0xc1c8d2 : 0x777f8c;
                particle.trailStrength = layer == 0 ? 1.12 : layer == 1 ? 0.92 : 0.72;
                particle.alphaRate = layer == 0 ? frand(1.45, 2.05) : layer == 1 ? frand(1.35, 1.95) : frand(1.25, 1.85);
                particle.headCoreStrength = layer == 0 ? 0.55 : layer == 1 ? 0.34 : 0.42;
                particle.sparkleRate = frand(0.18, 0.46);
                particle.trailLength = layer == 0 ? 8 + rand() % 4 : layer == 1 ? 10 + rand() % 5 : 12 + rand() % 5;
                particle.trailWidth = layer == 2 ? 2 : 3;
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 20)
            {
                const int32_t rayCount = firework.particleCount / 4;
                const int32_t layer = i / rayCount;
                const int32_t colorChance = rand() % 100;
                if (layer == 0)
                {
                    particle.color = colorChance < 22 ? 0xff7130 : colorChance < 72 ? firework.primaryColor : 0xd00024;
                    particle.tipColor = colorChance < 30 ? 0xfff1c4 : 0xff8a38;
                }
                else if (layer == 1)
                {
                    particle.color = colorChance < 24 ? 0xffcf72 : colorChance < 62 ? 0xff5338 : 0xf6e8ff;
                    particle.tipColor = RGB_WHITE;
                }
                else if (layer == 2)
                {
                    particle.color = colorChance < 18 ? 0xf4dcff : colorChance < 70 ? firework.secondaryColor : 0x6735cf;
                    particle.tipColor = colorChance < 38 ? RGB_WHITE : 0xdcc7ff;
                }
                else
                {
                    particle.color = colorChance < 28 ? 0xf8efff : colorChance < 72 ? 0xb57aff : 0x7040dc;
                    particle.tipColor = RGB_WHITE;
                }
                particle.trailStrength = layer == 0 ? frand(0.92, 1.18) : layer == 1 ? frand(0.72, 0.98)
                                       : layer == 2 ? frand(0.58, 0.82) : frand(0.45, 0.68);
                particle.alphaRate = layer == 0 ? frand(1.55, 2.15) : layer == 1 ? frand(1.45, 2.05)
                                    : layer == 2 ? frand(1.25, 1.85) : frand(1.1, 1.65);
                particle.headCoreStrength = layer == 0 ? 0.66 : layer == 1 ? 0.54 : layer == 2 ? 0.42 : 0.32;
                particle.sparkleRate = frand(0.16, 0.4);
                particle.trailLength = layer == 0 ? 6 + rand() % 4 : layer == 1 ? 8 + rand() % 4
                                     : layer == 2 ? 12 + rand() % 5 : 14 + rand() % 5;
                particle.trailWidth = layer == 0 ? 3 : layer == 1 ? 2 : 1;
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 21)
            {
                const int32_t headColor = rand() % 100;
                particle.color = headColor < 14 ? 0xffdb68 : headColor < 84 ? firework.primaryColor : firework.secondaryColor;
                particle.tipColor = headColor < 20 ? 0xfff6de : RGB_WHITE;
                particle.headGlowColor = headColor < 24 ? 0xff344d
                                       : headColor < 43 ? 0x55ff83
                                       : headColor < 58 ? 0xc75cff
                                       : headColor < 72 ? 0x72cfff
                                       : 0xffa52f;
                particle.headGlowStrength = frand(0.92, 1.16);
                particle.headCoreStrength = frand(0.72, 0.94);
                particle.trailStrength = frand(0.9, 1.14);
                particle.trailFadePower = frand(2.35, 2.8);
                particle.alphaRate = frand(2.0, 3.0);
                particle.sparkleRate = frand(0.12, 0.3);
                particle.trailLength = 4 + rand() % 4;
                particle.trailWidth = rand() % 100 < 42 ? 6 : 5;
                particle.trailEndWidth = 1;
                particle.taperedHead = false;
                particle.roundedHead = true;
            }
            else if (firework.burstType == 22)
            {
                const int32_t innerCount = max(1, firework.particleCount / 5);
                const int32_t middleCount = max(1, firework.particleCount / 4);
                const int32_t outerMiddleCount = max(1, firework.particleCount / 4);
                const int32_t layer = i < innerCount ? 0 : i < innerCount + middleCount ? 1
                                    : i < innerCount + middleCount + outerMiddleCount ? 2 : 3;
                particle.color = layer == 0 ? 0xffbc38 : layer == 1 ? 0xffc84a : layer == 2 ? 0xffd05a
                               : (rand() % 100 < 22 ? 0xffe28a : firework.primaryColor);
                particle.tipColor = 0xffffdc;
                particle.headGlowColor = layer == 0 ? 0xffb52d : layer == 1 ? 0xffba31
                                           : layer == 2 ? 0xffbf36 : 0xffc13b;
                particle.headGlowStrength = layer == 0 ? frand(1.32, 1.58)
                                              : layer == 1 ? frand(1.35, 1.62)
                                              : layer == 2 ? frand(1.38, 1.67) : frand(1.42, 1.72);
                particle.headCoreStrength = frand(0.86, 1.0);
                particle.trailStrength = layer == 0 ? 0.72 : layer == 1 ? frand(0.88, 1.04)
                                       : layer == 2 ? frand(0.98, 1.16) : frand(1.08, 1.28);
                particle.trailFadePower = layer == 0 ? 2.8 : layer == 1 ? frand(1.95, 2.25)
                                        : layer == 2 ? frand(1.65, 1.95) : frand(1.45, 1.75);
                particle.alphaRate = layer == 0 ? frand(2.2, 3.0)
                                   : layer == 1 ? frand(1.95, 2.55)
                                   : layer == 2 ? frand(1.78, 2.42) : frand(1.65, 2.35);
                particle.sparkleRate = frand(0.1, 0.24);
                particle.trailLength = layer == 0 ? 1 : layer == 1 ? 3 + rand() % 2
                                     : layer == 2 ? 4 + rand() % 3 : 6 + rand() % 4;
                particle.trailWidth = layer == 0 ? 2 : layer == 1 ? 4 : layer == 2 ? 4
                                    : (rand() % 100 < 35 ? 5 : 4);
                particle.trailEndWidth = 1;
                particle.taperedHead = true;
                particle.roundedHead = true;
                particle.solidHead = true;
            }
            else if (firework.burstType == 23)
            {
                const int32_t rayCount = firework.particleCount / 5;
                const bool dot = i < rayCount;
                const int32_t layer = dot ? -1 : (i - rayCount) / rayCount;
                particle.color = dot ? 0xffe84a : layer == 0 ? 0xff3d24 : layer == 1 ? 0xff8a28
                               : layer == 2 ? 0xd9e83b : 0x45d878;
                particle.tipColor = 0;
                particle.headGlowColor = dot ? 0xffd52e : 0;
                particle.headGlowStrength = dot ? frand(0.48, 0.68) : 0.0;
                particle.headCoreStrength = dot ? 0.16 : 0.0;
                particle.headHeatStrength = dot ? 0.12 : 0.06;
                particle.trailStrength = dot ? 0.0 : layer == 0 ? 0.92 : layer == 1 ? 0.94
                                       : layer == 2 ? 0.98 : 1.04;
                particle.trailFadePower = dot ? 3.0 : frand(1.35, 1.65);
                particle.alphaRate = dot ? frand(1.7, 2.2) : frand(1.05, 1.5);
                particle.sparkleRate = dot ? frand(0.16, 0.32) : frand(0.08, 0.2);
                particle.trailLength = dot ? 1 : 7 + rand() % 4;
                particle.trailWidth = dot ? 1 : (rand() % 100 < 62 ? 3 : 2);
                particle.trailEndWidth = 1;
                particle.taperedHead = !dot;
                particle.roundedHead = true;
                particle.solidHead = false;
            }

            const int32_t baseSize = frand(0.0, 1.0) > ((firework.burstType >= 4) ? 0.58 : 0.78) ? 2 : 1;
            
            //More particles provide fullscreen detail; keep sparks fine instead of enlarging them.
            particle.size = baseSize;
            if (firework.burstType == 15)
            {
                const int32_t sizeChance = rand() % 100;
                particle.size = sizeChance < 14 ? 3 : sizeChance < 82 ? 2 : 1;
            }
            else if (firework.burstType == 16)
            {
                const int32_t sizeChance = rand() % 100;
                particle.size = sizeChance < 15 ? 3 : sizeChance < 80 ? 2 : 1;
            }
            if (firework.burstType == 17)
            {
                const int32_t sizeChance = rand() % 100;
                particle.size = sizeChance < 15 ? 3 : sizeChance < 80 ? 2 : 1;
            }
            else if (firework.burstType == 18)
            {
                particle.size = rand() % 100 < 20 ? 3 : 2;
            }
            else if (firework.burstType == 19)
            {
                const int32_t rayCount = firework.particleCount / 3;
                const int32_t layer = i / rayCount;
                particle.size = layer == 0 ? (rand() % 100 < 24 ? 3 : 2) : layer == 1 ? 2 : (rand() % 100 < 28 ? 2 : 1);
            }
            else if (firework.burstType == 20)
            {
                const int32_t rayCount = firework.particleCount / 4;
                const int32_t layer = i / rayCount;
                particle.size = layer == 0 ? (rand() % 100 < 38 ? 4 : 3)
                              : layer == 1 ? (rand() % 100 < 35 ? 3 : 2)
                              : layer == 2 ? (rand() % 100 < 18 ? 2 : 1)
                              : 1;
            }
            else if (firework.burstType == 21)
            {
                particle.size = rand() % 100 < 42 ? 5 : 4;
            }
            else if (firework.burstType == 22)
            {
                const int32_t innerCount = max(1, firework.particleCount / 5);
                const int32_t middleCount = max(1, firework.particleCount / 4);
                const int32_t outerMiddleCount = max(1, firework.particleCount / 4);
                const int32_t layer = i < innerCount ? 0 : i < innerCount + middleCount ? 1
                                    : i < innerCount + middleCount + outerMiddleCount ? 2 : 3;
                particle.size = layer == 0 ? (rand() % 100 < 30 ? 5 : 4)
                              : layer == 1 ? (rand() % 100 < 38 ? 5 : 4)
                              : layer == 2 ? (rand() % 100 < 43 ? 5 : 4)
                                           : (rand() % 100 < 48 ? 5 : 4);
            }
            else if (firework.burstType == 23)
            {
                const int32_t rayCount = firework.particleCount / 5;
                particle.size = i < rayCount ? (rand() % 100 < 22 ? 2 : 1) : 1;
            }

            if (firework.burstType <= 15)
            {
                particle.trailLength = 6 + rand() % 7;
                const int32_t widthChance = rand() % 100;
                particle.trailWidth = widthChance < 20 ? 1 : widthChance < 75 ? 2 : 3;
                particle.trailEndWidth = 1;
                const int32_t sizeChance = rand() % 100;
                particle.size = sizeChance < 20 ? 1 : sizeChance < 85 ? 2 : 3;
            }

            if (firework.burstType == 4)
            {
                particle.trailWidth = rand() % 100 < 25 ? 4 : 3;
                particle.trailEndWidth = 2;
                particle.size = rand() % 100 < 72 ? 3 : 2;
            }

            particle.taperedHead = firework.burstType != 21
                                && !(firework.burstType == 23 && i < firework.particleCount / 5);
            particle.roundedHead = true;
            particle.strobe = (firework.burstType == 6) || (firework.burstType == 13) || ((firework.burstType >= 4) && (firework.burstType < 15) && (i % 7) == 0);
            resetTrails(particle);
        }
    }

    void blendFillEllipse(double centerX, double centerY, double directionX, double directionY, double halfLength, double halfWidth, uint32_t color)
    {
        const double normalX = -directionY;
        const double normalY = directionX;
        const int32_t radius = int32_t(ceil(max(halfLength, halfWidth) + 1.0));
        const int32_t minX = int32_t(floor(centerX)) - radius;
        const int32_t maxX = int32_t(ceil(centerX)) + radius;
        const int32_t minY = int32_t(floor(centerY)) - radius;
        const int32_t maxY = int32_t(ceil(centerY)) + radius;
        const double edgeScale = min(halfLength, halfWidth);

        for (int32_t y = minY; y <= maxY; y++)
        {
            for (int32_t x = minX; x <= maxX; x++)
            {
                const double offsetX = x - centerX;
                const double offsetY = y - centerY;
                const double along = (offsetX * directionX + offsetY * directionY) / halfLength;
                const double across = (offsetX * normalX + offsetY * normalY) / halfWidth;
                const double distance = sqrt(along * along + across * across);
                const double coverage = clamp((1.0 - distance) * edgeScale + 0.5, 0.0, 1.0);
                if (coverage > 0.0) putPixel(x, y, fadeColor(color, coverage * 255.0), BLEND_MODE_ADD);
            }
        }
    }

    void drawParticles(const FIREWORK_PARTICLE& particle, uint32_t color, int32_t age)
    {
        if (particle.alpha <= 0.0) return;

        double brightness = particle.alpha;
        if (particle.alpha < 185.0)
        {
            const double twinkle = 0.62 + 0.38 * sin(age * particle.sparkleRate + particle.sparkle);
            brightness *= twinkle * twinkle;
        }

        if (particle.strobe && particle.alpha < 225.0)
        {
            const double flash = sin(age * particle.sparkleRate * 1.9 + particle.sparkle);
            brightness *= flash > 0.15 ? 1.35 : 0.08;
        }

        //Fresh sparks have a white-hot core which cools into their shell color.
        const double heat = clamp((particle.alpha - 185.0) / 70.0, 0.0, 1.0);
        const uint32_t hotColor = particle.tipColor != 0 ? particle.tipColor
                                                        : mixColor(color, RGB_WHITE, heat * particle.headHeatStrength);
        const int32_t x = int32_t(particle.position.x);
        const int32_t y = int32_t(particle.position.y);
        const uint32_t headColor = boostColor(fadeColor(hotColor, brightness), 1.15);
        const double magnitude = sqrt(particle.velocity.x * particle.velocity.x + particle.velocity.y * particle.velocity.y);
        const double directionX = magnitude > 0.0 ? particle.velocity.x / magnitude : 1.0;
        const double directionY = magnitude > 0.0 ? particle.velocity.y / magnitude : 0.0;
        const double halfLength = particle.taperedHead ? 1.4 + particle.size * 1.35 : max(1.0, double(particle.size));
        const double halfWidth = particle.taperedHead ? 0.95 + particle.size * 0.78 : max(1.0, double(particle.size));
        if (particle.headGlowStrength > 0.0 && particle.headGlowColor != 0)
        {
            const double outerGlowSize = particle.size * (particle.solidHead ? 3.8 : 2.45);
            const double innerGlowSize = particle.size * (particle.solidHead ? 2.35 : 1.7);
            const double outerGlowStrength = particle.solidHead ? 0.58 : 0.34;
            addGlowEllipse(particle.position.x, particle.position.y, 1.0, 0.0,
                           outerGlowSize, outerGlowSize,
                           fadeColor(particle.headGlowColor, brightness * particle.headGlowStrength * outerGlowStrength), 16);
            addGlowEllipse(particle.position.x, particle.position.y, 1.0, 0.0,
                           innerGlowSize, innerGlowSize,
                           fadeColor(particle.headGlowColor, brightness * particle.headGlowStrength), 12);
        }
        addGlowEllipse(particle.position.x, particle.position.y, directionX, directionY,
                       halfLength, halfWidth, headColor, 10, particle.solidHead);
        const double coreSize = particle.headGlowStrength > 0.0 ? max(1.0, particle.size * 0.36) : 0.9;
        addGlowEllipse(x, y, 1.0, 0.0, coreSize, coreSize,
                       fadeColor(RGB_WHITE, brightness * particle.headCoreStrength), 6);

        //A dim halo gives bright sparks a small bloom without using SDL textures.
        if (brightness > 150.0 && !particle.taperedHead)
        {
            const uint32_t halo = fadeColor(color, brightness * 0.18);
            addGlowEllipse(x, y, 1.0, 0.0, 3.2, 3.2, halo, 8);
        }

        int32_t previousX = x;
        int32_t previousY = y;

        for (int32_t i = 0; i < particle.trailLength; i++)
        {
            const double trailScale = double(particle.trailLength - i) / particle.trailLength;
            const double fade = brightness * pow(trailScale, particle.trailFadePower) * particle.trailStrength;
            int32_t trailX = int32_t(particle.trail[i].x);
            int32_t trailY = int32_t(particle.trail[i].y);
            if (particle.wavyTrail)
            {
                const double velocityLength = sqrt(particle.velocity.x * particle.velocity.x + particle.velocity.y * particle.velocity.y);
                const double normalX = velocityLength > 0.0 ? -particle.velocity.y / velocityLength : 1.0;
                const double normalY = velocityLength > 0.0 ? particle.velocity.x / velocityLength : 0.0;
                const double tailPosition = double(i + 1) / particle.trailLength;
                const double amplitude = 0.35 + tailPosition * (particle.waveAmplitude - 0.35);
                const double wave = sin(i * 0.86 - age * 0.48 + particle.sparkle) * amplitude;
                trailX = int32_t(round(particle.trail[i].x + normalX * wave));
                trailY = int32_t(round(particle.trail[i].y + normalY * wave));
            }

            const uint32_t trailColor = boostColor(fadeColor(color, fade), 1.3);
            const int32_t trailWidth = particle.taperedHead
                    ? max(particle.trailEndWidth, particle.trailWidth >= 5
                    ? (trailScale > 0.72 ? 5 : trailScale > 0.42 ? 4 : trailScale > 0.2 ? 3 : 1)
                    : particle.trailWidth >= 4
                    ? (trailScale > 0.66 ? 4 : trailScale > 0.33 ? 3 : 1)
                    : particle.trailWidth == 3
                    ? (trailScale > 0.66 ? 3 : trailScale > 0.33 ? 2 : 1)
                    : particle.trailWidth == 2
                    ? (trailScale > 0.55 ? 2 : 1)
                    : 1) : max(particle.trailEndWidth,
                               int32_t(ceil(particle.trailWidth * (0.18 + 0.82 * trailScale))));

            addTrailSegment(previousX, previousY, trailX, trailY, double(trailWidth), trailColor);
            previousX = trailX;
            previousY = trailY;
        }
    }

    void drawFirework(const FIREWORK& firework)
    {
        if (firework.state == FIREWORK_LAUNCHING)
        {
            drawParticles(firework.rocket, firework.rocket.color, firework.age);
            addGlowEllipse(firework.rocket.position.x, firework.rocket.position.y,
                           1.0, 0.0, 1.0, 1.0, RGB_WHITE, 6);
        }
        else if (firework.state == FIREWORK_EXPLODING)
        {
            const bool grandBurst = (firework.burstType >= 4);
            if (firework.burstType == 19 && firework.age < 24)
            {
                const double life = 1.0 - firework.age / 24.0;
                const int32_t coreX = int32_t(firework.rocket.position.x);
                const int32_t coreY = int32_t(firework.rocket.position.y);
                const int32_t glowRadius = max(2, int32_t((2.5 + life * 7.0) * firework.scale));
                addGlowEllipse(coreX, coreY, 1.0, 0.0, glowRadius, glowRadius,
                               fadeColor(0xff3fae, 155.0 * life), 16);
                addGlowEllipse(coreX, coreY, 1.0, 0.0, max(1, glowRadius / 2), max(1, glowRadius / 2),
                               fadeColor(0xffb8e4, 235.0 * life), 12);
                addGlowEllipse(coreX, coreY, 1.0, 0.0, 1.0, 1.0,
                               fadeColor(RGB_WHITE, 255.0 * life), 6);
            }
            else if (firework.burstType == 20 && firework.age < 18)
            {
                const double life = 1.0 - firework.age / 18.0;
                const double glowRadius = (3.0 + life * 8.0) * firework.scale;
                addGlowEllipse(firework.rocket.position.x, firework.rocket.position.y,
                               1.0, 0.0, glowRadius, glowRadius,
                               fadeColor(0xd80028, 150.0 * life), 16);
                addGlowEllipse(firework.rocket.position.x, firework.rocket.position.y,
                               1.0, 0.0, glowRadius * 0.42, glowRadius * 0.42,
                               fadeColor(0xff6038, 220.0 * life), 12);
            }

            if (firework.age < (grandBurst ? 6 : 4))
            {
                const int32_t radius = int32_t((grandBurst ? 15 - firework.age * 2 : 8 - firework.age * 2) * firework.scale);
                addGlowEllipse(firework.rocket.position.x, firework.rocket.position.y, 1.0, 0.0,
                               radius, radius, fadeColor(0xfff4c8, 190.0 - firework.age * 28.0), 16);
            }

            if (grandBurst && firework.age > 1 && firework.age < 14)
                addGlowRing(firework.rocket.position.x, firework.rocket.position.y,
                            firework.age * 3 * firework.scale, 1.4,
                            fadeColor(firework.primaryColor, 150.0 - firework.age * 9.0));

            for (int32_t i = 0; i < firework.particleCount; i++)
            {
                uint32_t color = firework.particles[i].color;
                if (firework.burstType == 4 || firework.burstType == 10)
                {
                    const uint32_t shiftingColor = palette[(i / 9 + firework.age / 7) % FIREWORK_COLOR_COUNT];
                    color = mixColor(color, shiftingColor, 0.42);
                }
                else if (firework.burstType == 23)
                {
                    const int32_t rayCount = firework.particleCount / 5;
                    const bool dot = i < rayCount;
                    if (dot && firework.age >= COLOR_SHELL_STATE3_AGE) continue;

                    if (dot)
                        color = firework.age < COLOR_SHELL_STATE2_AGE ? 0xffe43d : 0xb8ff42;
                    else
                    {
                        const int32_t layer = (i - rayCount) / rayCount;
                        if (firework.age < COLOR_SHELL_STATE2_AGE)
                            color = layer == 0 ? 0xff1d28 : layer == 1 ? 0xff7924
                                  : layer == 2 ? 0xffd832 : 0x43d84f;
                        else if (firework.age < COLOR_SHELL_STATE3_AGE)
                            color = layer == 0 ? 0xff9a28 : layer == 1 ? 0xe7df36
                                  : layer == 2 ? 0x45dc62 : 0x348eff;
                        else
                            color = layer == 0 ? 0xffad2d : layer == 1 ? 0xcde13b
                                  : layer == 2 ? 0x3ed475 : 0x5264ff;
                    }
                }
                drawParticles(firework.particles[i], color, firework.age);
            }
        }
    }

    void updateFirework(int32_t index, int32_t width, int32_t height)
    {
        FIREWORK& firework = fireworkList[index];

        if (firework.state == FIREWORK_WAITING)
        {
            if (--firework.waitFrames <= 0) launchFirework(firework, width, height);
            return;
        }

        if (firework.state == FIREWORK_LAUNCHING)
        {
            firework.age++;
            const FIREWORK_VECTOR previous = firework.rocket.position;
            firework.rocket.position.x += firework.rocket.velocity.x;
            firework.rocket.position.y += firework.rocket.velocity.y;
            firework.rocket.velocity.y += 0.16 * firework.scale;
            moveTrails(firework.rocket, previous);
            if (firework.rocket.velocity.y >= -0.15 || firework.rocket.position.y < height * 0.12) explodeFirework(firework);
            return;
        }

        firework.age++;
        bool alive = false;
        for (int32_t i = 0; i < firework.particleCount; i++)
        {
            FIREWORK_PARTICLE& particle = firework.particles[i];
            if (firework.burstType == 23 && firework.age == COLOR_SHELL_STATE3_AGE
                && i >= firework.particleCount / 5)
            {
                const int32_t rayCount = firework.particleCount / 5;
                const int32_t layer = (i - rayCount) / rayCount;
                const double expansion = layer == 0 ? 1.05 : layer == 1 ? 1.12 : layer == 2 ? 1.23 : 1.36;
                particle.velocity.x *= expansion;
                particle.velocity.y *= expansion;
            }
            particle.alpha = max(0.0, particle.alpha - particle.alphaRate);
            alive |= particle.alpha > 0.0;

            const FIREWORK_VECTOR previous = particle.position;
            particle.position.x += particle.velocity.x;
            particle.position.y += particle.velocity.y;
            particle.velocity.x *= firework.drag;
            particle.velocity.y = particle.velocity.y * firework.drag + firework.gravity;
            moveTrails(particle, previous);
        }

        if (!alive) scheduleFirework(index, int32_t(frand(18.0, 90.0)));
    }
}

void fireworksDemo()
{
    if (!initScreen(640, 480, 32, 0, "Fireworks - Press Enter for next demo", 0, 1)) return;
    setRenderVSync(1);

    const int width = getDrawBufferWidth();
    const int height = getDrawBufferHeight();

    burstSelection = RANDOM_BURST;
    activeCount = MAX_FIREWORK_COUNT;
    fireworkVertices.clear();
    fireworkIndices.clear();
    fireworkVertices.reserve(450000);
    fireworkIndices.reserve(700000);

    for (int32_t i = 0; i < activeCount; i++) scheduleFirework(i, 1 + i * 9 + rand() % 12);

    do {
        readKeys();
        selectBurst();
        fireworkVertices.clear();
        fireworkIndices.clear();
        for (int32_t i = 0; i < activeCount; i++)
        {
            drawFirework(fireworkList[i]);
            updateFirework(i, width, height);
        }

        renderGeometry(fireworkVertices.data(), int32_t(fireworkVertices.size()),
                       fireworkIndices.data(), int32_t(fireworkIndices.size()),
                       SDL_BLENDMODE_ADD, false);
        delay(FPS_60);
    } while (!finished(SDL_SCANCODE_RETURN));

    cleanup();
}

void gfxEffects()
{
    /*juliaSet();
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
    crossFading();*/
    fireworksDemo();
    rayCasting();
    runRayCasting();
}
