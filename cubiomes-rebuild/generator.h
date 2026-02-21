#ifndef GENERATOR_REBUILD_H_
#define GENERATOR_REBUILD_H_

#include "biomenoise.h"

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Generator Flags
//==============================================================================

enum
{
    LARGE_BIOMES = 0x1,
};

//==============================================================================
// Generator Structure (1.18+ only)
//==============================================================================

typedef struct Generator
{
    int mc;              // MC version (MC_1_18 through MC_1_21+)
    int dim;             // Dimension (DIM_OVERWORLD, DIM_NETHER, DIM_END)
    uint32_t flags;      // Generator flags (LARGE_BIOMES)
    uint64_t seed;       // World seed
    uint64_t sha;        // Voronoi SHA-256 hash
    
    BiomeNoise bn;       // Overworld biome generation
    NetherNoise nn;      // Nether biome generation
    EndNoise en;         // End biome generation
} Generator;

//==============================================================================
// Core API
//==============================================================================

/**
 * Sets up a biome generator for a given MC version.
 * Only supports MC 1.18+.
 * 
 * @param g     Generator structure to initialize
 * @param mc    Minecraft version (MC_1_18 through MC_1_21+)
 * @param flags Generator flags (LARGE_BIOMES)
 */
void setupGenerator(Generator *g, int mc, uint32_t flags);

/**
 * Initializes the generator for a given dimension and seed.
 * 
 * @param g     Generator structure
 * @param dim   Dimension (DIM_OVERWORLD=0, DIM_NETHER=-1, DIM_END=1)
 * @param seed  World seed
 */
void applySeed(Generator *g, int dim, uint64_t seed);

/**
 * Calculates the minimum cache size required to generate a cuboidal volume.
 * 
 * @param g     Generator structure
 * @param scale Scale factor (1, 4, 16, 64, or 256)
 * @param sx    Width
 * @param sy    Height (0 for 2D plane)
 * @param sz    Depth
 * @return      Required cache size in number of ints
 */
size_t getMinCacheSize(const Generator *g, int scale, int sx, int sy, int sz);

/**
 * Allocates a cache buffer for the given range.
 * 
 * @param g     Generator structure
 * @param r     Range to generate
 * @return      Allocated int buffer (must be freed by caller)
 */
int *allocCache(const Generator *g, Range r);

/**
 * Generates biomes for a cuboidal scaled range.
 * 
 * @param g     Generator structure
 * @param cache Output buffer (must be large enough for range)
 * @param r     Range to generate
 * @return      0 on success, non-zero on error
 */
int genBiomes(const Generator *g, int *cache, Range r);

/**
 * Gets the biome at a specific scaled position.
 * 
 * @param g     Generator structure
 * @param scale Scale factor (1 for block coords, 4 for biome coords)
 * @param x     X coordinate
 * @param y     Y coordinate
 * @param z     Z coordinate
 * @return      Biome ID, or none (-1) on failure
 */
int getBiomeAt(const Generator *g, int scale, int x, int y, int z);

#ifdef __cplusplus
}
#endif

#endif /* GENERATOR_REBUILD_H_ */
