# Cubiomes Rebuild

A clean, modern reimplementation of the Cubiomes library for Minecraft 1.18+. This library provides fast, accurate biome and structure generation matching Minecraft's world generation.

## What's Different?

This is a **complete rebuild** of the original Cubiomes library with:
- **1.18+ only** - No legacy code (Beta through 1.17 removed)
- **65% smaller** - ~4,000 lines vs ~11,500 lines
- **Cleaner API** - Simplified, modern C code
- **Fully tested** - Validated against original Cubiomes
- **Easy to build** - Simple batch file build system

## Quick Start

### Build the Library

```batch
cd cubiomes-rebuild
build.bat
```

Output: `lib/libcubiomes.a`

### Run Tests

```batch
build.bat test
test.bat
```

### Use in Your Project

```cpp
#include "generator.h"
#include "finders.h"
#include "util.h"

int main() {
    // Create generator for Minecraft 1.18
    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    // Get biome at coordinates
    int biome = getBiomeAt(&g, 1, 0, 63 >> 2, 0);
    printf("Biome: %s\n", biome2str(MC_1_18, biome));
    
    return 0;
}
```

## Core Concepts

### Versions

Supported Minecraft versions:
- `MC_1_18` - 1.18 - 1.18.2
- `MC_1_19_2` - 1.19.2 - 1.19.3
- `MC_1_19_4` - 1.19.4 - 1.20.5
- `MC_1_20_6` - 1.20.6 - 1.21.0
- `MC_1_21_1` - 1.21.1
- `MC_1_21_3` - 1.21.3
- `MC_1_21_WD` - 1.21 Winter Drop (latest)

### Dimensions

Three dimensions are supported:
- `DIM_OVERWORLD` - Normal world
- `DIM_NETHER` - Nether
- `DIM_END` - The End

### Scales

Biome generation works at different scales:
- `scale = 1` - 1:1 block resolution (with voronoi)
- `scale = 4` - 1:4 block resolution (climate parameters)
- `scale = 64` - 1:64 block resolution (biome source)

Higher scales are faster but less accurate.

## API Overview

### Generator Setup

```cpp
Generator g;
setupGenerator(&g, MC_1_18, 0);           // Initialize for version
applySeed(&g, DIM_OVERWORLD, seed);       // Apply world seed
```

### Get Single Biome

```cpp
// Get biome at block coordinates (x, y, z)
int biome = getBiomeAt(&g, scale, x >> 2, y >> 2, z >> 2);
```

Note: Coordinates are in quarter-blocks (divide by 4 or shift right by 2).

### Generate Biome Area

```cpp
Range r = {scale, x, z, width, height, y, 1};
int *biomes = allocCache(&g, r);
genBiomes(&g, biomes, r);

// Access biomes[i] where i = (z * width + x)
free(biomes);
```

### Find Structures

```cpp
Pos pos;
if (getStructurePos(Village, MC_1_18, seed, regX, regZ, &pos)) {
    printf("Village at: %d, %d\n", pos.x, pos.z);
}

// Check if structure can spawn at position
if (isViableStructurePos(Village, &g, pos.x, pos.z, 0)) {
    printf("Village is viable!\n");
}
```

### Structure Types

Common structures:
- `Desert_Pyramid`, `Jungle_Temple`, `Swamp_Hut`, `Igloo`
- `Village`, `Outpost`, `Mansion`
- `Monument`, `Ocean_Ruin`, `Shipwreck`
- `Ancient_City` (1.19+), `Trail_Ruins` (1.20+), `Trial_Chambers` (1.21+)
- `Fortress`, `Bastion` (Nether)
- `End_City` (End)

### Find Strongholds

```cpp
StrongholdIter sh;
Pos first = initFirstStronghold(&sh, MC_1_18, seed);
printf("First stronghold: %d, %d\n", first.x, first.z);

// Iterate through all 128 strongholds
while (nextStronghold(&sh, &g) > 0) {
    printf("Stronghold %d: %d, %d\n", sh.index, sh.pos.x, sh.pos.z);
}
```

### Biome Colors

```cpp
// Get default color for a biome
const unsigned char *color = getBiomeColor(plains);
printf("RGB: %d, %d, %d\n", color[0], color[1], color[2]);

// Or initialize full palette
unsigned char colors[256][3];
initBiomeColors(colors);
```

### Utility Functions

```cpp
// Version conversion
const char *version = mc2str(MC_1_18);    // "1.18"
int mc = str2mc("1.21");                  // MC_1_21

// Biome names
const char *name = biome2str(MC_1_18, plains);  // "plains"

// Structure names
const char *sname = struct2str(Village);  // "village"

// Biome checks
if (isOceanic(biome)) { /* ... */ }
if (isSnowy(biome)) { /* ... */ }
if (biomeExists(MC_1_18, biome)) { /* ... */ }
```

## Examples

### Example 1: Generate Biome Map

```cpp
Generator g;
setupGenerator(&g, MC_1_18, 0);
applySeed(&g, DIM_OVERWORLD, 12345);

// Generate 512x512 area at spawn
Range r = {4, -256, -256, 512, 512, 63, 1};
int *biomes = allocCache(&g, r);
genBiomes(&g, biomes, r);

// Process biomes
for (int z = 0; z < 512; z++) {
    for (int x = 0; x < 512; x++) {
        int biome = biomes[z * 512 + x];
        const unsigned char *color = getBiomeColor(biome);
        // Draw pixel at (x, z) with color
    }
}

free(biomes);
```

### Example 2: Find Nearest Village

```cpp
Generator g;
setupGenerator(&g, MC_1_18, 0);
applySeed(&g, DIM_OVERWORLD, 12345);

int found = 0;
for (int regX = -10; regX < 10 && !found; regX++) {
    for (int regZ = -10; regZ < 10 && !found; regZ++) {
        Pos pos;
        if (getStructurePos(Village, MC_1_18, 12345, regX, regZ, &pos)) {
            if (isViableStructurePos(Village, &g, pos.x, pos.z, 0)) {
                printf("Found village at: %d, %d\n", pos.x, pos.z);
                found = 1;
            }
        }
    }
}
```

### Example 3: Check Biome at Player Position

```cpp
Generator g;
setupGenerator(&g, MC_1_18, 0);
applySeed(&g, DIM_OVERWORLD, seed);

int playerX = 1000, playerY = 64, playerZ = 2000;
int biome = getBiomeAt(&g, 1, playerX >> 2, playerY >> 2, playerZ >> 2);

printf("You are in: %s\n", biome2str(MC_1_18, biome));

if (isSnowy(biome)) {
    printf("It's cold here!\n");
}
```

## Performance Tips

1. **Use appropriate scale**: Scale 4 is usually sufficient for biome maps
2. **Cache generators**: Don't recreate Generator for every query
3. **Batch queries**: Use `genBiomes()` for multiple positions
4. **Pre-allocate**: Reuse cache buffers when possible

## File Structure

```
cubiomes-rebuild/
├── biomes.h/c          - Biome definitions and helpers
├── noise.h/c           - Perlin noise generation
├── biomenoise.h/c      - Climate-based biome noise (1.18+)
├── generator.h/c       - Main biome generator
├── finders.h/c         - Structure position finding
├── util.h/c            - Utility functions
├── rng.h               - Random number generation
├── tables/             - Biome tree lookup tables
├── build.bat           - Build script
├── test.bat            - Test runner
└── tests/              - Test suite
```

## Building

See [BUILD.md](BUILD.md) for detailed build instructions.

## Testing

All functionality is tested:
- Noise generation (18 tests)
- Biome generation (all dimensions)
- Structure finding (31 tests)
- Utility functions (21 tests)
- Validation against original Cubiomes (12 tests)

Run tests: `test.bat`

## Differences from Original Cubiomes

**Removed**:
- All Beta through 1.17 support
- Layer system (replaced with BiomeNoise)
- Quad-base finder system
- Legacy structure configs
- Image generation utilities

**Simplified**:
- Single Generator structure (no LayerStack)
- Unified biome generation API
- Cleaner structure finding
- Modern C code style

**Kept**:
- 100% accurate biome generation
- All structure types
- Stronghold finding
- RNG compatibility
- Performance optimizations

## License

Same as original Cubiomes (MIT License).

## Credits

Based on [Cubiomes](https://github.com/Cubitect/cubiomes) by Cubitect.
Rebuilt and modernized for 1.18+ only.
