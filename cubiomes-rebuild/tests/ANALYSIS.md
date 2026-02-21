# Phase 0: Analysis - What's Needed for 1.21+

## Core Components Required

### 1. Noise Generation (noise.c/h)
**Status**: NEEDED - Used by 1.18+ biome generation

**Functions to Keep**:
- `perlinInit()` - Initialize Perlin noise
- `xPerlinInit()` - Xoroshiro-based init (1.18+)
- `samplePerlin()` - Sample Perlin noise
- `sampleSimplex2D()` - 2D simplex noise
- `octaveInit()` - Initialize octave noise
- `xOctaveInit()` - Xoroshiro-based octave init (1.18+)
- `sampleOctave()` - Sample octave noise
- `sampleOctaveAmp()` - Sample with amplitude
- `doublePerlinInit()` - Initialize double Perlin
- `xDoublePerlinInit()` - Xoroshiro-based double Perlin (1.18+)
- `sampleDoublePerlin()` - Sample double Perlin

**Functions to Skip**:
- `octaveInitBeta()` - Beta-specific
- `sampleOctaveBeta17Biome()` - Beta-specific
- `sampleOctaveBeta17Terrain()` - Beta-specific

**Structures Needed**:
- `PerlinNoise`
- `OctaveNoise`
- `DoublePerlinNoise`

---

### 2. RNG System (rng.h)
**Status**: NEEDED - Used for seeds and structure positioning

**Functions to Keep**:
- Java RNG: `setSeed()`, `next()`, `nextInt()`, `nextLong()`, `nextFloat()`, `nextDouble()`, `skipNextN()`
- Xoroshiro: `xSetSeed()`, `xNextLong()`, `xNextInt()`, `xNextDouble()`, `xNextFloat()`, `xNextLongJ()`, `xNextIntJ()`
- Minecraft helpers: `mcStepSeed()`, `mcFirstInt()`, `mcFirstIsZero()`, `getChunkSeed()`
- Layer helpers: `getLayerSalt()`, `getStartSalt()`, `getStartSeed()` (may not be needed if no layers)
- Math helpers: `lerp()`, `lerp2()`, `lerp3()`, `clampedLerp()`

**Structures Needed**:
- `Xoroshiro` (128++ RNG)

---

### 3. Biome Definitions (biomes.h/c)
**Status**: NEEDED - Core biome IDs and helpers

**Version Enum** (1.21+ only):
```c
enum MCVersion {
    MC_UNDEF,
    MC_1_21_1,
    MC_1_21_3,
    MC_1_21_WD,
    MC_1_21 = MC_1_21_WD,
    MC_NEWEST = MC_1_21,
};
```

**Biomes to Keep** (1.18+ only):
- Base: ocean, plains, desert, mountains, forest, taiga, swamp, river, beach, etc.
- Mountains: meadow, grove, snowy_slopes, jagged_peaks, frozen_peaks, stony_peaks
- Variants: sunflower_plains, flower_forest, ice_spikes, bamboo_jungle, etc.
- Caves: dripstone_caves, lush_caves, deep_dark
- Nether: nether_wastes, soul_sand_valley, crimson_forest, warped_forest, basalt_deltas
- End: the_end, small_end_islands, end_midlands, end_highlands, end_barrens
- 1.19+: mangrove_swamp, deep_dark
- 1.20+: cherry_grove
- 1.21+: pale_garden

**Biomes to Remove**:
- Beta: seasonal_forest, rainforest, shrubland
- Legacy variants: mountain_edge, desert_hills, wooded_hills, taiga_hills, etc.

**Functions to Keep**:
- `biomeExists()` - Check if biome exists in version
- `isOverworld()` - Check if overworld biome
- `getDimension()` - Get biome dimension
- `isOceanic()` - Check if ocean biome
- `isSnowy()` - Check if snowy biome
- `areSimilar()` - Check biome similarity (simplified)

---

### 4. BiomeNoise System (biomenoise.h/c)
**Status**: NEEDED - Core 1.18+ generation

**Structures to Keep**:
- `Range` - Area/volume specification
- `BiomeNoise` - Overworld 1.18+ generation
- `NetherNoise` - Nether 1.16+ generation
- `EndNoise` - End 1.9+ generation
- `Spline` - Spline interpolation
- `FixSpline` - Fixed spline
- `SplineStack` - Spline management

**Structures to Remove**:
- `BiomeNoiseBeta` - Beta generation
- `SurfaceNoiseBeta` - Beta surface
- `SeaLevelColumnNoiseBeta` - Beta sea level

**Functions to Keep**:
- `initBiomeNoise()` - Initialize 1.18+ biome noise
- `setBiomeSeed()` - Set seed for biome generation
- `sampleBiomeNoise()` - Sample biome at position
- `genBiomeNoiseScaled()` - Generate area of biomes
- `setNetherSeed()` - Set nether seed
- `getNetherBiome()` - Get nether biome
- `setEndSeed()` - Set end seed
- `getEndBiome()` - Get end biome
- Spline functions

**Functions to Remove**:
- All Beta-related functions
- `initSurfaceNoiseBeta()`
- `sampleSurfaceNoiseBeta()`

**Tables to Keep**:
- `tables/btree18.h` - 1.18 biome tree
- `tables/btree19.h` - 1.19 biome tree
- `tables/btree192.h` - 1.19.2 biome tree
- `tables/btree20.h` - 1.20 biome tree
- `tables/btree21wd.h` - 1.21 biome tree

---

### 5. Generator Core (generator.h/c)
**Status**: NEEDED - Main API

**Simplified Generator Structure**:
```c
typedef struct Generator {
    int mc;              // MC_1_21_1, MC_1_21_3, MC_1_21_WD
    int dim;             // DIM_OVERWORLD, DIM_NETHER, DIM_END
    uint32_t flags;      // LARGE_BIOMES
    uint64_t seed;
    uint64_t sha;        // Voronoi SHA-256 hash
    BiomeNoise bn;       // Overworld
    NetherNoise nn;      // Nether
    EndNoise en;         // End
} Generator;
```

**Functions to Keep**:
- `setupGenerator()` - Initialize generator (simplified)
- `applySeed()` - Apply seed (simplified)
- `genBiomes()` - Generate biomes (noise-only)
- `getBiomeAt()` - Get single biome (noise-only)
- `getMinCacheSize()` - Calculate cache size (simplified)
- `allocCache()` - Allocate cache

**Functions to Remove**:
- `setupLayerStack()` - Layer system
- `setLayerSeed()` - Layer system
- `getLayerForScale()` - Layer system
- `genArea()` - Layer-specific
- All layer-related code

---

### 6. Structure Finders (finders.h/c)
**Status**: NEEDED - Structure positioning

**Structures to Keep**:
- `StructureConfig` - Structure configuration
- `Pos`, `Pos3` - Position structures
- `StrongholdIter` - Stronghold iteration
- `StructureVariant` - Structure variants
- `BiomeFilter` - Biome filtering

**Structure Configs** (1.18+ only):
- Desert Pyramid, Jungle Temple, Swamp Hut, Igloo
- Village, Pillager Outpost
- Ocean Monument, Woodland Mansion
- Ocean Ruin, Shipwreck
- Ruined Portal (Overworld & Nether)
- Ancient City (1.19+)
- Trail Ruins (1.20+)
- Trial Chambers (1.21+)
- Buried Treasure, Mineshaft, Desert Well, Geode
- Nether Fortress, Bastion
- End City, End Gateway, End Island

**Functions to Keep**:
- `getStructureConfig()` - Get config (simplified)
- `getStructurePos()` - Get position (simplified)
- `isViableStructurePos()` - Check biome validity (simplified)
- `getPopulationSeed()` - Get population seed (1.18+ only)
- Structure-specific helpers

**Functions to Remove**:
- Pre-1.18 structure configs
- Legacy version checks

---

### 7. Utilities (util.h/c)
**Status**: NEEDED - Helper functions

**Functions to Keep**:
- `mc2str()` - Version to string (1.21+ only)
- `str2mc()` - String to version (1.21+ only)
- `biome2str()` - Biome to string (1.18+ biomes)
- `struct2str()` - Structure to string
- `initBiomeColors()` - Initialize biome colors
- `initBiomeTypeColors()` - Initialize type colors
- `parseBiomeColors()` - Parse color mappings
- `biomesToImage()` - Convert biomes to image
- `savePPM()` - Save PPM image
- `loadSavedSeeds()` - Load seeds from file

---

### 8. Quad-Base System (quadbase.h/c)
**Status**: NEEDED - Quad-structure finding

**Functions to Keep**:
- `isQuadBase()` - Check if seed is quad-base
- `isQuadBaseFeature24()` - Optimized for 32/24 structures
- `isQuadBaseFeature24Classic()` - Classic constellations
- `isQuadBaseFeature()` - General feature structures
- `isQuadBaseLarge()` - Large structures (monuments)
- `searchAll48()` - Search all 48-bit seeds
- `getOptimalAfk()` - Find optimal AFK position
- `scanForQuads()` - Scan area for quads

**Constants to Keep**:
- `low20QuadIdeal` - Ideal constellations
- `low20QuadClassic` - Classic constellations
- `low20QuadHutNormal` - Normal quad-huts
- `low20QuadHutBarely` - Barely-valid quad-huts

---

## Components NOT Needed

### 1. Layer System (layers.h/c)
**Status**: DELETE ENTIRELY
- ~2000 lines of legacy code
- Only used for MC Beta 1.8 - 1.17
- Completely replaced by BiomeNoise in 1.18+

### 2. Beta Biome Generation
**Status**: REMOVE
- BiomeNoiseBeta structure
- SurfaceNoiseBeta structure
- All Beta-specific functions

---

## Dependency Graph

```
Generator
├── BiomeNoise (Overworld 1.18+)
│   ├── Noise (Perlin, Octave, DoublePerlin)
│   │   └── RNG (Xoroshiro, Java)
│   ├── Spline system
│   └── Biome trees (btree*.h)
├── NetherNoise (Nether 1.16+)
│   ├── DoublePerlin
│   └── RNG
└── EndNoise (End 1.9+)
    ├── Perlin
    └── RNG

Structure Finders
├── StructureConfig (1.18+ only)
├── RNG (Java + Xoroshiro)
└── Generator (for biome validation)

Quad-Base Finder
├── Structure Finders
└── RNG
```

---

## Size Estimates

| Component | Original Size | Rebuilt Size | Reduction |
|-----------|--------------|--------------|-----------|
| layers.c/h | ~2200 lines | 0 lines | 100% |
| generator.c | ~800 lines | ~300 lines | 62% |
| biomes.c | ~500 lines | ~200 lines | 60% |
| biomenoise.c | ~1200 lines | ~800 lines | 33% |
| finders.c | ~5500 lines | ~3000 lines | 45% |
| noise.c | ~600 lines | ~500 lines | 17% |
| util.c | ~400 lines | ~300 lines | 25% |
| quadbase.c | ~300 lines | ~280 lines | 7% |
| **TOTAL** | **~11,500 lines** | **~5,380 lines** | **53%** |

---

## Next Steps (Phase 1)

Create new header files in `cubiomes-rebuild/`:
1. `biomes.h` - Clean biome definitions (1.21+ only)
2. `rng.h` - Copy needed RNG functions
3. `noise.h` - Copy needed noise structures

These will form the foundation for all other components.
