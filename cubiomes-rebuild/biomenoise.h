#ifndef BIOMENOISE_REBUILD_H_
#define BIOMENOISE_REBUILD_H_

#include "noise.h"
#include "biomes.h"

//==============================================================================
// Range Structure
//==============================================================================

STRUCT(Range)
{
    // Defines an area or volume for biome generation
    // @scale:  Horizontal scale factor (1, 4, 16, 64, or 256)
    // @x,z:    Horizontal position (north-west corner)
    // @sx,sz:  Horizontal size (width and height)
    // @y:      Vertical position (1:1 if scale==1, else 1:4)
    // @sy:     Vertical size (<=0 treated as 1)
    //
    // Output indexed as: out[i_y*sx*sz + i_z*sx + i_x]
    
    int scale;
    int x, z, sx, sz;
    int y, sy;
};

//==============================================================================
// Nether Biome Generator (1.16+)
//==============================================================================

STRUCT(NetherNoise)
{
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    PerlinNoise oct[8]; // buffer for octaves in double perlin noise
};

//==============================================================================
// End Biome Generator (1.9+)
//==============================================================================

STRUCT(EndNoise)
{
    PerlinNoise perlin;
    int mc;
};

//==============================================================================
// Spline System (1.18+)
//==============================================================================

STRUCT(Spline)
{
    int len, typ;
    float loc[12];
    float der[12];
    Spline *val[12];
};

STRUCT(FixSpline)
{
    int len;
    float val;
};

STRUCT(SplineStack)
{
    Spline stack[42];
    FixSpline fstack[151];
    int len, flen;
};

//==============================================================================
// Overworld Biome Generator (1.18+)
//==============================================================================

enum
{
    NP_TEMPERATURE      = 0,
    NP_HUMIDITY         = 1,
    NP_CONTINENTALNESS  = 2,
    NP_EROSION          = 3,
    NP_SHIFT            = 4,
    NP_DEPTH            = NP_SHIFT, // not a real climate parameter
    NP_WEIRDNESS        = 5,
    NP_MAX
};

STRUCT(BiomeNoise)
{
    DoublePerlinNoise climate[NP_MAX];
    PerlinNoise oct[2*23]; // buffer for octaves in double perlin noise
    Spline *sp;
    SplineStack ss;
    int nptype;
    int mc;
};

//==============================================================================
// Biome Tree
//==============================================================================

STRUCT(BiomeTree)
{
    const uint32_t *steps;
    const int32_t  *param;
    const uint64_t *nodes;
    uint32_t order;
    uint32_t len;
};

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Nether Biome Generation (1.16+)
//==============================================================================

void setNetherSeed(NetherNoise *nn, uint64_t seed);
int getNetherBiome(const NetherNoise *nn, int x, int y, int z, float *ndel);
int mapNether2D(const NetherNoise *nn, int *out, int x, int z, int w, int h);
int mapNether3D(const NetherNoise *nn, int *out, Range r, float confidence);
int genNetherScaled(const NetherNoise *nn, int *out, Range r, int mc, uint64_t sha);

//==============================================================================
// End Biome Generation (1.9+)
//==============================================================================

void setEndSeed(EndNoise *en, int mc, uint64_t seed);
int mapEndBiome(const EndNoise *en, int *out, int x, int z, int w, int h);
int mapEnd(const EndNoise *en, int *out, int x, int z, int w, int h);
float getEndHeightNoise(const EndNoise *en, int x, int z, int range);
int genEndScaled(const EndNoise *en, int *out, Range r, int mc, uint64_t sha);

//==============================================================================
// Overworld Biome Generation (1.18+)
//==============================================================================

enum {
    SAMPLE_NO_SHIFT = 0x1,  // skip local distortions
    SAMPLE_NO_DEPTH = 0x2,  // skip depth sampling for vertical biomes
    SAMPLE_NO_BIOME = 0x4,  // do not apply climate noise to biome mapping
};

void initBiomeNoise(BiomeNoise *bn, int mc);
void setBiomeSeed(BiomeNoise *bn, uint64_t seed, int large);
int sampleBiomeNoise(const BiomeNoise *bn, int64_t *np, int x, int y, int z,
    uint64_t *dat, uint32_t sample_flags);
int climateToBiome(int mc, const uint64_t np[6], uint64_t *dat);
void genBiomeNoiseChunkSection(const BiomeNoise *bn, int out[4][4][4],
    int cx, int cy, int cz, uint64_t *dat);
int genBiomeNoiseScaled(const BiomeNoise *bn, int *out, Range r, uint64_t sha);

//==============================================================================
// Utility Functions
//==============================================================================

void setClimateParaSeed(BiomeNoise *bn, uint64_t seed, int large, int nptype, int nmax);
double sampleClimatePara(const BiomeNoise *bn, int64_t *np, double x, double z);
Range getVoronoiSrcRange(Range r);

#ifdef __cplusplus
}
#endif

#endif /* BIOMENOISE_REBUILD_H_ */
