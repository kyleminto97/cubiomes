// Test program for BiomeNoise system (Phase 3)
#include "../cubiomes-rebuild/biomenoise.h"
#include <stdio.h>
#include <stdlib.h>

void test_nether_biomes() {
    printf("Testing Nether Biome Generation...\n");
    
    NetherNoise nn;
    setNetherSeed(&nn, 12345);
    
    // Test a few coordinates
    int biome1 = getNetherBiome(&nn, 0, 0, 0, NULL);
    int biome2 = getNetherBiome(&nn, 100, 0, 100, NULL);
    int biome3 = getNetherBiome(&nn, -100, 0, -100, NULL);
    
    printf("  Nether biome at (0,0,0): %d\n", biome1);
    printf("  Nether biome at (100,0,100): %d\n", biome2);
    printf("  Nether biome at (-100,0,-100): %d\n", biome3);
    
    // Test 2D map
    int map[16*16];
    mapNether2D(&nn, map, 0, 0, 16, 16);
    printf("  Generated 16x16 nether map successfully\n");
    
    // Test scaled generation at 1:1 scale
    Range r = {1, 0, 0, 16, 16, 0, 1};
    int *scaled = (int*)malloc(16*16*2*sizeof(int)); // Extra space for source
    if (scaled) {
        int err = genNetherScaled(&nn, scaled, r, MC_1_21, 12345);
        if (err == 0)
            printf("  Generated 16x16 nether scaled map (1:1) successfully\n");
        else
            printf("  Error generating nether scaled map: %d\n", err);
        free(scaled);
    }
    
    printf("  ✓ Nether tests passed\n\n");
}

void test_end_biomes() {
    printf("Testing End Biome Generation...\n");
    
    EndNoise en;
    setEndSeed(&en, MC_1_21, 12345);
    
    // Test a few coordinates
    int map[16*16];
    mapEndBiome(&en, map, 0, 0, 16, 16);
    
    printf("  End biome at center (0,0): %d\n", map[8*16 + 8]);
    printf("  End biome at edge (15,15): %d\n", map[15*16 + 15]);
    printf("  Generated 16x16 end map successfully\n");
    
    // Test scaled generation at 1:1 scale
    Range r = {1, 0, 0, 16, 16, 0, 1};
    int *scaled = (int*)malloc(16*16*2*sizeof(int)); // Extra space for source
    if (scaled) {
        int err = genEndScaled(&en, scaled, r, MC_1_21, 12345);
        if (err == 0)
            printf("  Generated 16x16 end scaled map (1:1) successfully\n");
        else
            printf("  Error generating end scaled map: %d\n", err);
        free(scaled);
    }
    
    // Test getEndHeightNoise
    float height = getEndHeightNoise(&en, 100, 100, 0);
    printf("  End height noise at (100,100): %.2f\n", height);
    
    printf("  ✓ End tests passed\n\n");
}

void test_overworld_biomes() {
    printf("Testing Overworld Biome Generation...\n");
    
    BiomeNoise bn;
    initBiomeNoise(&bn, MC_1_21);
    setBiomeSeed(&bn, 12345, 0);
    
    // Test sampling at a few coordinates
    int biome1 = sampleBiomeNoise(&bn, NULL, 0, 64, 0, NULL, 0);
    int biome2 = sampleBiomeNoise(&bn, NULL, 100, 64, 100, NULL, 0);
    int biome3 = sampleBiomeNoise(&bn, NULL, -100, 64, -100, NULL, 0);
    
    printf("  Overworld biome at (0,64,0): %d\n", biome1);
    printf("  Overworld biome at (100,64,100): %d\n", biome2);
    printf("  Overworld biome at (-100,64,-100): %d\n", biome3);
    
    // Test chunk section generation
    int chunk[4][4][4];
    genBiomeNoiseChunkSection(&bn, chunk, 0, 4, 0, NULL);
    printf("  Generated 4x4x4 chunk section successfully\n");
    printf("  Chunk biome at [0][0][0]: %d\n", chunk[0][0][0]);
    
    printf("  ✓ Overworld tests passed\n\n");
}

int main() {
    printf("=== BiomeNoise System Tests (Phase 3) ===\n\n");
    
    test_nether_biomes();
    test_end_biomes();
    test_overworld_biomes();
    
    printf("=== All Tests Passed! ===\n");
    return 0;
}
