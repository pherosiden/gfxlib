#include "gfxlib.h"

//CHR font information
#define CHR_WIDTH   8           //character width
#define CHR_HEIGHT  16          //character height
#define CHR_NUM     19          //number of character in font table
#define CHR_MAX     250         //max of character in font table
#define CHR_START   0           //start of character

#define LIMITX      640
#define LIMITY      400
#define SIGNED(x)   (((x) > 0) ? 1 : ((x) < 0) ? -1 : 0)

//Font buffer
typedef uint8_t FNTBUFF[CHR_WIDTH][CHR_HEIGHT];

//VNI font table
FNTBUFF     chrPixels[CHR_MAX] = {0};

//circle palette
RGB         cpal[256] = { 0 };

//Character buffer
int32_t     grData[120][2] = {0};
uint8_t     chrBuff[CHR_NUM * CHR_WIDTH][CHR_HEIGHT] = {0};

//Max coordinate buffer
int32_t     maxHeight[LIMITX] = {0};
int32_t     minHeight[LIMITX] = {0};

int32_t     c1 = 0, c2 = 0, c3 = 0, c4 = 0;
int32_t     lines = 0, points = 0;
int32_t     visiPrec = 0, visiCour = 0;

double      theta = 0, phi = 0;
double      gx1 = 0, gx2 = 0, gy1 = 0, gy2 = 0;
double      f1 = 0, f2 = 0, f3 = 0, f4 = 0;
double      incX = 0, incY = 0, echX = 0, echY = 0;

double      u, debutU = 0, finU = 0, du = 0;
double      v, debutV = 0, finV = 0, dv = 0;

char        vue = 'r';

double      (*FX)(double, double) = NULL;
double      (*FY)(double, double) = NULL;
double      (*FZ)(double, double) = NULL;

void findRepeat(double *rept)
{
    double lx = 0.0, ly = 0.0, r = 0.0;
    double lim1 = 0.0, lim2 = 0.0;
    double rmax = 200.0 * M_PI;
    double fx = 0.0, fy = 0.0;

    if (!rept) return;

    do {
        *rept += 2 * M_PI;
        r = sin(3 * *rept);
        lx = r * cos(*rept);
        ly = r * sin(*rept);
        lim1 = fabs(lx - fx);
        lim2 = fabs(ly - fy);
    } while ((lim1 >= 1000000.0 || lim2 >= 1000000.0) && *rept <= rmax); 
}

void drawCylodiod(int32_t xc, int32_t yc, int32_t rd, uint8_t a, uint8_t b, double rept, uint32_t col)
{
    double angle = 0.0;
    const double x1 = xc + rd * sin(3 * angle) * cos(angle) * cos(a * angle);
    const double y1 = yc + rd * sin(3 * angle) * sin(angle) * sin(b * angle);
    moveTo(int32_t(x1), int32_t(y1));

    while (angle < rept)
    {
        const double x2 = xc + rd * sin(3 * angle) * cos(angle) * cos(a * angle);
        const double y2 = yc + rd * sin(3 * angle) * sin(angle) * sin(b * angle);
        lineTo(int32_t(x2), int32_t(y2), uint32_t(angle + col));
        angle += 0.001;
    }
    render();
}

void drawPolygon(int32_t xc, int32_t yc, int32_t rd, uint8_t odre, uint8_t pas)
{
    int32_t angle = 0;
    
    while (angle < 360)
    {
        const double rad = M_PI * angle / 180;
        moveTo(int32_t(xc + rd * cos(rad)), int32_t(yc + rd * sin(rad)));
        lineTo(int32_t(xc + rd * cos(odre * rad)), int32_t(yc + rd * sin(odre * rad)), (angle % 192) + 16);
        angle += pas;
    }
    render();
}

void rotatePolygon(POINT2D *pt, int32_t n, int32_t xc, int32_t yc, int32_t rd, int32_t num, uint8_t odre, uint32_t col)
{
    int32_t i = 0, j = 0;
    double angle = 0.0;

    if (!pt) return;

    for (i = 0; i < n; i++)
    {
        pt[i].x = xc + rd * cos(angle);
        pt[i].y = yc - rd * sin(angle);
        angle += 2 * M_PI / n;
    }

    for (j = 0; j < num; j++)
    {
        drawPolygon(pt, n, j % 69 + col);
        for (i = 0; i < n; i++)
        {
            pt[i].x = pt[i].x + (pt[(i + 1) % n].x - pt[i].x) / odre;
            pt[i].y = pt[i].y + (pt[(i + 1) % n].y - pt[i].y) / odre;
        }
    }
    render();
}

void randomPoly(POINT2D *pt, int32_t n, int32_t xm, int32_t ym, int32_t num, uint8_t odre, uint32_t col)
{
    int32_t i = 0, j = 0, k = 0;

    if (!pt) return;

    srand(uint32_t(time(NULL)));

    for (i = 0; i < n; i++)
    {
        pt[i].x = rand() % xm;
        pt[i].y = rand() % ym;
    }

    for (j = 0; j < num; j++)
    {
        drawPolygon(pt, n, (j % 69) + col);
        for (k = 0; k < n; k++)
        {
            pt[k].x = pt[k].x + (pt[(k + 1) % n].x - pt[k].x) / odre;
            pt[k].y = pt[k].y + (pt[(k + 1) % n].y - pt[k].y) / odre;
        }
    }
    render();
}

void drawHexagon(POINT2D *pt, int32_t num, int32_t xc, int32_t yc, int32_t n, int32_t rd, uint8_t odre, uint32_t col)
{
    double angle = 0.0;
    const double coef = 2 * M_PI / num;
    int32_t i = 0, j = 0, k = 0, m = 0;

    if (!pt) return;

    for (i = 1; i <= num; i++)
    {
        pt[0].x = xc;
        pt[0].y = yc;

        angle = (i - 1.0) * coef;
        pt[1].x = xc + rd * cos(angle);
        pt[1].y = yc - rd * sin(angle);

        angle = i * coef;
        pt[2].x = xc + rd * cos(angle);
        pt[2].y = yc - rd * sin(angle);

        for (j = 0; j < n; j++)
        {
            drawPolygon(pt, 3, (j % 69) + col);		
            if (i % 2)
            {
                for (k = 2; k > 0; k--)
                {
                    pt[k].x = pt[k].x + (pt[(k - 1) % 3].x - pt[k].x) / odre;
                    pt[k].y = pt[k].y + (pt[(k - 1) % 3].y - pt[k].y) / odre;
                }

                pt[0].x = pt[0].x + (pt[2].x - pt[0].x) / odre;
                pt[0].y = pt[0].y + (pt[2].y - pt[0].y) / odre;

            }
            else
            {
                for (m = 0; m < 3; m++)
                {
                    pt[m].x = pt[m].x + (pt[(m + 1) % 3].x - pt[m].x) / odre;
                    pt[m].y = pt[m].y + (pt[(m + 1) % 3].y - pt[m].y) / odre;
                }
            }
        }
    }
    render();
}

void graphDemo0(int32_t xc, int32_t yc, int32_t xr, int32_t yr)
{
    double a = 0.0;
    const double x = xr * 0.4;
    const double y = yr * 0.4;

    for (int32_t i = 0; i < 800; i++)
    {
        const double x0 = xc + xr * cos(a);
        const double y0 = yc + yr * sin(5 * a) * cos(a / 1.5);

        const double m = sin(a);
        const double x1 = x * m;
        const double y1 = y * m;

        drawLine(int32_t(x0), int32_t(y0), int32_t(x0 + x1), int32_t(y0 + y1), i / 12 + 32);
        drawLine(int32_t(x0), int32_t(y0), int32_t(x0 + x1), int32_t(y0 - y1), i / 12 + 32);
        a += M_PI / 400;
    }
    render();
}

void graphDemo1(int32_t xc, int32_t yc, int32_t xr, int32_t yr)
{
    double angle = 0.0;

    for (int32_t i = 0; i < 500; i++)
    {
        const double m = sin(angle);
        const double n = cos(angle);
        const double x1 = xc + (1.2 * (xr + xr / 3.0 * (1 + 0.5 * cos(12 * angle)) * n) * n);
        const double x2 = xc + (1.2 * (yr + yr / 3.0 * (1 + 0.5 * sin(12 * angle)) * n) * n);
        const double y1 = yc - (xr + xr / 3.0 * (1 + 0.5 * cos(10 * angle)) * m) * m;
        const double y2 = yc - (yr + yr / 2.0 * (1 + 0.5 * cos(15 * angle)) * m) * m;
        drawLine(int32_t(x1), int32_t(y1), int32_t(x2), int32_t(y2), i / 7 + 32);
        angle += M_PI / 250.5;
    }
    render();
}

void graphDemo2(int32_t xc, int32_t yc, int32_t r)
{
    double angle = 0.0;

    for (int32_t i = 0; i < 1600; i++)
    {
        const double f = r * (1 + 0.25 * cos(20 * angle)) * (1 + sin(4 * angle));
        const double x1 = xc + f * cos(angle);
        const double x2 = xc + f * cos(angle + M_PI / 5);
        const double y1 = yc - f * sin(angle);
        const double y2 = yc - f * sin(angle + M_PI / 5);
        drawLine(int32_t(x1), int32_t(y1), int32_t(x2), int32_t(y2), i / 23 + 32);
        angle += M_PI / 800;
    }
    render();
}

void graphDemo3(int32_t xc, int32_t yc, int32_t r)
{
    double angle = 0.0;
    
    for (int32_t i = 0; i < 1600; i++)
    {
        const double f = r * (1 + 0.25 * cos(4 * angle)) * (1 + sin(8 * angle));
        const double x1 = xc + f * cos(angle);
        const double x2 = xc + f * cos(angle + M_PI / 8);
        const double y1 = yc - f * sin(angle);
        const double y2 = yc - f * sin(angle + M_PI / 8);
        drawLine(int32_t(x1), int32_t(y1), int32_t(x2), int32_t(y2), i / 23 + 32);
        angle += M_PI / 800;
    }
    render();
}

void graphDemo4(int32_t xc, int32_t yc, int32_t r)
{
    double angle = 0.0;

    for (int32_t i = 0; i < 800; i++)
    {
        const double e = r * (1 + 0.5 * sin(2.5 * angle));
        const double x1 = xc + e * cos(angle);
        const double x2 = xc + e * cos(angle + M_PI / 4);
        const double y1 = yc - e * sin(angle);
        const double y2 = yc - e * sin(angle + M_PI / 4);
        drawLine(int32_t(x1), int32_t(y1), int32_t(x2), int32_t(y2), i / 12 + 32);
        angle += M_PI / 200;
    }
    render();
}

void graphDemo5(int32_t xi, int32_t yi, int32_t r, int32_t xr, int32_t yr)
{
    double sx = 0, sy = 0;

    for (int32_t n = 2; n <= 7; n++)
    {
        for (int32_t j = 1; j <= 6; j++)
        {
            double angle = 0.0;
            const int32_t k = !(n % 2) ? 2 : 1;
            
            for (int32_t i = 0; i <= 15 * n * k; i++)
            {
                const double e = r / 5.0 * sin(angle * n * j) + r * sin(n * angle);
                const double x = e * cos(angle) + xr * (n - 2.0) + xi;
                const double y = e * sin(angle) + yr * (j - 1.0) + yi;

                if (!i)
                {
                    moveTo(int32_t(x), int32_t(y));
                    sx = x;
                    sy = y;
                }
                
                lineTo(int32_t(x), int32_t(y), 6 * n + j + 48);
                angle += M_PI / 15.0 / n;
            }
            lineTo(int32_t(sx), int32_t(sy), 6 * n + j + 48);
        }
    }
    render();
}

void graphDemo6(int32_t xc, int32_t yc, int32_t r)
{
    int32_t xx[120] = {0};
    int32_t yy[120] = {0};
   
    int32_t i = 0;
    double alpha = 0.0;

    for (i = 0; i < 120; i++)
    {
        const double val = 66 * sqrt(fabs(cos(3 * alpha))) + 12 * sqrt(fabs(cos(9 * alpha)));
        xx[i] = int32_t(val * cos(alpha) * 1.2 / 320.0 * r);
        yy[i] = int32_t(val * sin(alpha) / 320.0 * r);
        alpha += M_PI / 60;
    }

    const double x = 4.0 * r;
    double sx = 0, sy = 0;

    for (int32_t py = 1; py <= 2; py++)
    {
        for (int32_t px = 1; px <= 8; px++)
        {
            for (i = 0; i < 120; i++)
            {
                const double x1 = double(xx[i]) + (px * r >> 1) - (r >> 2);
                const double y1 = double(yy[i]) + (py * r >> 1) - (r >> 2);
                const double rad = 2 * M_PI * (x - x1) / x;
                const double x2 = xc + y1 * cos(rad);
                const double y2 = yc + y1 * sin(rad);
    
                if (i == 0)
                {
                    moveTo(int32_t(x2), int32_t(y2));
                    sx = x2;
                    sy = y2;
                }
                lineTo(int32_t(x2), int32_t(y2), (120 * (2 * py + px) + i) / 22 + 32);
            }
            lineTo(int32_t(sx), int32_t(sy), (120 * (2 * py + px) + i) / 22 + 32);
        }
    }
    render();
}

void graphDemo7(int32_t xc, int32_t yc, int32_t r)
{
    int32_t xx[120] = {0};
    int32_t yy[120] = {0};

    int32_t i = 0;
    double alpha = 0.0;

    for (i = 0; i < 120; i++)
    {
        const double val = 40 * sin(4 * (alpha + M_PI / 8));
        const double m = sin(alpha);
        const double n = cos(alpha);

        xx[i] = int32_t((val * n + 45 * n * n * n) / 320.0 * r);
        yy[i] = int32_t((val * m + 45 * m * m * m) / 320.0 * r);

        alpha += M_PI / 60;
    }

    const double x = 4.0 * r;
    double sx = 0, sy = 0;

    for (int32_t py = 1; py <= 2; py++)
    {
        for (int32_t px = 1; px <= 8; px++)
        {
            for (i = 0; i < 120; i++)
            {
                const double x1 = double(xx[i]) + (px * r >> 1) - (r >> 2);
                const double y1 = double(yy[i]) + (py * r >> 1) - (r >> 2);
                const double rad = 2 * M_PI * (x - x1) / x;
                const double x2 = xc + y1 * cos(rad);
                const double y2 = yc + y1 * sin(rad);

                if (i == 0)
                {
                    moveTo(int32_t(x2), int32_t(y2));
                    sx = x2;
                    sy = y2;
                }
                lineTo(int32_t(x2), int32_t(y2), (120 * (2 * py + px) + i) / 22 + 32);
            }
            lineTo(int32_t(sx), int32_t(sy), (120 * (2 * py + px) + i) / 22 + 32);
        }
    }
    render();
}

void graphDemo8(int32_t xc, int32_t yc, int32_t d, int32_t r)
{
    int32_t xx[120] = {0};
    int32_t yy[120] = {0};
    int32_t i = 0;

    double alpha = 0.0;
    const double un = 12.0;
    const double uv = d / un;
    const double k = uv / 2.0;
    const double sc = uv / 100.0;
    const double dd = d / 2.0;

    for (i = 0; i < 120; i++)
    {
        double val = 90 * (0.8 + 0.2 * sin(12 * alpha)) * (0.5 + 0.5 * sin(4 * alpha));
        xx[i] = int32_t(val * cos(alpha));
        yy[i] = int32_t(val * sin(alpha));
        alpha += M_PI / 60;
    }

    double sx = 0, sy = 0;
    for (double px = 1; px <= un; px++)
    {
        for (double py = 1; py <= un; py++)
        {
            for (i = 0; i < 120; i++)
            {
                double x = xx[i] * sc + px * uv - dd - k;
                double y = yy[i] * sc + py * uv - dd - k;
                double sq = x * x + y * y;

                if (sq < double(r) * r)
                {
                    const double s = (x < 0) ? -1 : 1;
                    const double angle = atan(y / (x + 0.1));
                    const double m = r * sin(2 * atan(sqrt(sq) / r));
                    x = s * m * cos(angle);
                    y = s * m * sin(angle);
                }

                x = x * 23.0 / 15 + xc;
                y = y * 23.0 / 15 + yc;

                if (i == 0)
                {
                    moveTo(int32_t(x), int32_t(y));
                    sx = x;
                    sy = y;
                }
                lineTo(int32_t(x), int32_t(y), uint32_t((120 * (px + py) + i) / 42 + 32));
            }
            lineTo(int32_t(sx), int32_t(sy), uint32_t((120 * (px + py) + i) / 42 + 32));
        }
    }
    render();
}

void graphDemo9(int32_t xc, int32_t yc, double rd)
{
    const int32_t data[] = {7, 436, 245, 17, 775, 180, 31, 1020, 130};

    int32_t x = 0, y = 0;
    int32_t px = xc, py = yc;

    srand(uint32_t(time(NULL)));

    double r = 50 * rd;
    int32_t s = 8 - (rand() % 5);
    int32_t k = !(s % 2) ? 2 : 1;
    double a = 0.0;

    while (a <= k * M_PI + M_PI / 10.0 / s * 1.0)
    {
        x = int32_t((r / 4 * sin(3.0 * s * a) + r * sin(s * a)) * cos(a) + px);
        y = int32_t((r / 4 * sin(3.0 * s * a) + r * sin(s * a)) * sin(a) + py);
        if (a == 0.0) moveTo(x, y);
        lineTo(x, y, 32);
        a += M_PI / 8.0 / s * 1.0;
    }

    double ls = 0;
    int32_t i = 0;

    for (int32_t re = 0; re < 3; re++)
    {
        const int32_t ste = data[3 * re];
        const double di = data[3 * re + 1] / 6.0 * rd;
        r = data[3 * re + 2] / 6.0 * rd;

        if (re == 1) ls = (2 * M_PI / ste) - 0.1;
        else ls = 0.0;

        double aa = 0.0;

        while (aa <= 2 * M_PI - ls)
        {
            px = int32_t(xc + di * cos(aa));
            py = int32_t(yc + di * sin(aa));
            s = 8 - (rand() % 5);
            k = !(s % 2) ? 2 : 1;
            a = 0.0;

            while (a <= k * M_PI + M_PI / 10.0 / s)
            {

                x = int32_t((r / 4 * sin(3.0 * s * a) + r * sin(s * a)) * cos(a) + px);
                y = int32_t((r / 4 * sin(3.0 * s * a) + r * sin(s * a)) * sin(a) + py);

                if (a == 0.0) moveTo(x, y);
                lineTo(x, y, i + 33);

                a += M_PI / 8.0 / s * 1.0;
            }
            
            aa += 2 * M_PI / ste;
            i++;
        }
    }

    a = 0.0;
    i = 0;

    while (a <= 14 * M_PI)
    {
        x = int32_t(xc + 250 * rd * (1 + 1.0 / 5 * sin(9.06 * a)) * cos(a));
        y = int32_t(yc + 250 * rd * (1 + 1.0 / 5 * sin(9.06 * a)) * sin(a));
        if (a == 0.0) moveTo(x, y);
        lineTo(x, y, i % 72 + 32);
        a += M_PI / 60;
        i++;
    }
    render();
}

void initDemo10(int32_t num, int32_t n)
{
    int32_t i = 0;
    double a = 0.0, r = 0.0;

    switch (num)
    {
    case 1:
        for (i = 0; i < 120; i++)
        {
            r = 100 * (0.5 + 0.5 * sin(n * a));
            grData[i][0] = int32_t(r * cos(a));
            grData[i][1] = int32_t(r * sin(a));
            a += M_PI / 60;
        }
        break;

    case 2:
        for (i = 0; i < 120; i++)
        {
            r = 100 * (0.82 + 0.18 * sin(3.0 * n * a)) * (0.5 + 0.5 * sin(n * a));
            grData[i][0] = int32_t(r * cos(a));
            grData[i][1] = int32_t(r * sin(a));
            a +=  M_PI / 60;
        }
        break;

    case 3:
        for (i = 0; i < 120; i++)
        {
            r = 100 * (0.33 * sin(0.5 * n * a) + sin(n * a));
            grData[i][0] = int32_t(r * cos(2 * a));
            grData[i][1] = int32_t(r * sin(2 * a));
            a += M_PI / 30;
        }
        break;

    case 4:
        for (i = 0; i < 120; i++)
        {
            grData[i][0] = int32_t(100 * sin(n * a) * cos(a));
            grData[i][1] = int32_t(100 * sin(n * a + a) * sin(a));
            a += M_PI / 60;
        }
        break;
    }
}

void graphDemo10(int32_t xc, int32_t yc, int32_t rx, int32_t ry, int32_t col)
{
    int32_t i = 0;
    int32_t data[120][2] = {0};

    for (i = 0; i < 120; i++)
    {
        data[i][0] = grData[i][0] * rx / 100 + xc;
        data[i][1] = grData[i][1] * ry / 100 + yc;
    }

    for (i = 0; i < 119; i++) drawLine(data[i][0], data[i][1], data[i + 1][0], data[i + 1][1], col);
    drawLine(data[119][0], data[119][1], data[0][0], data[0][1], col);
    render();
}

void makePalette(uint8_t n, uint8_t r, uint8_t g, uint8_t b)
{
    int32_t i = 0, white = 10;
    
    for (i = 0; i <= 63 - white; i++)
    {
        cpal[n + i].r = ((r * i) / (63 - white)) << 2;
        cpal[n + i].g = ((g * i) / (63 - white)) << 2;
        cpal[n + i].b = ((b * i) / (63 - white)) << 2;
    }

    for (i = 0; i <= white; i++)
    {
        cpal[n + i + 63 - white].r = (r + (63 - r) * i / white) << 2;
        cpal[n + i + 63 - white].g = (g + (63 - g) * i / white) << 2;
        cpal[n + i + 63 - white].b = (b + (63 - b) * i / white) << 2;
    }
}

void graphDemo11()
{
    uint32_t frames = 0;

    srand(uint32_t(time(NULL)));
    makePalette(0, 63, 32, 16);
    makePalette(64, 32, 63, 16);
    makePalette(128, 16, 16, 63);
    makePalette(128 + 64, 63, 16, 16);
    setPalette(cpal);

    while (frames < 200 && !finished(SDL_SCANCODE_RETURN))
    {
        const int32_t x = rand() % getMaxX();
        const int32_t y = rand() % getMaxY();
        const int32_t col = (rand() % 4) << 6;
        for (int32_t i = 0; i < 64; i++) fillCircle(x + (64 - i) / 2, y + (64 - i) / 2, (64 - i) * 2, i + col);
        render();
        delay(FPS_90);
        frames++;
    }
}

void checkBounds(int32_t a, int32_t c, int32_t *b)
{
    if (!b) return;
    if (a > c) *b = -1;
    else if (a < 0) *b = 1;
}

void lineBob()
{
    uint32_t frames = 0;
    const int32_t cwidth = getBufferWidth();
    const int32_t cheight = getBufferHeight();
    
    srand(uint32_t(time(NULL)));

    int32_t x1 = rand() % cwidth;
    int32_t x2 = rand() % cwidth;
    int32_t y1 = rand() % cheight;
    int32_t y2 = rand() % cheight;
    
    int32_t dx1 = 1;
    int32_t dx2 = -1;
    int32_t dy1 = 1;
    int32_t dy2 = -1;
    
    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();

    while (frames < 4000 && !finished(SDL_SCANCODE_RETURN))
    {
        x1 += dx1;
        x2 += dx2;
        y1 += dy1;
        y2 += dy2;
        
        checkBounds(x1, cmx, &dx1);
        checkBounds(x2, cmx, &dx2);
        checkBounds(y1, cmy, &dy1);
        checkBounds(y2, cmy, &dy2);

        drawLineBob(x1, y1, x2, y2);
        render();
        frames++;
    }
}

void graphDemo12()
{
    makeFunkyPalette();
    lineBob();
}

void graphDemo13()
{
    POINT2D points[] = { {659, 336}, {452, 374}, {602, 128}, {509, 90}, {433, 164}, {300, 71}, {113, 166}, {205, 185}, {113, 279}, {169, 278}, {206, 334}, {263, 279}, {355, 129}, {301, 335}, {432, 204}, {433, 297}, {245, 467}, {414, 392}, {547, 523} };

    srand(uint32_t(time(NULL)));
    makeLinearPalette();
    fillPolygon(points, sizeof(points) / sizeof(points[0]), 50);

    const int32_t cx = getCenterX();
    const int32_t cy = getCenterY();
    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();

    for (int32_t j = 50; j <= cmy - 50; j++)
    {
        for (int32_t i = 50; i <= cmx - 50; i++)
        {
            if (getPixel(i, j) == 50) putPixel(i, j, 16 + (((i + j) >> 2) % 192));
        }
    }

    rotatePalette(16, 207, 192, FPS_90);
    clearScreen();

    uint32_t frames = 0;
    POINT2D randPoints[30] = { 0 };
    
    while (frames++ < 20)
    {
        randomPolygon(cx, cy, 150, 0.7, 0.4, 20, randPoints);
        fillPolygon(randPoints, 20, 50);

        for (int32_t j = 0; j <= cmy; j++)
        {
            for (int32_t i = 0; i <= cmx; i++)
            {
                if (getPixel(i, j) == 50) putPixel(i, j, 16 + (((i + j) >> 2) % 192));
            }
        }

        render();
        delay(500);
        clearScreen();

        readKeys();
        if (keyDown(SDL_SCANCODE_RETURN)) break;
        if (keyDown(SDL_SCANCODE_ESCAPE)) quit();
    }

    makeRainbowPalette();
    for (int32_t y = 0; y < cmy; y++) horizLine(0, y, cmx, 1 + (uint32_t(y / 1.87) % 255));
    rotatePalette(1, 255, 255, FPS_90);
}

double FX1(double x, double y)
{
     double ph = sqrt(x * x + y * y);
     if (ph == 0.0) ph = DBL_MIN;
     return 10 * sin(ph) / ph;
}

double FX2(double x, double y)
{
     const double ph = sqrt(x * x + y * y);
     return -ph + fabs(sin(ph));
}

double FX3(double x, double y)
{
    if (x == 0.0) x = DBL_MIN;
    if (y == 0.0) y = DBL_MIN;
    return 10 * sin(x) / x * sin(y) / y;
}

double FX4(double x, double y)
{
    if (x == 0.0) x = DBL_MIN;
    return 0.1 * (x * x + y * y) * sin(x) / x;
}

double FX5(double x, double y)
{
    return 0.2 * sin(x) * cos(y) - 3 * exp(-x * x - y * y) * cos(1.75 * (x * x + y * y));
}

double FX11(double a, double b)
{
    return 6 * cos(a) * cos(b);
}

double FY11(double a, double b)
{
  return 3 * cos(a) * sin(b);
}

double FZ11(double a, double b)
{
    b = 0;
    return 2 * sin(a) + b;
}

double FX21(double a, double b)
{
    return (6 + 3 * cos(a)) * cos(b);
}

double FY21(double a, double b)
{
  return (6 + 3 * cos(a)) * sin(b);
}

double FZ21(double a, double b)
{
    b = 0;
    return 3 * sin(a) + b;
}

double FX31(double a, double b)
{
    return (3 + 3 * cos(a)) * cos(b);
}

double FY31(double a, double b)
{
  return (3 + 3 * cos(a)) * sin(b);
}

double FZ31(double a, double b)
{
    b = 0;
    return 3 * sin(a) + b;
}

double FX41(double a, double b)
{
    b = 0;
    return a + b;
}

double FY41(double a, double b)
{
    a = 0;
    return b + a;
}

double FZ41(double a, double b)
{
    return a * a - b * b;
}

void familleDesCourbesEnU()
{
    u = debutU;
    while (u <= finU)
    {
        v = debutV;
        double x = FX(u, v);
        double y = FY(u, v);
        double z = FZ(u, v);
        deplaceEn(x, y, z);
        while (v <= finV)
        {
            x = FX(u, v);
            y = FY(u, v);
            z = FZ(u, v);
            traceVers(x, y, z, 50);
            v += dv;
        }
        u += du;
    }
}

void familleDesCourbesEnV()
{
    v = debutV;
    while (v <= finV)
    {
        u = debutU;
        double x = FX(u, v);
        double y = FY(u, v);
        double z = FZ(u, v);
        deplaceEn(x, y, z);
        while (u <= finU)
        {
            x = FX(u, v);
            y = FY(u, v);
            z = FZ(u, v);
            traceVers(x, y, z, 50);
            u += du;
        }
        v += dv;
    }
}

void initDiverses(double theta)
{
    incX = (gx2 - gx1) / points;
    incY = (gy2 - gy1) / lines;

    c1 = 70;
    c2 = 710;
    c3 = 57;
    c4 = 145;
    f1 = f2 = f3 = f4 = 0;

    memset(maxHeight, 0, sizeof(maxHeight));
    for (int32_t i = 0; i < LIMITX; i++) minHeight[i] = LIMITY;

    if (theta < 0 || theta > 180)
    {
        swapf(gx1, gx2);
        swapf(gy1, gy2);
        incX = -incX;
        incY = -incY;
    }
}

void initParameter1()
{
    gx1 = -9.2;
    gx2 = 9.2;
    gy1 = -9.2;
    gy2 = 9.2;
    lines = 90;
    vue = 'r';
    points = 120;
    theta = 45;
    phi = 22;
    initDiverses(theta);
    initProjection(theta, phi, 1);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter2()
{
    gx1 = -9.2;
    gx2 = 9.2;
    gy1 = -14.8;
    gy2 = 14.8;
    lines = 90;
    vue = 'r';
    points = 200;
    theta = 45;
    phi = 20;
    initDiverses(theta);
    initProjection(theta, phi, 1);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter3()
{
    gx1 = -9.5;
    gx2 = 9.5;
    gy1 = -9.5;
    gy2 = 9.5;
    lines = 90;
    vue = 'r';
    points = 200;
    theta = 40;
    phi = 20;
    initDiverses(theta);
    initProjection(theta, phi, 1);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter4()
{
    gx1 = -9.8;
    gx2 = 9.8;
    gy1 = -10.2;
    gy2 = 10.2;
    lines = 80;
    vue = 'r';
    points = 200;
    theta = 45;
    phi = 32;
    initDiverses(theta);
    initProjection(theta, phi, 1);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter5()
{
    gx1 = -4.0;
    gx2 = 4.0;
    gy1 = -2.8;
    gy2 = 2.8;
    lines = 50;
    vue = 'r';
    points = 200;
    theta = 40;
    phi = 15;
    initDiverses(theta);
    initProjection(theta, phi, 1);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter11()
{
    debutU = -M_PI / 2;
    finU = M_PI / 2;
    du = 0.2;
    debutV = -M_PI;
    finV = M_PI;
    dv = 0.2;
    theta = 60;
    phi = 30;
    initProjection(theta, phi, 55);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter21()
{
    debutU = -M_PI;
    finU = M_PI;
    du = 0.4;
    debutV = -M_PI;
    finV = M_PI;
    dv = 0.2;
    theta = 30;
    phi = 30;
    initProjection(theta, phi, 30);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter31()
{
    debutU = -M_PI;
    finU = M_PI;
    du = 0.4;
    debutV = -M_PI;
    finV = M_PI;
    dv = 0.2;
    theta = 40;
    phi = 30;
    initProjection(theta, phi, 40);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void initParameter41()
{
    debutU = -1;
    finU = 1;
    du = 0.1;
    debutV = -1;
    finV = 1;
    dv = 0.1;
    theta = 60;
    phi = 30;
    initProjection(theta, phi, 155);
    setProjection(PROJECTION_TYPE_PARALLELE);
}

void rechercheFenetre()
{
    for (int32_t i = 0; i < lines; i++)
    {
        const double y = gy2 - i * incY;
        for (int32_t j = 0; j < points; j++)
        {
            const double x = gx1 + j * incX;
            const double z = FX(x, y);

            double projX = 0, projY = 0;
            projette(x, y, z, &projX, &projY);

            if (projX < f1) f1 = projX;
            if (projX > f2) f2 = projX;
            if (projX < f3) f3 = projY;
            if (projX > f4) f4 = projY;
        }
    }
}

void calculeEchelles()
{
     echX = (double(c2) - c1) / (f2 - f1);
     echY = (double(c4) - c3) / (f4 - f3);
     if (vue == 'r')
     {
         if (echX < echY) echY = echX;
         else echX = echY;
     }
}

void horizontal(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    const int32_t dx = SIGNED(x2 - x1);
    if (dx == 0)
    {
        maxHeight[(x2 + 1) % LIMITX] = max(maxHeight[x2 % LIMITX], y2);
        minHeight[(x2 + 1) % LIMITX] = min(minHeight[x2 % LIMITX], y2);
    }
    else
    {
        const int32_t pente = (y2 - y1) / (x2 - x1);
        for (int32_t x = x2 + 1; x <= x1; x++)
        {
            const int32_t y = pente * (x - x1) + y1;
            maxHeight[x % LIMITX] = max(maxHeight[x % LIMITX], y);
            minHeight[x % LIMITX] = min(minHeight[x % LIMITX], y);
        }
    }
}

void visibilite(int32_t x, int32_t y, int32_t *vs)
{
    if ((y < maxHeight[x % LIMITX]) && (y > minHeight[x % LIMITX])) *vs = 0;
    else if (y >= maxHeight[x % LIMITX]) *vs = 1; else *vs = -1;
}

void inter(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t *taux, int32_t *xi, int32_t *yi)
{
    int32_t xii = 0, yii = 0;
    
    if (x2 == x1)
    {
        xii = x2;
        yii = taux[x2 % LIMITX];
    }
    else
    {
        const int32_t den = y2 - y1 - taux[x2 % LIMITX] + taux[x1 % LIMITX];
        if (den)
        {
            xii = (x1 * (y2 - taux[x2 % LIMITX]) + x2 * (taux[x1 % LIMITX] - y1)) / den;
            yii = (y2 * taux[x1 % LIMITX] - y1 * taux[x2 % LIMITX]) / den;
        }
        else
        {
            xii = x2;
            yii = y2;
        }
    }
    
    *xi = xii;
    *yi = yii;
}

void dessineFonction()
{
    int32_t xi = 0, yi = 0;

    for (int32_t i = 0; i < lines; i++)
    {
        double projX = 0, projY = 0;
        const double y = gy2 - i * incY;
        double x = gx1;
        double z = FX(x, y);
                
        projette(x, y, z, &projX, &projY);

        int32_t precX = int32_t((projX - f1) * echX + c1);
        int32_t precY = int32_t((projY - f3) * echY + c3);

        visibilite(precX, precY, &visiPrec);
        
        for (int32_t j = 0; j < points; j++)
        {
            x = gx1 + j * incX;
            z = FX(x, y);

            projette(x, y, z, &projX, &projY);
            
            const int32_t courX = int32_t((projX - f1) * echX + c1);
            const int32_t courY = int32_t((projY - f3) * echY + c3);
            
            visibilite(courX, courY, &visiCour);
            
            if (!maxHeight[courX % LIMITX] || minHeight[courX % LIMITX] == LIMITY) visiCour = visiPrec;
            
            if (visiCour == visiPrec)
            {
                if (visiCour == 1 || visiCour == -1)
                {
                    drawLine(precX, LIMITY - precY, courX, LIMITY - courY, 50);
                    horizontal(precX, precY, courX, courY);
                }
            }
            else
            {
                if (visiCour == 0)
                {
                    if (visiPrec == 1) inter(precX, precY, courX, courY, maxHeight, &xi, &yi);
                    else inter(precX, precY, courX, courY, minHeight, &xi, &yi);
                    drawLine(precX, LIMITY - precY, xi, LIMITY - yi, 50);
                    horizontal(precX, precY, xi, yi);
                }
                else
                {
                    if (visiCour == 1)
                    {
                        if (visiPrec == 0)
                        {
                            inter(precX, precY, courX, courY, maxHeight, &xi, &yi);
                            drawLine(xi, LIMITY - yi, courX, LIMITY - courY, 50);
                            horizontal(xi, yi, courX, courY);
                        }
                        else
                        {
                            inter(precX, precY, courX, courY, minHeight, &xi, &yi);
                            drawLine(precX, LIMITY - precY, xi, LIMITY - yi, 50);
                            horizontal(precX, precY, xi, yi);
                            inter(precX, precY, courX, courY, maxHeight, &xi, &yi);
                            drawLine(xi, LIMITY - yi, courX, LIMITY - courY, 50);
                            horizontal(xi, yi, courX, courY);
                        }
                    }
                    else
                    {
                        if (visiPrec == 0)
                        {
                            inter(precX, precY, courX, courY, minHeight, &xi, &yi);
                            drawLine(xi, LIMITY - yi, courX, LIMITY - courY, 50);
                            horizontal(xi, yi, courX, courY);
                        }
                        else
                        {
                            inter(precX, precY, courX, courY, maxHeight, &xi, &yi);
                            drawLine(precX, LIMITY - precY, xi, LIMITY - yi, 50);
                            horizontal(precX, precY, xi, yi);
                            inter(precX, precY, courX, courY, minHeight, &xi, &yi);
                            drawLine(xi, LIMITY - yi, courX, LIMITY - courY, 50);
                            horizontal(xi, yi, courX, courY);
                        }
                    }
                }
            }
            
            visiPrec = visiCour;
            precX = courX;
            precY = courY;
        }
    }
}

void affichage(int32_t range)
{
    int32_t i = 0, j = 0;
    uint8_t oldFont = getFontType();

    char buff[80] = { 0 };
    const char *strTitle = "Shapes 3D Transform";
    const int32_t cx = getCenterX();
    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();

    setFontType(1);
    const int32_t width = getFontWidth(strTitle);
    const int32_t height = getFontHeight(strTitle);
    const int32_t starty = 2;
    const int32_t startx = cx - (width >> 1);
    writeText(startx, starty, 40, 1, strTitle);

    setFontType(0);
    sprintf(buff, "X=[%.1f,%.1f] Y=[%.1f,%.1f] Theta=%.1f Phi=%.1f Lines=%d Points=%d", -gx1, gx1, -gy1, gy1, theta, phi, lines, points);
    writeText(cx - (getFontWidth(buff) >> 1), cmy - getFontHeight(buff) - 2, 37, 1, buff);

    for (i = starty; i < height; i++)
    {
        for (j = startx; j < startx + width; j++) if (getPixel(j, i) == 40) putPixel(j, i, 56 + (i / 3));
    }
    
    for (i = 50; i < cmx - 50; i++)
    {
        for (j = 50; j < cmx - 50; j++) if (getPixel(i, j) == 50) putPixel(i, j, 32 + (((i + j) / range) % 72));
    }
    
    setFontType(oldFont);
    render();
}

void resetParameters()
{
    c1 = c2 = c3 = c4 = lines = points = 0;
    gx1 = gx2 = gy1 = gy2 = incX = incY = 0;
    f1 = f2 = f3 = f4 = echX = echY = 0;
    resetProjection();
}

void setPixelChar()
{
    const GFX_FONT* font = getFont();
    memset(chrPixels, 0, sizeof(chrPixels));

    for (int32_t i = 32; i < CHR_MAX; i++)
    {
        writeText(0, 0, 31, 0, "%c", i);
        for (int32_t j = 0; j < font->hdr.subData.width; j++)
        {
            for (int32_t k = 0; k < font->hdr.subData.height; k++) if (getPixel(j, k) == 31) chrPixels[i][j][k] = 1;
        }
        fillRect(0, 0, font->hdr.subData.width, font->hdr.subData.height, 0);
    }
}

void scrollLed(const char *msg)
{
    int32_t i = 0, j = 0, k = 0, m = 0;
    const int32_t zx = 5, zy = 3, sy = 50;
    const GFX_FONT* font = getFont();
    const int32_t len = int32_t(strlen(msg));

    drawRoundRect(0, sy, getMaxX() - 1, font->hdr.subData.height << 2, 50, 10);
    
    while (!finished(SDL_SCANCODE_RETURN))
    {
        const uint8_t chr = msg[m];
        for (k = 0; k < font->hdr.subData.width; k++)
        {
            for (i = 0; i < CHR_NUM * font->hdr.subData.width - 1; i++)
            {
                for (j = 0; j < font->hdr.subData.height; j++) chrBuff[i][j] = chrBuff[i + 1][j];
            }

            for (j = 0; j < font->hdr.subData.height; j++) chrBuff[CHR_NUM * font->hdr.subData.width - 1][j] = chrPixels[chr][k][j];
            
            for (i = 0; i < CHR_NUM * font->hdr.subData.width; i++)
            {
                for (j = 0; j < font->hdr.subData.height; j++)
                {
                    if (chrBuff[i][j]) putPixel(font->hdr.subData.width + zx * (i + 2), sy + zy * (j + 2) + 4, 14);
                    else putPixel(font->hdr.subData.width + zx * (i + 2), sy + zy * (j + 2) + 4, 0);
                }
            }
            delay(10);
        }
        render();
        if (++m >= len) break;
    }
}

//Display sprite using simulate visual pages
void displaySprite(const char *fname)
{
    uint32_t frames = 0;
    GFX_IMAGE bkg = { 0 }, spr = { 0 }, img1 = { 0 }, img2 = { 0 }, page1 = { 0 }, page2 = { 0 };
    const char *fbkg[] = {"assets/1lan8.bmp", "assets/1lan16.bmp", "assets/1lan24.bmp", "assets/1lan32.bmp"};

    int32_t lx = 0;
    int32_t ly = 0;
    int32_t dx = 8;
    int32_t dy = 1;
    int32_t v1 = 0;
    int32_t v2 = 0;
    int32_t x = (20 + dx) >> 1;
    int32_t y = (12 + dy) >> 1;

    //load sprite bitmap
    if (!loadImage(fname, &spr)) return;
    if (!loadImage(fbkg[getBytesPerPixel() - 1], &bkg)) return;

    //create screen buffers
    if (!newImage(bkg.mWidth, bkg.mHeight, &page1)) return;
    if (!newImage(bkg.mWidth, bkg.mHeight, &page2)) return;

    //copy 1st page to 2nd page 
    memcpy(page1.mData, bkg.mData, bkg.mSize);
    memcpy(page2.mData, bkg.mData, bkg.mSize);

    //save current render buffer
    int32_t oldWidth = 0, oldHeight = 0;
    void* oldBuffer = getDrawBuffer(&oldWidth, &oldHeight);
    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();

    while (frames < 220 && !finished(SDL_SCANCODE_RETURN))
    {
        //drawing on page 1st
        setActivePage(&page1);
        setVisualPage(&page2);
        if (v1) putImage(alignedSize(lx), ly, &img1);
        
        lx = x;
        ly = y;
        v1 = 1;

        x += dx;
        if (x > cmx - spr.mWidth || x <= 0)
        {
            x -= dx;
            dx = -dx;
        }

        y += dy;
        if (y > cmy - spr.mHeight || y <= 0)
        {
            y -= dy;
            dy = -dy;
        }

        dy++;
        getImage(alignedSize(x), y, spr.mWidth, spr.mHeight, &img1);
        putSprite(alignedSize(x), y, 0, &spr);

        //draw on 2nd page
        setActivePage(&page2);
        setVisualPage(&page1);
        if (v2) putImage(alignedSize(lx), ly, &img2);
        
        lx = x;
        ly = y;
        v2 = 1;

        x += dx;
        if (x > cmx - spr.mWidth || x <= 0)
        {
            x -= dx;
            dx = -dx;
        }

        y += dy;
        if (y > cmy - spr.mHeight || y <= 0)
        {
            y -= dy;
            dy = -dy;
        }

        dy++;
        getImage(alignedSize(x), y, spr.mWidth, spr.mHeight, &img2);
        putSprite(alignedSize(x), y, 0, &spr);
        delay(FPS_60);
        frames++;
    }

    //restore and cleanup...
    changeDrawBuffer(oldBuffer, oldWidth, oldHeight);
    freeImage(&img1);
    freeImage(&img2);
    freeImage(&spr);
    freeImage(&page1);
    freeImage(&page2);
    freeImage(&bkg);
}

//show plasma effect
void displayPlasma()
{
    const char *str[] = {
        "That's all folks!",
        "I hope, you enjoyed this demo",
        "Thank you for making use of my programs",
        "More source code from me available at:",
        "https://github.com/pherosiden/",
        "",
        "If you have some improvements, additions,",
        "bug reports or something else, please contact me",
        "",
        "(c) 1998-2021 by Nguyen Ngoc Van",
        "Email: pherosiden@gmail.com",
        "",
        "Greets fly to:",
        "scene.org",
        "lodev.org",
        "sources.ru",
        "permadi.com",
        "eyecandyarchive.com",
        "crossfire-designs.de",
        "And all persons which helped me in any way.",
        "",
        "CYA!"
    };

    RGB pal[256] = { 0 };
    GFX_IMAGE src = { 0 }, dst = { 0 };

    int32_t ypos = 0, endPos = 0;
    int32_t x = 0, y = 0, decx = 0, decy = 0;
    const int32_t count = sizeof(str) / sizeof(str[0]);

    uint32_t frames = 0;
    uint8_t dx = 0, dy = 0;
    uint8_t sint[256] = {0};
    uint8_t cost[256] = {0};

    //load text font
    if (!loadFont("assets/hyena.xfn", 0)) return;

    //plasma image buffer
    if (!newImage(160, 120, &src)) return;

    //scale plasma image buffer
    if (!newImage(getBufferWidth(), getBufferHeight(), &dst)) return;
    initPlasma(sint, cost);

    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();

    //display plasma
    while (frames < 880 && !finished(SDL_SCANCODE_RETURN))
    {
        //create plasma buffer and display on screen
        createPlasma(&dx, &dy, sint, cost, &src);
        putImage(alignedSize(x), y, &src);
        render();
        delay(FPS_90);

        //check limitation
        if (decx) x--; else x++;
        if (decy) y--; else y++;
        if (x <= 0 || x >= cmx - src.mWidth) decx = !decx;
        if (y <= 0 || y >= cmy - src.mHeight) decy = !decy;
        frames++;
    }

    //setup font palette
    getPalette(pal);
    for (x = 0; x < 32; x++)
    {
        pal[x].r = x << 3;
        pal[x].g = x << 3;
        pal[x].b = x << 3;
    }
    setPalette(pal);

    ypos = getBufferHeight();

    //display scale image and scroll text
    do {
        createPlasma(&dx, &dy, sint, cost, &src);
        scaleImage(&dst, &src, INTERPOLATION_TYPE_NEARST);
        putImage(0, 0, &dst);
        endPos = drawText(ypos--, count, str);
        if (endPos <= 98) fadeDown(pal);
        render();
        delay(FPS_90);
    } while (ypos > -32767 && endPos > -30 && !finished(SDL_SCANCODE_RETURN));

    //cleanup...
    freeImage(&src);
    freeImage(&dst);
    freeFont(0);
}

void gfxDemoMix()
{
    RGB 	pal1[256] = {0};
    RGB 	pal2[256] = {0};
    POINT2D	pts[50] = { 0 };

    double ratio = 0.0, rept = 0.0;

    int32_t i = 0, j = 0;

    const int32_t a = 70, b = 20;
    const int32_t msgY = 20;
    const int32_t col[] = { 50, 35, 32, 40, 40, 32 };

    const char *logo = "GFXLIB";

    const char *msgWelcome[] = {
        "GFXLIB Library Demo",
        "Full supports 8/15/16/24/32 bits color",
        "Load/Save BMP & PNG 32 bits image",
        "Copyright (c) 1998 - 2021 by Nguyen Ngoc Van",
        "Please wait to continue..."
    };

    char msgTitle[][80] = {
        "Khoa Co6ng Nghe65 Tho6ng Tin - Kho1a 2000",
        "Tru7o72ng D9a5i Ho5c Ky4 Thua65t TP.HCM - HUTECH",
        "Thu7 Vie65n D9o62 Ho5a VESA 8/15/16/24/32 bits Ma2u",
        "Copyright (c) 1998 - 2021 by Nguye64n Ngo5c Va6n",
        "Trang chu3: https://github.com/pherosiden/"
    };

    const int32_t numTitles = sizeof(msgTitle) / sizeof(msgTitle[0]);

    char msgScroll[] = "*** Ca1m o7n ca1c ba5n d9a4 su73 du5ng chu7o7ng tri2nh na2y. Ba5n co1 the63 ta3i toa2n bo65 ma4 nguo62n cu3a chu7o7ng tri2nh ta5i d9i5a chi3 https://github.com/pherosiden/gfxlib. Chu1c Ca1c Ba5n Tha2nh Co6ng       ";
    char msgBanner[] = "Light Banner (c) 1998 - 2021 Nguye64n Ngo5c Va6n";
    char msgLoading[] = "D9ang ta3i du74 lie65u a3nh PNG & BMP 32bit ma2u, vui lo2ng d9o75i mo65t la1t....";

    if (!initScreen(800, 600, 32, 0, "GFX-Demo8")) return;
    if (!loadFont("assets/fontvn.xfn", 0)) return;

    const int32_t cmx = getMaxX();
    const int32_t cmy = getMaxY();
    const int32_t cx = getCenterX();
    const int32_t cy = getCenterY();

    makeFont(msgLoading);
    writeText(cx - (getFontWidth(msgLoading) >> 1), cy - getFontHeight(msgLoading), rgb(255, 255, 64), 0, msgLoading);
    render();
    sleepFor(2000);
    showPNG("assets/caibang.png");
    sleepFor(2000);
    fadeCircle(2, 0);
    showBMP("assets/1lan32.bmp");
    sleepFor(2000);
    fadeCircle(3, 0);
    
    handleMouseButton();
    displaySprite("assets/smile24.png");

    freeFont(0);
    cleanup();

    if (!initScreen(800, 600, 8, 0, "GFXLIB-Demo8")) return;
    const int32_t introY = cy - ((numTitles * CHR_HEIGHT + 20 + b * 2) >> 1);

    switch (getBufferWidth())
    {
        case  640: ratio = 1.0;	 break;
        case  800: ratio = 1.25; break;
        case 1024: ratio = 1.75; break;
        case 2048: ratio = 2.25; break;
        default:   ratio = 1.5;	 break;
    }

    getBasePalette(pal2);
    clearPalette();
    
    if (!loadFont("assets/odhl.xfn", 0)) return;
    writeText(cx - (getFontWidth(msgWelcome[0]) >> 1), msgY, 40, 1, msgWelcome[0]);
    writeText(cx - (getFontWidth(msgWelcome[1]) >> 1), msgY + 60, 32, 1, msgWelcome[1]);
    writeText(cx - (getFontWidth(msgWelcome[2]) >> 1), msgY + 120, 35, 1, msgWelcome[2]);
    writeText(cx - (getFontWidth(msgWelcome[3]) >> 1), msgY + 170, 35, 1, msgWelcome[3]);
    writeText(cx - (getFontWidth(msgWelcome[4]) >> 1), cmy - 70, 35, 1, msgWelcome[4]);
    freeFont(0);
    fadeIn(pal2, FPS_90);
    sleepFor(2000);
    fadeRollo(2, 0);

    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo0(cx, cy, int32_t(200 * ratio), int32_t(100 * ratio));
    rotatePalette(32, 103, 72, FPS_90);

    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo1(cx, cy, int32_t(160 * ratio), int32_t(40 * ratio));
    rotatePalette(32, 103, 72, FPS_90);

    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo2(cx, cy, int32_t(80 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo3(cx, cy, int32_t(80 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo4(cx, cy, int32_t(120 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo5(cmx / 7, cmy / 5 - 10, int32_t(28 * ratio), int32_t(90 * ratio), int32_t(62 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo6(cx, cy, int32_t(200 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo7(cx, cy, int32_t(200 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo8(cx, cy, int32_t(245 * ratio), int32_t(100 * ratio));
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    setPalette(pal1);
    graphDemo9(cx, cy, 0.6 * ratio);
    rotatePalette(32, 103, 72, FPS_90);
    
    clearScreen();
    memcpy(&pal1[64], &pal2[64], 40 * sizeof(RGB));
    setPalette(pal1);
    
    initDemo10(1, 4);
    for (i = 0; i <= 95; i++)
    graphDemo10(cx >> 1, cy >> 1, 140 - i, 140 - i, 64 + i / 3);
    
    initDemo10(2, 2);
    for (i = 0; i <= 119; i++)
    graphDemo10(cx + (cx >> 1), cy >> 1, 170 - i, 170 - i, 64 + i / 3);
    
    initDemo10(3, 5);
    for (i = 0; i <= 39; i++)
    graphDemo10(cx >> 1, cy + (cy >> 1), 110 - (i << 1), 110 - (i << 1), 83 - (i >> 1));
    
    initDemo10(4, 7);
    for (i = 0; i <= 19; i++)
    graphDemo10(cx + (cx >> 1), cy + (cy >> 1), 130 - (i << 2), 130 - (i << 2), 64 + i);
    rotatePalette(64, 103, 40, FPS_90);

    clearScreen();
    memcpy(pal1, pal2, 256 * sizeof(RGB));
    makeLinearPalette();
    drawPolygon(cx, cy, cx - 140, 11, 1);
    rotatePalette(16, 177, 162, FPS_90);
    
    clearScreen();
    memcpy(&pal1[32], &pal2[32], 130 * sizeof(RGB));
    setPalette(pal1);

    findRepeat(&rept);
    drawCylodiod(cmx / 7 - 20, cmy / 5 - 30, 80, 1, 1, rept, 40);
    drawCylodiod(cmx / 3 + 30, cmy / 5 - 30, 80, 1, 2, rept, 40);
    drawCylodiod(cx + 90, cmy / 5 - 30, 80, 1, 3, rept, 40);
    drawCylodiod(cmx - 100, cmy / 5 - 30, 80, 2, 1, rept, 40);
    
    drawCylodiod(cmx / 7 - 20, cy - 10, 80, 2, 2, rept, 40);
    drawCylodiod(cmx / 3 + 30, cy - 10, 80, 2, 3, rept, 40);
    drawCylodiod(cx + 90, cy - 10, 80, 3, 1, rept, 40);
    drawCylodiod(cmx - 100, cy - 20, 80, 3, 2, rept, 40);
    
    drawCylodiod(cmx / 7 - 20, cmy - 100, 80, 3, 3, rept, 40);
    drawCylodiod(cmx / 3 + 30, cmy - 100, 80, 4, 1, rept, 40);
    drawCylodiod(cx + 90, cmy - 100, 80, 4, 2, rept, 40);
    drawCylodiod(cmx - 100, cmy - 100, 80, 4, 3, rept, 40);
    sleepFor(2000);

    clearScreen();
    memcpy(&pal1[32], &pal2[32], 130 * sizeof(RGB));
    setPalette(pal1);

    drawCylodiod(cmx / 7 - 20, cmy / 5 - 30, 80, 5, 1, rept, 40);
    drawCylodiod(cmx / 3 + 30, cmy / 5 - 30, 80, 5, 2, rept, 40);
    drawCylodiod(cx + 90, cmy / 5 - 30, 80, 5, 3, rept, 40);
    drawCylodiod(cmx - 100, cmy / 5 - 30, 80, 5, 4, rept, 40);

    drawCylodiod(cmx / 7 - 20, cy - 10, 80, 1, 4, rept, 40);
    drawCylodiod(cmx / 3 + 30, cy - 10, 80, 2, 4, rept, 40);
    drawCylodiod(cx + 90, cy - 10, 80, 3, 4, rept, 40);
    drawCylodiod(cmx - 100, cy - 20, 80, 4, 4, rept, 40);
    
    drawCylodiod(cmx / 7 - 20, cmy - 100, 80, 6, 1, rept, 40);
    drawCylodiod(cmx / 3 + 30, cmy - 100, 80, 6, 2, rept, 40);
    drawCylodiod(cx + 90, cmy - 100, 80, 6, 3, rept, 40);
    drawCylodiod(cmx - 100, cmy - 100, 80, 6, 4, rept, 40);
    sleepFor(2000);

    clearScreen();
    memcpy(&pal1[40], &pal2[40], 64 * sizeof(RGB));
    rotatePolygon(pts, 6, cx, cy, cx - 140, 100, 20, 40);
    rotatePalette(40, 103, 64, FPS_90);
    
    clearScreen();
    memcpy(&pal1[37], &pal2[37], 67 * sizeof(RGB));
    randomPoly(pts, 12, cmx, cmy, 40, 20, 37);
    rotatePalette(37, 103, 67, FPS_90);

    clearScreen();
    memcpy(&pal1[40], &pal2[40], 64 * sizeof(RGB));
    drawHexagon(pts, 12, cx, cy, 35, cx - 140, 20, 40);
    rotatePalette(40, 103, 64, FPS_90);

    clearScreen();
    graphDemo11();
    fadeRollo(2, 0);
    
    clearScreen();
    graphDemo12();
    fadeRollo(1, 0);
    graphDemo13();
    clearScreen();
    clearPalette();

    if (!loadFont("assets/sys8x16.xfn", 0)) return;
    if (!loadFont("assets/trip.xfn", 1)) return;
    
    FX = FX1;

    initParameter1();
    rechercheFenetre();
    calculeEchelles();
    dessineFonction();
    affichage(15);

    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 45, FPS_90);
    fadeMin(FPS_90);
    
    clearScreen();
    resetParameters();

    FX = FX2;

    initParameter2();
    rechercheFenetre();
    calculeEchelles();
    dessineFonction();
    affichage(15);

    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 45, FPS_90);
    fadeMin(FPS_90);

    clearScreen();
    resetParameters();

    FX = FX3;

    initParameter3();
    rechercheFenetre();
    calculeEchelles();
    dessineFonction();
    affichage(15);

    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 45, FPS_90);
    fadeMin(FPS_90);

    clearScreen();
    resetParameters();

    FX = FX4;

    initParameter4();
    rechercheFenetre();
    calculeEchelles();
    dessineFonction();
    affichage(15);

    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 45, FPS_90);
    fadeMin(FPS_90);

    clearScreen();
    resetParameters();

    FX = FX5;

    initParameter5();
    rechercheFenetre();
    calculeEchelles();
    dessineFonction();
    affichage(15);

    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 45, FPS_90);
    fadeMin(FPS_90);
    
    FX = FX11;
    FY = FY11;
    FZ = FZ11;
    
    clearScreen();
    initParameter11();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);

    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 72, FPS_90);
    fadeMin(FPS_90);

    FX = FX21;
    FY = FY21;
    FZ = FZ21;
    
    clearScreen();
    initParameter21();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 72, FPS_90);
    fadeMin(FPS_90);

    FX = FX31;
    FY = FY31;
    FZ = FZ31;

    clearScreen();
    initParameter31();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 72, FPS_90);
    fadeMin(FPS_90);

    FX = FX41;
    FY = FY41;
    FZ = FZ41;

    clearScreen();
    initParameter41();
    familleDesCourbesEnU();
    familleDesCourbesEnV();
    affichage(14);
    
    fadeIn(pal2, FPS_90);
    rotatePalette(32, 103, 72, FPS_90);
    fadeMin(FPS_90);

    freeFont(0);
    freeFont(1);
    clearScreen();
    
    j = introY;
    if (!loadFont("assets/fontvn.xfn", 0)) return;
    for (i = 0; i < numTitles; i++)
    {
        j += i * getFontHeight(msgTitle[i]);
        makeFont(msgTitle[i]);
        writeText(cx - (getFontWidth(msgTitle[i]) >> 1), introY + i * getFontHeight(msgTitle[i]), col[i], 1, msgTitle[i]);
    }
    
    fillEllipse(cx, j - introY, a, b, 50);
    writeText(cx - (getFontWidth(logo) >> 1), j - introY - CHR_WIDTH, 32, 1, logo);

    fadeIn(pal2, FPS_90);
    memcpy(&pal1[32], &pal2[32], 72 * sizeof(RGB));
    rotatePalette(32, 103, 250, FPS_90);

    clearScreen();
    setPalette(pal2);
    setPixelChar();
    makeFont(msgBanner);
    writeText(cx - (getFontWidth(msgBanner) >> 1), cmy - getFontHeight(msgBanner) - 2, 40, 1, msgBanner);
    makeFont(msgScroll);
    scrollLed(msgScroll);
    freeFont(0);
    clearScreen();
    displayPlasma();
    cleanup();
}
