#include <vector>
#include <algorithm>
#include "gfxlib.h"

#define SCR_WIDTH	640
#define SCR_HEIGHT	480 

void juliaSet()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Julia-Set")) return;

    //get raw pixels data
    int32_t i = 0, cwidth = 0, cheight = 0;
    uint32_t* pbuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;

    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    
    pixels[0] = pbuff;
    for (i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    const int32_t iterations = 255;
    const double cre = -0.7, cim = 0.27015;
    
    const double xscale = 3.0 / cwidth;
    const double yscale = 2.0 / cheight;

    const double scale = max(xscale, yscale);
    const double mx = -0.5 * cwidth * scale;
    const double my = -0.5 * cheight * scale;

    /*============== use FMA version below ==========
    for (int32_t y = 0; y < cheight; y++)
    {
        const double y0 = y * scale + my;
        for (int32_t x = 0; x < cwidth; x++)
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
            pixels[y][x] = hsv2rgb(0xFF * i / iterations, 0xFF, (i < iterations) ? 0xFF : 0);
        }
    }
    ================================================*/

    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = 0; y < cheight; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < cwidth; x += 4)
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
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 13));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_test_all_ones(masks)) break;

                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                const __m256d t = _mm256_add_pd(x1, x1);
                y1 = _mm256_fmadd_pd(t, y1, xim);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            //extract iteration position for each pixel
            int32_t it[8] = { 0 };
            _mm256_storeu_epi32(it, iters);

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

const uint32_t firePalette[256] = {
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
    int32_t cwidth = 0, cheight = 0;
    uint32_t* frameBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!frameBuff) return;

    while (!finished(SDL_SCANCODE_RETURN))
    {
        for (i = cwidth + 1; i < (cheight - 1) * cwidth - 1; i++)
        {
            //Average the eight neighbours.
            sum =
                prevBuff[i - cwidth - 1] +
                prevBuff[i - cwidth    ] +
                prevBuff[i - cwidth + 1] +
                prevBuff[i - 1] +
                prevBuff[i + 1] +
                prevBuff[i + cwidth - 1] +
                prevBuff[i + cwidth    ] +
                prevBuff[i + cwidth + 1];

            avg = uint8_t(sum >> 3);

            //"Cool" the pixel if the two bottom bits of the
            // sum are clear (somewhat random). For the bottom
            // rows, cooling can overflow, causing "sparks".
            if (!(sum & 3) && (avg > 0 || i >= (cheight - 4) * cwidth)) avg--;
            dstBuff[i] = avg;
        }

        //Copy back and scroll up one row.
        //The bottom row is all zeros, so it can be skipped.
        for (i = 0; i < (cheight - 2) * cwidth; i++) prevBuff[i] = dstBuff[i + cwidth];

        //Remove dark pixels from the bottom rows (except again the
        // bottom row which is all zeros).
        for (i = (cheight - 7) * cwidth; i < (cheight - 1) * cwidth; i++)
        {
            if (dstBuff[i] < 15) dstBuff[i] = 22 - dstBuff[i];
        }

        //Copy to frame buffer and map to RGBA, scrolling up one row.
        for (i = 0; i < (cheight - 2) * cwidth; i++)
        {
            frameBuff[i] = firePalette[dstBuff[i + cwidth]];
        }

        //Update the texture and render it.
        render();
        delay(FPS_30);
    }

    cleanup();
}

static uint32_t palette[SIZE_256] = { 0 };
static uint32_t fires[SCR_HEIGHT][SCR_WIDTH] = { 0 };

void fireDemo2()
{
    //set up the screen
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Fire")) return;

    //get the drawing buffer
    int32_t cwidth = 0, cheight = 0;
    uint32_t* pbuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;

    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
    for (int32_t i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    //validation screen height
    if (cheight < 1) return;

    //the time of this and the previous frame, for timing
    uint32_t time = getTime(), oldTime = 0;

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
        palette[x] = hsl2rgb(x / 3, 255, min(255, x * 2));
    }

    //start the loop (one frame per loop)
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //timing: set to maximum 50 milliseconds per frame = 20 frames per second
        oldTime = time;
        waitFor(oldTime, 50);
        time = getTime();

        //randomize the bottom row of the fire buffer
        for (int32_t x = 0; x < cwidth; x++) fires[cheight - 1][x] = abs(32768 + rand()) % 256;

        //do the fire calculations for every pixel, from top to bottom
        for (int32_t y = 0; y < cheight - 1; y++)
        {
            for (int32_t x = 0; x < cwidth; x++)
            {
                fires[y][x] = ((
                fires[(y + 1) % cheight][(x - 1 + cwidth) % cwidth] +
                fires[(y + 1) % cheight][(x             ) % cwidth] +
                fires[(y + 1) % cheight][(x + 1         ) % cwidth] +
                fires[(y + 2) % cheight][(x             ) % cwidth]) * 32 / 129) % 256;
            }
        }

        //set the drawing buffer to the fire buffer, using the palette colors
        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++) pixels[y][x] = palette[fires[y][x]];
        }

        //draw the buffer
        render();
    }

    free(pixels);
    cleanup();
}

///////////////////////////////RAY CASTING/////////////////////////////////

#define TEXTURE_WIDTH		64
#define TEXTURE_HEIGHT		64
#define MAP_WIDTH			24
#define MAP_HEIGHT			24
#define NUM_SPRITES			19

int32_t miniMap[MAP_WIDTH][MAP_HEIGHT] =
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

SPRITE sprite[NUM_SPRITES] =
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

//1D Zbuffer
double zBuffer[SCR_WIDTH] = { 0 };

//arrays used to sort the sprites
int32_t spriteOrder[NUM_SPRITES] = { 0 };
double spriteDistance[NUM_SPRITES] = { 0 };

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
    double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane
    
    uint32_t time = 0, oldTime = 0;

    int32_t tw = 0, th = 0, i = 0;
    uint32_t* pbuffs[11] = { 0 };

    const char* fname[] = {
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
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Raycasting [Fast version] -- Keys: Use arrows to move your works!")) return;

    //maxtrix textures
    uint32_t** textures[11] = { 0 };

    //load some textures
    for (i = 0; i < 11; i++)
    {
        if (!loadTexture(&pbuffs[i], &tw, &th, fname[i])) return;
        textures[i] = (uint32_t**)calloc(th, sizeof(uint32_t*));
        if (!textures[i]) return;
        textures[i][0] = pbuffs[i];
        for (int32_t j = 1; j < th; j++) textures[i][j] = &textures[i][0][j * tw];
    }

    //get the drawing buffer
    int32_t cwidth = 0, cheight = 0;
    uint32_t* pBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pBuff) return;
    uint32_t** renderBuff = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!renderBuff) return;
    renderBuff[0] = pBuff;
    for (i = 0; i < cheight; i++) renderBuff[i] = &renderBuff[0][i * cwidth];

    const int32_t mheight = cheight >> 1;

    //start the main loop
    do {
        //FLOOR CASTING
        for (int32_t y = mheight + 1; y < cheight; y++)
        {
            //rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
            const double rayDirX0 = dirX - planeX;
            const double rayDirY0 = dirY - planeY;
            const double rayDirX1 = dirX + planeX;
            const double rayDirY1 = dirY + planeY;

            //current y position compared to the center of the screen (the horizon)
            const int32_t p = y - mheight;

            //vertical position of the camera.
            const double posZ = 0.5 * cheight;

            //horizontal distance from the camera to the floor for the current row.
            //0.5 is the z position exactly in the middle between floor and ceiling.
            const double rowDistance = posZ / p;

            //calculate the real world step vector we have to add for each x (parallel to camera plane)
            //adding step by step avoids multiplications with a weight in the inner loop
            const double floorStepX = rowDistance * (rayDirX1 - rayDirX0) / cwidth;
            const double floorStepY = rowDistance * (rayDirY1 - rayDirY0) / cwidth;

            //real world coordinates of the leftmost column. This will be updated as we step to the right.
            double floorX = posX + rowDistance * rayDirX0;
            double floorY = posY + rowDistance * rayDirY0;

            for (int32_t x = 0; x < cwidth; x++)
            {
                //the cell coord is simply got from the integer parts of floorX and floorY
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
                renderBuff[cheight - y - 1][x] = (color >> 1) & 8355711; //make a bit darker
            }
        }

        //WALL CASTING
        for (int32_t x = 0; x < cwidth; x++)
        {
            //calculate ray position and direction
            const double cameraX = 2.0 * x / intmax_t(cwidth) - 1; //x-coordinate in camera space
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
            const int32_t lineHeight = int32_t(cheight / perpWallDist);
            const int32_t mlineHeight = lineHeight >> 1;

            //calculate lowest and highest pixel to fill in current stripe
            int32_t drawStart = -mlineHeight + mheight;
            if (drawStart < 0) drawStart = 0;
            int32_t drawEnd = mlineHeight + mheight;
            if (drawEnd > cheight) drawEnd = cheight;

            //texturing calculations
            const int32_t texNum = miniMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

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
                //cast the texture coordinate to integer, and mask with (cheight - 1) in case of overflow
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

            const int32_t spriteScreenX = int32_t((cwidth >> 1) * (1 + transformX / transformY));

            //calculate height of the sprite on screen
            const int32_t spriteHeight = abs(int32_t(cheight / transformY));
            const int32_t mspriteHeight = spriteHeight >> 1;

            //calculate lowest and highest pixel to fill in current stripe
            int32_t drawStartY = -mspriteHeight + mheight;
            if (drawStartY < 0) drawStartY = 0;
            int32_t drawEndY = mspriteHeight + mheight;
            if (drawEndY > cheight) drawEndY = cheight;

            //calculate width of the sprite
            const int32_t spriteWidth = abs(int32_t(cheight / transformY));
            const int32_t mspriteWidth = spriteWidth >> 1;

            int32_t drawStartX = -mspriteWidth + spriteScreenX;
            if (drawStartX < 0) drawStartX = 0;
            int32_t drawEndX = mspriteWidth + spriteScreenX;
            if (drawEndX > cwidth) drawEndX = cwidth;

            //loop through every vertical stripe of the sprite on screen
            for (int32_t stripe = drawStartX; stripe < drawEndX; stripe++)
            {
                const int32_t texX = (((stripe - (-mspriteWidth + spriteScreenX)) << 8) * TEXTURE_WIDTH / spriteWidth) >> 8;
                //the conditions in the if are:
                //1) it's in front of camera plane so you don't see things behind you
                //2) it's on the screen (left)
                //3) it's on the screen (right)
                //4) ZBuffer, with perpendicular distance
                if (transformY > 0 && stripe > 0 && stripe < cwidth && transformY < zBuffer[stripe])
                {
                    //for every pixel of the current stripe
                    for (int32_t y = drawStartY; y < drawEndY; y++)
                    {
                        const int32_t d = (y << 8) - (cheight << 7) + (spriteHeight << 7);
                        const int32_t texY = ((d * TEXTURE_HEIGHT) / spriteHeight) >> 8;
                        const uint32_t color = textures[sprite[spriteOrder[i]].data][texY][texX];
                        if (color & 0x00FFFFFF) renderBuff[y][stripe] = color;
                    }
                }
            }
        }

        //timing for input and FPS counter
        oldTime = time;
        time = getTime();
        const double frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
        writeText(1, 1, RGB_WHITE, 0, "FPS:%.f", 1.0 / frameTime); //FPS counter
        render();

        //clear current render buffer
        memset(renderBuff[0], 0, sizeof(uint32_t)* cwidth* cheight);

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

    //the newline is drawn as a red line
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
    if (!result) return;
    
    uint32_t* itdst = result;
    uint32_t* itimg1 = image1;
    uint32_t* itimg2 = image2;

    for (int32_t i = 0; i < size; i++)
    {
        uint8_t* pdst = (uint8_t*)itdst++;
        const uint8_t* pimg1 = (const uint8_t*)itimg1++;
        const uint8_t* pimg2 = (const uint8_t*)itimg2++;

        //average
        pdst[2] = (pimg2[2] + pimg1[2]) >> 1;
        pdst[1] = (pimg2[1] + pimg1[1]) >> 1;
        pdst[0] = (pimg2[0] + pimg1[0]) >> 1;

        //adding
        //pdst[2] = min(pimg2[2] + pimg1[2], 255);
        //pdst[1] = min(pimg2[1] + pimg1[1], 255);
        //pdst[0] = min(pimg2[0] + pimg1[0], 255);

        //subtract
        //pdst[2] = min(pimg2[2] - pimg1[2], 0);
        //pdst[1] = min(pimg2[1] - pimg1[1], 0);
        //pdst[0] = min(pimg2[0] - pimg1[0], 0);

        //multiply
        //pdst[2] = uint8_t(255 * (pimg2[2] / 255.0 * pimg1[2] / 255.0));
        //pdst[1] = uint8_t(255 * (pimg2[1] / 255.0 * pimg1[1] / 255.0));
        //pdst[0] = uint8_t(255 * (pimg2[0] / 255.0 * pimg1[0] / 255.0));

        //differnce
        //pdst[2] = abs(pimg1[2] - pimg2[2]);
        //pdst[1] = abs(pimg1[1] - pimg2[1]);
        //pdst[0] = abs(pimg1[0] - pimg2[0]);

        //min
        //pdst[2] = min(pimg1[2], pimg2[2]);
        //pdst[1] = min(pimg1[1], pimg2[1]);
        //pdst[0] = min(pimg1[0], pimg2[0]);

        //max
        //pdst[2] = max(pimg1[2], pimg2[2]);
        //pdst[1] = max(pimg1[1], pimg2[1]);
        //pdst[0] = max(pimg1[0], pimg2[0]);

        //amplitude
        //pdst[2] = uint8_t(sqrt(double(pimg1[2]) * pimg1[2] + double(pimg2[2]) * pimg2[2]) / sqrt(2.0));
        //pdst[1] = uint8_t(sqrt(double(pimg1[1]) * pimg1[1] + double(pimg2[1]) * pimg2[1]) / sqrt(2.0));
        //pdst[0] = uint8_t(sqrt(double(pimg1[0]) * pimg1[0] + double(pimg2[0]) * pimg2[0]) / sqrt(2.0));

        //and
        //pdst[2] = pimg1[2] & pimg2[2];
        //pdst[1] = pimg1[1] & pimg2[1];
        //pdst[0] = pimg1[0] & pimg2[0];

        //or
        //pdst[2] = pimg1[2] | pimg2[2];
        //pdst[1] = pimg1[1] | pimg2[1];
        //pdst[0] = pimg1[0] | pimg2[0];

        //xor
        //pdst[2] = pimg1[2] ^ pimg2[2];
        //pdst[1] = pimg1[1] ^ pimg2[1];
        //pdst[0] = pimg1[0] ^ pimg2[0];
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
    if (!result) return;

    while (!finished(SDL_SCANCODE_RETURN))
    {
        uint32_t* itdst = result;
        uint32_t* itimg1 = image1;
        uint32_t* itimg2 = image2;

        const double weight = (1.0 + cos(getTime() / 1000.0)) / 2.0;

        //do the image arithmetic
        for (int32_t i = 0; i < size; i++)
        {
            uint8_t* pdst = (uint8_t*)itdst++;
            const uint8_t* pimg1 = (const uint8_t*)itimg1++;
            const uint8_t* pimg2 = (const uint8_t*)itimg2++;
            pdst[2] = uint8_t(pimg1[2] * weight + pimg2[2] * (1 - weight));
            pdst[1] = uint8_t(pimg1[1] * weight + pimg2[1] * (1 - weight));
            pdst[0] = uint8_t(pimg1[0] * weight + pimg2[0] * (1 - weight));
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
    int32_t showText = 0;

    //current and old time, and their difference (for input)
    uint32_t time = 0, oldTime = 0, frameTime = 0;

    //retrive the current pixel buffer
    int32_t i = 0, cwidth = 0, cheight = 0;
    uint32_t* pbuff= (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;
    
    //make memory access pixels
    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
    for (i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    //user input key
    int32_t input = 0;

    //interations
    int32_t iterations = 255;
    double cre = -0.7, cim = 0.27015;

    //scale unit
    const double xscale = 3.0 / cwidth;
    const double yscale = 2.0 / cheight;

    //calculate scale and current position
    double scale = max(xscale, yscale);
    double mx = -0.5 * cwidth * scale;
    double my = -0.5 * cheight * scale;

    do
    {
        /*=================== use fma version below ===============
        for (int32_t y = 0; y < cheight; y++)
        {
            //scan-x
            const double y0 = y * scale + my;
            for (int32_t x = 0; x < cwidth; x++)
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
                pixels[y][x] = hsv2rgb(0xFF * i / iterations, 0xFF, (i < iterations) ? 0xFF : 0);
            }
        }
        ===========================================================*/

        const __m256d xim = _mm256_set1_pd(cim);
        const __m256d xre = _mm256_set1_pd(cre);

        const __m256d dd = _mm256_set1_pd(scale);
        const __m256d tx = _mm256_set1_pd(mx);

        for (int32_t y = 0; y < cheight; y++)
        {
            const __m256d y0 = _mm256_set1_pd(y * scale + my);
            for (int32_t x = 0; x < cwidth; x += 4)
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
                    const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 13));

                    masks = _mm256_or_si256(cmp, masks);
                    if (_mm256_test_all_ones(masks)) break;

                    iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                    const __m256d t = _mm256_add_pd(x1, x1);
                    y1 = _mm256_fmadd_pd(t, y1, xim);
                    x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
                }

                //extract iteration position for each pixel
                int32_t it[8] = { 0 };
                _mm256_storeu_epi32(it, iters);

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
            writeText(1, cheight - 41, RGB_WHITE, 0, "Arrows move, I/O zooms");
            writeText(1, cheight - 31, RGB_WHITE, 0, "1,2,3,4 change shape");
            writeText(1, cheight - 21, RGB_WHITE, 0, "z,x changes iterations");
            writeText(1, cheight - 11, RGB_WHITE, 0, "h cycle texts");
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
            mx += cwidth * (scale - newScale) * 0.5;
            my += cheight * (scale - newScale) * 0.5;
            scale = newScale;
        }
        
        if (input == SDL_SCANCODE_O)
        {
            const double newScale = scale * 1.08;
            mx += cwidth * (scale - newScale) * 0.5;
            my += cheight * (scale - newScale) * 0.5;
            scale = newScale;
        }
        
        //MOVE keys
        if (input == SDL_SCANCODE_UP)
        {
            const double sy = -(cheight / 100.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_DOWN)
        {
            const double sy = (cheight / 100.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_LEFT)
        {
            const double sx = -(cwidth / 100.0);
            mx += sx * scale;
        }

        if (input == SDL_SCANCODE_RIGHT)
        {
            const double sx = (cwidth / 100.0);
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

    int32_t i = 0, cwidth = 0, cheight = 0;
    uint32_t* pbuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;

    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
    for (i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    const int32_t iterations = 255;
    const double xscale = 3.0 / cwidth;
    const double yscale = 2.0 / cheight;

    const double scale = max(xscale, yscale);
    const double mx = -0.5 * cwidth * scale - 0.5;
    const double my = -0.5 * cheight * scale;

    /*============== use fma version below ===============
    for (int32_t y = 0; y < cheight; y++)
    {
        const double y0 = y * scale + my;
        for (int32_t x = 0; x < cwidth; x++)
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
            pixels[y][x] = hsv2rgb(0xFF * i / iterations, 0xFF, (i < iterations) ? 0xFF : 0);
        }
    }
    ====================================================*/
    
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = 0; y < cheight; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < cwidth; x += 4)
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
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 13));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_test_all_ones(masks)) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                const __m256d t = _mm256_add_pd(x1, x1);
                y1 = _mm256_fmadd_pd(t, y1, y0);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            //extract iteration position for each pixel
            int32_t it[8] = { 0 };
            _mm256_storeu_epi32(it, iters);

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

    //after how much iterations the function should stop
    int32_t maxIterations = 255;

    //show hint text
    int32_t showText = 0;

    //current and old time, and their difference (for input)
    uint32_t time = 0, oldTime = 0, frameTime = 0;
    
    int32_t i = 0, cwidth = 0, cheight = 0;
    uint32_t* pbuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;

    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
    for (i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    //user input key
    int32_t input = 0;

    //interations
    int32_t iterations = 255;

    //scale unit
    const double xscale = 3.0 / cwidth;
    const double yscale = 2.0 / cheight;

    //calculate scale and current position
    double scale = max(xscale, yscale);
    double mx = -0.5 * cwidth * scale - 0.5;
    double my = -0.5 * cheight * scale;

    //begin main program loop
    do
    {
        /*=================== use fma version below =====================
        for (int32_t y = 0; y < cheight; y++)
        {
            //scan-x
            const double y0 = y * scale + my;
            for (int32_t x = 0; x < cwidth; x++)
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
                pixels[y][x] = hsv2rgb(0xFF * i / iterations, 0xFF, (i < iterations) ? 0xFF : 0);
            }
        }
        ===============================================================*/

        const __m256d dd = _mm256_set1_pd(scale);
        const __m256d tx = _mm256_set1_pd(mx);

        for (int32_t y = 0; y < cheight; y++)
        {
            const __m256d y0 = _mm256_set1_pd(y * scale + my);
            for (int32_t x = 0; x < cwidth; x += 4)
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
                    const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 13));

                    masks = _mm256_or_si256(cmp, masks);
                    if (_mm256_test_all_ones(masks)) break;

                    iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                    const __m256d t = _mm256_add_pd(x1, x1);
                    y1 = _mm256_fmadd_pd(t, y1, y0);
                    x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
                }

                //extract iteration position for each pixel
                int32_t it[8] = { 0 };
                _mm256_storeu_epi32(it, iters);

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
            writeText(1, cheight - 31, RGB_WHITE, 0, "Arrows move, I/O zooms");
            writeText(1, cheight - 21, RGB_WHITE, 0, "z,x changes iterations");
            writeText(1, cheight - 11, RGB_WHITE, 0, "h cycle texts");
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
            mx += cwidth * (scale - newScale) * 0.5;
            my += cheight * (scale - newScale) * 0.5;
            scale = newScale;
        }

        if (input == SDL_SCANCODE_O)
        {
            const double newScale = scale * 1.1;
            mx += cwidth * (scale - newScale) * 0.5;
            my += cheight * (scale - newScale) * 0.5;
            scale = newScale;
        }

        //MOVE keys
        if (input == SDL_SCANCODE_UP)
        {
            const double sy = -(cheight / 10.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_DOWN)
        {
            const double sy = (cheight / 10.0);
            my += sy * scale;
        }

        if (input == SDL_SCANCODE_LEFT)
        {
            const double sx = -(cwidth / 10.0);
            mx += sx * scale;
        }

        if (input == SDL_SCANCODE_RIGHT)
        {
            const double sx = (cwidth / 10.0);
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

    int32_t paletteShift = 0, cwidth = 0, cheight = 0;
    uint32_t* pbuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;

    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
    for (int32_t i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    //use HSV2RGB to vary the Hue of the color through the palette
    for (int32_t x = 0; x < 256; x++) colors[x] = hsv2rgb(x, 255, 255);

    const int32_t mwidth = cwidth >> 1;
    const int32_t mheight = cheight >> 1;

    //generate the plasma once
    for (int32_t y = 0; y < cheight; y++)
    {
        for (int32_t x = 0; x < cwidth; x++)
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
        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++) pixels[y][x] = colors[(plasma[y][x] + paletteShift) % 256];
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
    uint32_t** texture = (uint32_t**)calloc(th, sizeof(uint32_t*));
    if (!texture) return;
    texture[0] = ptext;
    for (i = 1; i < th; i++) texture[i] = &texture[0][i * tw];

    int32_t cwidth = 0, cheight = 0;
    uint32_t* pbuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pbuff) return;

    uint32_t** pixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
    for (i = 1; i < cheight; i++) pixels[i] = &pixels[0][i * cwidth];

    const double ratio = 128;
    const double scale = 1.5;

    const int32_t mwidth = cwidth >> 1;
    const int32_t mheight = cheight >> 1;

    //generate non-linear transformation table
    for (int32_t y = 0; y < cheight; y++)
    {
        for (int32_t x = 0; x < cwidth; x++)
        {
            distBuff[y][x] = int32_t(ratio * th / sqrt(sqr(double(x) - mwidth) + sqr(double(y) - mheight))) % th;
            angleBuff[y][x] = int32_t(scale * tw * atan2(double(y) - mheight, double(x) - mwidth) / M_PI);
        }
    }

    //begin the loop
    while (!finished(SDL_SCANCODE_RETURN))
    {
        const double animation = getTime() / 1000.0;

        //calculate the shift values out of the animation value
        const int32_t shiftX = int32_t(tw * animation * 0.5);
        const int32_t shiftY = int32_t(th * animation * 0.5);

        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++)
            {
                //get the texel from the texture by using the tables, shifted with the animation values
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

const double filter[][FILTER_WIDTH] =
{
    {0, 0, 1, 0, 0},
    {0, 1, 1, 1, 0},
    {1, 1, 1, 1, 1},
    {0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0},
};

const double bias = 0.0;
const double factor = 1.0 / 13.0;
*/

/*//Gaussian Blur (3 x 3)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

const double filter[][FILTER_WIDTH] =
{
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1},
};

const double bias = 0.0;
const double factor = 1.0 / 16.0;
*/

/*//Gaussian Blur (5 x 5)
#define FILTER_WIDTH    5
#define FILTER_HEIGHT   5

const double filter[][FILTER_WIDTH] =
{
    {1,  4,  6,  4,  1},
    {4, 16, 24, 16,  4},
    {6, 24, 36, 24,  6},
    {4, 16, 24, 16,  4},
    {1,  4,  6,  4,  1},
};

const double bias = 0.0;
const double factor = 1.0 / 256.0;
*/

/*//Gaussian Blur (3f x 3f)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

const double filter[][FILTER_WIDTH] =
{
    {0.077847, 0.123317, 0.077847},
    {0.123317, 0.195346, 0.123317},
    {0.077847, 0.123317, 0.077847},
};

const double bias = 0.0;
const double factor = 1.0;
*/

/*//Motion Blur
#define FILTER_WIDTH    9
#define FILTER_HEIGHT   9

const double filter[][FILTER_WIDTH] =
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

const double bias = 0.0;
const double factor = 1.0 / 9.0;
*/
/*
//Find Edges
#define FILTER_WIDTH    5
#define FILTER_HEIGHT   5

const double filter[][FILTER_WIDTH] =
{
    {-1,  0,  0,  0,  0},
    { 0, -2,  0,  0,  0},
    { 0,  0,  6,  0,  0},
    { 0,  0,  0, -2,  0},
    { 0,  0,  0,  0, -1},
};

const double bias = 0.0;
const double factor = 1.0;
*/

/*
//Sharpen
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

const double filter[][FILTER_WIDTH] =
{
    {-1, -1, -1},
    {-1,  9, -1},
    {-1, -1, -1},
};

const double bias = 0.0;
const double factor = 1.0;
*/

//Emboss (3 x 3)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

const double filter[][FILTER_WIDTH] = {
    {-1, -1,  0},
    {-1,  0,  1},
    { 0,  1,  1},
};

const double factor = 1.0;
const double bias = 128.0;

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

    uint32_t* pbuff = (uint32_t*)getDrawBuffer();
    if (!pbuff) return;
    uint32_t** pixels = (uint32_t**)calloc(th, sizeof(uint32_t*));
    if (!pixels) return;
    pixels[0] = pbuff;
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
                    const uint8_t* pixel = (const uint8_t*)&image[dy][dx];
                    red   += pixel[2] * filter[fy][fx];
                    green += pixel[1] * filter[fy][fx];
                    blue  += pixel[0] * filter[fy][fx];
                }
            }

            //make target pixel
            uint8_t* pdst = (uint8_t*)&pixels[y][x];

            //truncate values smaller than zero and larger than 255
            pdst[2] = clamp(int32_t(factor * red + bias), 0, 255);
            pdst[1] = clamp(int32_t(factor * green + bias), 0, 255);
            pdst[0] = clamp(int32_t(factor * blue + bias), 0, 255);

            //make grey
            pdst[2] = pdst[1] = pdst[0] = uint8_t(0.2126 * pdst[2] + 0.7152 * pdst[1] + 0.0722 * pdst[0]);

            //take absolute value and truncate to 255
            //pdst[2] = min(abs(int32_t(fact * red + bias)), 255);
            //pdst[1] = min(abs(int32_t(fact * green + bias)), 255);
            //pdst[0] = min(abs(int32_t(fact * blue + bias)), 255);
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

//=================================================================================//
//                     RAY CASTING WITH SHADER EFFECT                              //
// Reference: https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/  //
// Rewrite to C/C++ by pherosiden@gmail.com                                        //
//=================================================================================//

//size of tile (wall height)
#define TILE_SIZE               128
#define WALL_HEIGHT             128

//world map width
#define MINI_MAP_WIDTH          5

//brightness value
#define BASE_LIGHT_VALUE        180

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

//You must make sure these values are integers because we're using loopup tables.
#define ANGLE0                  0
#define ANGLE30                 (ANGLE60 / 2)
#define ANGLE90                 (ANGLE30 * 3)
#define ANGLE180                (ANGLE90 * 2)
#define ANGLE270                (ANGLE90 * 3)
#define ANGLE360                (ANGLE60 * 6)
#define ANGLE5                  (ANGLE30 / 6)

//trigonometric tables (the ones with "I" such as ISiTable are "Inverse" table)
double sinTable[ANGLE360 + 1]   = { 0 };
double isinTable[ANGLE360 + 1]  = { 0 };
double cosTable[ANGLE360 + 1]   = { 0 };
double icosTable[ANGLE360 + 1]  = { 0 };
double tanTable[ANGLE360 + 1]   = { 0 };
double itanTable[ANGLE360 + 1]  = { 0 };
double fishTable[ANGLE360 + 1]  = { 0 };
double stepTableX[ANGLE360 + 1] = { 0 };
double stepTableY[ANGLE360 + 1] = { 0 };

//player's attributes
int32_t playerX                 = PROJECTION_PLANE_WIDTH >> 1;
int32_t playerY                 = PROJECTION_PLANE_HEIGHT >> 1;
int32_t playerArc               = ANGLE60;
int32_t playerHeight            = WALL_HEIGHT >> 1;

//half of the screen height
int32_t projectionPlaneCenterY  = PROJECTION_PLANE_HEIGHT >> 1;

//the following variables are used to keep the player coordinate in the overhead map
int32_t playerMapX              = 0;
int32_t playerMapY              = 0;

//build-in world map
uint8_t worldMap[WORLD_MAP_HEIGHT][WORLD_MAP_WIDTH] = {
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

//scren resolution
static int32_t      cwidth = 0, cheight = 0;

double arcToRad(double arcAngle)
{
    return arcAngle * M_PI / (double)ANGLE180;
}

void drawLineBuffer(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    //validate range
    if (x1 < 0 || x1 > cwidth - 1 || x2 < 0 || x2 > cwidth - 1 || y1 < 0 || y1 > cheight - 1 || y2 < 0 || y2 > cheight - 1) return;

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
        rawPixels[y % cheight][x % cwidth] = color;
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

void drawWallSliceRectangleTinted(int32_t x, int32_t y, int32_t height, int32_t offset, double brightnessLevel)
{
    //range check
    if (x >= cwidth) x = cwidth - 1;
    if (y >= cheight) y = cheight - 1;
    if (brightnessLevel > 1) brightnessLevel = 1;

    int32_t heightToDraw = height;
    
    //clip bottom
    if (y + heightToDraw > cheight) heightToDraw = cheight - y;

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
        const uint8_t* color = (const uint8_t*)&wallTexture[offsetY][offsetX];

        //while there's a row to draw & not end of drawing area
        while (error >= wallWidth)
        {
            error -= wallWidth;
            if (y >= 0)
            {
                //modify the pixel
                uint8_t* pixel = (uint8_t*)&rawPixels[y][x];
                pixel[2] = uint8_t(color[2] * brightnessLevel);
                pixel[1] = uint8_t(color[1] * brightnessLevel);
                pixel[0] = uint8_t(color[0] * brightnessLevel);
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

void drawFillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
    for (int32_t h = 0; h < height; h++)
    {
        for (int32_t w = 0; w < width; w++) rawPixels[y + h][x + w] = color;
    }
}

int32_t initData()
{
    int32_t i = 0;
    double radian = 0;

    //retrive render buffer
    uint32_t* pBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);

    //setup draw buffer as maxtrix to easy access data
    rawPixels = (uint32_t**)calloc(cheight, sizeof(uint32_t*));
    if (!rawPixels) return 0;

    //assign offset data
    rawPixels[0] = pBuff;
    for (i = 1; i < cheight; i++) rawPixels[i] = &rawPixels[0][i * cwidth];

    //load texture data
    uint32_t *pWall = NULL, *pFloor = NULL, *pCeiling = NULL;
    if (!loadTexture(&pWall, &wallWidth, &wallHeight, "assets/wallr.png")) return 0;
    if (!loadTexture(&pFloor, &floorWidth, &floorHeight, "assets/floor.png")) return 0;
    if (!loadTexture(&pCeiling, &ceilingWidth, &ceilingHeight, "assets/ceil.png")) return 0;

    //setup texture data as matrix to easy access data
    wallTexture = (uint32_t**)calloc(wallHeight, sizeof(uint32_t*));
    if (!wallTexture) return 0;
    wallTexture[0] = pWall;
    for (i = 1; i < wallHeight; i++) wallTexture[i] = &wallTexture[0][i * wallWidth];

    floorTexture = (uint32_t**)calloc(floorHeight, sizeof(uint32_t*));
    if (!floorTexture) return 0;
    floorTexture[0] = pFloor;
    for (i = 1; i < floorHeight; i++) floorTexture[i] = &floorTexture[0][i * floorWidth];

    ceilingTexture = (uint32_t**)calloc(ceilingHeight, sizeof(uint32_t*));
    if (!ceilingTexture) return 0;
    ceilingTexture[0] = pCeiling;
    for (i = 1; i < ceilingHeight; i++) ceilingTexture[i] = &ceilingTexture[0][i * ceilingWidth];

    //setup lookup table
    for (i = 0; i <= ANGLE360; i++)
    {
        //populate tables with their radian values.
        //(the addition of 0.0001 is a kludge to avoid divisions by 0. Removing it will produce unwanted holes in the wall when a ray is at 0, 90, 180, or 270 degree angles)
        radian = arcToRad(i) + 0.0001;
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
        radian = arcToRad(i);
        fishTable[i + ANGLE30] = 1.0 / cos(radian);
    }

    return 1;
}

void drawOverheadMap()
{
    for (int32_t row = 0; row < WORLD_MAP_HEIGHT; row++)
    {
        for (int32_t col = 0; col < WORLD_MAP_WIDTH; col++)
        {
            if (worldMap[row][col]) drawFillRectangle(PROJECTION_PLANE_WIDTH + col * MINI_MAP_WIDTH, row * MINI_MAP_WIDTH, MINI_MAP_WIDTH, MINI_MAP_WIDTH, RGB_BLACK);
            else drawFillRectangle(PROJECTION_PLANE_WIDTH + col * MINI_MAP_WIDTH, row * MINI_MAP_WIDTH, MINI_MAP_WIDTH, MINI_MAP_WIDTH, RGB_WHITE);
        }
    }

    //draw player position on the overhead map
    playerMapX = PROJECTION_PLANE_WIDTH + playerX / TILE_SIZE * MINI_MAP_WIDTH;
    playerMapY = playerY / TILE_SIZE * MINI_MAP_WIDTH;
}

//draw ray on the overhead map (for illustartion purpose)
//this is not part of the ray-casting process
void drawRayOnOverheadMap(int32_t x, int32_t y)
{
    //draw line from the player position to the position where the ray intersect with wall
    drawLineBuffer(playerMapX, playerMapY, PROJECTION_PLANE_WIDTH + x * MINI_MAP_WIDTH / TILE_SIZE, y * MINI_MAP_WIDTH / TILE_SIZE, RGB_GREEN);
}

//draw player POV on the overhead map (for illustartion purpose)
//this is not part of the ray-casting process
void drawPlayerPOVOnOverheadMap()
{
    //draw a red line indication the player's direction
    drawLineBuffer(playerMapX, playerMapY, int32_t(playerMapX + cosTable[playerArc] * 10), int32_t(playerMapY + sinTable[playerArc] * 10), RGB_RED);
}

void doRayCasting()
{
    //horizotal or vertical coordinate of intersection theoritically, this will be multiple of TILE_SIZE, but some trick did here might cause the values off by 1
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
        //ray is between 0 to 180 degree (1st and 2nd quadrant).
        double tmpX = 0.0, tmpY = 0.0;

        //ray is facing down
        if (castArc > ANGLE0 && castArc < ANGLE180)
        {
            //truncuate then add to get the coordinate of the FIRST grid (horizontal wall) that is in front of the player (this is in pixel unit)
            horizontalGrid = playerY / TILE_SIZE * TILE_SIZE + TILE_SIZE;

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
            horizontalGrid = playerY / TILE_SIZE * TILE_SIZE;
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
            verticalGrid = TILE_SIZE + playerX / TILE_SIZE * TILE_SIZE;
            distToNextVerticalGrid = TILE_SIZE;
            tmpY = tanTable[castArc] * (intmax_t(verticalGrid) - playerX);
            intersectionY = tmpY + playerY;
        }
        //RAY FACING LEFT
        else
        {
            verticalGrid = playerX / TILE_SIZE * TILE_SIZE;
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

        //determine which ray strikes a closer wall.
        //if yray distance to the wall is closer, the yDistance will be shorter than the xDistance
        if (distToHorizontalGridBeingHit < distToVerticalGridBeingHit)
        {
            //the next function call (drawRayOnMap()) is not a part of raycating rendering part, 
            //it just draws the ray on the overhead map to illustrate the raycasting process
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
            //the next function call (drawRayOnMap()) is not a part of raycating rendering part, 
            //it just draws the ray on the overhead map to illustrate the raycasting process
            drawRayOnOverheadMap(verticalGrid, int32_t(intersectionY));
            distance = distToVerticalGridBeingHit / fishTable[castColumn];
            ratio = PLAYER_PROJECTION_PLAN / distance;
            bottomOfWall = int32_t(ratio * playerHeight + projectionPlaneCenterY);
            scale = intmax_t(PLAYER_PROJECTION_PLAN) * WALL_HEIGHT / distance;
            topOfWall = bottomOfWall - int32_t(scale);
            offset = int32_t(intersectionY) % TILE_SIZE;
        }

        //add simple shading so that farther wall slices appear darker.
        //use arbitrary value of the farthest distance.  
        //trick to give different shades between vertical and horizontal (you could also use different textures for each if you wish to)
        drawWallSliceRectangleTinted(castColumn, topOfWall, bottomOfWall - topOfWall + 1, offset, BASE_LIGHT_VALUE / floor(distance));
    
        //FLOOR CASTING at the simplest! Try to find ways to optimize this, you can do it!
        if (floorTexture)
        {
            //find the first bit so we can just add the width to get the next row (of the same column)
            for (int32_t row = bottomOfWall; row < PROJECTION_PLANE_HEIGHT; row++)
            {
                const double straightDistance = double(playerHeight) / (intmax_t(row) - projectionPlaneCenterY) * PLAYER_PROJECTION_PLAN;
                const double actualDistance = straightDistance * fishTable[castColumn];

                int32_t endX = int32_t(actualDistance * cosTable[castArc]);
                int32_t endY = int32_t(actualDistance * sinTable[castArc]);
                
                //translate relative to viewer coordinates:
                endX += playerX;
                endY += playerY;

                //get the tile intersected by ray:
                const int32_t cellX = endX / TILE_SIZE;
                const int32_t cellY = endY / TILE_SIZE;

                //make sure the tile is within our map
                if (cellX < WORLD_MAP_WIDTH && cellY < WORLD_MAP_HEIGHT && cellX >= 0 && cellY >= 0 && endX >= 0 && endY >= 0)
                {
                    //cheap shading trick
                    double brightnessLevel = BASE_LIGHT_VALUE / actualDistance;
                    if (brightnessLevel < 0) brightnessLevel = 0;
                    if (brightnessLevel > 1) brightnessLevel = 1;

                    //make target pixel and color
                    uint8_t* pixel = (uint8_t*)&rawPixels[row][castColumn];

                    //find offset of tile and column in texture                    
                    const uint8_t* color = (uint8_t*)&floorTexture[endY % TILE_SIZE][endX % TILE_SIZE];

                    //draw the pixel
                    pixel[2] = uint8_t(color[2] * brightnessLevel);
                    pixel[1] = uint8_t(color[1] * brightnessLevel);
                    pixel[0] = uint8_t(color[0] * brightnessLevel);
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

                int32_t endX = int32_t(diagonalDistance * cosTable[castArc]);
                int32_t endY = int32_t(diagonalDistance * sinTable[castArc]);
                
                //translate relative to viewer coordinates:
                endX += playerX;
                endY += playerY;

                //Get the tile intersected by ray:
                const int32_t cellX = endX / TILE_SIZE;
                const int32_t cellY = endY / TILE_SIZE;

                //make sure the tile is within our map
                if (cellX < WORLD_MAP_WIDTH && cellY < WORLD_MAP_HEIGHT && cellX >= 0 && cellY >= 0 && endX >= 0 && endY >= 0)
                {
                    //cheap shading trick
                    double brightnessLevel = BASE_LIGHT_VALUE / diagonalDistance;
                    if (brightnessLevel < 0) brightnessLevel = 0;
                    if (brightnessLevel > 1) brightnessLevel = 1;

                    //make target pixel and color
                    uint8_t* pixel = (uint8_t*)&rawPixels[row][castColumn];

                    //find offset of tile and column in texture
                    const uint8_t* color = (uint8_t*)&ceilingTexture[endY % TILE_SIZE][endX % TILE_SIZE];

                    //draw the pixel
                    pixel[2] = uint8_t(color[2] * brightnessLevel);
                    pixel[1] = uint8_t(color[1] * brightnessLevel);
                    pixel[0] = uint8_t(color[0] * brightnessLevel);
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
    if (!initScreen(SCR_WIDTH + 100, SCR_HEIGHT, 32, 0, "Raycasting [Shader version] -- Keys: Arrows move; Q/Z: vertical lookup; E/C: fly & crouch")) return;
    if (!initData()) return;

    //start the main loop
    do {
        drawOverheadMap();
        doRayCasting();
        drawPlayerPOVOnOverheadMap();

        //timing for input and FPS counter
        oldTime = time;
        time = getTime();

        //frametime is the time this frame has taken, in seconds
        const double frameTime = (time - oldTime) / 1000.0;
        
        //report FPS counter
        writeText(1, 1, RGB_WHITE, 0, "FPS:%.f", 1.0 / frameTime);
        render();
        
        //clear background
        memset(rawPixels[0], RGB_WHITE, sizeof(uint32_t) * cwidth * cheight);
        
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
