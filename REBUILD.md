# Cubiomes Rebuild Analysis for Minecraft 1.21+

## Executive Summary

Cubiomes is a C library that mimics Minecraft's biome and structure generation. The current codebase supports versions from Beta 1.7 through 1.21, resulting in significant complexity from legacy code paths. This document provides a comprehensive analysis for rebuilding Cubiomes to support only Minecraft 1.21+, eliminating ~70% of version-specific code and simplifying the architecture.

**Note**: Quad-base finder system has been removed from scope as it's not essential for core biome generation functionality.

## Current Architecture Overview

### Core Components

The library is organized into several key modules:

1. **Generator System** (`generator.h/c`)
   - Main entry point for biome generation
   - Contains version-specific initialization logic
   - Manages three different generation systems (layered, noise-based, beta)

2. **Biome Generation Systems**
   - **Layered System** (`layers.h/c`) - MC Beta 1.8 through 1.17
   - **Noise-Based System** (`biomenoise.h/c`) - MC 1.18+
   - **Beta System** (`biomenoise.h/c`) - MC Beta 1.7 and earlier

3. **Structure Finders** (`finders.h/c`)
   - Locates structures (villages, temples, monuments, etc.)
   - Contains version-specific structure configurations
   - Handles biome validation for structure placement

4. **Noise Generation** (`noise.h/c`)
   - Perlin noise implementation
   - Octave noise for terrain
   - Used by both modern and legacy systems

5. **RNG System** (`rng.h`)
   - Java-compatible random number generation
   - Xoroshiro128++ for 1.18+
   - Seed manipulation utilities

6. **Utility Functions** (`util.h/c`)
   - Biome color mapping
   - Image generation
   - Version string conversion

7. **Quad-Base Finder** (`quadbase.h/c`)
   - Specialized system for finding quad-structure seeds
   - Used for quad-witch-huts, quad-monuments, etc.

### Data Flow

```
User Code
    ↓
setupGenerator() → Initializes version-specific generator
    ↓
applySeed() → Applies world seed to appropriate system
    ↓
genBiomes() / getBiomeAt() → Generates biomes
    ↓
[For 1.18+] BiomeNoise system → Multi-dimensional noise sampling
[For ≤1.17] Layer system → Cascading layer transformations
```

## Version-Specific Code Analysis

### What Changes in 1.18+

Minecraft 1.18 ("Caves & Cliffs Part II") fundamentally changed biome generation:

1. **Noise-Based 3D Biomes**: Replaced the 2D layered system with 3D noise-based generation
2. **Multi-Noise Parameters**: Biomes determined by temperature, humidity, continentalness, erosion, depth, weirdness
3. **Vertical Biome Variation**: Biomes can now vary by Y-coordinate (caves, peaks, etc.)
4. **New Biomes**: Added meadow, grove, snowy_slopes, jagged_peaks, frozen_peaks, stony_peaks, dripstone_caves, lush_caves

### Legacy Code to Remove

#### 1. Entire Layered System (layers.c/h)
- **Size**: ~2000 lines
- **Purpose**: Generates biomes through cascading 2D layers (MC Beta 1.8 - 1.17)
- **Components**:
  - 60+ layer types (L_CONTINENT_4096, L_ZOOM_*, L_BIOME_256, etc.)
  - Layer stack management
  - Zoom, fuzzy zoom, land addition, biome placement
  - River generation layers
  - Ocean temperature layers (1.13+)
  - Voronoi zoom (1.14+)

#### 2. Beta Biome Generation
- **Location**: biomenoise.c, generator.c
- **Purpose**: Handles MC Beta 1.7 and earlier
- **Components**:
  - BiomeNoiseBeta structure
  - SurfaceNoiseBeta
  - Beta-specific octave initialization
  - Old biome IDs (seasonal_forest, rainforest, shrubland)

#### 3. Version-Specific Structure Logic
- **Location**: finders.c
- **Affected Structures**:
  - Feature (pre-1.13 combined structure type)
  - Ocean ruins/shipwrecks (different configs pre-1.16)
  - Villages (different configs pre-1.18)
  - Fortresses (different logic pre-1.18)
  - Ruined portals (different configs pre-1.18)
  - Desert wells, geodes (version-specific salts)

#### 4. Legacy Biome IDs
- **Location**: biomes.h/c
- **Remove**:
  - Beta biomes: seasonal_forest, rainforest, shrubland
  - Removed variants: mountain_edge, desert_hills, wooded_hills, etc.
  - Pre-1.18 mutated variants that don't exist in 1.18+

#### 5. Version Checks Throughout Codebase
- Hundreds of `if (mc >= MC_1_X)` checks
- Version-specific initialization paths
- Compatibility layers for old behavior

## Minecraft 1.21+ Requirements

### Supported Biomes (1.21)

**Overworld** (64 biomes):
- Base: ocean, plains, desert, forest, taiga, swamp, river, beach, etc.
- Mountains: meadow, grove, snowy_slopes, jagged_peaks, frozen_peaks, stony_peaks
- Variants: sunflower_plains, flower_forest, ice_spikes, bamboo_jungle, etc.
- Caves: dripstone_caves, lush_caves, deep_dark
- 1.19+: mangrove_swamp, deep_dark
- 1.20+: cherry_grove
- 1.21+: pale_garden

**Nether** (5 biomes):
- nether_wastes, soul_sand_valley, crimson_forest, warped_forest, basalt_deltas

**End** (5 biomes):
- the_end, small_end_islands, end_midlands, end_highlands, end_barrens

### Supported Structures (1.21)

**Overworld**:
- Villages, Pillager Outposts, Monuments, Mansions
- Desert Pyramids, Jungle Temples, Swamp Huts, Igloos
- Ocean Ruins, Shipwrecks, Ruined Portals
- Ancient Cities (1.19+), Trail Ruins (1.20+), Trial Chambers (1.21+)
- Buried Treasure, Mineshafts, Desert Wells, Geodes

**Nether**:
- Fortresses, Bastions, Ruined Portals

**End**:
- End Cities, End Gateways, End Islands

### Generation System (1.18+)

**Multi-Noise Parameters**:
- Temperature
- Humidity (Vegetation)
- Continentalness
- Erosion
- Depth (Peaks/Valleys)
- Weirdness

**Biome Trees**:
- Version-specific lookup tables in `tables/btree*.h`
- btree18.h (1.18), btree19.h (1.19), btree192.h (1.19.2), btree20.h (1.20), btree21wd.h (1.21)

## Incremental Rebuild Strategy

This rebuild follows a **ground-up reconstruction approach** where we build NEW files from scratch in a separate directory, analyzing what's needed and implementing only 1.21+ functionality. We DO NOT modify or delete existing files - instead, we create a clean, modern implementation alongside the original codebase.

### Rebuild Philosophy

1. **Analyze, Don't Copy**: Understand what each component does, then rebuild it cleanly
2. **New Files Only**: Create fresh implementations in `cubiomes-rebuild/` directory
3. **Test Incrementally**: Each phase produces working, testable code
4. **Keep Original**: Original codebase remains untouched as reference
5. **No Line-by-Line Deletion**: Build what's needed, ignore what's not

### Directory Structure

```
cubiomes/                    # Original codebase (untouched)
├── generator.c
├── layers.c
├── finders.c
└── ...

cubiomes-rebuild/            # New 1.21+ implementation
├── generator.c              # Rebuilt from scratch
├── biomenoise.c             # Rebuilt from scratch
├── finders.c                # Rebuilt from scratch
└── ...
```

### Phase 0: Analysis & Testing Infrastructure

**Goal**: Analyze what's needed for 1.21+ and set up testing framework.

**Analysis Tasks**:
1. Study `biomenoise.c` - understand 1.18+ noise generation
2. Study `generator.c` - identify 1.18+ code paths
3. Study `finders.c` - identify 1.18+ structure logic
4. Study `noise.c` and `rng.h` - understand dependencies
5. Document data structures needed for 1.21+
6. Map out function dependencies

**Testing Tasks**:
1. Create `tests/` directory
2. Build test suite using ORIGINAL codebase
3. Generate reference outputs for 1.21 seeds:
   - Biome generation at specific coordinates
   - Structure positions for known seeds
   - Nether/End biome generation
4. Save reference outputs as test expectations
5. Create automated test runner

**Test Cases**:
```c
// tests/test_reference.c
// Uses ORIGINAL cubiomes to generate reference data
void generate_reference_data() {
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    // Generate reference biomes
    int biome = getBiomeAt(&g, 4, 0, 15, 0);
    printf("Reference: seed=12345, pos=(0,15,0), biome=%d\n", biome);
    
    // Save to reference file
    save_reference("biome_12345_0_15_0.ref", biome);
}
```

**Deliverables**:
- `ANALYSIS.md` - Document of what's needed
- `tests/reference/` - Reference outputs from original code
- `tests/test_runner.c` - Automated test framework
- Dependency graph of required functions

**Success Criteria**:
- ✓ Complete understanding of 1.18+ code paths
- ✓ Reference data generated for 20+ test cases
- ✓ Test framework ready
- ✓ Clear list of what to rebuild

**Duration**: 1.5 days

---

### Phase 1: Rebuild Core Data Structures

**Goal**: Create NEW header files with clean, 1.21+-only data structures.

**New Files to Create**:
- `cubiomes-rebuild/biomes.h` - Biome IDs and enums (1.21+ only)
- `cubiomes-rebuild/rng.h` - RNG functions (copy needed parts)
- `cubiomes-rebuild/noise.h` - Noise structures (copy needed parts)

**Approach**:
1. **Analyze** original `biomes.h` - identify 1.18+ biomes
2. **Rebuild** with ONLY 1.21+ biomes (no legacy IDs)
3. **Analyze** original `rng.h` - identify needed RNG functions
4. **Copy** only Xoroshiro and Java RNG (used by 1.18+)
5. **Analyze** original `noise.h` - identify noise structures
6. **Copy** Perlin, Octave, DoublePerlin (used by 1.18+)

**Example - biomes.h (NEW FILE)**:
```c
// cubiomes-rebuild/biomes.h
#ifndef BIOMES_REBUILD_H_
#define BIOMES_REBUILD_H_

// Minecraft versions - 1.21+ ONLY
enum MCVersion {
    MC_UNDEF,
    MC_1_21_1,
    MC_1_21_3,
    MC_1_21_WD,
    MC_1_21 = MC_1_21_WD,
    MC_NEWEST = MC_1_21,
};

// Dimensions
enum Dimension {
    DIM_NETHER = -1,
    DIM_OVERWORLD = 0,
    DIM_END = +1,
};

// Biome IDs - 1.18+ ONLY (no legacy variants)
enum BiomeID {
    none = -1,
    // Base biomes
    ocean = 0,
    plains,
    desert,
    mountains,
    forest,
    taiga,
    swamp,
    river,
    // ... (only biomes that exist in 1.18+)
    
    // 1.18+ new biomes
    meadow = 177,
    grove = 178,
    snowy_slopes = 179,
    jagged_peaks = 180,
    frozen_peaks = 181,
    stony_peaks = 182,
    
    // 1.19+
    deep_dark = 183,
    mangrove_swamp = 184,
    
    // 1.20+
    cherry_grove = 185,
    
    // 1.21+
    pale_garden = 186,
};

// Helper functions (to be implemented in biomes.c)
int biomeExists(int mc, int id);
int isOverworld(int mc, int id);
int isOceanic(int id);
int isSnowy(int id);

#endif
```

**Testing**:
- Compile new headers
- Verify enums are correct
- Check no legacy biomes included

**Success Criteria**:
- ✓ New headers compile cleanly
- ✓ Only 1.21+ biomes included
- ✓ ~50% smaller than original biomes.h
- ✓ No legacy version references

**Duration**: 1 day

---

### Phase 2: Rebuild Noise Generation

**Goal**: Create NEW noise generation files by analyzing and rebuilding from original.

**New Files to Create**:
- `cubiomes-rebuild/noise.c` - Noise implementation
- `cubiomes-rebuild/noise.h` - Already created in Phase 1

**Approach**:
1. **Analyze** original `noise.c` - understand Perlin/Octave/DoublePerlin
2. **Identify** functions used by 1.18+ (grep for usage in biomenoise.c)
3. **Rebuild** ONLY the needed functions:
   - `perlinInit()` - Used by BiomeNoise
   - `xPerlinInit()` - Used by 1.18+
   - `samplePerlin()` - Core sampling
   - `octaveInit()` - Octave initialization
   - `xOctaveInit()` - 1.18+ octave init
   - `sampleOctave()` - Octave sampling
   - `doublePerlinInit()` - Double perlin init
   - `xDoublePerlinInit()` - 1.18+ double perlin
   - `sampleDoublePerlin()` - Double perlin sampling
4. **Skip** Beta-specific functions (octaveInitBeta, etc.)
5. **Test** each function as you rebuild it

**Implementation Strategy**:
```c
// cubiomes-rebuild/noise.c
#include "noise.h"
#include "rng.h"
#include <math.h>

// Rebuild perlinInit by analyzing original
void perlinInit(PerlinNoise *noise, uint64_t *seed) {
    // Study original implementation
    // Understand the algorithm
    // Rewrite cleanly without legacy code
    // ... implementation
}

// Continue for each needed function
```

**Testing**:
- Unit test each noise function
- Compare outputs with original for same seeds
- Verify noise patterns match

**Success Criteria**:
- ✓ All noise functions work correctly
- ✓ Outputs match original implementation
- ✓ No Beta-specific code included
- ✓ ~30% smaller than original noise.c

**Duration**: 1.5 days

---

### Phase 3: Rebuild BiomeNoise System

**Goal**: Create NEW biomenoise implementation for 1.18+ generation.

**New Files to Create**:
- `cubiomes-rebuild/biomenoise.h` - BiomeNoise structures
- `cubiomes-rebuild/biomenoise.c` - BiomeNoise implementation
- Copy `tables/btree*.h` - Biome tree tables (needed as-is)

**Approach**:
1. **Analyze** original `biomenoise.h` - identify structures:
   - `BiomeNoise` - KEEP (1.18+ overworld)
   - `NetherNoise` - KEEP (1.16+ nether)
   - `EndNoise` - KEEP (1.9+ end)
   - `BiomeNoiseBeta` - SKIP (legacy)
   - `SurfaceNoiseBeta` - SKIP (legacy)
2. **Analyze** original `biomenoise.c` - identify functions:
   - `initBiomeNoise()` - REBUILD
   - `setBiomeSeed()` - REBUILD
   - `sampleBiomeNoise()` - REBUILD
   - `genBiomeNoiseScaled()` - REBUILD
   - `setNetherSeed()` - REBUILD
   - `getNetherBiome()` - REBUILD
   - `setEndSeed()` - REBUILD
   - `getEndBiome()` - REBUILD
   - Skip all Beta functions
3. **Rebuild** each function cleanly

**Example - biomenoise.h (NEW FILE)**:
```c
// cubiomes-rebuild/biomenoise.h
#ifndef BIOMENOISE_REBUILD_H_
#define BIOMENOISE_REBUILD_H_

#include "noise.h"
#include "biomes.h"

// Range structure for area generation
typedef struct Range {
    int scale;  // 1, 4, 16, 64, or 256
    int x, z, y;
    int sx, sz, sy;
} Range;

// Overworld biome noise (1.18+)
typedef struct BiomeNoise {
    int mc;
    // ... (analyze original and rebuild structure)
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    DoublePerlinNoise continentalness;
    DoublePerlinNoise erosion;
    DoublePerlinNoise weirdness;
    // ... other fields
} BiomeNoise;

// Nether biome noise (1.16+)
typedef struct NetherNoise {
    DoublePerlinNoise temperature;
    DoublePerlinNoise humidity;
    PerlinNoise oct[8];
} NetherNoise;

// End biome noise (1.9+)
typedef struct EndNoise {
    int mc;
    PerlinNoise perlin;
} EndNoise;

// Functions
void initBiomeNoise(BiomeNoise *bn, int mc);
void setBiomeSeed(BiomeNoise *bn, uint64_t seed, int large);
int sampleBiomeNoise(const BiomeNoise *bn, int *dat, int x, int y, int z, void *data, uint32_t flags);
int genBiomeNoiseScaled(const BiomeNoise *bn, int *out, Range r, uint64_t sha);

void setNetherSeed(NetherNoise *nn, uint64_t seed);
int getNetherBiome(const NetherNoise *nn, int x, int y, int z, float *ndel);

void setEndSeed(EndNoise *en, int mc, uint64_t seed);
int getEndBiome(const EndNoise *en, int x, int y, int z);

#endif
```

**Testing**:
- Test biome generation for known seeds
- Compare with reference data from Phase 0
- Test all three dimensions
- Verify biome tree integration works

**Success Criteria**:
- ✓ Biome generation matches reference
- ✓ All dimensions work (Overworld, Nether, End)
- ✓ No Beta code included
- ✓ ~40% smaller than original biomenoise.c

**Duration**: 2 days

---

### Phase 4: Rebuild Generator Core

**Goal**: Create NEW generator.c with clean 1.21+ implementation.

**New Files to Create**:
- `cubiomes-rebuild/generator.h` - Generator structure
- `cubiomes-rebuild/generator.c` - Generator implementation

**Approach**:
1. **Analyze** original `generator.h` - understand Generator structure
2. **Design** NEW simplified Generator (no LayerStack, no Beta)
3. **Analyze** original `generator.c` - identify 1.18+ functions:
   - `setupGenerator()` - REBUILD (simplified)
   - `applySeed()` - REBUILD (simplified)
   - `genBiomes()` - REBUILD (noise-only)
   - `getBiomeAt()` - REBUILD (noise-only)
   - `getMinCacheSize()` - REBUILD (simplified)
   - `allocCache()` - REBUILD
4. **Skip** all layer-related functions
5. **Skip** all Beta-related functions

**Example - generator.h (NEW FILE)**:
```c
// cubiomes-rebuild/generator.h
#ifndef GENERATOR_REBUILD_H_
#define GENERATOR_REBUILD_H_

#include "biomenoise.h"

// Generator flags
enum {
    LARGE_BIOMES = 0x1,
};

// Simplified Generator structure (1.21+ only)
typedef struct Generator {
    int mc;              // MC_1_21_1, MC_1_21_3, MC_1_21_WD
    int dim;             // DIM_OVERWORLD, DIM_NETHER, DIM_END
    uint32_t flags;      // LARGE_BIOMES
    uint64_t seed;
    uint64_t sha;        // Voronoi SHA-256 hash
    
    BiomeNoise bn;       // Overworld biome generation
    NetherNoise nn;      // Nether biome generation
    EndNoise en;         // End biome generation
} Generator;

// Core API
void setupGenerator(Generator *g, int mc, uint32_t flags);
void applySeed(Generator *g, int dim, uint64_t seed);
int genBiomes(const Generator *g, int *cache, Range r);
int getBiomeAt(const Generator *g, int scale, int x, int y, int z);
size_t getMinCacheSize(const Generator *g, int scale, int sx, int sy, int sz);
int *allocCache(const Generator *g, Range r);

#endif
```

**Example - generator.c (NEW FILE)**:
```c
// cubiomes-rebuild/generator.c
#include "generator.h"
#include <stdlib.h>
#include <stdio.h>

void setupGenerator(Generator *g, int mc, uint32_t flags) {
    // Version validation
    if (mc < MC_1_21_1) {
        fprintf(stderr, "ERROR: Only MC 1.21+ supported\n");
        exit(1);
    }
    
    // Simple initialization (no version branching)
    g->mc = mc;
    g->dim = DIM_UNDEF;
    g->flags = flags;
    g->seed = 0;
    g->sha = 0;
    
    // Initialize biome noise system
    initBiomeNoise(&g->bn, mc);
}

void applySeed(Generator *g, int dim, uint64_t seed) {
    g->dim = dim;
    g->seed = seed;
    
    // Apply seed to appropriate dimension
    if (dim == DIM_OVERWORLD) {
        setBiomeSeed(&g->bn, seed, g->flags & LARGE_BIOMES);
    }
    else if (dim == DIM_NETHER) {
        setNetherSeed(&g->nn, seed);
    }
    else if (dim == DIM_END) {
        setEndSeed(&g->en, g->mc, seed);
    }
    
    // Set voronoi hash
    g->sha = getVoronoiSHA(seed);
}

int genBiomes(const Generator *g, int *cache, Range r) {
    // Analyze original and rebuild for 1.18+ only
    if (g->dim == DIM_OVERWORLD) {
        return genBiomeNoiseScaled(&g->bn, cache, r, g->sha);
    }
    // ... handle other dimensions
    return 0;
}

// ... rebuild other functions
```

**Testing**:
- Test setupGenerator() with 1.21 versions
- Test applySeed() for all dimensions
- Test genBiomes() against reference data
- Test getBiomeAt() for spot checks

**Success Criteria**:
- ✓ Generator works for all 1.21+ versions
- ✓ All dimensions functional
- ✓ Matches reference data from Phase 0
- ✓ ~70% smaller than original generator.c

**Duration**: 2 days

---

### Phase 5: Rebuild Biome Helpers

**Goal**: Create NEW biomes.c with clean helper functions.

**New Files to Create**:
- `cubiomes-rebuild/biomes.c` - Biome helper implementation

**Approach**:
1. **Analyze** original `biomes.c` - identify needed functions:
   - `biomeExists()` - REBUILD (1.21+ only)
   - `isOverworld()` - REBUILD (simplified)
   - `getDimension()` - REBUILD
   - `isOceanic()` - REBUILD
   - `isSnowy()` - REBUILD
   - `areSimilar()` - REBUILD (simplified)
2. **Skip** all legacy version checks
3. **Rebuild** each function cleanly

**Example - biomes.c (NEW FILE)**:
```c
// cubiomes-rebuild/biomes.c
#include "biomes.h"

int biomeExists(int mc, int id) {
    // Check version-specific additions
    if (id == pale_garden) return mc >= MC_1_21_WD;
    if (id == cherry_grove) return mc >= MC_1_20;
    if (id == deep_dark || id == mangrove_swamp) return mc >= MC_1_19_2;
    
    // Nether biomes (1.16+)
    if (id >= soul_sand_valley && id <= basalt_deltas) return 1;
    
    // End biomes (1.9+)
    if (id >= small_end_islands && id <= end_barrens) return 1;
    
    // 1.18+ overworld biomes
    switch (id) {
    case ocean:
    case plains:
    case desert:
    // ... (list all 1.18+ biomes)
    case meadow:
    case grove:
    case snowy_slopes:
    case jagged_peaks:
    case frozen_peaks:
    case stony_peaks:
    case dripstone_caves:
    case lush_caves:
        return 1;
    default:
        return 0;
    }
}

int isOverworld(int mc, int id) {
    // Simple check - no legacy biomes
    return biomeExists(mc, id) && getDimension(id) == DIM_OVERWORLD;
}

int getDimension(int id) {
    if (id >= soul_sand_valley && id <= basalt_deltas)
        return DIM_NETHER;
    if (id >= small_end_islands && id <= end_barrens)
        return DIM_END;
    if (id == the_end)
        return DIM_END;
    if (id == nether_wastes)
        return DIM_NETHER;
    return DIM_OVERWORLD;
}

// ... rebuild other helpers
```

**Testing**:
- Test biomeExists() for all 1.21 biomes
- Test dimension detection
- Test biome categorization functions

**Success Criteria**:
- ✓ All helper functions work correctly
- ✓ No legacy biomes referenced
- ✓ ~60% smaller than original biomes.c
- ✓ All tests pass

**Duration**: 1 day

---

### Phase 6: Rebuild Structure Finders

**Goal**: Create NEW finders.c with 1.18+ structure logic only.

**New Files to Create**:
- `cubiomes-rebuild/finders.h` - Structure finder API
- `cubiomes-rebuild/finders.c` - Structure finder implementation

**Approach**:
1. **Analyze** original `finders.h` - identify needed structures:
   - `StructureConfig` - KEEP
   - `Pos`, `Pos3` - KEEP
   - `BiomeFilter` - KEEP
   - Other helper structures - ANALYZE and KEEP if needed
2. **Analyze** original `finders.c` - identify 1.18+ functions:
   - `getStructureConfig()` - REBUILD (1.18+ configs only)
   - `getStructurePos()` - REBUILD (simplified)
   - `isViableStructurePos()` - REBUILD (simplified)
   - `getPopulationSeed()` - REBUILD (1.18+ only)
   - Structure-specific helpers - REBUILD as needed
3. **Remove** all pre-1.18 structure configs
4. **Simplify** all version checks

**Example - Structure Configs (NEW)**:
```c
// cubiomes-rebuild/finders.c

// 1.18+ structure configurations ONLY
static const StructureConfig
    s_desert_pyramid = { 14357617, 32, 24, Desert_Pyramid, 0, 0},
    s_igloo = { 14357618, 32, 24, Igloo, 0, 0},
    s_jungle_temple = { 14357619, 32, 24, Jungle_Pyramid, 0, 0},
    s_swamp_hut = { 14357620, 32, 24, Swamp_Hut, 0, 0},
    s_outpost = {165745296, 32, 24, Outpost, 0, 0},
    s_village = { 10387312, 34, 26, Village, 0, 0},
    s_ocean_ruin = { 14357621, 20, 12, Ocean_Ruin, 0, 0},
    s_shipwreck = {165745295, 24, 20, Shipwreck, 0, 0},
    s_monument = { 10387313, 32, 27, Monument, 0, 0},
    s_mansion = { 10387319, 80, 60, Mansion, 0, 0},
    s_ruined_portal = { 34222645, 40, 25, Ruined_Portal, 0, 0},
    s_ruined_portal_n = { 34222645, 40, 25, Ruined_Portal, DIM_NETHER, 0},
    s_ancient_city = { 20083232, 24, 16, Ancient_City, 0, 0},
    s_trail_ruins = { 83469867, 34, 26, Trail_Ruins, 0, 0},
    s_trial_chambers = { 94251327, 34, 22, Trial_Chambers, 0, 0},
    s_treasure = { 10387320, 1, 1, Treasure, 0, 0},
    s_mineshaft = { 0, 1, 1, Mineshaft, 0, 0},
    s_desert_well = { 40002, 1, 1, Desert_Well, 0, 1.f/1000},
    s_geode = { 20002, 1, 1, Geode, 0, 1.f/24},
    s_fortress = { 30084232, 27, 23, Fortress, DIM_NETHER, 0},
    s_bastion = { 30084232, 27, 23, Bastion, DIM_NETHER, 0},
    s_end_city = { 10387313, 20, 9, End_City, DIM_END, 0},
    s_end_gateway = { 40000, 1, 1, End_Gateway, DIM_END, 1.f/700},
    s_end_island = { 0, 1, 1, End_Island, DIM_END, 1.f/14};

int getStructureConfig(int structureType, int mc, StructureConfig *sconf) {
    // Simple switch - no version branching for pre-1.18
    switch (structureType) {
    case Desert_Pyramid:
        *sconf = s_desert_pyramid;
        return 1;
    case Village:
        *sconf = s_village;
        return 1;
    case Ancient_City:
        *sconf = s_ancient_city;
        return mc >= MC_1_19_2;
    case Trail_Ruins:
        *sconf = s_trail_ruins;
        return mc >= MC_1_20;
    case Trial_Chambers:
        *sconf = s_trial_chambers;
        return mc >= MC_1_21_1;
    // ... etc
    default:
        return 0;
    }
}

uint64_t getPopulationSeed(int mc, uint64_t ws, int x, int z) {
    // Always use 1.18+ path (no version check)
    Xoroshiro xr;
    xSetSeed(&xr, ws);
    uint64_t a = xNextLongJ(&xr);
    uint64_t b = xNextLongJ(&xr);
    a |= 1;
    b |= 1;
    return (x * a + z * b) ^ ws;
}
```

**Testing**:
- Test structure positioning for all structure types
- Compare with reference data from Phase 0
- Test biome validation for structures
- Test all dimensions

**Success Criteria**:
- ✓ All structure types position correctly
- ✓ Matches reference data
- ✓ ~50% smaller than original finders.c
- ✓ All tests pass

**Duration**: 2.5 days

---

### Phase 7: Rebuild Utilities

**Goal**: Create NEW util.c with simplified utility functions.

**New Files to Create**:
- `cubiomes-rebuild/util.h` - Utility API
- `cubiomes-rebuild/util.c` - Utility implementation

**Approach**:
1. **Analyze** original `util.c` - identify needed functions:
   - `mc2str()` - REBUILD (1.21+ only)
   - `str2mc()` - REBUILD (1.21+ only)
   - `biome2str()` - REBUILD (1.18+ biomes only)
   - `struct2str()` - REBUILD
   - `initBiomeColors()` - COPY (no changes needed)
   - `biomesToImage()` - COPY (no changes needed)
   - `savePPM()` - COPY (no changes needed)
   - `loadSavedSeeds()` - COPY (no changes needed)
2. **Skip** legacy version handling

**Example - util.c (NEW FILE)**:
```c
// cubiomes-rebuild/util.c
#include "util.h"
#include "biomes.h"
#include <string.h>

const char* mc2str(int mc) {
    switch(mc) {
        case MC_1_21_1: return "1.21.1";
        case MC_1_21_3: return "1.21.3";
        case MC_1_21_WD: return "1.21-wd";
        default: return "unknown";
    }
}

int str2mc(const char *s) {
    if (!strcmp(s, "1.21.1") || !strcmp(s, "1.21")) return MC_1_21_1;
    if (!strcmp(s, "1.21.3")) return MC_1_21_3;
    if (!strcmp(s, "1.21-wd")) return MC_1_21_WD;
    return MC_UNDEF;
}

const char *biome2str(int mc, int id) {
    // Only 1.18+ biomes
    switch(id) {
    case ocean: return "minecraft:ocean";
    case plains: return "minecraft:plains";
    case desert: return "minecraft:desert";
    // ... (all 1.18+ biomes)
    case meadow: return "minecraft:meadow";
    case grove: return "minecraft:grove";
    case cherry_grove: return "minecraft:cherry_grove";
    case pale_garden: return "minecraft:pale_garden";
    default: return NULL;
    }
}

// Copy unchanged functions from original
// - initBiomeColors()
// - biomesToImage()
// - savePPM()
// - loadSavedSeeds()
```

**Testing**:
- Test version string conversion
- Test biome name lookup
- Test image generation
- Test seed loading

**Success Criteria**:
- ✓ All utility functions work
- ✓ ~50% smaller than original util.c
- ✓ All tests pass

**Duration**: 0.5 days

---

### Phase 8: Rebuild Quad-Base System

**Goal**: Create NEW quadbase.c with updated structure references.

**New Files to Create**:
- `cubiomes-rebuild/quadbase.h` - Quad-base API
- `cubiomes-rebuild/quadbase.c` - Quad-base implementation

**Approach**:
1. **Analyze** original `quadbase.h` - understand quad-base algorithm
2. **Copy** core algorithm (still valid for 1.18+)
3. **Update** structure references to 1.18+ configs
4. **Remove** references to legacy structures
5. **Test** with 1.18+ structure positioning

**Example - quadbase.h (NEW FILE)**:
```c
// cubiomes-rebuild/quadbase.h
// Most of this file can be copied as-is since the algorithm is version-independent
// Just update structure references and remove legacy structure types

#ifndef QUADBASE_REBUILD_H_
#define QUADBASE_REBUILD_H_

#include "finders.h"
#include <stdio.h>
#include <math.h>

// Low 20-bit constellations (unchanged)
static const uint64_t low20QuadIdeal[] = {
    0x43f18, 0xc751a, 0xf520a, 0
};

// ... (copy other constellation arrays)

// Core functions (copy from original)
static inline float isQuadBase(const StructureConfig sconf, uint64_t seed, int radius);
static inline float isQuadBaseFeature24(const StructureConfig sconf, uint64_t seed, int ax, int ay, int az);
// ... (other inline functions)

int searchAll48(/* ... */);
Pos getOptimalAfk(Pos p[4], int ax, int ay, int az, int *spcnt);
int scanForQuads(/* ... */);

#endif
```

**Testing**:
- Test quad-hut finding
- Test quad-monument finding
- Compare with reference data
- Verify searchAll48() still works

**Success Criteria**:
- ✓ Quad-base finding works correctly
- ✓ Matches reference data
- ✓ ~10% smaller (minimal changes)
- ✓ All tests pass

**Duration**: 1 day

---

### Phase 9: Build System & Integration

**Goal**: Create build system for rebuilt library and ensure it compiles.

**New Files to Create**:
- `cubiomes-rebuild/Makefile` - Build configuration
- `cubiomes-rebuild/CMakeLists.txt` - CMake configuration

**Approach**:
1. **Create** Makefile for rebuilt library
2. **Create** CMakeLists.txt for rebuilt library
3. **Compile** all rebuilt files
4. **Link** into libcubiomes-rebuild.a
5. **Test** linking with example programs

**Example - Makefile (NEW FILE)**:
```makefile
# cubiomes-rebuild/Makefile
CC = gcc
AR = ar
ARFLAGS = cr
LDFLAGS = -lm
CFLAGS = -Wall -Wextra -fwrapv -O3

SOURCES = \
    noise.c \
    biomes.c \
    biomenoise.c \
    generator.c \
    finders.c \
    util.c \
    quadbase.c

OBJECTS = $(SOURCES:.c=.o)

all: libcubiomes-rebuild.a

libcubiomes-rebuild.a: $(OBJECTS)
    $(AR) $(ARFLAGS) $@ $^

%.o: %.c
    $(CC) -c $(CFLAGS) $<

clean:
    rm -f *.o *.a

.PHONY: all clean
```

**Example - CMakeLists.txt (NEW FILE)**:
```cmake
# cubiomes-rebuild/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(cubiomes-rebuild)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -fwrapv")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(HEADERS
    biomes.h
    generator.h
    biomenoise.h
    noise.h
    rng.h
    util.h
    finders.h
    quadbase.h
)

set(SOURCES
    biomes.c
    generator.c
    biomenoise.c
    noise.c
    util.c
    finders.c
    quadbase.c
)

add_library(cubiomes-rebuild STATIC ${SOURCES})
target_link_libraries(cubiomes-rebuild m)

install(TARGETS cubiomes-rebuild DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/cubiomes-rebuild)
```

**Testing**:
- Compile with make
- Compile with cmake
- Link test programs
- Run all Phase 0 tests with rebuilt library

**Success Criteria**:
- ✓ Compiles without errors
- ✓ Links successfully
- ✓ All tests pass with rebuilt library
- ✓ Binary size ~60% smaller than original

**Duration**: 0.5 days

## File-by-File Rebuild Plan

### Files to DELETE Entirely
- `layers.c` (2000+ lines of legacy code)
- `layers.h` (layer definitions and enums)

### Files to HEAVILY MODIFY

#### generator.h/c
- Remove LayerStack from Generator union
- Remove Beta system from union
- Simplify setupGenerator() - single code path
- Simplify applySeed() - remove layer/beta branches
- Remove getLayerForScale() - no longer needed
- Remove setupLayerStack() - no longer needed
- Remove genArea() - layer-specific
- Keep genBiomes() - update for noise-only

#### finders.h/c
- Remove legacy structure configs (pre-1.18)
- Simplify getStructureConfig() - remove version checks
- Simplify getStructurePos() - single RNG path
- Simplify isViableStructurePos() - remove version branches
- Update structure biome validation for 1.18+
- Keep quad-structure logic

#### biomes.h/c
- Remove legacy biome IDs (Beta, pre-1.18 variants)
- Simplify biomeExists() - single version check
- Simplify isOverworld() - remove legacy biomes
- Keep 1.18+ biome helpers

#### biomenoise.h/c
- Remove BiomeNoiseBeta structure
- Remove SurfaceNoiseBeta
- Remove Beta initialization functions
- Keep BiomeNoise (1.18+)
- Keep NetherNoise (1.16+)
- Keep EndNoise (1.9+)
- Maintain biome tree integration

### Files to LIGHTLY MODIFY

#### noise.h/c
- Keep all noise functions (used by 1.18+)
- Remove beta-specific octave functions if any

#### rng.h
- Keep both Java RNG and Xoroshiro
- Remove beta-specific RNG if any

#### util.h/c
- Simplify version string functions
- Keep biome colors and image generation
- Update biome2str() for 1.18+ biomes only

#### quadbase.h/c
- Update structure references
- Remove legacy structure types
- Keep core quad-finding algorithm

### Files to KEEP Unchanged

#### tables/btree*.h
- Keep all biome tree tables
- These define 1.18+ biome generation

## Dependencies and Relationships

### Critical Dependencies (Must Keep)

```
Generator (simplified)
    ├── BiomeNoise (1.18+ overworld)
    │   ├── Noise (Perlin, Octave, DoublePerlin)
    │   ├── Spline system
    │   └── Biome trees (btree*.h)
    ├── NetherNoise (1.16+ nether)
    │   └── DoublePerlin noise
    └── EndNoise (1.9+ end)
        └── SimplexNoise

Structure Finders
    ├── StructureConfig (1.18+ only)
    ├── RNG (Java + Xoroshiro)
    └── Generator (for biome validation)

Quad-Base Finder
    ├── Structure Finders
    └── RNG
```

### Removed Dependencies

```
LayerStack (DELETED)
    ├── 60+ Layer types
    ├── Layer chaining
    └── Voronoi zoom

BiomeNoiseBeta (DELETED)
    └── Beta-specific octaves

Legacy Structure Configs (DELETED)
    └── Pre-1.18 structure positioning
```

## Performance Improvements

### Expected Gains

1. **Reduced Code Size**: ~70% reduction in total lines
   - layers.c: ~2000 lines removed
   - Version checks: ~500 lines removed
   - Legacy configs: ~300 lines removed

2. **Faster Compilation**: Fewer files, simpler dependencies

3. **Better Cache Locality**: Smaller Generator structure

4. **Simplified Control Flow**: No version branching

5. **Easier Optimization**: Single code path to optimize

### Maintained Performance

- Noise generation speed (unchanged)
- Structure finding speed (unchanged)
- Quad-base search speed (unchanged)

## API Compatibility

### Breaking Changes

1. **Version Support**: Only MC 1.21+ supported
2. **Generator Structure**: Simplified (no LayerStack)
3. **Removed Functions**:
   - setupLayerStack()
   - getLayerForScale()
   - genArea()
   - setLayerSeed()
   - All layer map functions (mapContinent, mapZoom, etc.)

### Maintained API

```c
// Core API remains the same
void setupGenerator(Generator *g, int mc, uint32_t flags);
void applySeed(Generator *g, int dim, uint64_t seed);
int genBiomes(const Generator *g, int *cache, Range r);
int getBiomeAt(const Generator *g, int scale, int x, int y, int z);

// Structure finding
int getStructureConfig(int structureType, int mc, StructureConfig *sconf);
int getStructurePos(int structureType, int mc, uint64_t seed, 
                    int regX, int regZ, Pos *pos);
int isViableStructurePos(int structureType, const Generator *g, 
                         int x, int z, uint32_t flags);

// Quad-base finding
float isQuadBase(const StructureConfig sconf, uint64_t seed, int radius);
int searchAll48(uint64_t **seedbuf, uint64_t *buflen, const char *path,
                int threads, const uint64_t *lowBits, int lowBitN,
                int (*check)(uint64_t s48, void *data), void *data,
                volatile char *stop);
```

## Testing Strategy

### Unit Tests Needed

1. **Biome Generation**
   - Test known seeds produce correct biomes
   - Verify 3D biome variation
   - Check biome boundaries

2. **Structure Positioning**
   - Verify structure positions match Minecraft
   - Test all structure types
   - Validate biome requirements

3. **Noise Generation**
   - Test noise consistency
   - Verify multi-noise parameters
   - Check spline interpolation

4. **Quad-Base Finding**
   - Verify quad-hut bases
   - Test quad-monument bases
   - Validate enclosing radius calculations

### Integration Tests

1. **End-to-End Generation**
   - Generate large areas
   - Compare with Minecraft output
   - Verify performance

2. **Cross-Version Compatibility**
   - Test 1.21.1, 1.21.3, 1.21-WD
   - Verify biome differences
   - Check structure changes

## Migration Guide for Users

### Code Changes Required

```c
// OLD (pre-rebuild)
Generator g;
setupGenerator(&g, MC_1_7, 0);  // Supported any version

// NEW (post-rebuild)
Generator g;
setupGenerator(&g, MC_1_21, 0);  // Only 1.21+ supported
// Will error if mc < MC_1_21_1
```

### Removed Features

1. **No Legacy Version Support**: Cannot generate for MC < 1.21
2. **No Layer Access**: Cannot access individual layers
3. **No Beta Biomes**: seasonal_forest, rainforest, shrubland removed

### Recommended Approach

For users needing legacy support:
- Keep using original Cubiomes for old versions
- Use rebuilt Cubiomes for 1.21+ only
- Maintain both libraries if needed

## Build System Updates

### CMakeLists.txt
```cmake
# Simplified source list
set(SOURCES
    finders.c
    generator.c
    biomenoise.c
    biomes.c
    noise.c
    util.c
    quadbase.c
    # layers.c REMOVED
)

set(HEADERS
    finders.h
    generator.h
    biomenoise.h
    biomes.h
    noise.h
    rng.h
    util.h
    quadbase.h
    # layers.h REMOVED
)
```

### Makefile
```makefile
# Remove layers.o from dependencies
libcubiomes: noise.o biomes.o biomenoise.o generator.o finders.o util.o quadbase.o
    $(AR) $(ARFLAGS) libcubiomes.a $^
```

## Documentation Updates

### README.md Changes

1. Update version support statement
2. Remove layer-related examples
3. Update API examples to 1.21+
4. Add migration guide section

### Code Comments

1. Remove version-specific comments
2. Update function documentation
3. Add "1.21+ only" notes where relevant

## Risk Assessment

### Low Risk
- Noise generation (well-tested, unchanged)
- Structure positioning (simplified, not changed fundamentally)
- RNG (keeping both systems)

### Medium Risk
- Biome validation (removing version checks)
- Structure biome requirements (1.18+ only)
- Generator initialization (simplified paths)

### Mitigation
- Extensive testing with known seeds
- Comparison with Minecraft output
- Gradual rollout with beta period

## Timeline Summary

| Phase | Description | Duration | Cumulative |
|-------|-------------|----------|------------|
| 0 | Analysis & Testing Infrastructure | 1.5 days | 1.5 days |
| 1 | Rebuild Core Data Structures | 1 day | 2.5 days |
| 2 | Rebuild Noise Generation | 1.5 days | 4 days |
| 3 | Rebuild BiomeNoise System | 2 days | 6 days |
| 4 | Rebuild Generator Core | 2 days | 8 days |
| 5 | Rebuild Biome Helpers | 1 day | 9 days |
| 6 | Rebuild Structure Finders | 2.5 days | 11.5 days |
| 7 | Rebuild Utilities | 0.5 days | 12 days |
| 8 | Rebuild Quad-Base System | 1 day | 13 days |
| 9 | Build System & Integration | 0.5 days | 13.5 days |
| 10 | Documentation & Examples | 1 day | 14.5 days |

**Total: 14.5 days**

### Key Milestones

- **Day 1.5**: Analysis complete, testing infrastructure ready
- **Day 4**: Noise generation rebuilt and tested
- **Day 8**: Core generator rebuilt, biomes generating correctly
- **Day 12**: All components rebuilt, utilities complete
- **Day 14.5**: Documentation complete, ready for release

### Rebuild Approach Benefits

1. **No Risk of Breaking Original**: Original code untouched
2. **Clean Implementation**: No legacy code to work around
3. **Incremental Testing**: Each component tested as built
4. **Clear Understanding**: Forces deep analysis of each component
5. **Better Architecture**: Opportunity to improve design
6. **Easy Comparison**: Can compare with original at any time

### Testing Checkpoints

After each phase:
1. Compile rebuilt components
2. Run tests against reference data from Phase 0
3. Compare outputs with original implementation
4. Validate performance
5. Document any differences
6. Commit to git with phase tag

### Rollback Strategy

Since original code is untouched:
1. Any phase can be redone without affecting others
2. Can reference original implementation at any time
3. Can test both implementations side-by-side
4. No risk of losing working code

## Success Criteria (Per Phase)

### Phase 0
- ✓ Test suite runs successfully
- ✓ All baseline tests pass
- ✓ Known seeds produce expected results
- ✓ Documentation complete

### Phase 1
- ✓ All Phase 0 tests still pass
- ✓ Pre-1.21 versions rejected with clear error
- ✓ 1.21+ versions work normally
- ✓ No functionality broken

### Phase 2
- ✓ Compiles with and without CUBIOMES_REBUILD flag
- ✓ All tests pass with CUBIOMES_REBUILD enabled
- ✓ Legacy code paths not executed in rebuild mode
- ✓ Code size reduction visible in binary

### Phase 3
- ✓ Compiles successfully
- ✓ All Phase 0 tests pass
- ✓ Beta biomes removed from enum
- ✓ ~200 lines removed

### Phase 4
- ✓ Compiles successfully
- ✓ All Phase 0 tests pass
- ✓ ~2200 lines removed
- ✓ Binary size significantly reduced
- ✓ No performance regression

### Phase 5
- ✓ All Phase 0 tests pass
- ✓ Legacy biome IDs removed
- ✓ ~300 lines removed from biomes.c
- ✓ Biome helpers simplified

### Phase 6
- ✓ All Phase 0 tests pass
- ✓ All structure types position correctly
- ✓ ~500 lines removed from finders.c
- ✓ Structure validation works

### Phase 7
- ✓ All Phase 0 tests pass
- ✓ Version utilities simplified
- ✓ ~100 lines removed
- ✓ All utilities functional

### Phase 8
- ✓ All tests pass
- ✓ No preprocessor guards remaining
- ✓ Code fully cleaned up
- ✓ Performance validated
- ✓ Documentation updated

### Phase 9
- ✓ All documentation complete
- ✓ Examples compile and run
- ✓ Migration guide clear
- ✓ Ready for release

### Overall Success Criteria
1. ✓ All 1.21+ biomes generate correctly
2. ✓ All 1.21+ structures position correctly
3. ✓ Biome validation works for all structures
4. ✓ Quad-base finding still works
5. ✓ Code size reduced by >60%
6. ✓ No performance regression
7. ✓ All tests pass at every phase
8. ✓ Documentation complete

## Conclusion

The rebuild is highly feasible and will result in a significantly cleaner, more maintainable codebase. The key insight is that Minecraft 1.18 fundamentally changed biome generation, making the entire layered system obsolete. By targeting only 1.21+, we can remove ~70% of the complexity while maintaining all modern functionality.

The rebuild should follow a systematic approach:
1. Remove the entire layered system
2. Simplify the Generator structure
3. Clean up version checks throughout
4. Update structure configurations
5. Maintain the noise-based generation system

The result will be a focused, efficient library for modern Minecraft versions, with significantly reduced maintenance burden and improved code clarity.


### Phase 10: Documentation & Examples

**Goal**: Create documentation and example programs for rebuilt library.

**New Files to Create**:
- `cubiomes-rebuild/README.md` - Library documentation
- `cubiomes-rebuild/examples/` - Example programs
- `cubiomes-rebuild/MIGRATION.md` - Migration guide

**Approach**:
1. **Write** README with 1.21+ focus
2. **Create** example programs:
   - `find_biome.c` - Find specific biome
   - `generate_map.c` - Generate biome map
   - `find_structure.c` - Find structure
   - `quad_huts.c` - Find quad-witch-huts
3. **Write** migration guide for users
4. **Document** API differences from original

**Example - README.md (NEW FILE)**:
```markdown
# Cubiomes Rebuild - Minecraft 1.21+ Only

A clean, modern implementation of Minecraft biome and structure generation
for versions 1.21 and above.

## Features

- Fast biome generation using 1.18+ noise-based system
- Structure finding for all 1.21+ structures
- Quad-structure base finding
- 60% smaller codebase than original
- No legacy code or version branching

## Supported Versions

- Minecraft 1.21.1
- Minecraft 1.21.3
- Minecraft 1.21 Winter Drop

## Quick Start

```c
#include "generator.h"

int main() {
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    int biome = getBiomeAt(&g, 4, 0, 15, 0);
    printf("Biome at (0, 15, 0): %d\n", biome);
    
    return 0;
}
```

## Building

```bash
cd cubiomes-rebuild
make
```

## Migration from Original Cubiomes

See MIGRATION.md for details on differences and how to update your code.
```

**Example Programs**:
```c
// examples/find_biome.c
#include "../generator.h"
#include <stdio.h>

int main() {
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    
    uint64_t seed;
    for (seed = 0; ; seed++) {
        applySeed(&g, DIM_OVERWORLD, seed);
        int biome = getBiomeAt(&g, 1, 0, 63, 0);
        
        if (biome == cherry_grove) {
            printf("Seed %llu has cherry grove at origin\n", seed);
            break;
        }
    }
    
    return 0;
}
```

**Testing**:
- Compile all examples
- Run examples and verify output
- Review documentation for clarity

**Success Criteria**:
- ✓ README complete and clear
- ✓ All examples compile and run
- ✓ Migration guide helpful
- ✓ API documented

**Duration**: 1 day

---
