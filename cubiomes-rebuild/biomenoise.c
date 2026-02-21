// Cubiomes Rebuild - BiomeNoise System (1.21+ only)
// Rebuilt from original cubiomes, keeping only 1.18+ functionality

#include "biomenoise.h"

#include "tables/btree18.h"
#include "tables/btree192.h"
#include "tables/btree19.h"
#include "tables/btree20.h"
#include "tables/btree21wd.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#ifdef _OPENMP
#include <omp.h>
#endif

//==============================================================================
// Forward Declarations
//==============================================================================

static inline void getVoronoiCell(uint64_t sha, int a, int b, int c, int *x, int *y, int *z);
void voronoiAccess3D(uint64_t sha, int x, int y, int z, int *x4, int *y4, int *z4);

//==============================================================================
// Nether Biome Generation (1.16+)
//==============================================================================

void setNetherSeed(NetherNoise *nn, uint64_t seed)
{
    uint64_t s;
    setSeed(&s, seed);
    doublePerlinInit(&nn->temperature, &s, &nn->oct[0], &nn->oct[2], -7, 2);
    setSeed(&s, seed + 1);
    doublePerlinInit(&nn->humidity, &s, &nn->oct[4], &nn->oct[6], -7, 2);
}

int getNetherBiome(const NetherNoise *nn, int x, int y, int z, float *ndel)
{
    const float npoints[5][4] = {
        { 0,    0,      0,              nether_wastes       },
        { 0,   -0.5,    0,              soul_sand_valley    },
        { 0.4,  0,      0,              crimson_forest      },
        { 0,    0.5,    0.375*0.375,    warped_forest       },
        {-0.5,  0,      0.175*0.175,    basalt_deltas       },
    };

    y = 0; // Nether biomes don't vary by Y
    float temp = sampleDoublePerlin(&nn->temperature, x, y, z);
    float humidity = sampleDoublePerlin(&nn->humidity, x, y, z);

    int i, id = 0;
    float dmin = FLT_MAX;
    float dmin2 = FLT_MAX;
    
    for (i = 0; i < 5; i++)
    {
        float dx = npoints[i][0] - temp;
        float dy = npoints[i][1] - humidity;
        float dsq = dx*dx + dy*dy + npoints[i][2];
        
        if (dsq < dmin)
        {
            dmin2 = dmin;
            dmin = dsq;
            id = i;
        }
        else if (dsq < dmin2)
        {
            dmin2 = dsq;
        }
    }

    if (ndel)
        *ndel = sqrtf(dmin2) - sqrtf(dmin);

    id = (int) npoints[id][3];
    return id;
}

int mapNether2D(const NetherNoise *nn, int *out, int x, int z, int w, int h)
{
    int i, j;
    #pragma omp parallel for collapse(2) schedule(static) private(i, j)
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            out[j*w + i] = getNetherBiome(nn, x+i, 0, z+j, NULL);
        }
    }
    return 0;
}

static void fillRad3D(int *out, int x, int y, int z, int sx, int sy, int sz,
    int id, float rad)
{
    int r = (int) rad;
    if (r <= 0)
        return;
    
    int rsq = (int) floor(rad * rad);
    int i, j, k;

    for (k = -r; k <= r; k++)
    {
        int ak = y + k;
        if (ak < 0 || ak >= sy)
            continue;
        int ksq = k*k;
        int *yout = &out[(int64_t)ak*sx*sz];

        for (j = -r; j <= r; j++)
        {
            int aj = z + j;
            if (aj < 0 || aj >= sz)
                continue;
            int jksq = j*j + ksq;
            
            for (i = -r; i <= r; i++)
            {
                int ai = x + i;
                if (ai < 0 || ai >= sx)
                    continue;
                int ijksq = i*i + jksq;
                if (ijksq > rsq)
                    continue;

                yout[(int64_t)aj*sx + ai] = id;
            }
        }
    }
}

int mapNether3D(const NetherNoise *nn, int *out, Range r, float confidence)
{
    int64_t i, j, k;
    if (r.sy <= 0)
        r.sy = 1;
    if (r.scale <= 3)
    {
        fprintf(stderr, "mapNether3D() invalid scale\n");
        return 1;
    }
    
    int scale = r.scale / 4;
    memset(out, 0, sizeof(int) * r.sx*r.sy*r.sz);

    // Noise delta optimization
    float invgrad = 1.0 / (confidence * 0.05 * 2) / scale;

    for (k = 0; k < r.sy; k++)
    {
        int *yout = &out[k*r.sx*r.sz];

        for (j = 0; j < r.sz; j++)
        {
            for (i = 0; i < r.sx; i++)
            {
                if (yout[j*r.sx + i])
                    continue;

                float noisedelta;
                int xi = (r.x + i) * scale;
                int yk = (r.y + k);
                int zj = (r.z + j) * scale;
                int v = getNetherBiome(nn, xi, yk, zj, &noisedelta);
                yout[j*r.sx + i] = v;
                
                float cellrad = noisedelta * invgrad;
                fillRad3D(out, i, j, k, r.sx, r.sy, r.sz, v, cellrad);
            }
        }
    }
    return 0;
}

int genNetherScaled(const NetherNoise *nn, int *out, Range r, int mc, uint64_t sha)
{
    (void)mc; // unused - kept for API compatibility
    if (r.scale <= 0) r.scale = 4;
    if (r.sy == 0) r.sy = 1;

    uint64_t siz = (uint64_t)r.sx*r.sy*r.sz;

    if (r.scale == 1)
    {
        Range s = getVoronoiSrcRange(r);
        int *src;
        if (siz > 1)
        {   // the source range is large enough that we can try optimizing
            src = out + siz;
            int err = mapNether3D(nn, src, s, 1.0);
            if (err)
                return err;
        }
        else
        {
            src = NULL;
        }

        int i, j, k;
        #pragma omp parallel for collapse(3) schedule(static) private(i, j, k)
        for (k = 0; k < r.sy; k++)
        {
            for (j = 0; j < r.sz; j++)
            {
                for (i = 0; i < r.sx; i++)
                {
                    int x4, z4, y4;
                    voronoiAccess3D(sha, r.x+i, r.y+k, r.z+j, &x4, &y4, &z4);
                    int idx = k*r.sx*r.sz + j*r.sx + i;
                    if (src)
                    {
                        x4 -= s.x; y4 -= s.y; z4 -= s.z;
                        out[idx] = src[(int64_t)y4*s.sx*s.sz + (int64_t)z4*s.sx + x4];
                    }
                    else
                    {
                        out[idx] = getNetherBiome(nn, x4, y4, z4, NULL);
                    }
                }
            }
        }
        return 0;
    }
    else
    {
        return mapNether3D(nn, out, r, 1.0);
    }
}

//==============================================================================
// End Biome Generation (1.9+)
//==============================================================================

void setEndSeed(EndNoise *en, int mc, uint64_t seed)
{
    en->mc = mc;
    uint64_t s;
    setSeed(&s, seed);
    skipNextN(&s, 17292);
    perlinInit(&en->perlin, &s);
}

int mapEndBiome(const EndNoise *en, int *out, int x, int z, int w, int h)
{
    int i, j;
    #pragma omp parallel for collapse(2) schedule(static) private(i, j)
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int bx = (x + i) >> 2;
            int bz = (z + j) >> 2;
            
            if (bx*bx + bz*bz <= 4096)
            {
                out[j*w + i] = the_end;
            }
            else
            {
                double noise = sampleSimplex2D(&en->perlin, bx * 2 + 1, bz * 2 + 1);
                
                if (noise > 40)
                    out[j*w + i] = end_highlands;
                else if (noise >= 0)
                    out[j*w + i] = end_midlands;
                else if (noise >= -20)
                    out[j*w + i] = end_barrens;
                else
                    out[j*w + i] = small_end_islands;
            }
        }
    }
    return 0;
}

int mapEnd(const EndNoise *en, int *out, int x, int z, int w, int h)
{
    int i, j;
    int sw = w / 4 + 2;
    int sh = h / 4 + 2;
    int *buf = (int*) malloc(sw * sh * sizeof(int));
    if (!buf)
        return -1;
    
    int err = mapEndBiome(en, buf, x / 4 - 1, z / 4 - 1, sw, sh);
    if (err)
    {
        free(buf);
        return err;
    }
    
    #pragma omp parallel for collapse(2) schedule(static) private(i, j)
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int jj = (j >> 2);
            int dy = (j & 3);
            int ii = (i >> 2);
            int dx = (i & 3);
            
            int v00 = buf[(jj+0)*sw + (ii+0)];
            int v01 = buf[(jj+0)*sw + (ii+1)];
            int v10 = buf[(jj+1)*sw + (ii+0)];
            int v11 = buf[(jj+1)*sw + (ii+1)];
            
            // Simple nearest neighbor
            int v;
            if (dx < 2)
                v = (dy < 2) ? v00 : v10;
            else
                v = (dy < 2) ? v01 : v11;
            
            out[j*w + i] = v;
        }
    }
    
    free(buf);
    return 0;
}

// Placeholder for genEndScaled - will implement after voronoi functions
float getEndHeightNoise(const EndNoise *en, int x, int z, int range)
{
    int hx = x / 2;
    int hz = z / 2;
    int oddx = x % 2;
    int oddz = z % 2;
    int i, j;

    int64_t h = 64 * (x*(int64_t)x + z*(int64_t)z);
    if (range == 0)
        range = 12;

    for (j = -range; j <= range; j++)
    {
        for (i = -range; i <= range; i++)
        {
            int64_t rx = hx + i;
            int64_t rz = hz + j;
            uint64_t rsq = rx*rx + rz*rz;
            uint16_t v = 0;
            if (rsq > 4096 && sampleSimplex2D(&en->perlin, rx, rz) < -0.9f)
            {
                v = (unsigned int)(
                        fabsf((float)rx) * 3439.0f + fabsf((float)rz) * 147.0f
                    ) % 13 + 9;
                rx = (oddx - i * 2);
                rz = (oddz - j * 2);
                rsq = rx*rx + rz*rz;
                int64_t noise = rsq * v*v;
                if (noise < h)
                    h = noise;
            }
        }
    }

    float ret = 100 - sqrtf((float) h);
    if (ret < -100) ret = -100;
    if (ret > 80) ret = 80;
    return ret;
}

int genEndScaled(const EndNoise *en, int *out, Range r, int mc, uint64_t sha)
{
    (void)mc; // unused - kept for API compatibility
    if (r.sy == 0)
        r.sy = 1;

    int err, iy;

    if (r.scale == 1)
    {
        // For 1.18+, use voronoi mapping
        Range s = getVoronoiSrcRange(r);
        int *src = out + (int64_t)r.sx*r.sy*r.sz;
        err = mapEnd(en, src, s.x, s.z, s.sx, s.sz);
        if (err) return err;

        // Apply 3D voronoi
        int iy2, j2, i2;
        #pragma omp parallel for collapse(3) schedule(static) private(iy2, j2, i2)
        for (iy2 = 0; iy2 < r.sy; iy2++)
        {
            for (j2 = 0; j2 < r.sz; j2++)
            {
                for (i2 = 0; i2 < r.sx; i2++)
                {
                    int x4, y4, z4;
                    voronoiAccess3D(sha, r.x+i2, r.y+iy2, r.z+j2, &x4, &y4, &z4);
                    x4 -= s.x; y4 -= s.y; z4 -= s.z;
                    int idx = iy2*r.sx*r.sz + j2*r.sx + i2;
                    if (x4 >= 0 && x4 < s.sx && z4 >= 0 && z4 < s.sz)
                        out[idx] = src[z4*s.sx + x4];
                    else
                        out[idx] = the_end;
                }
            }
        }
        return 0;
    }
    else if (r.scale == 4)
    {
        err = mapEnd(en, out, r.x, r.z, r.sx, r.sz);
        if (err) return err;
    }
    else if (r.scale == 16)
    {
        err = mapEndBiome(en, out, r.x, r.z, r.sx, r.sz);
        if (err) return err;
    }
    else
    {
        float d = r.scale / 8.0f;
        int i, j;
        #pragma omp parallel for collapse(2) schedule(static) private(i, j)
        for (j = 0; j < r.sz; j++)
        {
            for (i = 0; i < r.sx; i++)
            {
                int64_t hx = (int64_t)((i+r.x) * d);
                int64_t hz = (int64_t)((j+r.z) * d);
                uint64_t rsq = hx*hx + hz*hz;
                if (rsq <= 16384L)
                {
                    out[j*r.sx+i] = the_end;
                    continue;
                }
                float h = getEndHeightNoise(en, hx, hz, 4);
                if (h > 40)
                    out[j*r.sx+i] = end_highlands;
                else if (h >= 0)
                    out[j*r.sx+i] = end_midlands;
                else if (h >= -20)
                    out[j*r.sx+i] = end_barrens;
                else
                    out[j*r.sx+i] = small_end_islands;
            }
        }
    }

    // expanding 2D into 3D
    if (r.sy > 1)
    {
        int64_t siz = (int64_t)r.sx*r.sz;
        #pragma omp parallel for schedule(static)
        for (iy = 1; iy < r.sy; iy++)
        {
            memcpy(out + iy*siz, out, siz * sizeof(int));
        }
    }

    return 0;
}

//==============================================================================
// Overworld Biome Generation (1.18+)
//==============================================================================

// Climate Parameter Initialization

static int init_climate_seed(
    DoublePerlinNoise *dpn, PerlinNoise *oct,
    uint64_t xlo, uint64_t xhi, int large, int nptype, int nmax
    )
{
    Xoroshiro pxr;
    int n = 0;

    switch (nptype)
    {
    case NP_SHIFT: {
        static const double amp[] = {1, 1, 1, 0};
        pxr.lo = xlo ^ 0x080518cf6af25384;
        pxr.hi = xhi ^ 0x3f3dfb40a54febd5;
        n += xDoublePerlinInit(dpn, &pxr, oct, amp, -3, 4, nmax);
        } break;

    case NP_TEMPERATURE: {
        static const double amp[] = {1.5, 0, 1, 0, 0, 0};
        pxr.lo = xlo ^ (large ? 0x944b0073edf549db : 0x5c7e6b29735f0d7f);
        pxr.hi = xhi ^ (large ? 0x4ff44347e9d22b96 : 0xf7d86f1bbc734988);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp, large ? -12 : -10, 6, nmax);
        } break;

    case NP_HUMIDITY: {
        static const double amp[] = {1, 1, 0, 0, 0, 0};
        pxr.lo = xlo ^ (large ? 0x71b8ab943dbd5301 : 0x81bb4d22e8dc168e);
        pxr.hi = xhi ^ (large ? 0xbb63ddcf39ff7a2b : 0xf1c8b4bea16303cd);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp, large ? -10 : -8, 6, nmax);
        } break;

    case NP_CONTINENTALNESS: {
        static const double amp[] = {1, 1, 2, 2, 2, 1, 1, 1, 1};
        pxr.lo = xlo ^ (large ? 0x9a3f51a113fce8dc : 0x83886c9d0ae3a662);
        pxr.hi = xhi ^ (large ? 0xee2dbd157e5dcdad : 0xafa638a61b42e8ad);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp, large ? -11 : -9, 9, nmax);
        } break;

    case NP_EROSION: {
        static const double amp[] = {1, 1, 0, 1, 1};
        pxr.lo = xlo ^ (large ? 0x8c984b1f8702a951 : 0xd02491e6058f6fd8);
        pxr.hi = xhi ^ (large ? 0xead7b1f92bae535f : 0x4792512c94c17a80);
        n += xDoublePerlinInit(dpn, &pxr, oct, amp, large ? -11 : -9, 5, nmax);
        } break;
    case NP_WEIRDNESS: {
        static const double amp[] = {1, 2, 1, 0, 0, 0};
        // md5 "minecraft:ridge"
        pxr.lo = xlo ^ 0xefc8ef4d36102b34;
        pxr.hi = xhi ^ 0x1beeeb324a0f24ea;
        n += xDoublePerlinInit(dpn, &pxr, oct, amp, -7, 6, nmax);
        } break;

    default:
        fprintf(stderr, "unsupported climate parameter %d\n", nptype);
        exit(1);
    }
    return n;
}

void setBiomeSeed(BiomeNoise *bn, uint64_t seed, int large)
{
    Xoroshiro pxr;
    xSetSeed(&pxr, seed);
    uint64_t xlo = xNextLong(&pxr);
    uint64_t xhi = xNextLong(&pxr);

    int n = 0, i = 0;
    for (; i < NP_MAX; i++)
        n += init_climate_seed(&bn->climate[i], bn->oct+n, xlo, xhi, large, i, -1);

    if ((size_t)n > sizeof(bn->oct) / sizeof(*bn->oct))
    {
        fprintf(stderr, "setBiomeSeed(): BiomeNoise is malformed, buffer too small\n");
        exit(1);
    }
    bn->nptype = -1;
}

void setClimateParaSeed(BiomeNoise *bn, uint64_t seed, int large, int nptype, int nmax)
{
    Xoroshiro pxr;
    xSetSeed(&pxr, seed);
    uint64_t xlo = xNextLong(&pxr);
    uint64_t xhi = xNextLong(&pxr);
    if (nptype == NP_DEPTH)
    {
        int n = 0;
        n += init_climate_seed(bn->climate + NP_CONTINENTALNESS,
            bn->oct + n, xlo, xhi, large, NP_CONTINENTALNESS, nmax);
        n += init_climate_seed(bn->climate + NP_EROSION,
            bn->oct + n, xlo, xhi, large, NP_EROSION, nmax);
        n += init_climate_seed(bn->climate + NP_WEIRDNESS,
            bn->oct + n, xlo, xhi, large, NP_WEIRDNESS, nmax);
    }
    else
    {
        init_climate_seed(bn->climate + nptype, bn->oct, xlo, xhi, large, nptype, nmax);
    }
    bn->nptype = nptype;
}

//==============================================================================
// Spline System
//==============================================================================

enum { SP_CONTINENTALNESS, SP_EROSION, SP_RIDGES, SP_WEIRDNESS };

static void addSplineVal(Spline *rsp, float loc, Spline *val, float der)
{
    rsp->loc[rsp->len] = loc;
    rsp->val[rsp->len] = val;
    rsp->der[rsp->len] = der;
    rsp->len++;
}

static Spline *createFixSpline(SplineStack *ss, float val)
{
    FixSpline *sp = &ss->fstack[ss->flen++];
    sp->len = 1;
    sp->val = val;
    return (Spline*)sp;
}

static float getOffsetValue(float weirdness, float continentalness)
{
    float f0 = 1.0F - (1.0F - continentalness) * 0.5F;
    float f1 = 0.5F * (1.0F - continentalness);
    float f2 = (weirdness + 1.17F) * 0.46082947F;
    float off = f2 * f0 - f1;
    if (weirdness < -0.7F)
        return off > -0.2222F ? off : -0.2222F;
    else
        return off > 0 ? off : 0;
}

static Spline *createSpline_38219(SplineStack *ss, float f, int bl)
{
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = SP_RIDGES;

    float i = getOffsetValue(-1.0F, f);
    float k = getOffsetValue( 1.0F, f);
    float l = 1.0F - (1.0F - f) * 0.5F;
    float u = 0.5F * (1.0F - f);
    l = u / (0.46082947F * l) - 1.17F;

    if (-0.65F < l && l < 1.0F)
    {
        float p, q, r, s;
        u = getOffsetValue(-0.65F, f);
        p = getOffsetValue(-0.75F, f);
        q = (p - i) * 4.0F;
        r = getOffsetValue(l, f);
        s = (k - r) / (1.0F - l);

        addSplineVal(sp, -1.0F,     createFixSpline(ss, i), q);
        addSplineVal(sp, -0.75F,    createFixSpline(ss, p), 0);
        addSplineVal(sp, -0.65F,    createFixSpline(ss, u), 0);
        addSplineVal(sp, l-0.01F,   createFixSpline(ss, r), 0);
        addSplineVal(sp, l,         createFixSpline(ss, r), s);
        addSplineVal(sp, 1.0F,      createFixSpline(ss, k), s);
    }
    else
    {
        u = (k - i) * 0.5F;
        if (bl) {
            addSplineVal(sp, -1.0F, createFixSpline(ss, i > 0.2 ? i : 0.2), 0);
            addSplineVal(sp,  0.0F, createFixSpline(ss, lerp(0.5F, i, k)), u);
        } else {
            addSplineVal(sp, -1.0F, createFixSpline(ss, i), u);
        }
        addSplineVal(sp, 1.0F,      createFixSpline(ss, k), u);
    }
    return sp;
}

static Spline *createFlatOffsetSpline(
    SplineStack *ss, float f, float g, float h, float i, float j, float k)
{
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = SP_RIDGES;

    float l = 0.5F * (g - f); if (l < k) l = k;
    float m = 5.0F * (h - g);

    addSplineVal(sp, -1.0F, createFixSpline(ss, f), l);
    addSplineVal(sp, -0.4F, createFixSpline(ss, g), l < m ? l : m);
    addSplineVal(sp,  0.0F, createFixSpline(ss, h), m);
    addSplineVal(sp,  0.4F, createFixSpline(ss, i), 2.0F*(i-h));
    addSplineVal(sp,  1.0F, createFixSpline(ss, j), 0.7F*(j-i));

    return sp;
}

static Spline *createLandSpline(
    SplineStack *ss, float f, float g, float h, float i, float j, float k, int bl)
{
    Spline *sp1 = createSpline_38219(ss, lerp(i, 0.6F, 1.5F), bl);
    Spline *sp2 = createSpline_38219(ss, lerp(i, 0.6F, 1.0F), bl);
    Spline *sp3 = createSpline_38219(ss, i, bl);
    const float ih = 0.5F * i;
    Spline *sp4 = createFlatOffsetSpline(ss, f-0.15F, ih, ih, ih, i*0.6F, 0.5F);
    Spline *sp5 = createFlatOffsetSpline(ss, f, j*i, g*i, ih, i*0.6F, 0.5F);
    Spline *sp6 = createFlatOffsetSpline(ss, f, j, j, g, h, 0.5F);
    Spline *sp7 = createFlatOffsetSpline(ss, f, j, j, g, h, 0.5F);

    Spline *sp8 = &ss->stack[ss->len++];
    sp8->typ = SP_RIDGES;
    addSplineVal(sp8, -1.0F, createFixSpline(ss, f), 0.0F);
    addSplineVal(sp8, -0.4F, sp6, 0.0F);
    addSplineVal(sp8,  0.0F, createFixSpline(ss, h + 0.07F), 0.0F);

    Spline *sp9 = createFlatOffsetSpline(ss, -0.02F, k, k, g, h, 0.0F);
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = SP_EROSION;
    addSplineVal(sp, -0.85F, sp1, 0.0F);
    addSplineVal(sp, -0.7F,  sp2, 0.0F);
    addSplineVal(sp, -0.4F,  sp3, 0.0F);
    addSplineVal(sp, -0.35F, sp4, 0.0F);
    addSplineVal(sp, -0.1F,  sp5, 0.0F);
    addSplineVal(sp,  0.2F,  sp6, 0.0F);
    if (bl) {
        addSplineVal(sp, 0.4F,  sp7, 0.0F);
        addSplineVal(sp, 0.45F, sp8, 0.0F);
        addSplineVal(sp, 0.55F, sp8, 0.0F);
        addSplineVal(sp, 0.58F, sp7, 0.0F);
    }
    addSplineVal(sp, 0.7F, sp9, 0.0F);
    return sp;
}

float getSpline(const Spline *sp, const float *vals)
{
    if (!sp || sp->len <= 0 || sp->len >= 12)
    {
        fprintf(stderr, "getSpline(): bad parameters\n");
        exit(1);
    }

    if (sp->len == 1)
        return ((FixSpline*)sp)->val;

    float f = vals[sp->typ];
    int i;

    for (i = 0; i < sp->len; i++)
        if (sp->loc[i] >= f)
            break;
    if (i == 0 || i == sp->len)
    {
        if (i) i--;
        float v = getSpline(sp->val[i], vals);
        return v + sp->der[i] * (f - sp->loc[i]);
    }
    const Spline *sp1 = sp->val[i-1];
    const Spline *sp2 = sp->val[i];
    float g = sp->loc[i-1];
    float h = sp->loc[i];
    float k = (f - g) / (h - g);
    float l = sp->der[i-1];
    float m = sp->der[i];
    float n = getSpline(sp1, vals);
    float o = getSpline(sp2, vals);
    float p = l * (h - g) - (o - n);
    float q = -m * (h - g) + (o - n);
    float r = lerp(k, n, o) + k * (1.0F - k) * lerp(k, p, q);
    return r;
}

void initBiomeNoise(BiomeNoise *bn, int mc)
{
    SplineStack *ss = &bn->ss;
    memset(ss, 0, sizeof(*ss));
    Spline *sp = &ss->stack[ss->len++];
    sp->typ = SP_CONTINENTALNESS;

    Spline *sp1 = createLandSpline(ss, -0.15F, 0.00F, 0.0F, 0.1F, 0.00F, -0.03F, 0);
    Spline *sp2 = createLandSpline(ss, -0.10F, 0.03F, 0.1F, 0.1F, 0.01F, -0.03F, 0);
    Spline *sp3 = createLandSpline(ss, -0.10F, 0.03F, 0.1F, 0.7F, 0.01F, -0.03F, 1);
    Spline *sp4 = createLandSpline(ss, -0.05F, 0.03F, 0.1F, 1.0F, 0.01F,  0.01F, 1);

    addSplineVal(sp, -1.10F, createFixSpline(ss,  0.044F), 0.0F);
    addSplineVal(sp, -1.02F, createFixSpline(ss, -0.2222F), 0.0F);
    addSplineVal(sp, -0.51F, createFixSpline(ss, -0.2222F), 0.0F);
    addSplineVal(sp, -0.44F, createFixSpline(ss, -0.12F), 0.0F);
    addSplineVal(sp, -0.18F, createFixSpline(ss, -0.12F), 0.0F);
    addSplineVal(sp, -0.16F, sp1, 0.0F);
    addSplineVal(sp, -0.15F, sp1, 0.0F);
    addSplineVal(sp, -0.10F, sp2, 0.0F);
    addSplineVal(sp,  0.25F, sp3, 0.0F);
    addSplineVal(sp,  1.00F, sp4, 0.0F);

    bn->sp = sp;
    bn->mc = mc;
}


//==============================================================================
// Biome Tree Lookup
//==============================================================================

static uint64_t get_np_dist(const uint64_t np[6], const BiomeTree *bt, int idx)
{
    uint64_t ds = 0, node = bt->nodes[idx];
    uint64_t a, b, d;
    uint32_t i;

    for (i = 0; i < 6; i++)
    {
        idx = (node >> 8*i) & 0xFF;
        a = np[i] - bt->param[2*idx + 1];
        b = bt->param[2*idx + 0] - np[i];
        d = (int64_t)a > 0 ? a : (int64_t)b > 0 ? b : 0;
        d = d * d;
        ds += d;
    }
    return ds;
}

static int get_resulting_node(const uint64_t np[6], const BiomeTree *bt, int idx,
    int alt, uint64_t ds, int depth)
{
    if (bt->steps[depth] == 0)
        return idx;
    uint32_t step;
    do
    {
        step = bt->steps[depth];
        depth++;
    }
    while (idx+step >= bt->len);

    uint64_t node = bt->nodes[idx];
    uint16_t inner = node >> 48;

    int leaf = alt;
    uint32_t i, n;

    for (i = 0, n = bt->order; i < n; i++)
    {
        uint64_t ds_inner = get_np_dist(np, bt, inner);
        if (ds_inner < ds)
        {
            int leaf2 = get_resulting_node(np, bt, inner, leaf, ds, depth);
            uint64_t ds_leaf2;
            if (inner == leaf2)
                ds_leaf2 = ds_inner;
            else
                ds_leaf2 = get_np_dist(np, bt, leaf2);
            if (ds_leaf2 < ds)
            {
                ds = ds_leaf2;
                leaf = leaf2;
            }
        }

        inner += step;
        if (inner >= bt->len)
            break;
    }

    return leaf;
}

int climateToBiome(int mc, const uint64_t np[6], uint64_t *dat)
{
    static const BiomeTree btree18 = {
        btree18_steps, &btree18_param[0][0], btree18_nodes, btree18_order,
        sizeof(btree18_nodes) / sizeof(uint64_t)
    };
    static const BiomeTree btree192 = {
        btree192_steps, &btree192_param[0][0], btree192_nodes, btree192_order,
        sizeof(btree192_nodes) / sizeof(uint64_t)
    };
    static const BiomeTree btree19 = {
        btree19_steps, &btree19_param[0][0], btree19_nodes, btree19_order,
        sizeof(btree19_nodes) / sizeof(uint64_t)
    };
    static const BiomeTree btree20 = {
        btree20_steps, &btree20_param[0][0], btree20_nodes, btree20_order,
        sizeof(btree20_nodes) / sizeof(uint64_t)
    };
    static const BiomeTree btree21wd = {
        btree21wd_steps, &btree21wd_param[0][0], btree21wd_nodes, btree21wd_order,
        sizeof(btree21wd_nodes) / sizeof(uint64_t)
    };

    const BiomeTree *bt;
    int idx;

    if (mc >= MC_1_21_WD)
        bt = &btree21wd;
    else if (mc >= MC_1_20_6)
        bt = &btree20;
    else if (mc >= MC_1_19_4)
        bt = &btree19;
    else if (mc >= MC_1_19_2)
        bt = &btree192;
    else
        bt = &btree18;

    if (dat)
    {
        int alt = (int) *dat;
        uint64_t ds = get_np_dist(np, bt, alt);
        idx = get_resulting_node(np, bt, 0, alt, ds, 0);
        *dat = (uint64_t) idx;
    }
    else
    {
        idx = get_resulting_node(np, bt, 0, 0, -1, 0);
    }

    return (bt->nodes[idx] >> 48) & 0xFF;
}

//==============================================================================
// Biome Sampling
//==============================================================================

int sampleBiomeNoise(const BiomeNoise *bn, int64_t *np, int x, int y, int z,
    uint64_t *dat, uint32_t sample_flags)
{
    if (bn->nptype >= 0)
    {   // initialized for a specific climate parameter
        if (np)
            memset(np, 0, NP_MAX*sizeof(*np));
        int64_t id = (int64_t) (10000.0 * sampleClimatePara(bn, np, x, z));
        return (int) id;
    }

    float t = 0, h = 0, c = 0, e = 0, d = 0, w = 0;
    double px = x, pz = z;
    if (!(sample_flags & SAMPLE_NO_SHIFT))
    {
        px += sampleDoublePerlin(&bn->climate[NP_SHIFT], x, 0, z) * 4.0;
        pz += sampleDoublePerlin(&bn->climate[NP_SHIFT], z, x, 0) * 4.0;
    }

    c = sampleDoublePerlin(&bn->climate[NP_CONTINENTALNESS], px, 0, pz);
    e = sampleDoublePerlin(&bn->climate[NP_EROSION], px, 0, pz);
    w = sampleDoublePerlin(&bn->climate[NP_WEIRDNESS], px, 0, pz);

    if (!(sample_flags & SAMPLE_NO_DEPTH))
    {
        float np_param[] = {
            c, e, -3.0F * ( fabsf( fabsf(w) - 0.6666667F ) - 0.33333334F ), w,
        };
        double off = getSpline(bn->sp, np_param) + 0.015F;

        d = 1.0 - (y * 4) / 128.0 - 83.0/160.0 + off;
    }

    t = sampleDoublePerlin(&bn->climate[NP_TEMPERATURE], px, 0, pz);
    h = sampleDoublePerlin(&bn->climate[NP_HUMIDITY], px, 0, pz);

    int64_t l_np[6];
    int64_t *p_np = np ? np : l_np;
    p_np[0] = (int64_t)(10000.0F*t);
    p_np[1] = (int64_t)(10000.0F*h);
    p_np[2] = (int64_t)(10000.0F*c);
    p_np[3] = (int64_t)(10000.0F*e);
    p_np[4] = (int64_t)(10000.0F*d);
    p_np[5] = (int64_t)(10000.0F*w);

    int id = none;
    if (!(sample_flags & SAMPLE_NO_BIOME))
        id = climateToBiome(bn->mc, (const uint64_t*)p_np, dat);
    return id;
}

double sampleClimatePara(const BiomeNoise *bn, int64_t *np, double x, double z)
{
    if (bn->nptype == NP_DEPTH)
    {
        float c, e, w;
        c = sampleDoublePerlin(bn->climate + NP_CONTINENTALNESS, x, 0, z);
        e = sampleDoublePerlin(bn->climate + NP_EROSION, x, 0, z);
        w = sampleDoublePerlin(bn->climate + NP_WEIRDNESS, x, 0, z);

        float np_param[] = {
            c, e, -3.0F * ( fabsf( fabsf(w) - 0.6666667F ) - 0.33333334F ), w,
        };
        double off = getSpline(bn->sp, np_param) + 0.015F;
        int y = 0;
        float d = 1.0 - (y * 4) / 128.0 - 83.0/160.0 + off;
        if (np)
        {
            np[2] = (int64_t)(10000.0F*c);
            np[3] = (int64_t)(10000.0F*e);
            np[4] = (int64_t)(10000.0F*d);
            np[5] = (int64_t)(10000.0F*w);
        }
        return d;
    }
    double p = sampleDoublePerlin(bn->climate + bn->nptype, x, 0, z);
    if (np)
        np[bn->nptype] = (int64_t)(10000.0F*p);
    return p;
}

//==============================================================================
// Chunk and Scaled Generation
//==============================================================================

void genBiomeNoiseChunkSection(const BiomeNoise *bn, int out[4][4][4],
    int cx, int cy, int cz, uint64_t *dat)
{
    uint64_t buf = 0;
    int i, j, k;
    int x4 = cx * 4, y4 = cy * 4, z4 = cz * 4;
    if (dat == NULL)
        dat = &buf;
    if (*dat == 0)
    {   // try to determine the ending point of the last chunk section
        sampleBiomeNoise(bn, NULL, x4+3, y4-1, z4+3, dat, 0);
    }

    // iteration order is important
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            for (k = 0; k < 4; ++k) {
                out[i][j][k] = sampleBiomeNoise(bn, NULL, x4+i, y4+j, z4+k, dat, 0);
            }
        }
    }
}

static void genBiomeNoise3D(const BiomeNoise *bn, int *out, Range r, int opt)
{
    uint32_t flags = opt ? SAMPLE_NO_SHIFT : 0;
    int i, j, k;
    int scale = r.scale > 4 ? r.scale / 4 : 1;
    int mid = scale / 2;

    #pragma omp parallel for collapse(2) schedule(dynamic, 4) private(i, j, k)
    for (k = 0; k < r.sy; k++)
    {
        for (j = 0; j < r.sz; j++)
        {
            uint64_t dat = 0;
            uint64_t *p_dat = opt ? &dat : NULL;
            int yk = (r.y+k);
            int zj = (r.z+j)*scale + mid;
            int base = k*r.sx*r.sz + j*r.sx;
            for (i = 0; i < r.sx; i++)
            {
                int xi = (r.x+i)*scale + mid;
                out[base + i] = sampleBiomeNoise(bn, NULL, xi, yk, zj, p_dat, flags);
            }
        }
    }
}

int genBiomeNoiseScaled(const BiomeNoise *bn, int *out, Range r, uint64_t sha)
{
    if (r.sy == 0)
        r.sy = 1;

    uint64_t siz = (uint64_t)r.sx*r.sy*r.sz;
    int i, j, k;

    if (r.scale == 1)
    {
        Range s = getVoronoiSrcRange(r);
        int *src;
        if (siz > 1)
        {   // the source range is large enough that we can try optimizing
            src = out + siz;
            genBiomeNoise3D(bn, src, s, 0);
        }
        else
        {
            src = NULL;
        }

        #pragma omp parallel for collapse(3) schedule(static) private(i, j, k)
        for (k = 0; k < r.sy; k++)
        {
            for (j = 0; j < r.sz; j++)
            {
                for (i = 0; i < r.sx; i++)
                {
                    int x4, z4, y4;
                    voronoiAccess3D(sha, r.x+i, r.y+k, r.z+j, &x4, &y4, &z4);
                    int idx = k*r.sx*r.sz + j*r.sx + i;
                    if (src)
                    {
                        x4 -= s.x; y4 -= s.y; z4 -= s.z;
                        out[idx] = src[(int64_t)y4*s.sx*s.sz + (int64_t)z4*s.sx + x4];
                    }
                    else
                    {
                        out[idx] = sampleBiomeNoise(bn, 0, x4, y4, z4, 0, 0);
                    }
                }
            }
        }
    }
    else
    {
        // There is (was?) an optimization that causes MC-241546, and should
        // not be enabled for accurate results. However, if the scale is higher
        // than 1:4, the accuracy becomes questionable anyway. Furthermore
        // situations that want to use a higher scale are usually better off
        // with a faster, if imperfect, result.
        genBiomeNoise3D(bn, out, r, r.scale > 4);
    }
    return 0;
}

//==============================================================================
// Utility Functions
//==============================================================================

Range getVoronoiSrcRange(Range r)
{
    if (r.scale != 1)
        return r;
    
    Range s = r;
    s.scale = 4;
    s.x = (r.x - 2) >> 2;
    s.z = (r.z - 2) >> 2;
    s.sx = ((r.x + r.sx + 2) >> 2) - s.x + 1;
    s.sz = ((r.z + r.sz + 2) >> 2) - s.z + 1;
    s.y = r.y >> 2;
    s.sy = ((r.y + r.sy) >> 2) - s.y + 1;
    
    return s;
}

//==============================================================================
// Voronoi Functions (for 1:1 scale generation)
//==============================================================================

static inline void getVoronoiCell(uint64_t sha, int a, int b, int c,
        int *x, int *y, int *z)
{
    uint64_t s = sha;
    s = mcStepSeed(s, a);
    s = mcStepSeed(s, b);
    s = mcStepSeed(s, c);
    s = mcStepSeed(s, a);
    s = mcStepSeed(s, b);
    s = mcStepSeed(s, c);

    *x = (((s >> 24) & 1023) - 512) * 36;
    s = mcStepSeed(s, sha);
    *y = (((s >> 24) & 1023) - 512) * 36;
    s = mcStepSeed(s, sha);
    *z = (((s >> 24) & 1023) - 512) * 36;
}

void voronoiAccess3D(uint64_t sha, int x, int y, int z, int *x4, int *y4, int *z4)
{
    x -= 2;
    y -= 2;
    z -= 2;
    int pX = x >> 2;
    int pY = y >> 2;
    int pZ = z >> 2;
    int dx = (x & 3) * 10240;
    int dy = (y & 3) * 10240;
    int dz = (z & 3) * 10240;
    int ax = 0, ay = 0, az = 0;
    uint64_t dmin = (uint64_t)-1;
    int i;

    for (i = 0; i < 8; i++)
    {
        int bx = (i & 4) != 0;
        int by = (i & 2) != 0;
        int bz = (i & 1) != 0;
        int cx = pX + bx;
        int cy = pY + by;
        int cz = pZ + bz;
        int rx, ry, rz;

        getVoronoiCell(sha, cx, cy, cz, &rx, &ry, &rz);

        rx += dx - 40*1024*bx;
        ry += dy - 40*1024*by;
        rz += dz - 40*1024*bz;

        uint64_t d = rx*(uint64_t)rx + ry*(uint64_t)ry + rz*(uint64_t)rz;
        if (d < dmin)
        {
            dmin = d;
            ax = cx;
            ay = cy;
            az = cz;
        }
    }

    if (x4) *x4 = ax;
    if (y4) *y4 = ay;
    if (z4) *z4 = az;
}
