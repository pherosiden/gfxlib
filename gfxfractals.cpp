#include "gfxlib.h"
#include <windows.h>

//screen size
#define MAXX    1280
#define MAXY    900

#define _128_shuffle(x, y, n0, n1, n2, n3)  _mm_shuffle_ps(x, y, combine_4_2bits(n0, n1, n2, n3))
#define _128i_shuffle(x, y, n0, n1, n2, n3) _mm_castps_si128(_128_shuffle(_mm_castsi128_ps(x), _mm_castsi128_ps(y), n0, n1, n2, n3))
#define combine_4_2bits(n0, n1, n2, n3)     (n0 + (n1 << 2) + (n2 << 4) + (n3 << 6))

typedef void func_t(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t sx, int32_t sy);

double cim = 0;
double cre = 0;
int32_t iterations = 0;

void mandelbrot_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t j = ys; j < py; j++)
    {
        const float y0 = j * float(scale) + float(my);
        for (int32_t i = 0; i < px; i++)
        {
            const float x0 = i * float(scale) + float(mx);
            float x1 = x0;
            float y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const float x2 = x1 * x1;
                const float y2 = y1 * y1;
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + y0;
                x1 = x2 - y2 + x0;
            }
            *out++ = n;
        }
    }
}

void mandelbrot_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t j = ys; j < py; j++)
    {
        const double y0 = j * scale + my;
        for (int32_t i = 0; i < px; i++)
        {
            const double x0 = i * scale + mx;
            double x1 = x0;
            double y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const double x2 = x1 * x1;
                const double y2 = y1 * y1;
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + y0;
                x1 = x2 - y2 + x0;
            }
            *out++ = n;
        }
    }
}

void julia_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t j = ys; j < py; j++)
    {
        const float y0 = j * float(scale) + float(my);
        for (int32_t i = 0; i < px; i++)
        {
            const float x0 = i * float(scale) + float(mx);
            float x1 = x0;
            float y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const float x2 = x1 * x1;
                const float y2 = y1 * y1;
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + float(cim);
                x1 = x2 - y2 + float(cre);
            }
            *out++ = n;
        }
    }
}

void julia_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    for (int32_t j = ys; j < py; j++)
    {
        const double y0 = j * scale + my;
        for (int32_t i = 0; i < px; i++)
        {
            const double x0 = i * scale + mx;
            double x1 = x0;
            double y1 = y0;
            int32_t n = 0;
            for (; n < iterations; n++)
            {
                const double x2 = x1 * x1;
                const double y2 = y1 * y1;
                if (x2 + y2 >= 4) break;
                y1 = 2 * x1 * y1 + cim;
                x1 = x2 - y2 + cre;
            }
            //*out++ = n;
            *(uint32_t*)out = n;
            out += 4;
        }
    }
}

void mandelbrot_sse_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128 dd = _mm_set1_ps(float(scale));
    const __m128 tx = _mm_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m128 y0 = _mm_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m128 x0 = _mm_add_ps(tx, _mm_mul_ps(dd, _mm_cvtepi32_ps(ind)));
            __m128 x = x0;
            __m128 y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                __m128 x2 = _mm_mul_ps(x, x);
                __m128 y2 = _mm_mul_ps(y, y);
                __m128 abs = _mm_add_ps(x2, y2);
                __m128i cmp = _mm_castps_si128(_mm_cmplt_ps(abs, _mm_set1_ps(4)));

                cmp_mask = _mm_and_si128(cmp_mask, cmp);
                if (_mm_testz_si128(cmp_mask, cmp_mask)) break; //SSE4.1 instruction

                counts = _mm_sub_epi32(counts, cmp_mask);
                __m128 t = _mm_mul_ps(x, y);
                t = _mm_add_ps(t, t);
                y = _mm_add_ps(t, y0);
                x = _mm_add_ps(_mm_sub_ps(x2, y2), x0);
            }

            const __m128i result = _mm_shuffle_epi8(counts, _mm_setr_epi8(0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12)); //SSSE3 instruction
            *(uint32_t*)out = _mm_extract_epi32(result, 0);
            out += 4;
        }
    }
}

void mandelbrot_sse_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128d dd = _mm_set1_pd(scale);
    const __m128d tx = _mm_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m128d y0 = _mm_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 2)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, 0, 0);
            const __m128d x0 = _mm_add_pd(tx, _mm_mul_pd(dd, _mm_cvtepi32_pd(ind)));
            __m128d x = x0;
            __m128d y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                __m128d x2 = _mm_mul_pd(x, x);
                __m128d y2 = _mm_mul_pd(y, y);
                __m128d abs = _mm_add_pd(x2, y2);
                __m128i cmp = _mm_castpd_si128(_mm_cmplt_pd(abs, _mm_set1_pd(4)));

                cmp_mask = _mm_and_si128(cmp_mask, cmp);
                if (_mm_testz_si128(cmp_mask, cmp_mask)) break;

                counts = _mm_sub_epi64(counts, cmp_mask);

                __m128d t = _mm_mul_pd(x, y);
                t = _mm_add_pd(t, t);
                y = _mm_add_pd(t, y0);
                x = _mm_add_pd(_mm_sub_pd(x2, y2), x0);
            }

            __m128i result = _mm_shuffle_epi8(counts, _mm_setr_epi8(0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8));
            *(uint16_t*)out = _mm_extract_epi16(result, 0);
            out += 2;
        }
    }
}

void julia_sse_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128 xim = _mm_set1_ps(float(cim));
    const __m128 xre = _mm_set1_ps(float(cre));

    const __m128 dd = _mm_set1_ps(float(scale));
    const __m128 tx = _mm_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m128 y0 = _mm_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m128 x0 = _mm_add_ps(tx, _mm_mul_ps(dd, _mm_cvtepi32_ps(ind)));
            __m128 x = x0;
            __m128 y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                __m128 x2 = _mm_mul_ps(x, x);
                __m128 y2 = _mm_mul_ps(y, y);
                __m128 abs = _mm_add_ps(x2, y2);
                __m128i cmp = _mm_castps_si128(_mm_cmplt_ps(abs, _mm_set1_ps(4)));

                cmp_mask = _mm_and_si128(cmp_mask, cmp);
                if (_mm_testz_si128(cmp_mask, cmp_mask)) break; //SSE4.1 instruction

                counts = _mm_sub_epi32(counts, cmp_mask);
                __m128 t = _mm_mul_ps(x, y);
                t = _mm_add_ps(t, t);
                y = _mm_add_ps(t, xim);
                x = _mm_add_ps(_mm_sub_ps(x2, y2), xre);
            }

            const __m128i result = _mm_shuffle_epi8(counts, _mm_setr_epi8(0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12)); //SSSE3 instruction
            *(uint32_t*)out = _mm_extract_epi32(result, 0);
            out += 4;
        }
    }
}

void julia_sse_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m128d xim = _mm_set1_pd(cim);
    const __m128d xre = _mm_set1_pd(cre);

    const __m128d dd = _mm_set1_pd(scale);
    const __m128d tx = _mm_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m128d y0 = _mm_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 2)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, 0, 0);
            const __m128d x0 = _mm_add_pd(tx, _mm_mul_pd(dd, _mm_cvtepi32_pd(ind)));
            __m128d x = x0;
            __m128d y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                __m128d x2 = _mm_mul_pd(x, x);
                __m128d y2 = _mm_mul_pd(y, y);
                __m128d abs = _mm_add_pd(x2, y2);
                __m128i cmp = _mm_castpd_si128(_mm_cmplt_pd(abs, _mm_set1_pd(4)));

                cmp_mask = _mm_and_si128(cmp_mask, cmp);
                if (_mm_testz_si128(cmp_mask, cmp_mask)) break;

                counts = _mm_sub_epi64(counts, cmp_mask);

                __m128d t = _mm_mul_pd(x, y);
                t = _mm_add_pd(t, t);
                y = _mm_add_pd(t, xim);
                x = _mm_add_pd(_mm_sub_pd(x2, y2), xre);
            }

            __m128i result = _mm_shuffle_epi8(counts, _mm_setr_epi8(0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8));
            *(uint16_t*)out = _mm_extract_epi16(result, 0);
            out += 2;
        }
    }
}

void mandelbrot_avx_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m256 y0 = _mm256_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 8)
        {
            const __m256i ind = _mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x = x0;
            __m256 y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x, x);
                const __m256 y2 = _mm256_mul_ps(y, y);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), 1));
                const __m128i cmp_lo = _mm256_extractf128_si256(cmp, 0);
                const __m128i cmp_hi = _mm256_extractf128_si256(cmp, 1);
                const __m128i bytes_lo = _mm_shuffle_epi8(cmp_lo, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
                const __m128i bytes_hi = _mm_shuffle_epi8(cmp_hi, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
                const __m128i cmp_bytes = _128i_shuffle(bytes_lo, bytes_hi, 0, 0, 0, 0);

                cmp_mask = _mm_and_si128(cmp_mask, cmp_bytes);
                if (_mm_testnzc_si128(cmp_mask, cmp_mask)) break;

                counts = _mm_sub_epi8(counts, cmp_mask);
                __m256 t = _mm256_mul_ps(x, y);
                t = _mm256_add_ps(t, t);
                y = _mm256_add_ps(t, y0);
                x = _mm256_add_ps(_mm256_sub_ps(x2, y2), x0);
            }

            counts = _mm_shuffle_epi32(counts, combine_4_2bits(0, 2, 0, 2));
            _mm_storel_epi64((__m128i*)out, counts);
            out += 8;
        }
    }
}

void mandelbrot_avx_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m256d y0 = _mm256_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
            __m256d x = x0;
            __m256d y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x, x);
                const __m256d y2 = _mm256_mul_pd(y, y);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 1));
                const __m128i cmp_lo = _mm256_extractf128_si256(cmp, 0);
                const __m128i cmp_hi = _mm256_extractf128_si256(cmp, 1);
                const __m128i cmp_bytes = _128i_shuffle(cmp_lo, cmp_hi, 0, 2, 0, 2);
                // AAAABBBB CCCCDDDD

                cmp_mask = _mm_and_si128(cmp_mask, cmp_bytes);
                if(_mm_testz_si128(cmp_mask, cmp_mask)) break;

                counts = _mm_sub_epi32(counts, cmp_mask);
                __m256d t = _mm256_mul_pd(x, y);
                t = _mm256_add_pd(t, t);
                y = _mm256_add_pd(t, y0);
                x = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            const __m128i result = _mm_shuffle_epi8(counts, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
            *(uint32_t*)out = _mm_extract_epi32(result, 0);
            out += 4;
        }
    }
}

void julia_avx_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 xim = _mm256_set1_ps(float(cim));
    const __m256 xre = _mm256_set1_ps(float(cre));

    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m256 y0 = _mm256_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 8)
        {
            const __m256i ind = _mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x = x0;
            __m256 y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x, x);
                const __m256 y2 = _mm256_mul_ps(y, y);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), 1));
                const __m128i cmp_lo = _mm256_extractf128_si256(cmp, 0);
                const __m128i cmp_hi = _mm256_extractf128_si256(cmp, 1);
                const __m128i bytes_lo = _mm_shuffle_epi8(cmp_lo, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
                const __m128i bytes_hi = _mm_shuffle_epi8(cmp_hi, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
                const __m128i cmp_bytes = _128i_shuffle(bytes_lo, bytes_hi, 0, 0, 0, 0);

                cmp_mask = _mm_and_si128(cmp_mask, cmp_bytes);
                if (_mm_testnzc_si128(cmp_mask, cmp_mask)) break;

                counts = _mm_sub_epi8(counts, cmp_mask);
                __m256 t = _mm256_mul_ps(x, y);
                t = _mm256_add_ps(t, t);
                y = _mm256_add_ps(t, xim);
                x = _mm256_add_ps(_mm256_sub_ps(x2, y2), xre);
            }

            counts = _mm_shuffle_epi32(counts, combine_4_2bits(0, 2, 0, 2));
            _mm_storel_epi64((__m128i*)out, counts);
            out += 8;
        }
    }
}

void julia_avx_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m256d y0 = _mm256_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
            __m256d x = x0;
            __m256d y = y0;
            __m128i counts = _mm_setzero_si128();
            __m128i cmp_mask = _mm_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x, x);
                const __m256d y2 = _mm256_mul_pd(y, y);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 1));
                const __m128i cmp_lo = _mm256_extractf128_si256(cmp, 0);
                const __m128i cmp_hi = _mm256_extractf128_si256(cmp, 1);
                const __m128i cmp_bytes = _128i_shuffle(cmp_lo, cmp_hi, 0, 2, 0, 2);
                // AAAABBBB CCCCDDDD

                cmp_mask = _mm_and_si128(cmp_mask, cmp_bytes);
                if(_mm_testz_si128(cmp_mask, cmp_mask)) break;

                counts = _mm_sub_epi32(counts, cmp_mask);
                __m256d t = _mm256_mul_pd(x, y);
                t = _mm256_add_pd(t, t);
                y = _mm256_add_pd(t, xim);
                x = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            const __m128i result = _mm_shuffle_epi8(counts, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
            *(uint32_t*)out = _mm_extract_epi32(result, 0);
            out += 4;
        }
    }
}

void mandelbrot_avx2_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m256 y0 = _mm256_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 8)
        {
            const __m256i ind = _mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x = x0;
            __m256 y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x, x);
                const __m256 y2 = _mm256_mul_ps(y, y);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), 1));

                cmp_mask = _mm256_and_si256(cmp_mask, cmp);
                if (_mm256_testz_si256(cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi32(counts, cmp_mask);
                __m256 t = _mm256_mul_ps(x, y);
                t = _mm256_add_ps(t, t);
                y = _mm256_add_ps(t, y0);
                x = _mm256_add_ps(_mm256_sub_ps(x2, y2), x0);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12));
            __m128i result128 = _128i_shuffle(_mm256_extractf128_si256(result, 0), _mm256_extractf128_si256(result, 1), 0, 0, 0, 0);
            result128 = _mm_shuffle_epi32(result128, combine_4_2bits(0, 2, 0, 2));
            _mm_storel_epi64((__m128i*)out, result128);
            out += 8;
        }
    }
}

void mandelbrot_avx2_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m256d y0 = _mm256_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
            __m256d x = x0;
            __m256d y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x, x);
                const __m256d y2 = _mm256_mul_pd(y, y);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 1));

                cmp_mask = _mm256_and_si256(cmp_mask, cmp);
                if (_mm256_testz_si256(cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi64(counts, cmp_mask);
                __m256d t = _mm256_mul_pd(x, y);
                t = _mm256_add_pd(t, t);
                y = _mm256_add_pd(t, y0);
                x = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8));
            *(uint32_t*)out = _mm_extract_epi16(_mm256_extracti128_si256(result, 0), 0) | (_mm_extract_epi16(_mm256_extracti128_si256(result, 1), 0) << 16);
            out += 4;
        }
    }
}

void julia_avx2_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 xim = _mm256_set1_ps(float(cim));
    const __m256 xre = _mm256_set1_ps(float(cre));

    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m256 y0 = _mm256_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 8)
        {
            const __m256i ind = _mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
            const __m256 x0 = _mm256_add_ps(tx, _mm256_mul_ps(dd, _mm256_cvtepi32_ps(ind)));
            __m256 x = x0;
            __m256 y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x, x);
                const __m256 y2 = _mm256_mul_ps(y, y);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), 1));

                cmp_mask = _mm256_and_si256(cmp_mask, cmp);
                if (_mm256_testz_si256(cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi32(counts, cmp_mask);
                __m256 t = _mm256_mul_ps(x, y);
                t = _mm256_add_ps(t, t);
                y = _mm256_add_ps(t, xim);
                x = _mm256_add_ps(_mm256_sub_ps(x2, y2), xre);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12));
            __m128i result128 = _128i_shuffle(_mm256_extractf128_si256(result, 0), _mm256_extractf128_si256(result, 1), 0, 0, 0, 0);
            result128 = _mm_shuffle_epi32(result128, combine_4_2bits(0, 2, 0, 2));
            _mm_storel_epi64((__m128i*)out, result128);
            out += 8;
        }
    }
}

void julia_avx2_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m256d y0 = _mm256_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m256d x0 = _mm256_add_pd(tx, _mm256_mul_pd(dd, _mm256_cvtepi32_pd(ind)));
            __m256d x = x0;
            __m256d y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x, x);
                const __m256d y2 = _mm256_mul_pd(y, y);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 1));

                cmp_mask = _mm256_and_si256(cmp_mask, cmp);
                if (_mm256_testz_si256(cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi64(counts, cmp_mask);
                __m256d t = _mm256_mul_pd(x, y);
                t = _mm256_add_pd(t, t);
                y = _mm256_add_pd(t, xim);
                x = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8));
            *(uint32_t*)out = _mm_extract_epi16(_mm256_extracti128_si256(result, 0), 0) | (_mm_extract_epi16(_mm256_extracti128_si256(result, 1), 0) << 16);
            out += 4;
        }
    }
}

void mandelbrot_fma_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m256 y0 = _mm256_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 8)
        {
            const __m256i ind = _mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
            const __m256 x0 = _mm256_fmadd_ps(dd, _mm256_cvtepi32_ps(ind), tx);
            __m256 x = x0;
            __m256 y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x, x);
                const __m256 y2 = _mm256_mul_ps(y, y);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), 1));

                cmp_mask = _mm256_and_si256 (cmp_mask, cmp);
                if (_mm256_testz_si256 (cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi32 (counts, cmp_mask);
                const __m256 t = _mm256_add_ps (x, x);
                y = _mm256_fmadd_ps (t, y, y0);
                x = _mm256_add_ps (_mm256_sub_ps (x2, y2), x0);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12));
            __m128i result128 = _128i_shuffle(_mm256_extractf128_si256(result, 0), _mm256_extractf128_si256(result, 1), 0, 0, 0, 0);
            result128 = _mm_shuffle_epi32(result128, combine_4_2bits(0, 2, 0, 2));
            _mm_storel_epi64((__m128i*)out, result128);
            out += 8;
        }
    }
}

void mandelbrot_fma_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m256d y0 = _mm256_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m256d x0 = _mm256_fmadd_pd(dd, _mm256_cvtepi32_pd(ind), tx);
            __m256d x = x0;
            __m256d y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x, x);
                const __m256d y2 = _mm256_mul_pd(y, y);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 1));

                cmp_mask = _mm256_and_si256(cmp_mask, cmp);
                if (_mm256_testz_si256(cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi64(counts, cmp_mask);
                const __m256d t = _mm256_add_pd(x, x);
                y = _mm256_fmadd_pd(t, y, y0);
                x = _mm256_add_pd(_mm256_sub_pd(x2, y2), x0);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8));
            *(uint32_t*)out = _mm_extract_epi16(_mm256_extracti128_si256(result, 0), 0) | (_mm_extract_epi16(_mm256_extracti128_si256(result, 1), 0) << 16);
            out += 4;
        }
    }
}

void julia_fma_float(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256 xim = _mm256_set1_ps(float(cim));
    const __m256 xre = _mm256_set1_ps(float(cre));

    const __m256 dd = _mm256_set1_ps(float(scale));
    const __m256 tx = _mm256_set1_ps(float(mx));

    for (int32_t j = ys; j < py; j++)
    {
        const __m256 y0 = _mm256_set1_ps(j * float(scale) + float(my));
        for (int32_t i = 0; i < px; i += 8)
        {
            const __m256i ind = _mm256_setr_epi32(i, i + 1, i + 2, i + 3, i + 4, i + 5, i + 6, i + 7);
            const __m256 x0 = _mm256_fmadd_ps(dd, _mm256_cvtepi32_ps(ind), tx);
            __m256 x = x0;
            __m256 y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256 x2 = _mm256_mul_ps(x, x);
                const __m256 y2 = _mm256_mul_ps(y, y);
                const __m256 abs = _mm256_add_ps(x2, y2);
                const __m256i cmp = _mm256_castps_si256(_mm256_cmp_ps(abs, _mm256_set1_ps(4), 1));

                cmp_mask = _mm256_and_si256 (cmp_mask, cmp);
                if (_mm256_testz_si256 (cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi32 (counts, cmp_mask);
                const __m256 t = _mm256_add_ps (x, x);
                y = _mm256_fmadd_ps (t, y, xim);
                x = _mm256_add_ps (_mm256_sub_ps (x2, y2), xre);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12, 0, 4, 8, 12));
            __m128i result128 = _128i_shuffle(_mm256_extractf128_si256(result, 0), _mm256_extractf128_si256(result, 1), 0, 0, 0, 0);
            result128 = _mm_shuffle_epi32(result128, combine_4_2bits(0, 2, 0, 2));
            _mm_storel_epi64((__m128i*)out, result128);
            out += 8;
        }
    }
}

void julia_fma_double(uint8_t* out, double mx, double my, double scale, int32_t ys, int32_t px, int32_t py)
{
    const __m256d xim = _mm256_set1_pd(cim);
    const __m256d xre = _mm256_set1_pd(cre);

    const __m256d dd = _mm256_set1_pd(scale);
    const __m256d tx = _mm256_set1_pd(mx);

    for (int32_t j = ys; j < py; j++)
    {
        const __m256d y0 = _mm256_set1_pd(j * scale + my);
        for (int32_t i = 0; i < px; i += 4)
        {
            const __m128i ind = _mm_setr_epi32(i, i + 1, i + 2, i + 3);
            const __m256d x0 = _mm256_fmadd_pd(dd, _mm256_cvtepi32_pd(ind), tx);
            __m256d x = x0;
            __m256d y = y0;
            __m256i counts = _mm256_setzero_si256();
            __m256i cmp_mask = _mm256_set1_epi32(0xFFFFFFFFu);

            for (int32_t n = 0; n < iterations; n++)
            {
                const __m256d x2 = _mm256_mul_pd(x, x);
                const __m256d y2 = _mm256_mul_pd(y, y);
                const __m256d abs = _mm256_add_pd(x2, y2);
                const __m256i cmp = _mm256_castpd_si256(_mm256_cmp_pd(abs, _mm256_set1_pd(4), 1));

                cmp_mask = _mm256_and_si256(cmp_mask, cmp);
                if (_mm256_testz_si256(cmp_mask, cmp_mask)) break;

                counts = _mm256_sub_epi64(counts, cmp_mask);
                const __m256d t = _mm256_add_pd(x, x);
                y = _mm256_fmadd_pd(t, y, xim);
                x = _mm256_add_pd(_mm256_sub_pd(x2, y2), xre);
            }

            const __m256i result = _mm256_shuffle_epi8(counts, _mm256_setr_epi8(0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8));
            *(uint32_t*)out = _mm_extract_epi16(_mm256_extracti128_si256(result, 0), 0) | (_mm_extract_epi16(_mm256_extracti128_si256(result, 1), 0) << 16);
            out += 4;
        }
    }
}

func_t* calculateFunc[2] = { 0 };
func_t* funcs[2][20] = { 0 };

char sbuff[2000] = { 0 };
const char* methodNames[20] = { 0 };

int32_t funcCount[2] = { 0 };
int32_t funcIndex[2] = { 0 };

bool fullMode[2] = { 0 };

int32_t fractType = 1;

double xx = 0;
double yy = 0;
double scale = 0;

int32_t cx = 0;
int32_t cy = 0;

uint8_t *data = NULL;
uint32_t dataSize = 0;

uint32_t cpuCores = 0;

void initThreads()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    cpuCores = si.dwNumberOfProcessors;
    if (cpuCores > 64) cpuCores = 64;
}

void initFunctions(int32_t type)
{
    int32_t out[4] = { 0 };

    __cpuid(out, 1);

    bool sse42      = (out[2] & (1 << 20)) != 0;
    bool sse41      = (out[2] & (1 << 19)) != 0;
    bool fma        = (out[2] & (1 << 12)) != 0;
    bool avx        = (out[2] & (1 << 28)) != 0;
    bool osxsave    = (out[2] & (1 << 29)) != 0;

    __cpuidex(out, 7, 0);
    bool avx2 = (out[1] & 0x20) != 0;

    if (type)
    {
        int32_t i = 0;
        funcs[type][i] = julia_float;
        methodNames[i] = "float";
        if (fullMode[type]) i++;

        if (sse42)
        {
            funcs[1][i] = julia_sse_float;
            methodNames[i] = "float SSE";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = julia_avx_float;
            methodNames[i] = "float AVX";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = julia_avx2_float;
            methodNames[i] = "float AVX2";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = julia_fma_float;
            methodNames[i] = "float FMA";
            if (fullMode[type]) i++;
        }

        if (!fullMode[type]) i++;

        funcs[type][i] = julia_double;
        methodNames[i] = "double";
        if (fullMode[type]) i++;

        if (sse42)
        {
            funcs[type][i] = julia_sse_double;
            methodNames[i] = "double SSE";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = julia_avx_double;
            methodNames[i] = "double AVX";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = julia_avx2_double;
            methodNames[i] = "double AVX2";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = julia_fma_double;
            methodNames[i] = "double FMA";
            if (fullMode[type]) i++;
        }

        if (!fullMode[type]) i++;

        funcCount[type] = i;
        funcIndex[type] = 0;
        calculateFunc[type] = funcs[type][funcIndex[type]];
    }
    else
    {
        int32_t i = 0;
        funcs[type][i] = mandelbrot_float;
        methodNames[i] = "float";
        if (fullMode[type]) i++;

        if (sse42)
        {
            funcs[1][i] = mandelbrot_sse_float;
            methodNames[i] = "float SSE";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = mandelbrot_avx_float;
            methodNames[i] = "float AVX";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = mandelbrot_avx2_float;
            methodNames[i] = "float AVX2";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = mandelbrot_fma_float;
            methodNames[i] = "float FMA";
            if (fullMode[type]) i++;
        }

        if (!fullMode[type]) i++;

        funcs[type][i] = mandelbrot_double;
        methodNames[i] = "double";
        if (fullMode[type]) i++;

        if (sse42)
        {
            funcs[type][i] = mandelbrot_sse_double;
            methodNames[i] = "double SSE";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx)
        {
            funcs[type][i] = mandelbrot_avx_double;
            methodNames[i] = "double AVX";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2)
        {
            funcs[type][i] = mandelbrot_avx2_double;
            methodNames[i] = "double AVX2";
            if (fullMode[type]) i++;
        }

        if (osxsave && avx && avx2 && fma)
        {
            funcs[type][i] = mandelbrot_fma_double;
            methodNames[i] = "double FMA";
            if (fullMode[type]) i++;
        }

        if (!fullMode[type]) i++;

        funcCount[type] = i;
        funcIndex[type] = 0;
        calculateFunc[type] = funcs[type][funcIndex[type]];
    }
}

int32_t align(int32_t num)
{
    return (num + 7) & ~7;
}

void initPalette()
{
    RGB pal[256] = { 0 };

    for (int32_t i = 0; i < 16; i++)
    {
        pal[i].r = 63;
        pal[i].g = 0;
        pal[i].b = 63;
    }
    
    for (int32_t i = 0; i < 32; i++)
    {
        int32_t j = 16 + i;
        pal[j].r = 63;
        pal[j].g = 0;
        pal[j].b = 63 - (i << 1);

        j = 48 + i;
        pal[j].r = 63;
        pal[j].g = i << 1;
        pal[j].b = 0;

        j = 80 + i;
        pal[j].r = 63 - (i << 1);
        pal[j].g = 63;
        pal[j].b = 0;

        j = 112 + i;
        pal[j].r = 0;
        pal[j].g = 63;
        pal[j].b = i << 1;

        j = 144 + i;
        pal[j].r = 0;
        pal[j].g = 63 - (i << 1);
        pal[j].b = 63;

        j = 176 + i;
        pal[j].r = i << 1;
        pal[j].g = 0;
        pal[j].b = 63;
    }

    shiftPalette(pal);
    setPalette(pal);
}

void initBorders(int32_t sx, int32_t sy)
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

void allocData()
{
    const uint32_t MB = 1024 * 1024;
    uint32_t size = align(cx) * cy * bytesPerPixel;
    size = (size + MB - 1) & ~(MB - 1);
    if (size > dataSize)
    {
        uint8_t* pdata = NULL;
        if (data) pdata = (uint8_t*)_aligned_realloc(data, size, 32);
        else pdata = (uint8_t*)_aligned_malloc(size, 32);
        if (!pdata) exit(1);
        memset(pdata, 0, size);
        data = pdata;
        dataSize = size;
    }
}

volatile long ycurrent = 0;
const int32_t nlines = 64;

DWORD WINAPI threadProc(LPVOID lpThreadParameter)
{
    int32_t acx = align(cx);
    while (true)
    {
        int32_t y0 = InterlockedAdd(&ycurrent, nlines) - nlines;
        if (y0 >= cy) return 0;
        int32_t y1 = min(y0 + nlines, cy);
        calculateFunc[fractType](&data[y0 * acx], xx, yy, scale, y0, acx, y1);
    }
}

void calculateMultiThread()
{
    HANDLE threads[64] = { 0 };

    ycurrent = 0;
    for (uint32_t i = 0; i < cpuCores; i++) threads[i] = CreateThread(NULL, 0, &threadProc, NULL, 0, NULL);
    WaitForMultipleObjects(cpuCores, threads, TRUE, INFINITE);
    for (uint32_t i = 0; i < cpuCores; i++)
    {
        if (threads[i]) CloseHandle(threads[i]);
    }
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
    if (!initScreen(MAXX, MAXY, 8, 0, "DEMO")) return;

    initThreads();
    initFunctions(fractType);
    initBorders(texWidth, texHeight);
    initPalette();
    
    clock_t t1 = 0, t2 = 0;
    int32_t input = 0;
    uint32_t acx = 0;

    bool redraw = true;
    bool mouseDown = false;
    int32_t msx = 0, msy = 0;
    int32_t wheelm = 0;

    SDL_Cursor* oldCursor = SDL_GetCursor();
    SDL_Cursor* handCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    do {

        //only draw when needed
        if (redraw)
        {
            acx = align(cx);
            allocData();
            t1 = clock();
            calculateMultiThread();
            t2 = clock();

            //we use render user-defined buffer
            renderBuffer(data, acx, cy);

            //julia message
            if (fractType)
            {
                sprintf(sbuff, "Julia-Explorer [%s][%dx%d][%.18lf, %.18lf, %.18lf, %lf, %lf, %d][%ld ms][%u cores][%.2f FPS][%s][tab, arrows, spacer, i, o, s, 1, 2, 3, 4, q, w]",
                    methodNames[funcIndex[fractType]],
                    acx, cy,
                    xx, yy, scale,
                    cre, cim, iterations,
                    (t2 - t1),
                    cpuCores,
                    1000.0 / (intmax_t(t2) - t1),
                    fullMode[fractType] ? "ALL" : "SINGLE"
                );
            }
            else
            {
                sprintf(sbuff, "Mandelbrot-Explorer [%s][%dx%d][%.18lf, %.18lf, %.18lf, %d][%ld ms][%u cores][%.2f FPS][%s][tab, arrows, spacer, i, o, s]",
                    methodNames[funcIndex[fractType]],
                    acx, cy,
                    xx, yy, scale,
                    iterations,
                    (t2 - t1),
                    cpuCores,
                    1000.0 / (intmax_t(t2) - t1),
                    fullMode[fractType] ? "ALL" : "SINGLE"
                );
            }

            //raise windows title details
            setWindowTitle(sbuff);
            redraw = false;
        }

        //wait and process user input
        input = waitUserInput(
            INPUT_KEY_PRESSED |
            INPUT_MOUSE_CLICK |
            INPUT_MOUSE_MOTION |
            INPUT_MOUSE_WHEEL |
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
            funcIndex[fractType]++;
            if (funcIndex[fractType] == funcCount[fractType]) funcIndex[fractType] = 0;
            calculateFunc[fractType] = funcs[fractType][funcIndex[fractType]];
            redraw = true;
            break;

        case SDL_SCANCODE_TAB:
            fullMode[fractType] = !fullMode[fractType];
            initFunctions(fractType);
            redraw = true;
            break;

        case SDL_SCANCODE_S:
            fractType = !fractType;
            initBorders(texWidth, texHeight);
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
            if (fractType) cim += 0.00002;
            redraw = true;
            break;

        case SDL_SCANCODE_2:
            if (fractType) cim -= 0.00002;
            redraw = true;
            break;

        case SDL_SCANCODE_3:
            if (fractType) cre += 0.00002;
            redraw = true;
            break;

        case SDL_SCANCODE_4:
            if (fractType) cre -= 0.00002;
            redraw = true;
            break;
        
        case SDL_WINDOWEVENT_RESIZED:
            cx = winSizeX;
            cy = winSizeY;
            redraw = true;
            break;

        case SDL_MOUSEWHEEL:
            wheelm = mouseWheelY;
            while (wheelm > 0)
            {
                setScale(scale / 1.1);
                wheelm--;
            }
            while (wheelm < 0)
            {
                setScale(scale * 1.1);
                wheelm++;
            }
            redraw = true;
            break;

        case SDL_MOUSEBUTTONDOWN:
            mouseDown = true;
            getMouseState(&msx, &msy);
            SDL_SetCursor(handCursor);
            break;

        case SDL_MOUSEBUTTONUP:
            mouseDown = false;
            SDL_SetCursor(oldCursor);
            break;

        case SDL_MOUSEMOTION:
            if (mouseDown)
            {
                shift(double(msx) - mousePosX, double(msy) - mousePosY);
                msx = mousePosX;
                msy = mousePosY;
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
    free(data);
    SDL_FreeCursor(handCursor);
    cleanup();
}
