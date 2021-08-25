#include <vector>
#include <algorithm>
#include "gfxlib.h"

#define FRAME_RATE		30
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480 

void juliaSet()
{
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Julia-Set");
    
    //default zoom and position
    const double zoom = 1, moveX = 0, moveY = 0;

    //after how much iterations the function should stop
    const int32_t maxIterations = 300;

    //pick some values for the constant c, this determines the shape of the Julia Set
    const double cRe = -0.7;
    const double cIm = 0.27015;
    
    //get raw pixels data
    int32_t i = 0, cwidth, cheight;
    uint8_t* pixels = (uint8_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pixels) return;

    const int32_t mwidth = cwidth >> 1;
    const int32_t mheight = cheight >> 1;

    //loop through every pixel
    for (int32_t y = 0; y < cheight; y++)
    {
        for (int32_t x = 0; x < cwidth; x++)
        {
            //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
            double newRe = 1.5 * (intptr_t(x) - mwidth) / (0.5 * zoom * cwidth) + moveX;
            double newIm = (intptr_t(y) - mheight) / (0.5 * zoom * cheight) + moveY;

            //i will represent the number of iterations, start the iteration process
            for (i = 1; i <= maxIterations; i++)
            {
                //remember value of previous iteration
                const double oldRe = newRe;
                const double oldIm = newIm;

                //the actual iteration, the real and imaginary part are calculated
                newRe = oldRe * oldRe - oldIm * oldIm + cRe;
                newIm = 2 * oldRe * oldIm + cIm;

                //if the point is outside the circle with radius 2: stop
                if ((newRe * newRe + newIm * newIm) > 4) break;
            }

            //use color model conversion to get rainbow palette, make brightness black if maxIterations reached
            const RGB color = HSV2RGB(i & 255, 255, 255 * (i < maxIterations));

            //direct access texture
            pixels[2] = color.r;
            pixels[1] = color.g;
            pixels[0] = color.b;
            pixels += 4;
        }
    }

    render();
    while (!finished(SDL_SCANCODE_RETURN));
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

static uint8_t dstBuff[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };
static uint8_t prevBuff[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };

void fireDemo1()
{
    uint8_t avg = 0;
    int32_t i = 0, sum = 0;
    
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Fire");

    //get drawing buffer
    int32_t cwidth, cheight;
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
static uint32_t fireBuff[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };

void fireDemo2()
{
    //set up the screen
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Fire");

    //get the drawing buffer
    int32_t cwidth, cheight;
    uint32_t* renderBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!renderBuff) return;

    //validation screen height
    if (cheight < 1) return;

    //used during palette generation
    RGB color = { 0 };
    
    //the time of this and the previous frame, for timing
    double time = getTime(), oldTime = 0;

    //make sure the fire buffer is zero in the beginning
    memset(fireBuff, 0, sizeof(fireBuff));

    //generate the palette
    for (int32_t x = 0; x < 256; x++)
    {
        //HSL2RGB is used to generate colors:
        //Hue goes from 0 to 85: red to yellow
        //Saturation is always the maximum: 255
        //Lightness is 0..255 for x=0..128, and 255 for x=128..255
        color = HSL2RGB(x / 3, 255, min(255, x * 2));

        //set the palette to the calculated RGB value
        palette[x] = RGB2INT(color.r, color.g, color.b);
    }

    //start the loop (one frame per loop)
    while (!finished(SDL_SCANCODE_RETURN))
    {
        //timing: set to maximum 50 milliseconds per frame = 20 frames per second
        oldTime = time;
        waitFor(oldTime, 50);
        time = getTime();

        //randomize the bottom row of the fire buffer
        for (int32_t x = 0; x < cwidth; x++) fireBuff[(cheight - 1) * cwidth + x] = abs(32768 + rand()) % 256;

        //do the fire calculations for every pixel, from top to bottom
        for (int32_t y = 0; y < cheight - 1; y++)
        {
            for (int32_t x = 0; x < cwidth; x++)
            {
                fireBuff[y * cwidth + x] = ((
                fireBuff[((y + 1) % cheight) * cwidth + ((x - 1 + cwidth) % cwidth)] +
                fireBuff[((y + 1) % cheight) * cwidth + ((x             ) % cwidth)] +
                fireBuff[((y + 1) % cheight) * cwidth + ((x + 1         ) % cwidth)] +
                fireBuff[((y + 2) % cheight) * cwidth + ((x             ) % cwidth)]) * 32) / 129;
            }
        }

        //set the drawing buffer to the fire buffer, using the palette colors
        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++) renderBuff[y * cwidth + x] = palette[fireBuff[y * cwidth + x]];
        }

        //draw the buffer and redraw the initScreen
        render();
    }

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

struct SPRITE
{
    double x;
    double y;
    int32_t texture;
};

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
double zBuffer[SCREEN_WIDTH];

//arrays used to sort the sprites
int32_t spriteOrder[NUM_SPRITES];
double spriteDistance[NUM_SPRITES];

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

    double time = 0, oldTime = 0;

    int32_t tw = 0, th = 0;
    uint32_t* texture[11] = { 0 };

    //init initScreen mode
    loadFont("assets/sysfont.xfn", 0);
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Raycasting-Demo -- Keys controls: Arrows: move your works!");

    //load some textures
    loadTexture(&texture[0], &tw, &th, "assets/eagle.png");
    loadTexture(&texture[1], &tw, &th, "assets/redbrick.png");
    loadTexture(&texture[2], &tw, &th, "assets/purplestone.png");
    loadTexture(&texture[3], &tw, &th, "assets/greystone.png");
    loadTexture(&texture[4], &tw, &th, "assets/bluestone.png");
    loadTexture(&texture[5], &tw, &th, "assets/mossy.png");
    loadTexture(&texture[6], &tw, &th, "assets/wood.png");
    loadTexture(&texture[7], &tw, &th, "assets/colorstone.png");

    //load some sprite textures
    loadTexture(&texture[8], &tw, &th, "assets/barrel.png");
    loadTexture(&texture[9], &tw, &th, "assets/pillar.png");
    loadTexture(&texture[10], &tw, &th, "assets/greenlight.png");

    //get the drawing buffer
    int32_t cwidth, cheight;
    uint32_t* renderBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!renderBuff) return;

    //start the main loop
    do
    {
        //FLOOR CASTING
        for (int32_t y = cheight / 2 + 1; y < cheight; y++)
        {
            //rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
            const double rayDirX0 = dirX - planeX;
            const double rayDirY0 = dirY - planeY;
            const double rayDirX1 = dirX + planeX;
            const double rayDirY1 = dirY + planeY;

            //Current y position compared to the center of the initScreen (the horizon)
            const int32_t p = y - cheight / 2;

            //Vertical position of the camera.
            const double posZ = 0.5 * cheight;

            //Horizontal distance from the camera to the floor for the current row.
            //0.5 is the z position exactly in the middle between floor and ceiling.
            const double rowDistance = posZ / p;

            //calculate the real world step vector we have to add for each x (parallel to camera plane)
            //adding step by step avoids multiplications with a weight in the inner loop
            const double floorStepX = rowDistance * (rayDirX1 - rayDirX0) / cwidth;
            const double floorStepY = rowDistance * (rayDirY1 - rayDirY0) / cwidth;

            //real world coordinates of the leftmost column. This will be updated as we step to the right.
            double floorX = posX + rowDistance * rayDirX0;
            double floorY = posY + rowDistance * rayDirY0;

            for (int32_t x = 0; x < cwidth; ++x)
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
                color = texture[floorTexture][TEXTURE_WIDTH * ty + tx];
                renderBuff[y * cwidth + x] = (color >> 1) & 8355711; //make a bit darker

                //ceiling (symmetrical, at screenHeight - y - 1 instead of y)
                color = texture[ceilingTexture][TEXTURE_WIDTH * ty + tx];
                renderBuff[(cheight - y - 1) * cwidth + x] = (color >> 1) & 8355711; //make a bit darker
            }
        }

        //WALL CASTING
        for (int32_t x = 0; x < cwidth; x++)
        {
            //calculate ray position and direction
            const double cameraX = 2.0 * x / double(cwidth) - 1; //x-coordinate in camera space
            const double rayDirX = dirX + planeX * cameraX;
            const double rayDirY = dirY + planeY * cameraX;

            //which box of the map we're in
            int32_t mapX = int32_t(posX);
            int32_t mapY = int32_t(posY);

            //length of ray from current position to next x or y-side
            double sideDistX = 0;
            double sideDistY = 0;

            //length of ray from one x or y-side to next x or y-side
            double perpWallDist = 0;
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

                //Check if ray has hit a wall
                if (miniMap[mapX][mapY] > 0) hit = 1;
            }

            //Calculate distance of perpendicular ray (Euclidean distance will give fisheye effect!)
            if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
            else perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;

            //Calculate height of line to draw on initScreen
            const int32_t lineHeight = int32_t(cheight / perpWallDist);

            //calculate lowest and highest pixel to fill in current stripe
            int32_t drawStart = -lineHeight / 2 + cheight / 2;
            if (drawStart < 0) drawStart = 0;
            int32_t drawEnd = lineHeight / 2 + cheight / 2;
            if (drawEnd >= cheight) drawEnd = cheight - 1;

            //texturing calculations
            const int32_t texNum = miniMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

            //calculate value of wallX
            double wallX = 0;
            if (side == 0) wallX = posY + perpWallDist * rayDirY;
            else wallX = posX + perpWallDist * rayDirX;

            wallX -= floor((wallX));

            //x coordinate on the texture
            int32_t texX = int32_t(wallX * double(TEXTURE_WIDTH));
            if (side == 0 && rayDirX > 0) texX = TEXTURE_WIDTH - texX - 1;
            if (side == 1 && rayDirY < 0) texX = TEXTURE_WIDTH - texX - 1;

            //How much to increase the texture coordinate per initScreen pixel
            const double step =  double(TEXTURE_HEIGHT) / lineHeight;

            //Starting texture coordinate
            double texPos = (drawStart - cheight / 2.0 + lineHeight / 2.0) * step;

            for (int32_t y = drawStart; y < drawEnd; y++)
            {
                //Cast the texture coordinate to integer, and mask with (cheight - 1) in case of overflow
                const int32_t texY = int32_t(texPos) & (TEXTURE_HEIGHT - 1);
                texPos += step;

                uint32_t color = texture[texNum][TEXTURE_WIDTH * texY + texX];
                //make color darker for y-sides: R, G and B byte each divided through two with a 'shift' and an 'and'
                if (side == 1) color = (color >> 1) & 8355711;
                renderBuff[y * cwidth + x] = color;
            }

            //SET THE ZBUFFER FOR THE SPRITE CASTING
            zBuffer[x] = perpWallDist; //perpendicular distance is used
        }

        //SPRITE CASTING
        //sort sprites from far to close
        for (int32_t i = 0; i < NUM_SPRITES; i++)
        {
            spriteOrder[i] = i;
            spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y)); //sqrt not taken, unneeded
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

            const double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication

            const double transformX = invDet * (dirY * spriteX - dirX * spriteY);
            const double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the initScreen, that what Z is in 3D

            const int32_t spriteScreenX = int32_t((cwidth / 2) * (1 + transformX / transformY));

            //calculate height of the sprite on initScreen
            const int32_t spriteHeight = abs(int32_t(cheight / transformY)); //using 'transformY' instead of the real distance prevents fisheye

            //calculate lowest and highest pixel to fill in current stripe
            int32_t drawStartY = -spriteHeight / 2 + cheight / 2;
            if (drawStartY < 0) drawStartY = 0;

            int32_t drawEndY = spriteHeight / 2 + cheight / 2;
            if (drawEndY >= cheight) drawEndY = cheight - 1;

            //calculate width of the sprite
            const int32_t spriteWidth = abs(int32_t(cheight / transformY));
            
            int32_t drawStartX = -spriteWidth / 2 + spriteScreenX;
            if (drawStartX < 0) drawStartX = 0;
            
            int32_t drawEndX = spriteWidth / 2 + spriteScreenX;
            if (drawEndX >= cwidth) drawEndX = cwidth - 1;

            //loop through every vertical stripe of the sprite on initScreen
            for (int32_t stripe = drawStartX; stripe < drawEndX; stripe++)
            {
                const int32_t texX = int32_t(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * TEXTURE_WIDTH / spriteWidth) / 256;
                //the conditions in the if are:
                //1) it's in front of camera plane so you don't see things behind you
                //2) it's on the initScreen (left)
                //3) it's on the initScreen (right)
                //4) ZBuffer, with perpendicular distance
                if (transformY > 0 && stripe > 0 && stripe < cwidth && transformY < zBuffer[stripe])
                {
                    for (int32_t y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
                    {
                        const int32_t d = y * 256 - cheight * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
                        const int32_t texY = ((d * TEXTURE_HEIGHT) / spriteHeight) / 256;
                        const uint32_t color = texture[sprite[spriteOrder[i]].texture][TEXTURE_WIDTH * texY + texX]; //get current color from the texture
                        if ((color & 0x00FFFFFF) != 0) renderBuff[y * cwidth + stripe] = color; //paint pixel if it isn't black, black is the invisible color
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
        memset(renderBuff, 0, sizeof(uint32_t)* cwidth* cheight);

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

    for (int32_t i = 0; i != 11; i++) free(texture[i]);
    freeFont(0);
    cleanup();
}

void basicDrawing()
{
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "2D primitives");

    //this is outsize screen
    int32_t x1 = -50, y1 = -20, x2 = 1000, y2 = 1200;

    //the new line represents the part of the old line that is visible on screen
    clipLine(&x1, &y1, &x2, &y2);

    //The newline is drawn as a red line
    drawLineAlpha(x1, y1, x2, y2, RGB_RED);
    drawCircleAlpha(100, 100, 30, RGB_GREEN);
    fillCircle(200, 100, 40, RGB_YELLOW);
    drawRect(150, 150, 200, 200, RGB_CYAN);
    vertLine(320, 100, 300, RGB_WHITE);

    //make all visible on screen
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    cleanup();
}

void imageArithmetic()
{
    int32_t w = 0, h = 0;

    //declare image buffers
    uint32_t *image1, *image2;

    //load the images into the buffers. This assumes all have the same size.
    loadTexture(&image1, &w, &h, "assets/photo1.png");
    loadTexture(&image2, &w, &h, "assets/photo2.png");

    //set up the initScreen
    initScreen(w, h, 32, 0, "Image Arithmetic");

    const int32_t size = w * h;
    uint32_t* result = (uint32_t*)getDrawBuffer();
    if (!result) return;
    
    uint32_t* itdst = result;
    uint32_t* itimg1 = image1;
    uint32_t* itimg2 = image2;

    for (int32_t i = 0; i < size; i++)
    {
        uint8_t* pdst = (uint8_t*)itdst++;
        uint8_t* pimg1 = (uint8_t*)itimg1++;
        uint8_t* pimg2 = (uint8_t*)itimg2++;

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
    uint32_t *image1, *image2;

    //load the images into the buffers. This assumes all have the same size.
    loadTexture(&image1, &w, &h, "assets/photo1.png");
    loadTexture(&image2, &w, &h, "assets/photo2.png");
    
    //set up the initScreen
    initScreen(w, h, 32, 0, "Cross-Fading");
    
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
            uint8_t* pimg1 = (uint8_t*)itimg1++;
            uint8_t* pimg2 = (uint8_t*)itimg2++;
            
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
    loadFont("assets/sysfont.xfn", 0);
    initScreen(320, 240, 32, 0, "Julia-Explorer");

    //use to show/hide text
    int32_t showText = 0;

    //after how much iterations the function should stop
    int32_t maxIterations = 128;

    //you can change these to zoom and change position
    double zoom = 1, moveX = 0, moveY = 0;

    //current and old time, and their difference (for input)
    double time = 0, oldTime = 0, frameTime = 0;
    
    //pick some values for the constant c, this determines the shape of the Julia Set
    double cRe = -0.7;
    double cIm = 0.27015;

    //retrive the current pixel buffer
    int32_t i = 0, cwidth, cheight;
    uint8_t* pixels = (uint8_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pixels) return;

    const int32_t mwidth = cwidth >> 1;
    const int32_t mheight = cheight >> 1;

    //begin the program loop
    do
    {
        uint8_t* plots = pixels;

        //draw the fractal
        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++)
            {
                //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
                double newRe = 1.5 * (intptr_t(x) - mwidth) / (0.5 * zoom * cwidth) + moveX;
                double newIm = (intptr_t(y) - mheight) / (0.5 * zoom * cheight) + moveY;

                //start the iteration process
                for (i = 0; i < maxIterations; i++)
                {
                    //remember value of previous iteration
                    const double oldRe = newRe;
                    const double oldIm = newIm;

                    //the actual iteration, the real and imaginary part are calculated
                    newRe = oldRe * oldRe - oldIm * oldIm + cRe;
                    newIm = 2 * oldRe * oldIm + cIm;

                    //if the point is outside the circle with radius 2: stop
                    if ((newRe * newRe + newIm * newIm) > 4) break;
                }

                //use color model conversion to get rainbow palette, make brightness black if maxIterations reached
                const RGB color = HSV2RGB(i % 256, 255, 255 * (i < maxIterations));

                //draw the pixel
                plots[2] = color.r;
                plots[1] = color.g;
                plots[0] = color.b;
                plots += 4;
            }
        }

        //print the values of all variables on initScreen if that option is enabled
        if (showText <= 1)
        {
            writeText(1, 1, RGB_WHITE, 0, "X:%.16lf", moveX);
            writeText(1, 9, RGB_WHITE, 0, "Y:%.16lf", moveY);
            writeText(1, 17, RGB_WHITE, 0, "Z:%.16lf", zoom);
            writeText(1, 25, RGB_WHITE, 0, "R:%.16lf", cRe);
            writeText(1, 33, RGB_WHITE, 0, "I:%.16lf", cIm);
            writeText(1, 41, RGB_WHITE, 0, "N:%d", maxIterations);
        }

        //print the help text on initScreen if that option is enabled
        if (showText == 0)
        {
            writeText(1, cheight - 33, RGB_WHITE, 0, "Arrows move (X,Y), I,O zooms (+/-)");
            writeText(1, cheight - 25, RGB_WHITE, 0, "Key 2,4,6,8 change shape (R,I)     ");
            writeText(1, cheight - 17, RGB_WHITE, 0, "Keypad z,x changes iterations (N)    ");
            writeText(1, cheight - 9, RGB_WHITE, 0, "F1=cycle texts");
        }

        render();

        //get the time and old time for time dependent input
        oldTime = time;
        time = getTime();
        frameTime = time - oldTime;

        readKeys();
        
        //ZOOM keys
        if (keyDown(SDL_SCANCODE_I)) { zoom *= pow(1.001, frameTime); }
        if (keyDown(SDL_SCANCODE_O)) { zoom /= pow(1.001, frameTime); }
        
        //MOVE keys
        if (keyDown(SDL_SCANCODE_DOWN)) { moveY += 0.0003 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_UP)) { moveY -= 0.0003 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_RIGHT)) { moveX += 0.0003 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_LEFT)) { moveX -= 0.0003 * frameTime / zoom; }

        //CHANGE SHAPE keys
        if (keyDown(SDL_SCANCODE_2)) { cIm += 0.0002 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_8)) { cIm -= 0.0002 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_6)) { cRe += 0.0002 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_4)) { cRe -= 0.0002 * frameTime / zoom; }

        //keys to change number of iterations
        if (keyPressed(SDL_SCANCODE_Z)) { maxIterations *= 2; }
        if (keyPressed(SDL_SCANCODE_X)) { if (maxIterations > 2) maxIterations /= 2; }

        //key to change the text options
        if (keyPressed(SDL_SCANCODE_F1)) { showText++; showText %= 3; }
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

        //reduce CPU time
        delay(1);
    } while (!keyDown(SDL_SCANCODE_RETURN));

    freeFont(0);
    cleanup();
}

void mandelbrotSet()
{
    //make larger to see more detail!
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Mandelbrot-Set");

    //after how much iterations the function should stop
    const int32_t maxIterations = 300;

    //you can change these to zoom and change position
    const double zoom = 1, moveX = -0.5, moveY = 0;

    int32_t i, cwidth = 0, cheight = 0;
    uint8_t* pixels = (uint8_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pixels) return;

    const int32_t mwidth = cwidth >> 1;
    const int32_t mheight = cheight >> 1;

    //loop through every pixel
    for (int32_t y = 0; y < cheight; y++)
    {
        for (int32_t x = 0; x < cwidth; x++)
        {
            double newRe = 0, newIm = 0;

            //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
            const double pr = 1.5 * (intptr_t(x) - mwidth) / (0.5 * zoom * cwidth) + moveX;
            const double pi = (intptr_t(y) - mheight) / (0.5 * zoom * cheight) + moveY;
            
            //start the iteration process
            for (i = 1; i <= maxIterations; i++)
            {
                //remember value of previous iteration
                const double oldRe = newRe;
                const double oldIm = newIm;
                
                //the actual iteration, the real and imaginary part are calculated
                newRe = oldRe * oldRe - oldIm * oldIm + pr;
                newIm = 2 * oldRe * oldIm + pi;
            
                //if the point is outside the circle with radius 2: stop
                if ((newRe * newRe + newIm * newIm) > 4) break;
            }

            //use color model conversion to get rainbow palette, make brightness black if maxIterations reached
            const RGB color = HSV2RGB(i % 256, 255, 255 * (i < maxIterations));

            //draw the pixel
            pixels[2] = color.r;
            pixels[1] = color.g;
            pixels[0] = color.b;
            pixels += 4;
        }
    }

    //make the Mandelbrot Set visible and wait to exit
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    cleanup();
}

void mandelbrotExporer()
{
    loadFont("assets/sysfont.xfn", 0);
    initScreen(320, 240, 32, 0, "Mandelbrot-Explorer");

    //after how much iterations the function should stop
    int32_t maxIterations = 128;

    //you can change these to zoom and change position
    double zoom = 1, moveX = -0.5, moveY = 0;

    //show hint text
    int32_t showText = 0;

    //current and old time, and their difference (for input)
    double time = 0, oldTime = 0, frameTime = 0;
    
    int32_t i = 0, cwidth = 0, cheight = 0;
    uint8_t* pixels = (uint8_t*)getDrawBuffer(&cwidth, &cheight);
    if (!pixels) return;

    const int32_t mwidth = cwidth >> 1;
    const int32_t mheight = cheight >> 1;

    //begin main program loop
    do
    {
        uint8_t* plots = pixels;

        //draw the fractal
        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++)
            {
                double newRe = 0, newIm = 0;

                //calculate the initial real and imaginary part of z, based on the pixel location and zoom and position values
                const double pr = 1.5 * (intptr_t(x) - mwidth) / (0.5 * zoom * cwidth) + moveX;
                const double pi = (intptr_t(y) - mheight) / (0.5 * zoom * cheight) + moveY;
                
                //start the iteration process
                for (i = 1; i <= maxIterations; i++)
                {
                    //remember value of previous iteration
                    const double oldRe = newRe;
                    const double oldIm = newIm;

                    //the actual iteration, the real and imaginary part are calculated
                    newRe = oldRe * oldRe - oldIm * oldIm + pr;
                    newIm = 2 * oldRe * oldIm + pi;

                    //if the point is outside the circle with radius 2: stop
                    if ((newRe * newRe + newIm * newIm) > 4) break;
                }

                //use color model conversion to get rainbow palette, make brightness black if maxIterations reached
                const RGB color = HSV2RGB(i % 256, 255, 255 * (i < maxIterations));

                //draw the pixel
                plots[2] = color.r;
                plots[1] = color.g;
                plots[0] = color.b;
                plots += 4;
            }
        }

        //print the values of all variables on initScreen if that option is enabled
        if (showText <= 1)
        {
            writeText(1, 1, RGB_WHITE, 0, "X:%.16lf", moveX);
            writeText(1, 9, RGB_WHITE, 0, "Y:%.16lf", moveY);
            writeText(1, 17, RGB_WHITE, 0, "Z:%.16lf", zoom);
            writeText(1, 25, RGB_WHITE, 0, "N:%d", maxIterations);
        }

        //print the help text on initScreen if that option is enabled
        if (showText == 0)
        {
            writeText(1, cheight - 33, RGB_WHITE, 0, "Arrows move (X,Y), 1,0 zooms (+/-)");
            writeText(1, cheight - 25, RGB_WHITE, 0, "Keypad z,x changes iterations (N)    ");
            writeText(1, cheight - 17, RGB_WHITE, 0, "F1=cycle texts");
        }

        render();

        //get the time and old time for time dependent input
        oldTime = time;
        time = getTime();
        frameTime = time - oldTime;

        readKeys();

        //ZOOM keys
        if (keyDown(SDL_SCANCODE_I)) { zoom *= pow(1.001, frameTime); }
        if (keyDown(SDL_SCANCODE_O)) { zoom /= pow(1.001, frameTime); }

        //MOVE keys
        if (keyDown(SDL_SCANCODE_DOWN)) { moveY += 0.0003 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_UP)) { moveY -= 0.0003 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_RIGHT)) { moveX += 0.0003 * frameTime / zoom; }
        if (keyDown(SDL_SCANCODE_LEFT)) { moveX -= 0.0003 * frameTime / zoom; }

        //keys to change number of iterations
        if (keyPressed(SDL_SCANCODE_Z)) { maxIterations *= 2; }
        if (keyPressed(SDL_SCANCODE_X)) { if (maxIterations > 2) maxIterations /= 2; }

        //key to change the text options
        if (keyPressed(SDL_SCANCODE_F1)) { showText++; showText %= 3; }
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();

        //reduce CPU time
        delay(1);
    } while (!keyDown(SDL_SCANCODE_RETURN));

    freeFont(0);
    cleanup();
}

static uint32_t colors[SIZE_256] = { 0 };
static uint32_t plasma[SIZE_256][SIZE_256] = { 0 };

void plasmaDemo()
{
    initScreen(256, 256, 32, 0, "Plasma");

    int32_t paletteShift = 0, cwidth = 0, cheight = 0;
    uint32_t* renderBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!renderBuff) return;

    //generate the palette
    RGB rgb = { 0 };
    for (int32_t x = 0; x < 256; x++)
    {
        //use HSV2RGB to vary the Hue of the color through the palette
        rgb = HSV2RGB(x, 255, 255);
        colors[x] = RGB2INT(rgb.r, rgb.g, rgb.b);
    }

    uint32_t color;

    //generate the plasma once
    for (int32_t y = 0; y < cheight; y++)
    {
        for (int32_t x = 0; x < cwidth; x++)
        {
            //the plasma buffer is a sum of sines
            color = uint32_t (
                128.0 + (128.0 * sin(x / 16.0))
                + 128.0 + (128.0 * sin(y / 16.0))
                ) / 2;

            plasma[y][x] = color;

            color = uint32_t (
                128.0 + (128.0 * sin(x / 16.0))
                + 128.0 + (128.0 * sin(y / 8.0))
                + 128.0 + (128.0 * sin((double(x) + y) / 16.0))
                + 128.0 + (128.0 * sin(sqrt(double(x) * x + double(y) * y) / 8.0))
                ) / 4;

            plasma[y][x] = color;

            color = uint32_t (
                  128.0 + (128.0 * sin(x / 16.0))
                + 128.0 + (128.0 * sin(y / 32.0))
                + 128.0 + (128.0 * sin(sqrt((double(x) - cwidth / 2.0) * (double(x) - cwidth / 2.0) + (double(y) - cheight / 2.0) * (double(y) - cheight / 2.0)) / 8.0))
                + 128.0 + (128.0 * sin(sqrt(double(x) * x + double(y) * y) / 8.0))) / 4;
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
            for (int32_t x = 0; x < cwidth; x++)
            {
                renderBuff[y * cwidth + x] = colors[(plasma[y][x] + paletteShift) % 256];
            }
        }

        //make everything visible
        render();
    }

    cleanup();
}

static int32_t distBuff[SCREEN_HEIGHT][SCREEN_WIDTH] = { 0 };
static int32_t angleBuff[SCREEN_HEIGHT][SCREEN_WIDTH] = { 0 };

void tunnelDemo()
{
    int32_t w = 0, h = 0;
    uint32_t* texture = NULL;

    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Tunnel");

    //load tunnel texture
    loadTexture(&texture, &w, &h, "assets/map03.png");
    if (!texture) return;

    const double ratio = 100.0;
    const double scale = 1.5;

    int32_t cwidth, cheight;
    uint32_t* renderBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!renderBuff) return;

    //generate non-linear transformation table
    for (int32_t y = 0; y < cheight; y++)
    {
        for (int32_t x = 0; x < cwidth; x++)
        {
            const int32_t distance = int32_t(ratio * h / sqrt((x - cwidth / 2.0) * (x - cwidth / 2.0) + (y - cheight / 2.0) * (y - cheight / 2.0))) % h;
            const int32_t angle = int32_t(scale * w * atan2(y - cheight / 2.0, x - cwidth / 2.0) / M_PI);
            distBuff[y][x] = distance;
            angleBuff[y][x] = angle;
        }
    }

    //begin the loop
    while (!finished(SDL_SCANCODE_RETURN))
    {
        const double animation = getTime() / 1000;

        //calculate the shift values out of the animation value
        const int32_t shiftX = int32_t(w * animation * 0.5);
        const int32_t shiftY = int32_t(h * animation * 0.5);

        for (int32_t y = 0; y < cheight; y++)
        {
            for (int32_t x = 0; x < cwidth; x++)
            {
                //get the texel from the texture by using the tables, shifted with the animation values
                const int32_t oy = (distBuff[y][x] + shiftX) % h;
                const int32_t ox = (angleBuff[y][x] + shiftY) % w;
                int32_t offset = oy * w + ox;
                if (offset < 0) offset = 0;
                renderBuff[y * cwidth + x] = texture[offset];
            }
        }
        delay(FPS_30);
        render();
    }

    free(texture);
    cleanup();
}

/*
//blur
#define filterWidth 5
#define filterHeight 5

double filter[filterHeight][filterWidth] =
{
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0,
    0, 0, 1, 0, 0,
};

double factor = 1.0 / 13.0;
double bias = 0.0;
*/

/*
//Motion Blur
#define filterWidth 9
#define filterHeight 9

double filter[filterHeight][filterWidth] =
{
    1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1,
};

double factor = 1.0 / 9.0;
double bias = 0.0;
*/

/*
//Find Edges
#define filterWidth 5
#define filterHeight 5

double filter[filterHeight][filterWidth] =
{
  -1,  0,  0,  0,  0,
   0, -2,  0,  0,  0,
   0,  0,  6,  0,  0,
   0,  0,  0, -2,  0,
   0,  0,  0,  0, -1,
};

double factor = 1.0;
double bias = 0.0;
*/

/*
//Sharpen
#define filterWidth 3
#define filterHeight 3

double filter[filterHeight][filterWidth] =
{
  -1, -1, -1,
  -1,  9, -1,
  -1, -1, -1
};

double factor = 1.0;
double bias = 0.0;
*/

//Emboss (3 x 3)
#define FILTER_WIDTH    3
#define FILTER_HEIGHT   3

static const double filter[FILTER_HEIGHT][FILTER_WIDTH] = {
  -1, -1,  0,
  -1,  0,  1,
   0,  1,  1
};

static const double fact = 1.0;
static const double bias = 128.0;

void imageFillter()
{
    //load the image into the buffer
    uint32_t* image;
    int32_t w = 0, h = 0;
    loadTexture(&image, &w, &h, "assets/photo3.png");

    //set up the initScreen
    initScreen(w, h, 32, 0, "Filters");
    uint32_t* result = (uint32_t*)getDrawBuffer();
    if (!result) return;

    //apply the filter
    for (int32_t y = 0; y < h; y++)
    {
        for (int32_t x = 0; x < w; x++)
        {
            double red = 0.0, green = 0.0, blue = 0.0;

            //multiply every value of the filter with corresponding image pixel
            for (int32_t fy = 0; fy < FILTER_HEIGHT; fy++)
            {
                for (int32_t fx = 0; fx < FILTER_WIDTH; fx++)
                {
                    const int32_t dx = (x - FILTER_WIDTH / 2 + fx + w) % w;
                    const int32_t dy = (y - FILTER_HEIGHT / 2 + fy + h) % h;
                    uint8_t* pixel = (uint8_t*)&image[dy * w + dx];
                    red   += pixel[2] * filter[fy][fx];
                    green += pixel[1] * filter[fy][fx];
                    blue  += pixel[0] * filter[fy][fx];
                }
            }

            //make target pixel
            uint8_t* pdst = (uint8_t*)&result[y * w + x];

            //truncate values smaller than zero and larger than 255
            pdst[2] = min(max(int32_t(fact * red + bias), 0), 255);
            pdst[1] = min(max(int32_t(fact * green + bias), 0), 255);
            pdst[0] = min(max(int32_t(fact * blue + bias), 0), 255);

            //take absolute value and truncate to 255
            //pdst[2] = min(abs(int32_t(fact * red + bias)), 255);
            //pdst[1] = min(abs(int32_t(fact * green + bias)), 255);
            //pdst[0] = min(abs(int32_t(fact * blue + bias)), 255);
        }
    }

    //redraw & sleep
    render();
    while (!finished(SDL_SCANCODE_RETURN));
    free(image);
    cleanup();
}

//=================================================================================//
//                     RAY CASTING WITH SHADER EFFECT                              //
// Reference: https://permadi.com/1996/05/ray-casting-tutorial-table-of-contents/  //
// Rewrite to C/C++ by pherosiden@gmail.com                                        //
//=================================================================================//

//initScreen size
#define SCREEN_WIDTH			740 //640 + world map width (20 * MINI_MAP_WIDTH)
#define SCREEN_HEIGHT			480

//size of tile (wall height)
#define TILE_SIZE				128
#define WALL_HEIGHT				128

//world map width
#define MINI_MAP_WIDTH			5

//brightness value
#define BASE_LIGHT_VALUE		180

//MIN distance to wall
#define MIN_DISTANCE_TO_WALL	60

//Player speed
#define PLAYER_SPEED			32

//2D map
#define WORLD_MAP_WIDTH			20
#define WORLD_MAP_HEIGHT		20

//Remember that PROJECTIONPLANE = screen size
#define PROJECTION_PLANE_WIDTH	640
#define PROJECTION_PLANE_HEIGHT	480

//Player distance to projection plan = PROJECTION_PLANE_WIDTH / 2 / tan(30)
#define PLAYER_PROJECTION_PLAN	554

//We use FOV of 60 degrees.  So we use this FOV basis of the table, taking into account
//that we need to cast 320 rays (PROJECTIONPLANEWIDTH) within that 60 degree FOV.
#define ANGLE60					PROJECTION_PLANE_WIDTH

//You must make sure these values are integers because we're using loopup tables.
#define ANGLE0					0
#define ANGLE30					(ANGLE60 / 2)
#define ANGLE90					(ANGLE30 * 3)
#define ANGLE180				(ANGLE90 * 2)
#define ANGLE270				(ANGLE90 * 3)
#define ANGLE360				(ANGLE60 * 6)
#define ANGLE5					(ANGLE30 / 6)

//trigonometric tables (the ones with "I" such as ISiTable are "Inverse" table)
double sinTable[ANGLE360 + 1] = { 0 };
double isinTable[ANGLE360 + 1] = { 0 };
double cosTable[ANGLE360 + 1] = { 0 };
double icosTable[ANGLE360 + 1] = { 0 };
double tanTable[ANGLE360 + 1] = { 0 };
double itanTable[ANGLE360 + 1] = { 0 };
double fishTable[ANGLE360 + 1] = { 0 };
double stepTableX[ANGLE360 + 1] = { 0 };
double stepTableY[ANGLE360 + 1] = { 0 };

//player's attributes
int32_t playerX = PROJECTION_PLANE_WIDTH >> 1;
int32_t playerY = PROJECTION_PLANE_HEIGHT >> 1;
int32_t playerArc = ANGLE60;
int32_t playerHeight = WALL_HEIGHT >> 1;

//Half of the initScreen height
int32_t projectionPlaneCenterY = PROJECTION_PLANE_HEIGHT >> 1;

//the following variables are used to keep the player coordinate in the overhead map
int32_t playerMapX = 0;
int32_t playerMapY = 0;

//build-in world map
uint8_t worldMap[WORLD_MAP_HEIGHT * WORLD_MAP_WIDTH] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1,
    1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
    1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,1,1,0,1,
    1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,0,1,0,1,
    1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,
    1,0,0,0,1,0,1,0,1,0,0,1,0,0,0,0,0,1,0,1,
    1,0,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,
    1,0,0,0,1,1,1,0,1,0,0,1,0,0,1,1,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1,
    1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,1,
    1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1,
    1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1,0,1,
    1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1,
    1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,
    1,0,0,1,0,1,0,1,0,0,0,1,0,0,1,1,0,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

//some textures raw pixels data
static uint32_t*    wallTexturePixels;
static uint32_t*    floorTexturePixels;
static uint32_t*    ceilingTexturePixels;
static uint32_t*    drawBuff;
static int32_t      cwidth, cheight;

//some textures size
static int32_t      wallTextureWidth, wallTextureHeight;
static int32_t      floorTextureWidth, floorTextureHeight;
static int32_t      ceilingTextureWidth, ceilingTextureHeight;

double arcToRad(double arcAngle)
{
    return arcAngle * M_PI / (double)ANGLE180;
}

void drawLineBuffer(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
    //validate range
    if (x1 < 0 || x1 > cwidth - 1 || x2 < 0 || x2 > cwidth - 1 || y1 < 0 || y1 > cheight - 1 || y2 < 0 || y2 > cheight - 1) return;

    const int32_t deltaX = abs(x2 - x1); //the difference between the x's
    const int32_t deltaY = abs(y2 - y1); //the difference between the y's
    int32_t x = x1; //start x off at the first pixel
    int32_t y = y1; //start y off at the first pixel
    int32_t incX1, incX2, incY1, incY2;
    int32_t den, num, numAdd, numPixels, currPixel;

    if (x2 >= x1) //the x-values are increasing
    {
        incX1 = 1;
        incX2 = 1;
    }
    else //the x-values are decreasing
    {
        incX1 = -1;
        incX2 = -1;
    }
    if (y2 >= y1) //the y-values are increasing
    {
        incY1 = 1;
        incY2 = 1;
    }
    else //the y-values are decreasing
    {
        incY1 = -1;
        incY2 = -1;
    }
    if (deltaX >= deltaY) //there is at least one x-value for every y-value
    {
        incX1 = 0; //don't change the x when numerator >= denominator
        incY2 = 0; //don't change the y for every iteration
        den = deltaX;
        num = deltaX / 2;
        numAdd = deltaY;
        numPixels = deltaX; //there are more x-values than y-values
    }
    else //there is at least one y-value for every x-value
    {
        incX2 = 0; //don't change the x for every iteration
        incY1 = 0; //don't change the y when numerator >= denominator
        den = deltaY;
        num = deltaY / 2;
        numAdd = deltaX;
        numPixels = deltaY; //there are more y-values than x-values
    }

    for (currPixel = 0; currPixel < numPixels; currPixel++)
    {
        //draw the current pixel to initScreen buffer
        drawBuff[(y % cheight) * cwidth + (x % cwidth)] = color;
        num += numAdd;  //increase the numerator by the top of the fraction

        if (num >= den) //check if numerator >= denominator
        {
            num -= den; //calculate the new numerator value
            x += incX1; //change the x as appropriate
            y += incY1; //change the y as appropriate
        }

        x += incX2; //change the x as appropriate
        y += incY2; //change the y as appropriate
    }
}

void drawWallSliceRectangleTinted(int32_t x, int32_t y, int32_t height, int32_t offsetX, double brightnessLevel)
{
    //range check
    if (x > cwidth - 1) x = cwidth - 1;
    if (y > cheight - 1) y = cheight - 1;
    if (brightnessLevel < 0) brightnessLevel = 0;
    if (brightnessLevel > 1) brightnessLevel = 1;

    int32_t heightToDraw = height;
    int32_t targetOffset = y * cwidth + x;
    const int32_t lastSourceOffset = offsetX + wallTextureWidth * wallTextureHeight;
    
    //clip bottom
    if (y + heightToDraw > cheight) heightToDraw = cheight - y;

    int32_t error = 0;

    //we need to check this, otherwise, program might crash when trying
    //to fetch the shade if this condition is true (possible if height is 0)
    if (heightToDraw < 0) return;

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
        uint8_t* color = (uint8_t*)&wallTexturePixels[offsetX];

        //while there's a row to draw & not end of drawing area
        while (error >= wallTextureWidth)
        {
            error -= wallTextureWidth;
            if (targetOffset >= 0)
            {
                //make target pixel and color
                uint8_t* pixel = (uint8_t*)&drawBuff[targetOffset];

                //draw the pixel
                pixel[2] = uint8_t(color[2] * brightnessLevel);
                pixel[1] = uint8_t(color[1] * brightnessLevel);
                pixel[0] = uint8_t(color[0] * brightnessLevel);
            }
            targetOffset += cwidth;

            //clip bottom (just return if we reach bottom)
            heightToDraw--;
            if (heightToDraw < 1) return;
        }

        offsetX += wallTextureWidth;
        if (offsetX > lastSourceOffset) offsetX = lastSourceOffset;
    }
}

void drawFillRectangle(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
    uint32_t targetOffset = cwidth * y + x;
    for (int32_t h = 0; h < height; h++)
    {
        for (int32_t w = 0; w < width; w++) drawBuff[targetOffset++] = color;
        targetOffset += (cwidth - width);
    }
}

void initData()
{
    int32_t i;
    double radian = 0;

    loadTexture(&wallTexturePixels, &wallTextureWidth, &wallTextureHeight, "assets/wallr.png");
    loadTexture(&floorTexturePixels, &floorTextureWidth, &floorTextureHeight, "assets/floor.png");
    loadTexture(&ceilingTexturePixels, &ceilingTextureWidth, &ceilingTextureHeight, "assets/ceil.png");

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
}

void drawOverheadMap()
{
    for (int32_t row = 0; row < WORLD_MAP_HEIGHT; row++)
    {
        for (int32_t col = 0; col < WORLD_MAP_WIDTH; col++)
        {
            if (worldMap[row * WORLD_MAP_WIDTH + col]) drawFillRectangle(PROJECTION_PLANE_WIDTH + col * MINI_MAP_WIDTH, row * MINI_MAP_WIDTH, MINI_MAP_WIDTH, MINI_MAP_WIDTH, RGB2INT(0, 0, 0));
            else drawFillRectangle(PROJECTION_PLANE_WIDTH + col * MINI_MAP_WIDTH, row * MINI_MAP_WIDTH, MINI_MAP_WIDTH, MINI_MAP_WIDTH, RGB2INT(255, 255, 255));
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
    drawLineBuffer(playerMapX, playerMapY, PROJECTION_PLANE_WIDTH + x * MINI_MAP_WIDTH / TILE_SIZE, y * MINI_MAP_WIDTH / TILE_SIZE, RGB2INT(0, 255, 0));
}

//draw player POV on the overhead map (for illustartion purpose)
//this is not part of the ray-casting process
void drawPlayerPOVOnOverheadMap()
{
    //draw a red line indication the player's direction
    drawLineBuffer(playerMapX, playerMapY, int32_t(playerMapX + cosTable[playerArc] * 10), int32_t(playerMapY + sinTable[playerArc] * 10), RGB2INT(255, 0, 0));
}

void doRayCasting()
{
    //horizotal or vertical coordinate of intersection theoritically, this will be multiple of TILE_SIZE, but some trick did here might cause the values off by 1
    int32_t verticalGrid = 0, horizontalGrid = 0;

    //how far to the next bound (this is multiple of tile size)
    int32_t distToNextVerticalGrid = 0, distToNextHorizontalGrid = 0;

    //x, y intersections
    double intersectionX = 0, intersectionY = 0;
    double distToNextIntersectionX = 0, distToNextIntersectionY = 0;

    //the current cell that the ray is in
    int32_t gridIndexX = 0, gridIndexY = 0;

    //the distance of the x and y ray intersections from the viewpoint
    double distToVerticalGridBeingHit = 0;
    double distToHorizontalGridBeingHit = 0;

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
        int32_t mapIndex = 0;
        double tmpX = 0, tmpY = 0;

        //ray is facing down
        if (castArc > ANGLE0 && castArc < ANGLE180)
        {
            //truncuate then add to get the coordinate of the FIRST grid (horizontal wall) that is in front of the player (this is in pixel unit)
            horizontalGrid = playerY / TILE_SIZE * TILE_SIZE + TILE_SIZE;

            //compute distance to the next horizontal wall
            distToNextHorizontalGrid = TILE_SIZE;

            tmpX = itanTable[castArc] * (double(horizontalGrid) - playerY);
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
            tmpX = itanTable[castArc] * (double(horizontalGrid) - playerY);
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
                mapIndex = gridIndexY * WORLD_MAP_WIDTH + gridIndexX;

                //if we've looked as far as outside the map range, then bail out
                if (gridIndexX >= WORLD_MAP_WIDTH || gridIndexY >= WORLD_MAP_HEIGHT || gridIndexX < 0 || gridIndexY < 0)
                {
                    distToHorizontalGridBeingHit = DBL_MAX;
                    break;
                }
                //if the grid is not an Opening, then stop
                else if (worldMap[mapIndex])
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
            tmpY = tanTable[castArc] * (double(verticalGrid) - playerX);
            intersectionY = tmpY + playerY;
        }
        //RAY FACING LEFT
        else
        {
            verticalGrid = playerX / TILE_SIZE * TILE_SIZE;
            distToNextVerticalGrid = -TILE_SIZE;
            tmpY = tanTable[castArc] * (double(verticalGrid) - playerX);
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
                mapIndex = gridIndexY * WORLD_MAP_WIDTH + gridIndexX;

                if (gridIndexX >= WORLD_MAP_WIDTH || gridIndexY >= WORLD_MAP_HEIGHT || gridIndexX < 0 || gridIndexY < 0)
                {
                    distToVerticalGridBeingHit = DBL_MAX;
                    break;
                }
                else if (worldMap[mapIndex])
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
        double ratio = 0;
        double scale = 0;
        double distance = 0;
        
        int32_t offsetX = 0;        //x offset of drawing texture
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
            scale = double(PLAYER_PROJECTION_PLAN) * WALL_HEIGHT / distance;
            topOfWall = bottomOfWall - int32_t(scale);
            offsetX = int32_t(intersectionX) % TILE_SIZE;
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
            scale = double(PLAYER_PROJECTION_PLAN) * WALL_HEIGHT / distance;
            topOfWall = bottomOfWall - int32_t(scale);
            offsetX = int32_t(intersectionY) % TILE_SIZE;
        }

        //add simple shading so that farther wall slices appear darker.
        //use arbitrary value of the farthest distance.  
        //trick to give different shades between vertical and horizontal (you could also use different textures for each if you wish to)
        drawWallSliceRectangleTinted(castColumn, topOfWall, bottomOfWall - topOfWall + 1, offsetX, BASE_LIGHT_VALUE / floor(distance));
    
        //validate range
        if (topOfWall < 0) topOfWall = 0;
        if (bottomOfWall < 0) bottomOfWall = 0;
        if (topOfWall > cheight - 1) topOfWall = cheight - 1;
        if (bottomOfWall > cheight - 1) bottomOfWall = cheight - 1;

        //FLOOR CASTING at the simplest! Try to find ways to optimize this, you can do it!
        if (floorTexturePixels)
        {
            //find the first bit so we can just add the width to get the next row (of the same column)
            uint32_t targetOffset = bottomOfWall * cwidth + castColumn;

            for (int32_t row = bottomOfWall; row < PROJECTION_PLANE_HEIGHT; row++)
            {
                const double straightDistance = double(playerHeight) / (double(row) - projectionPlaneCenterY) * PLAYER_PROJECTION_PLAN;
                const double actualDistance = straightDistance * fishTable[castColumn];

                int32_t endY = int32_t(actualDistance * sinTable[castArc]);
                int32_t endX = int32_t(actualDistance * cosTable[castArc]);

                //translate relative to viewer coordinates:
                endX += playerX;
                endY += playerY;

                //get the tile intersected by ray:
                int32_t cellX = endX / TILE_SIZE;
                int32_t cellY = endY / TILE_SIZE;

                //make sure the tile is within our map
                if (cellX < WORLD_MAP_WIDTH && cellY < WORLD_MAP_HEIGHT && cellX >= 0 && cellY >= 0 && endX >= 0 && endY >= 0)
                {
                    //find offset of tile and column in texture
                    endY %= TILE_SIZE;
                    endX %= TILE_SIZE;

                    //pixel to draw
                    uint32_t sourceOffset = endY * floorTextureWidth + endX;

                    //cheap shading trick
                    double brightnessLevel = BASE_LIGHT_VALUE / actualDistance;
                    if (brightnessLevel < 0) brightnessLevel = 0;
                    if (brightnessLevel > 1) brightnessLevel = 1;

                    //make target pixel and color
                    uint8_t* pixel = (uint8_t*)&drawBuff[targetOffset];
                    uint8_t* color = (uint8_t*)&floorTexturePixels[sourceOffset];

                    //draw the pixel
                    pixel[2] = uint8_t(color[2] * brightnessLevel);
                    pixel[1] = uint8_t(color[1] * brightnessLevel);
                    pixel[0] = uint8_t(color[0] * brightnessLevel);

                    //go to the next pixel (directly under the current pixel)
                    targetOffset += cwidth;
                }
            }
        }
        
        //CEILING CASTING at the simplest! Try to find ways to optimize this, you can do it!
        if (ceilingTexturePixels)
        {
            //find the first bit so we can just add the width to get the next row (of the same column)
            uint32_t targetOffset = topOfWall * cwidth + castColumn;

            for (int32_t row = topOfWall; row >= 0; row--)
            {
                const double zoom = (double(WALL_HEIGHT) - playerHeight) / (double(projectionPlaneCenterY) - row);
                const double diagonalDistance = zoom * PLAYER_PROJECTION_PLAN * fishTable[castColumn];

                int32_t endY = int32_t(diagonalDistance * sinTable[castArc]);
                int32_t endX = int32_t(diagonalDistance * cosTable[castArc]);

                //translate relative to viewer coordinates:
                endX += playerX;
                endY += playerY;

                //Get the tile intersected by ray:
                const int32_t cellX = endX / TILE_SIZE;
                const int32_t cellY = endY / TILE_SIZE;

                //make sure the tile is within our map
                if (cellX < WORLD_MAP_WIDTH && cellY < WORLD_MAP_HEIGHT && cellX >= 0 && cellY >= 0 && endX >= 0 && endY >= 0)
                {
                    //find offset of tile and column in texture
                    endY %= TILE_SIZE;
                    endX %= TILE_SIZE;

                    //pixel to draw
                    const uint32_t sourceOffset = endY * ceilingTextureWidth + endX;
                    
                    //cheap shading trick
                    double brightnessLevel = BASE_LIGHT_VALUE / diagonalDistance;
                    if (brightnessLevel < 0) brightnessLevel = 0;
                    if (brightnessLevel > 1) brightnessLevel = 1;

                    //make target pixel and color
                    uint8_t* pixel = (uint8_t*)&drawBuff[targetOffset];
                    uint8_t* color = (uint8_t*)&ceilingTexturePixels[sourceOffset];
                    
                    //draw the pixel
                    pixel[2] = uint8_t(color[2] * brightnessLevel);
                    pixel[1] = uint8_t(color[1] * brightnessLevel);
                    pixel[0] = uint8_t(color[0] * brightnessLevel);

                    //go to the next pixel (directly above the current pixel)
                    targetOffset -= cwidth;
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
    double time = 0; //time of current frame
    double oldTime = 0; //time of previous frame

    loadFont("assets/sysfont.xfn", 0);
    initScreen(SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, "Raycasting-Demo -- Keys control: Arrows move; Q/Z: vertical lookup; E/C: fly & crouch");
    initData();

    drawBuff = (uint32_t*)getDrawBuffer(&cwidth, &cheight);
    if (!drawBuff) return;

    do
    {
        drawOverheadMap();
        doRayCasting();
        drawPlayerPOVOnOverheadMap();

        //timing for input and FPS counter
        oldTime = time;
        time = getTime();
        const double frameTime = (time - oldTime) / 1000.0; //frametime is the time this frame has taken, in seconds
        writeText(1, 1, RGB_WHITE, 0, "FPS:%.f", 1.0 / frameTime); //FPS counter
        render();
        memset(drawBuff, RGB2INT(255, 255, 255), sizeof(uint32_t) * cwidth * cheight);

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
            if (worldMap[playerCellY * WORLD_MAP_WIDTH + playerCellX + 1] && (playerCellOffsetX > (TILE_SIZE - MIN_DISTANCE_TO_WALL)))
            {
                //back player up
                playerX -= (playerCellOffsetX - (TILE_SIZE - MIN_DISTANCE_TO_WALL));
            }
        }
        else
        {
            //moving left
            if (worldMap[playerCellY * WORLD_MAP_WIDTH + playerCellX - 1] && (playerCellOffsetX < MIN_DISTANCE_TO_WALL))
            {
                //back player up
                playerX += (MIN_DISTANCE_TO_WALL - playerCellOffsetX);
            }
        }

        if (dy < 0)
        {
            //moving up
            if (worldMap[(playerCellY - 1) * WORLD_MAP_WIDTH + playerCellX] && (playerCellOffsetY < MIN_DISTANCE_TO_WALL))
            {
                //back player up 
                playerY += (MIN_DISTANCE_TO_WALL - playerCellOffsetY);
            }
        }
        else
        {
            //moving down                                  
            if (worldMap[(playerCellY + 1) * WORLD_MAP_WIDTH + playerCellX] && (playerCellOffsetY > (TILE_SIZE - MIN_DISTANCE_TO_WALL)))
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

    free(wallTexturePixels);
    free(floorTexturePixels);
    free(ceilingTexturePixels);
    freeFont(0);
    cleanup();
}

void gfxEffects32()
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
