// Cubiomes Rebuild - Generator Core (1.18+ only)
// Rebuilt from original cubiomes, keeping only 1.18+ functionality

#include "generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//==============================================================================
// Utility Functions
//==============================================================================

/**
 * Computes the first 8 bytes of SHA-256 hash of the world seed.
 * Used for voronoi noise initialization in 1.15+.
 */
uint64_t getVoronoiSHA(uint64_t seed)
{
    static const uint32_t K[64] = {
        0x428a2f98,0x71374491, 0xb5c0fbcf,0xe9b5dba5,
        0x3956c25b,0x59f111f1, 0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01, 0x243185be,0x550c7dc3,
        0x72be5d74,0x80deb1fe, 0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786, 0x0fc19dc6,0x240ca1cc,
        0x2de92c6f,0x4a7484aa, 0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d, 0xb00327c8,0xbf597fc7,
        0xc6e00bf3,0xd5a79147, 0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138, 0x4d2c6dfc,0x53380d13,
        0x650a7354,0x766a0abb, 0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b, 0xc24b8b70,0xc76c51a3,
        0xd192e819,0xd6990624, 0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08, 0x2748774c,0x34b0bcb5,
        0x391c0cb3,0x4ed8aa4a, 0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f, 0x84c87814,0x8cc70208,
        0x90befffa,0xa4506ceb, 0xbef9a3f7,0xc67178f2,
    };
    static const uint32_t B[8] = {
        0x6a09e667,0xbb67ae85, 0x3c6ef372,0xa54ff53a,
        0x510e527f,0x9b05688c, 0x1f83d9ab,0x5be0cd19,
    };

    uint32_t m[64];
    uint32_t a0,a1,a2,a3,a4,a5,a6,a7;
    uint32_t i, x, y;
    
    // Prepare message schedule
    m[0] = BSWAP32((uint32_t)(seed));
    m[1] = BSWAP32((uint32_t)(seed >> 32));
    m[2] = 0x80000000;
    for (i = 3; i < 15; i++)
        m[i] = 0;
    m[15] = 0x00000040;

    for (i = 16; i < 64; ++i)
    {
        m[i] = m[i - 7] + m[i - 16];
        x = m[i - 15];
        m[i] += rotr32(x,7) ^ rotr32(x,18) ^ (x >> 3);
        x = m[i - 2];
        m[i] += rotr32(x,17) ^ rotr32(x,19) ^ (x >> 10);
    }

    // Initialize working variables
    a0 = B[0];
    a1 = B[1];
    a2 = B[2];
    a3 = B[3];
    a4 = B[4];
    a5 = B[5];
    a6 = B[6];
    a7 = B[7];

    // Main compression loop
    for (i = 0; i < 64; i++)
    {
        x = a7 + K[i] + m[i];
        x += rotr32(a4,6) ^ rotr32(a4,11) ^ rotr32(a4,25);
        x += (a4 & a5) ^ (~a4 & a6);

        y = rotr32(a0,2) ^ rotr32(a0,13) ^ rotr32(a0,22);
        y += (a0 & a1) ^ (a0 & a2) ^ (a1 & a2);

        a7 = a6;
        a6 = a5;
        a5 = a4;
        a4 = a3 + x;
        a3 = a2;
        a2 = a1;
        a1 = a0;
        a0 = x + y;
    }

    a0 += B[0];
    a1 += B[1];

    return BSWAP32(a0) | ((uint64_t)BSWAP32(a1) << 32);
}


//==============================================================================
// Generator Core Functions
//==============================================================================

void setupGenerator(Generator *g, int mc, uint32_t flags)
{
    // Version validation - only support 1.18+
    if (mc < MC_1_18)
    {
        fprintf(stderr, "ERROR: Only MC 1.18+ supported (got version %d)\n", mc);
        exit(1);
    }
    
    // Initialize generator structure
    memset(g, 0, sizeof(Generator));
    g->mc = mc;
    g->dim = DIM_UNDEF;
    g->flags = flags;
    g->seed = 0;
    g->sha = 0;
    
    // Initialize biome noise system for overworld
    initBiomeNoise(&g->bn, mc);
}

void applySeed(Generator *g, int dim, uint64_t seed)
{
    g->dim = dim;
    g->seed = seed;
    
    // Apply seed to appropriate dimension
    if (dim == DIM_OVERWORLD)
    {
        int large = (g->flags & LARGE_BIOMES) != 0;
        setBiomeSeed(&g->bn, seed, large);
    }
    else if (dim == DIM_NETHER)
    {
        setNetherSeed(&g->nn, seed);
    }
    else if (dim == DIM_END)
    {
        setEndSeed(&g->en, g->mc, seed);
    }
    
    // Set voronoi hash for 1:1 scale generation
    g->sha = getVoronoiSHA(seed);
}

size_t getMinCacheSize(const Generator *g, int scale, int sx, int sy, int sz)
{
    if (sy == 0)
        sy = 1;
    
    size_t size = (size_t)sx * sy * sz;
    
    // For 1:1 scale, we need extra space for voronoi source
    if (scale == 1)
    {
        Range r = {scale, 0, 0, sx, sz, 0, sy};
        Range s = getVoronoiSrcRange(r);
        size += (size_t)s.sx * s.sy * s.sz;
    }
    
    return size;
}

int *allocCache(const Generator *g, Range r)
{
    size_t size = getMinCacheSize(g, r.scale, r.sx, r.sy, r.sz);
    int *cache = (int*) calloc(size, sizeof(int));
    if (!cache)
    {
        fprintf(stderr, "allocCache(): allocation failed (%lu ints)\n", (unsigned long)size);
    }
    return cache;
}

int genBiomes(const Generator *g, int *cache, Range r)
{
    if (!cache)
        return -1;
    
    if (g->dim == DIM_OVERWORLD)
    {
        return genBiomeNoiseScaled(&g->bn, cache, r, g->sha);
    }
    else if (g->dim == DIM_NETHER)
    {
        return genNetherScaled(&g->nn, cache, r, g->mc, g->sha);
    }
    else if (g->dim == DIM_END)
    {
        return genEndScaled(&g->en, cache, r, g->mc, g->sha);
    }
    
    return -1;
}

int getBiomeAt(const Generator *g, int scale, int x, int y, int z)
{
    if (scale != 1 && scale != 4)
    {
        fprintf(stderr, "getBiomeAt(): invalid scale %d (must be 1 or 4)\n", scale);
        return none;
    }
    
    Range r = {scale, x, z, 1, 1, y, 1};
    // For scale==1, genBiomes writes past &biome via voronoi src buffer.
    // Allocate enough space to avoid stack corruption.
    int buf[16];
    
    int err = genBiomes(g, buf, r);
    if (err)
        return none;
    
    return buf[0];
}
