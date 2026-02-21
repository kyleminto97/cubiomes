# Cubiomes Rebuild - API Reference

Quick reference for all public functions.

## Generator (generator.h)

### Setup

```cpp
int setupGenerator(Generator *g, int mc, uint32_t flags);
```
Initialize generator for a Minecraft version. Returns 0 on success.
- `mc`: Version (MC_1_18, MC_1_21, etc.)
- `flags`: Optional flags (LARGE_BIOMES = 0x1)

```cpp
void applySeed(Generator *g, int dim, uint64_t seed);
```
Apply world seed to generator for a dimension.
- `dim`: DIM_OVERWORLD, DIM_NETHER, or DIM_END
- `seed`: 64-bit world seed

### Biome Queries

```cpp
int getBiomeAt(const Generator *g, int scale, int x, int y, int z);
```
Get biome at coordinates. Returns biome ID.
- `scale`: Resolution (1, 4, 64)
- `x, y, z`: Coordinates in quarter-blocks (divide by 4)

```cpp
int genBiomes(const Generator *g, int *cache, Range r);
```
Generate biomes for an area. Returns 0 on success.
- `cache`: Output buffer (allocate with allocCache)
- `r`: Range structure defining area

### Memory

```cpp
size_t getMinCacheSize(const Generator *g, int scale, int sx, int sy, int sz);
```
Calculate required cache size for area.

```cpp
int *allocCache(const Generator *g, Range r);
```
Allocate cache for range. Must free() after use.

### Range Structure

```cpp
typedef struct {
    int scale;      // Resolution (1, 4, 64)
    int x, z;       // Start position
    int sx, sz;     // Size (width, height)
    int y, sy;      // Y position and height
} Range;
```

---

## Structures (finders.h)

### Structure Positions

```cpp
int getStructurePos(int structureType, int mc, uint64_t seed, 
                    int regX, int regZ, Pos *pos);
```
Get structure position in region. Returns 1 if valid.
- `structureType`: Desert_Pyramid, Village, etc.
- `regX, regZ`: Region coordinates
- `pos`: Output position

```cpp
int isViableStructurePos(int structType, Generator *g, 
                         int blockX, int blockZ, uint32_t flags);
```
Check if structure can spawn at position. Returns 1 if viable.

```cpp
int isViableFeatureBiome(int mc, int structureType, int biomeID);
```
Check if biome supports structure type. Returns 1 if valid.

### Structure Types

```cpp
enum StructureType {
    Desert_Pyramid, Jungle_Temple, Swamp_Hut, Igloo,
    Village, Ocean_Ruin, Shipwreck, Monument, Mansion,
    Outpost, Ruined_Portal, Ancient_City, Treasure,
    Mineshaft, Desert_Well, Geode,
    Fortress, Bastion,              // Nether
    End_City, End_Gateway, End_Island,  // End
    Trail_Ruins, Trial_Chambers     // 1.20+, 1.21+
};
```

### Strongholds

```cpp
Pos initFirstStronghold(StrongholdIter *sh, int mc, uint64_t s48);
```
Initialize stronghold iterator. Returns approximate first position.

```cpp
int nextStronghold(StrongholdIter *sh, const Generator *g);
```
Find next stronghold. Returns remaining count.

### Other

```cpp
int getMineshafts(int mc, uint64_t seed, int chunkX, int chunkZ,
                  int chunkW, int chunkH, Pos *out, int nout);
```
Find mineshaft chunks in area. Returns count.

```cpp
int isSlimeChunk(uint64_t seed, int chunkX, int chunkZ);
```
Check if chunk is a slime chunk. Returns 1 if true.

```cpp
int getEndIslands(EndIsland islands[2], int mc, uint64_t seed, 
                  int chunkX, int chunkZ);
```
Find small end islands in chunk. Returns count.

---

## Biomes (biomes.h)

### Biome IDs

All 1.18+ biomes are defined as enum values:
```cpp
ocean, plains, desert, mountains, forest, taiga, swamp, river,
frozen_ocean, snowy_tundra, mushroom_fields, beach, jungle,
deep_ocean, birch_forest, dark_forest, snowy_taiga, savanna,
badlands, warm_ocean, lukewarm_ocean, cold_ocean,
sunflower_plains, flower_forest, ice_spikes, bamboo_jungle,
soul_sand_valley, crimson_forest, warped_forest, basalt_deltas,
dripstone_caves, lush_caves, meadow, grove, snowy_slopes,
jagged_peaks, frozen_peaks, stony_peaks, deep_dark,
mangrove_swamp, cherry_grove, pale_garden
// ... and more
```

### Biome Checks

```cpp
int biomeExists(int mc, int id);
```
Check if biome exists in version. Returns 1 if exists.

```cpp
int isOverworld(int mc, int id);
```
Check if biome is overworld. Returns 1 if true.

```cpp
int getDimension(int id);
```
Get dimension for biome. Returns DIM_OVERWORLD/NETHER/END.

```cpp
int isOceanic(int id);
int isShallowOcean(int id);
int isDeepOcean(int id);
int isSnowy(int id);
int isMesa(int id);
```
Check biome categories. Return 1 if true.

```cpp
int areSimilar(int mc, int id1, int id2);
```
Check if two biomes are similar. Returns 1 if similar.

---

## Utilities (util.h)

### Version Conversion

```cpp
const char* mc2str(int mc);
```
Convert version enum to string. Returns "1.18", "1.21", etc.

```cpp
int str2mc(const char *s);
```
Convert string to version enum. Returns MC_1_18, etc.

### Biome Names

```cpp
const char *biome2str(int mc, int id);
```
Get biome resource name. Returns "plains", "ocean", etc.

### Structure Names

```cpp
const char *struct2str(int stype);
```
Get structure resource name. Returns "village", "monument", etc.

### Colors

```cpp
void initBiomeColors(unsigned char biomeColors[256][3]);
```
Initialize color palette. Colors are RGB triplets.

```cpp
const unsigned char* getBiomeColor(int id);
```
Get RGB color for biome. Returns pointer to [R, G, B].

---

## Data Types

### Pos
```cpp
typedef struct { int x, z; } Pos;
```
2D position (block coordinates).

### Pos3
```cpp
typedef struct { int x, y, z; } Pos3;
```
3D position (block coordinates).

### Generator
```cpp
typedef struct {
    int mc;         // Minecraft version
    int dim;        // Dimension
    uint32_t flags; // Generation flags
    uint64_t seed;  // World seed
    BiomeNoise bn;  // Biome noise data
    // ... internal fields
} Generator;
```
Main generator structure. Initialize with setupGenerator().

### StrongholdIter
```cpp
typedef struct {
    Pos pos;        // Current stronghold position
    Pos nextapprox; // Next approximate position
    int index;      // Stronghold index (0-127)
    int ringnum;    // Ring number
    // ... internal fields
} StrongholdIter;
```
Stronghold iterator. Initialize with initFirstStronghold().

---

## Constants

### Versions
```cpp
MC_1_18, MC_1_19_2, MC_1_19_4, MC_1_20_6, 
MC_1_21_1, MC_1_21_3, MC_1_21_WD
```

### Dimensions
```cpp
DIM_OVERWORLD = 0
DIM_NETHER = -1
DIM_END = 1
```

### Flags
```cpp
LARGE_BIOMES = 0x1  // Use for large biomes worlds
```

---

## Common Patterns

### Initialize and Query
```cpp
Generator g;
setupGenerator(&g, MC_1_18, 0);
applySeed(&g, DIM_OVERWORLD, seed);
int biome = getBiomeAt(&g, 1, x >> 2, y >> 2, z >> 2);
```

### Generate Area
```cpp
Range r = {4, x, z, width, height, y, 1};
int *cache = allocCache(&g, r);
genBiomes(&g, cache, r);
// Use cache[z * width + x]
free(cache);
```

### Find Structure
```cpp
Pos pos;
if (getStructurePos(Village, MC_1_18, seed, regX, regZ, &pos)) {
    if (isViableStructurePos(Village, &g, pos.x, pos.z, 0)) {
        // Village found and viable
    }
}
```

### Iterate Strongholds
```cpp
StrongholdIter sh;
initFirstStronghold(&sh, MC_1_18, seed);
while (nextStronghold(&sh, &g) > 0) {
    // Process sh.pos
}
```
