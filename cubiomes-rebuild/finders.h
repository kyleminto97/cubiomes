// Cubiomes Rebuild - Structure Finders (1.18+ only)
// Rebuilt from original cubiomes, keeping only 1.18+ functionality

#ifndef FINDERS_REBUILD_H_
#define FINDERS_REBUILD_H_

#include "generator.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MASK48 (((int64_t)1 << 48) - 1)

//==============================================================================
// Structure Types (1.18+ only)
//==============================================================================

enum StructureType
{
    Desert_Pyramid,
    Jungle_Temple,
    Swamp_Hut,
    Igloo,
    Village,
    Ocean_Ruin,
    Shipwreck,
    Monument,
    Mansion,
    Outpost,
    Ruined_Portal,
    Ruined_Portal_N,
    Ancient_City,
    Treasure,
    Mineshaft,
    Desert_Well,
    Geode,
    Fortress,
    Bastion,
    End_City,
    End_Gateway,
    End_Island,
    Trail_Ruins,
    Trial_Chambers,
    STRUCTURE_NUM
};

//==============================================================================
// Structure Configuration
//==============================================================================

typedef struct
{
    int32_t salt;
    int8_t  regionSize;
    int8_t  chunkRange;
    uint8_t structType;
    int8_t  dim;
    float   rarity;
} StructureConfig;

//==============================================================================
// Position Types
//==============================================================================

typedef struct { int x, z; } Pos;
typedef struct { int x, y, z; } Pos3;

//==============================================================================
// Stronghold Iterator
//==============================================================================

typedef struct
{
    Pos pos;        // accurate location of current stronghold
    Pos nextapprox; // approximate location (+/-112 blocks) of next stronghold
    int index;      // stronghold index counter
    int ringnum;    // ring number for index
    int ringmax;    // max index within ring
    int ringidx;    // index within ring
    double angle;   // next angle within ring
    double dist;    // next distance from origin (in chunks)
    uint64_t rnds;  // random number seed (48 bit)
    int mc;         // minecraft version
} StrongholdIter;

//==============================================================================
// Structure Variant Info
//==============================================================================

typedef struct
{
    uint8_t abandoned   :1; // is zombie village
    uint8_t giant       :1; // giant portal variant
    uint8_t underground :1; // underground portal
    uint8_t airpocket   :1; // portal with air pocket
    uint8_t basement    :1; // igloo with basement
    uint8_t cracked     :1; // geode with crack
    uint8_t size;           // geode size | igloo middle pieces
    uint8_t start;          // starting piece index
    short   biome;          // biome variant
    uint8_t rotation;       // 0:0, 1:cw90, 2:cw180, 3:cw270=ccw90
    uint8_t mirror;
    int16_t x, y, z;
    int16_t sx, sy, sz;
} StructureVariant;

//==============================================================================
// End Island
//==============================================================================

typedef struct
{
    int x, y, z;
    int r;
} EndIsland;

//==============================================================================
// Core Functions
//==============================================================================

/* Get structure configuration for a given version (1.18+ only) */
int getStructureConfig(int structureType, int mc, StructureConfig *sconf);

/* Find structure position in a region */
int getStructurePos(int structureType, int mc, uint64_t seed, int regX, int regZ, Pos *pos);

/* Get feature position (uniform distribution) */
static inline Pos getFeaturePos(StructureConfig config, uint64_t seed, int regX, int regZ);
static inline Pos getFeatureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ);

/* Get large structure position (triangular distribution) */
static inline Pos getLargeStructurePos(StructureConfig config, uint64_t seed, int regX, int regZ);
static inline Pos getLargeStructureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ);

/* Check if structure can generate at position */
int isViableStructurePos(int structType, Generator *g, int blockX, int blockZ, uint32_t flags);
int isViableFeatureBiome(int mc, int structureType, int biomeID);

/* Mineshaft positions */
int getMineshafts(int mc, uint64_t seed, int chunkX, int chunkZ,
        int chunkW, int chunkH, Pos *out, int nout);

/* Slime chunks */
static inline int isSlimeChunk(uint64_t seed, int chunkX, int chunkZ)
{
    uint64_t rnd = seed;
    rnd += (int)(chunkX * 0x5ac0db);
    rnd += (int)(chunkX * chunkX * 0x4c1906);
    rnd += (int)(chunkZ * 0x5f24f);
    rnd += (int)(chunkZ * chunkZ) * 0x4307a7ULL;
    rnd ^= 0x3ad8025fULL;
    setSeed(&rnd, rnd);
    return nextInt(&rnd, 10) == 0;
}

/* End islands */
int getEndIslands(EndIsland islands[2], int mc, uint64_t seed, int chunkX, int chunkZ);

/* Strongholds */
Pos initFirstStronghold(StrongholdIter *sh, int mc, uint64_t s48);
int nextStronghold(StrongholdIter *sh, const Generator *g);

/* Spawn */
Pos estimateSpawn(const Generator *g, uint64_t *rng);
Pos getSpawn(const Generator *g);

/* Structure variant info */
int getVariant(StructureVariant *sv, int structType, int mc, uint64_t seed,
        int blockX, int blockZ, int biomeID);

//==============================================================================
// Inline Implementations
//==============================================================================

static inline Pos getFeatureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos;
    const uint64_t K = 0x5deece66dULL;
    const uint64_t M = (1ULL << 48) - 1;
    const uint64_t b = 0xb;

    seed = seed + regX*341873128712ULL + regZ*132897987541ULL + config.salt;
    seed = (seed ^ K);
    seed = (seed * K + b) & M;

    uint64_t r = config.chunkRange;
    if (r & (r-1))
    {
        pos.x = (int)(seed >> 17) % r;
        seed = (seed * K + b) & M;
        pos.z = (int)(seed >> 17) % r;
    }
    else
    {
        pos.x = (int)((r * (seed >> 17)) >> 31);
        seed = (seed * K + b) & M;
        pos.z = (int)((r * (seed >> 17)) >> 31);
    }

    return pos;
}

static inline Pos getFeaturePos(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos = getFeatureChunkInRegion(config, seed, regX, regZ);
    pos.x = (int)(((uint64_t)regX*config.regionSize + pos.x) << 4);
    pos.z = (int)(((uint64_t)regZ*config.regionSize + pos.z) << 4);
    return pos;
}

static inline Pos getLargeStructureChunkInRegion(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos;
    const uint64_t K = 0x5deece66dULL;
    const uint64_t M = (1ULL << 48) - 1;
    const uint64_t b = 0xb;

    seed = seed + regX*341873128712ULL + regZ*132897987541ULL + config.salt;
    seed = (seed ^ K);

    seed = (seed * K + b) & M;
    pos.x = (int)(seed >> 17) % config.chunkRange;
    seed = (seed * K + b) & M;
    pos.x += (int)(seed >> 17) % config.chunkRange;

    seed = (seed * K + b) & M;
    pos.z = (int)(seed >> 17) % config.chunkRange;
    seed = (seed * K + b) & M;
    pos.z += (int)(seed >> 17) % config.chunkRange;

    pos.x >>= 1;
    pos.z >>= 1;

    return pos;
}

static inline Pos getLargeStructurePos(StructureConfig config, uint64_t seed, int regX, int regZ)
{
    Pos pos = getLargeStructureChunkInRegion(config, seed, regX, regZ);
    pos.x = (int)(((uint64_t)regX*config.regionSize + pos.x) << 4);
    pos.z = (int)(((uint64_t)regZ*config.regionSize + pos.z) << 4);
    return pos;
}

#ifdef __cplusplus
}
#endif

#endif // FINDERS_REBUILD_H_
