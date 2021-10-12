#include "gfxlib.h"
#ifdef __APPLE__
#include <pthread.h>
#include <cpuid.h>
#include <sys/sysctl.h>
#else
#include <windows.h>
#endif

//screen size
#define SCR_WIDTH       1280
#define SCR_HEIGHT      900
#define THREAD_COUNT    64

//calculation function type
typedef void func_t(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t sx, int32_t sy);

//for Julia's set
double cim = 0;
double cre = 0;

//max iterations (default value should be 255)
int32_t iterations = 255;

void mandelbrotFloat(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t y = ys; y < py; y++)
    {
        const float y0 = y * float(scale) + float(my);
        for (int32_t x = 0; x < px; x++)
        {
            const float x0 = x * float(scale) + float(mx);
            float x1 = x0;
            float y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const float x2 = sqr(x1);
                const float y2 = sqr(y1);
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + y0;
                x1 = x2 - y2 + x0;
            }
            *out++ = hsv2rgb(255 * n / iterations, 255, (n < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotDouble(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t y = ys; y < py; y++)
    {
        const double y0 = y * scale + my;
        for (int32_t x = 0; x < px; x++)
        {
            const double x0 = x * scale + mx;
            double x1 = x0;
            double y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const double x2 = sqr(x1);
                const double y2 = sqr(y1);
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + y0;
                x1 = x2 - y2 + x0;
            }
            *out++ = hsv2rgb(255 * n / iterations, 255, (n < iterations) ? 255 : 0);
        }
    }
}

void juliaFloat(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t y = ys; y < py; y++)
    {
        const float y0 = y * float(scale) + float(my);
        for (int32_t x = 0; x < px; x++)
        {
            const float x0 = x * float(scale) + float(mx);
            float x1 = x0;
            float y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const float x2 = sqr(x1);
                const float y2 = sqr(y1);
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + float(cim);
                x1 = x2 - y2 + float(cre);
            }
            *out++ = hsv2rgb(255 * n / iterations, 255, (n < iterations) ? 255 : 0);
        }
    }
}

void juliaDouble(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t y = ys; y < py; y++)
    {
        const double y0 = y * scale + my;
        for (int32_t x = 0; x < px; x++)
        {
            const double x0 = x * scale + mx;
            double x1 = x0;
            double y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const double x2 = sqr(x1);
                const double y2 = sqr(y1);
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + cim;
                x1 = x2 - y2 + cre;
            }
            *out++ = hsv2rgb(255 * n / iterations, 255, (n < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotFloatSSE(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128 dd = _mm_set1_ps(float(scale));
    const __m128 tx = _mm_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m128 y0 = _mm_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m128 x0 = _mm_add_ps(tx, _mm_mul_ps(dd, _mm_cvtepi32_ps(ind)));
            __m128 x1 = x0;
            __m128 y1 = y0;
            __m128i iters = _mm_setzero_si128();
            __m128i masks = _mm_setzero_si128();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m128 x2 = _mm_mul_ps(x1, x1);
                const __m128 y2 = _mm_mul_ps(y1, y1);
                const __m128 abs = _mm_add_ps(x2, y2);
                const __m128i cmp = _mm_castps_si128(_mm_cmpge_ps(abs, _mm_set1_ps(4)));

                masks = _mm_or_si128(cmp, masks);
                if (_mm_test_all_ones(masks)) break;
                iters = _mm_add_epi32(iters, _mm_andnot_si128(masks, _mm_set1_epi32(1)));

                __m128 tmp = _mm_mul_ps(x1, y1);
                tmp = _mm_add_ps(tmp, tmp);
                y1 = _mm_add_ps(tmp, y0);
                x1 = _mm_add_ps(_mm_sub_ps(x2, y2), x0);
            }

            int32_t itpos[4] = { 0 };
            _mm_store_si128((__m128i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotDoubleSSE2(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128d dd = _mm_set1_pd(scale);
    const __m128d tx = _mm_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m128d y0 = _mm_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 2)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, 0, 0);
            const __m128d x0 = _mm_add_pd(tx, _mm_mul_pd(dd, _mm_cvtepi32_pd(ind)));
            __m128d x1 = x0;
            __m128d y1 = y0;
            __m128i iters = _mm_setzero_si128();
            __m128i masks = _mm_setzero_si128();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m128d x2 = _mm_mul_pd(x1, x1);
                const __m128d y2 = _mm_mul_pd(y1, y1);
                const __m128d abs = _mm_add_pd(x2, y2);
                const __m128i cmp = _mm_castpd_si128(_mm_cmpge_pd(abs, _mm_set1_pd(4)));

                masks = _mm_or_si128(cmp, masks);
                if (_mm_test_all_ones(masks)) break;
                iters = _mm_add_epi32(iters, _mm_andnot_si128(masks, _mm_set1_epi32(1)));

                __m128d tmp = _mm_mul_pd(x1, y1);
                tmp = _mm_add_pd(tmp, tmp);
                y1 = _mm_add_pd(tmp, y0);
                x1 = _mm_add_pd(_mm_sub_pd(x2, y2), x0);
            }

            int32_t itpos[4] = { 0 };
            _mm_store_si128((__m128i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
        }
    }
}

void juliaFloatSSE(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128 xim = _mm_set1_ps(float(cim));
    const __m128 xre = _mm_set1_ps(float(cre));

    const __m128 dd = _mm_set1_ps(float(scale));
    const __m128 tx = _mm_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m128 y0 = _mm_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m128 x0 = _mm_add_ps(tx, _mm_mul_ps(dd, _mm_cvtepi32_ps(ind)));
            __m128 x1 = x0;
            __m128 y1 = y0;
            __m128i iters = _mm_setzero_si128();
            __m128i masks = _mm_setzero_si128();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m128 x2 = _mm_mul_ps(x1, x1);
                const __m128 y2 = _mm_mul_ps(y1, y1);
                const __m128 abs = _mm_add_ps(x2, y2);
                const __m128i cmp = _mm_castps_si128(_mm_cmpge_ps(abs, _mm_set1_ps(4)));

                masks = _mm_or_si128(cmp, masks);
                if (_mm_test_all_ones(masks)) break;
                iters = _mm_add_epi32(iters, _mm_andnot_si128(masks, _mm_set1_epi32(1)));

                __m128 tmp = _mm_mul_ps(x1, y1);
                tmp = _mm_add_ps(tmp, tmp);
                y1 = _mm_add_ps(tmp, xim);
                x1 = _mm_add_ps(_mm_sub_ps(x2, y2), xre);
            }

            int32_t itpos[4] = { 0 };
            _mm_store_si128((__m128i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
        }
    }
}

void juliaDoubleSSE2(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128d xim = _mm_set1_pd(cim);
    const __m128d xre = _mm_set1_pd(cre);

    const __m128d dd = _mm_set1_pd(scale);
    const __m128d tx = _mm_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m128d y0 = _mm_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 2)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, 0, 0);
            const __m128d x0 = _mm_add_pd(tx, _mm_mul_pd(dd, _mm_cvtepi32_pd(ind)));
            __m128d x1 = x0;
            __m128d y1 = y0;
            __m128i iters = _mm_setzero_si128();
            __m128i masks = _mm_setzero_si128();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m128d x2 = _mm_mul_pd(x1, x1);
                const __m128d y2 = _mm_mul_pd(y1, y1);
                const __m128d abs = _mm_add_pd(x2, y2);
                const __m128i cmp = _mm_castpd_si128(_mm_cmpge_pd(abs, _mm_set1_pd(4)));

                masks = _mm_or_si128(cmp, masks);
                if (_mm_test_all_ones(masks)) break;
                iters = _mm_add_epi32(iters, _mm_andnot_si128(masks, _mm_set1_epi32(1)));

                __m128d tmp = _mm_mul_pd(x1, y1);
                tmp = _mm_add_pd(tmp, tmp);
                y1 = _mm_add_pd(tmp, xim);
                x1 = _mm_add_pd(_mm_sub_pd(x2, y2), xre);
            }

            int32_t itpos[4] = { 0 };
            _mm_store_si128((__m128i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotFloatAVX(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m256 y0 = _mm256_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x1 = x0;
            __m256 y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x1, x1);
                const __m256 y2 = _mm256_mul_ps(y1, y1);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                __m256 tmp = _mm256_mul_ps(x1, y1);
                tmp = _mm256_add_ps(tmp, tmp);
                y1 = _mm256_add_ps(tmp, y0);
                x1 = _mm256_add_ps(_mm256_sub_ps(x2, y2), x0);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[5] / iterations, 255, (itpos[5] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[7] / iterations, 255, (itpos[7] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotDoubleAVX(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
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

                __m256d tmp = _mm256_mul_pd(x1, y1);
                tmp = _mm256_add_pd(tmp, tmp);
                y1 = _mm256_add_pd(tmp, y0);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
        }
    }
}

void juliaFloatAVX(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 xim = _mm256_set1_ps(float(cim));
    const __m256 xre = _mm256_set1_ps(float(cre));

    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m256 y0 = _mm256_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x1 = x0;
            __m256 y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x1, x1);
                const __m256 y2 = _mm256_mul_ps(y1, y1);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                __m256 tmp = _mm256_mul_ps(x1, y1);
                tmp = _mm256_add_ps(tmp, tmp);
                y1 = _mm256_add_ps(tmp, xim);
                x1 = _mm256_add_ps(_mm256_sub_ps(x2, y2), xre);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[5] / iterations, 255, (itpos[5] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[7] / iterations, 255, (itpos[7] < iterations) ? 255 : 0);
        }
    }
}

void juliaDoubleAVX(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
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

                __m256d tmp = _mm256_mul_pd(x1, y1);
                tmp = _mm256_add_pd(tmp, tmp);
                y1 = _mm256_add_pd(tmp, xim);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotFloatAVX2(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m256 y0 = _mm256_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x1 = x0;
            __m256 y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x1, x1);
                const __m256 y2 = _mm256_mul_ps(y1, y1);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                __m256 tmp = _mm256_mul_ps(x1, y1);
                tmp = _mm256_add_ps(tmp, tmp);
                y1 = _mm256_add_ps(tmp, y0);
                x1 = _mm256_add_ps(_mm256_sub_ps(x2, y2), x0);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[5] / iterations, 255, (itpos[5] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[7] / iterations, 255, (itpos[7] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotDoubleAVX2(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
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

                __m256d tmp = _mm256_mul_pd(x1, y1);
                tmp = _mm256_add_pd(tmp, tmp);
                y1 = _mm256_add_pd(tmp, y0);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
        }
    }
}

void juliaFloatAVX2(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 xim = _mm256_set1_ps(float(cim));
    const __m256 xre = _mm256_set1_ps(float(cre));

    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m256 y0 = _mm256_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x1 = x0;
            __m256 y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x1, x1);
                const __m256 y2 = _mm256_mul_ps(y1, y1);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                __m256 tmp = _mm256_mul_ps(x1, y1);
                tmp = _mm256_add_ps(tmp, tmp);
                y1 = _mm256_add_ps(tmp, xim);
                x1 = _mm256_add_ps(_mm256_sub_ps(x2, y2), xre);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[5] / iterations, 255, (itpos[5] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[7] / iterations, 255, (itpos[7] < iterations) ? 255 : 0);
        }
    }
}

void juliaDoubleAVX2(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 4)
        {
            const __m128i ind = _mm_setr_epi32(x, x + 1, x + 2, x + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
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

                __m256d tmp = _mm256_mul_pd(x1, y1);
                tmp = _mm256_add_pd(tmp, tmp);
                y1 = _mm256_add_pd(tmp, xim);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotFloatFMA(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m256 y0 = _mm256_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m256 x0 = _mm256_fmadd_ps(dd, _mm256_cvtepi32_ps(ind), tx);
            __m256 x1 = x0;
            __m256 y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x1, x1);
                const __m256 y2 = _mm256_mul_ps(y1, y1);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                const __m256 tmp = _mm256_add_ps (x1, x1);
                y1 = _mm256_fmadd_ps (tmp, y1, y0);
                x1 = _mm256_add_ps (_mm256_sub_ps (x2, y2), x0);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[5] / iterations, 255, (itpos[5] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[7] / iterations, 255, (itpos[7] < iterations) ? 255 : 0);
        }
    }
}

void mandelbrotDoubleFMA(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 4)
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

                const __m256d tmp = _mm256_add_pd(x1, x1);
                y1 = _mm256_fmadd_pd(tmp, y1, y0);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
        }
    }
}

void juliaFloatFMA(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 xim = _mm256_set1_ps(float(cim));
    const __m256 xre = _mm256_set1_ps(float(cre));

    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t y = ys; y < py; y++)
    {
        const __m256 y0 = _mm256_set1_ps(y * float(scale) + float(my));
        for (int32_t x = 0; x < px; x += 8)
        {
            const __m256i ind = _mm256_setr_epi32(x, x + 1, x + 2, x + 3, x + 4, x + 5, x + 6, x + 7);
            const __m256 x0 = _mm256_fmadd_ps(dd, _mm256_cvtepi32_ps(ind), tx);
            __m256 x1 = x0;
            __m256 y1 = y0;
            __m256i iters = _mm256_setzero_si256();
            __m256i masks = _mm256_setzero_si256();

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x1, x1);
                const __m256 y2 = _mm256_mul_ps(y1, y1);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), _CMP_GE_OS));

                masks = _mm256_or_si256(cmp, masks);
                if (_mm256_testc_si256(masks, _mm256_cmpeq_epi32(masks, masks))) break;
                iters = _mm256_add_epi32(iters, _mm256_andnot_si256(masks, _mm256_set1_epi32(1)));

                const __m256 tmp = _mm256_add_ps (x1, x1);
                y1 = _mm256_fmadd_ps (tmp, y1, xim);
                x1 = _mm256_add_ps (_mm256_sub_ps (x2, y2), xre);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[1] / iterations, 255, (itpos[1] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[3] / iterations, 255, (itpos[3] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[5] / iterations, 255, (itpos[5] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[7] / iterations, 255, (itpos[7] < iterations) ? 255 : 0);
        }
    }
}

void juliaDoubleFMA(uint32_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t y = ys; y < py; y++)
    {
        const __m256d y0 = _mm256_set1_pd(y * scale + my);
        for (int32_t x = 0; x < px; x += 4)
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

                const __m256d tmp = _mm256_add_pd(x1, x1);
                y1 = _mm256_fmadd_pd(tmp, y1, xim);
                x1 = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            int32_t itpos[8] = { 0 };
            _mm256_storeu_si256((__m256i*)itpos, iters);
            *out++ = hsv2rgb(255 * itpos[0] / iterations, 255, (itpos[0] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[2] / iterations, 255, (itpos[2] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[4] / iterations, 255, (itpos[4] < iterations) ? 255 : 0);
            *out++ = hsv2rgb(255 * itpos[6] / iterations, 255, (itpos[6] < iterations) ? 255 : 0);
        }
    }
}

func_t* funcs[2][20] = { 0 };
func_t* calculateFuncs[2] = { 0 };

char sbuff[2000] = { 0 };
const char* methodNames[20] = { 0 };

int32_t funcCounts[2] = { 0 };
int32_t funcIndexs[2] = { 0 };

bool fractType = true;
bool fullModes[2] = { 0 };

double scale = 0;

double xx = 0;
double yy = 0;

int32_t cx = 0;
int32_t cy = 0;

uint32_t *data = NULL;
uint32_t dataSize = 0;

int32_t cpuCores = 0;

void initThreads()
{
#ifdef __APPLE__
    size_t len = sizeof(cpuCores);
    sysctlbyname("hw.logicalcpu", &cpuCores, &len, NULL, 0);
#else
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    cpuCores = si.dwNumberOfProcessors;
#endif

    if (cpuCores < 1) cpuCores = 1;
    if (cpuCores > THREAD_COUNT) cpuCores = THREAD_COUNT;
}

void initFunctions(int32_t type)
{
    int32_t out[4] = { 0 };

#ifdef __APPLE__
    __cpuid(1, out[0], out[1], out[2], out[3]);
#else
    __cpuid(out, 1);
#endif

    bool sse42      = (out[2] & (1 << 20)) != 0;
    bool sse41      = (out[2] & (1 << 19)) != 0;
    bool fma        = (out[2] & (1 << 12)) != 0;
    bool avx        = (out[2] & (1 << 28)) != 0;
    bool osxsave    = (out[2] & (1 << 29)) != 0;

#ifdef __APPLE__
    __cpuid_count(7, 0, out[0], out[1], out[2], out[3]);
#else
    __cpuidex(out, 7, 0);
#endif

    bool avx2 = (out[1] & (1 << 5)) != 0;

    if (type)
    {
        int32_t i = 0;
        funcs[type][i] = juliaFloat;
        methodNames[i] = "float";
        if (fullModes[type]) i++;

        if (sse42)
        {
            funcs[1][i] = juliaFloatSSE;
            methodNames[i] = "float SSE";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = juliaFloatAVX;
            methodNames[i] = "float AVX";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = juliaFloatAVX2;
            methodNames[i] = "float AVX2";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = juliaFloatFMA;
            methodNames[i] = "float FMA";
            if (fullModes[type]) i++;
        }

        if (!fullModes[type]) i++;

        funcs[type][i] = juliaDouble;
        methodNames[i] = "double";
        if (fullModes[type]) i++;

        if (sse42)
        {
            funcs[type][i] = juliaDoubleSSE2;
            methodNames[i] = "double SSE";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = juliaDoubleAVX;
            methodNames[i] = "double AVX";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = juliaDoubleAVX2;
            methodNames[i] = "double AVX2";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = juliaDoubleFMA;
            methodNames[i] = "double FMA";
            if (fullModes[type]) i++;
        }

        if (!fullModes[type]) i++;

        funcCounts[type] = i;
        funcIndexs[type] = 0;
        calculateFuncs[type] = funcs[type][funcIndexs[type]];
    }
    else
    {
        int32_t i = 0;
        funcs[type][i] = mandelbrotFloat;
        methodNames[i] = "float";
        if (fullModes[type]) i++;

        if (sse42)
        {
            funcs[1][i] = mandelbrotFloatSSE;
            methodNames[i] = "float SSE";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = mandelbrotFloatAVX;
            methodNames[i] = "float AVX";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = mandelbrotFloatAVX2;
            methodNames[i] = "float AVX2";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = mandelbrotFloatFMA;
            methodNames[i] = "float FMA";
            if (fullModes[type]) i++;
        }

        if (!fullModes[type]) i++;

        funcs[type][i] = mandelbrotDouble;
        methodNames[i] = "double";
        if (fullModes[type]) i++;

        if (sse42)
        {
            funcs[type][i] = mandelbrotDoubleSSE2;
            methodNames[i] = "double SSE";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = mandelbrotDoubleAVX;
            methodNames[i] = "double AVX";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = mandelbrotDoubleAVX2;
            methodNames[i] = "double AVX2";
            if (fullModes[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = mandelbrotDoubleFMA;
            methodNames[i] = "double FMA";
            if (fullModes[type]) i++;
        }

        if (!fullModes[type]) i++;

        funcCounts[type] = i;
        funcIndexs[type] = 0;
        calculateFuncs[type] = funcs[type][funcIndexs[type]];
    }
}

int32_t alignSize(int32_t num)
{
    return (num + 7) & ~7;
}

void initFractals(int32_t sx, int32_t sy)
{
    double added = 0;
    const double xscale = 3.0 / sx;
    const double yscale = 2.0 / sy;

    if (!fractType) added = 0.5;
    else
    {
        cre = -0.7;
        cim = 0.27015;
    }

    scale = max(xscale, yscale);
    xx = -0.5 * sx * scale - added;
    yy = -0.5 * sy * scale;

    cx = sx;
    cy = sy;

    iterations = 255;
}

void allocBuffer()
{
    const uint32_t size = alignSize(cx) * cy * getBytesPerPixel();
    if (size > dataSize)
    {
        if (data) _mm_free(data);
        data = (uint32_t*)_mm_malloc(size, 32);
        if (!data) exit(1);
        memset(data, 0, size);
        dataSize = size;
    }
}

const int32_t yadd = 32;
volatile long yprocessed = 0;

#ifdef __APPLE__
void* threadProc(void* args)
#else
DWORD WINAPI threadProc(LPVOID lpThreadParameter)
#endif
{
    int32_t acx = alignSize(cx);
    while (true)
    {
#ifdef __APPLE__
        int32_t y0 = __sync_add_and_fetch(&yprocessed, yadd) - yadd;
#else
        int32_t y0 = InterlockedAdd(&yprocessed, yadd) - yadd;
#endif
        if (y0 >= cy) return 0;
        int32_t y1 = min(y0 + yadd, cy);
        calculateFuncs[fractType](&data[y0 * acx], xx, yy, scale, y0, acx, y1);
    }

    return 0;
}

void calculateMultiThread()
{
#ifdef __APPLE__
    yprocessed = 0;
    pthread_t threads[THREAD_COUNT] = { 0 };
    
    for (int32_t i = 0; i < cpuCores; i++) pthread_create(&threads[i], NULL, threadProc, NULL);
    for (int32_t i = 0; i < cpuCores; i++) pthread_join(threads[i], NULL);
#else
    yprocessed = 0;
    HANDLE threads[THREAD_COUNT] = { 0 };

    for (int32_t i = 0; i < cpuCores; i++) threads[i] = CreateThread(NULL, 0, threadProc, NULL, 0, NULL);
    WaitForMultipleObjects(cpuCores, threads, TRUE, INFINITE);
    for (int32_t i = 0; i < cpuCores; i++)
    {
        if (threads[i]) CloseHandle(threads[i]);
    }
#endif
}

void setScale(double newScale)
{
    xx += cx * (scale - newScale) * 0.5;
    yy += cy * (scale - newScale) * 0.5;
    scale = newScale;
}

void shift(double sx, double sy)
{
    xx += sx * scale;
    yy += sy * scale;
}

void gfxFractals()
{
    if (!initScreen(SCR_WIDTH, SCR_HEIGHT, 32, 0, "Fractals Explorer")) return;

    initThreads();
    initFunctions(fractType);
    initFractals(getDrawBufferWidth(), getDrawBufferHeight());

    bool redraw = true;
    bool mouseDown = false;

    int32_t msx = 0, msy = 0;
    int32_t input = 0, dataY = 0;

    SDL_Cursor* oldCursor = SDL_GetCursor();
    SDL_Cursor* handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    if (!handCursor)
    {
        messageBox(GFX_ERROR, "Error create hand cursor:%s!", SDL_GetError());
        return;
    }

    do {
        //only draw when needed
        if (redraw)
        {
            uint32_t acx = alignSize(cx);
            allocBuffer();
            clock_t t1 = clock();
            calculateMultiThread();
            clock_t t2 = clock();

            //we use render user-defined buffer
            renderBuffer(data, acx, cy);

            //Julia message
            if (fractType)
            {
                sprintf(sbuff, "Julia-Explorer [%s][%dx%d][%g, %g, %g, %g, %g, %d][%ld ms][%u cores][%.2f FPS][%s][tab, arrows, spacer, 1, 2, 3, 4, i, o, s, q, w]",
                    methodNames[funcIndexs[fractType]],
                    acx, cy,
                    xx, yy, scale,
                    cre, cim, iterations,
                    (t2 - t1),
                    cpuCores,
                    1000.0 / (intmax_t(t2) - t1),
                    fullModes[fractType] ? "ALL" : "SINGLE"
                );
            }
            else
            {
                sprintf(sbuff, "Mandelbrot-Explorer [%s][%dx%d][%g, %g, %g, %d][%ld ms][%u cores][%.2f FPS][%s][tab, arrows, spacer, i, o, s, q, w]",
                    methodNames[funcIndexs[fractType]],
                    acx, cy,
                    xx, yy, scale,
                    iterations,
                    (t2 - t1),
                    cpuCores,
                    1000.0 / (intmax_t(t2) - t1),
                    fullModes[fractType] ? "ALL" : "SINGLE"
                );
            }

            //raise windows title details
            setWindowTitle(sbuff);
            redraw = false;
        }

        //wait and process user input
        input = waitUserInput(
            INPUT_KEY_PRESSED   |
            INPUT_MOUSE_CLICK   |
            INPUT_MOUSE_MOTION  |
            INPUT_MOUSE_WHEEL   |
            INPUT_WIN_RESIZED
        );

        switch (input)
        {
        case SDL_SCANCODE_LEFT:
            shift(-(cx / 10.0), 0);
            redraw = true;
            break;

        case SDL_SCANCODE_RIGHT:
            shift(cx / 10.0, 0);
            redraw = true;
            break;

        case SDL_SCANCODE_UP:
            shift(0, -(cy / 10.0));
            redraw = true;
            break;

        case SDL_SCANCODE_DOWN:
            shift(0, cy / 10.0);
            redraw = true;
            break;

        case SDL_SCANCODE_ESCAPE:
            quit();
            break;

        case SDL_SCANCODE_I:
            setScale(scale / 1.1);
            redraw = true;
            break;

        case SDL_SCANCODE_O:
            setScale(scale * 1.1);
            redraw = true;
            break;

        case SDL_SCANCODE_SPACE:
            funcIndexs[fractType]++;
            if (funcIndexs[fractType] == funcCounts[fractType]) funcIndexs[fractType] = 0;
            calculateFuncs[fractType] = funcs[fractType][funcIndexs[fractType]];
            redraw = true;
            break;

        case SDL_SCANCODE_TAB:
            fullModes[fractType] = !fullModes[fractType];
            initFunctions(fractType);
            redraw = true;
            break;

        case SDL_SCANCODE_S:
            fractType = !fractType;
            initFractals(getDrawBufferWidth(), getDrawBufferHeight());
            initFunctions(fractType);
            redraw = true;
            break;

        case SDL_SCANCODE_Q:
            iterations <<= 1;
            redraw = true;
            break;

        case SDL_SCANCODE_W:
            if (iterations > 2) iterations >>= 1;
            redraw = true;
            break;

        case SDL_SCANCODE_1:
            if (fractType) cim += 0.0002;
            redraw = true;
            break;

        case SDL_SCANCODE_2:
            if (fractType) cim -= 0.0002;
            redraw = true;
            break;

        case SDL_SCANCODE_3:
            if (fractType) cre += 0.0002;
            redraw = true;
            break;

        case SDL_SCANCODE_4:
            if (fractType) cre -= 0.0002;
            redraw = true;
            break;
        
        case SDL_WINDOWEVENT_RESIZED:
            cx = getInputDataX();
            cy = getInputDataY();
            redraw = true;
            break;

        case SDL_MOUSEWHEEL:
            dataY = getInputDataY();
            while (dataY > 0)
            {
                setScale(scale / 1.1);
                dataY--;
            }
            while (dataY < 0)
            {
                setScale(scale * 1.1);
                dataY++;
            }
            redraw = true;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseDown = true;
            msx = getInputDataX();
            msy = getInputDataY();
            SDL_SetCursor(handCursor);
            break;

        case SDL_MOUSEBUTTONUP:
            mouseDown = false;
            SDL_SetCursor(oldCursor);
            break;

        case SDL_MOUSEMOTION:
            if (mouseDown)
            {
                const int32_t ptx = getInputDataX();
                const int32_t pty = getInputDataY();
                shift(double(msx) - ptx, double(msy) - pty);
                msx = ptx;
                msy = pty;
                redraw = true;
            }
            break;

        default:
            break;
        }

        //reduce CPU time
        if (!redraw) delay(1);

    } while (input != SDL_SCANCODE_RETURN);

    //cleanup
    _mm_free(data);
    SDL_FreeCursor(handCursor);
    cleanup();
}
