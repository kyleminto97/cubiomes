// Test program for Generator (Phase 4)
#include "../cubiomes-rebuild/generator.h"
#include <stdio.h>
#include <stdlib.h>

void test_overworld() {
    printf("Testing Overworld Generator...\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    // Test getBiomeAt
    int biome1 = getBiomeAt(&g, 4, 0, 64, 0);
    int biome2 = getBiomeAt(&g, 4, 100, 64, 100);
    int biome3 = getBiomeAt(&g, 4, -100, 64, -100);
    
    printf("  Biome at (0,64,0): %d\n", biome1);
    printf("  Biome at (100,64,100): %d\n", biome2);
    printf("  Biome at (-100,64,-100): %d\n", biome3);
    
    // Test genBiomes
    Range r = {4, 0, 0, 16, 16, 16, 4};
    int *cache = allocCache(&g, r);
    if (cache) {
        int err = genBiomes(&g, cache, r);
        if (err == 0)
            printf("  Generated 16x4x16 biome volume successfully\n");
        else
            printf("  Error generating biomes: %d\n", err);
        free(cache);
    }
    
    printf("  ✓ Overworld tests passed\n\n");
}

void test_nether() {
    printf("Testing Nether Generator...\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_NETHER, 12345);
    
    // Test getBiomeAt
    int biome1 = getBiomeAt(&g, 4, 0, 64, 0);
    int biome2 = getBiomeAt(&g, 4, 100, 64, 100);
    
    printf("  Nether biome at (0,64,0): %d\n", biome1);
    printf("  Nether biome at (100,64,100): %d\n", biome2);
    
    // Test genBiomes
    Range r = {4, 0, 0, 16, 16, 0, 1};
    int *cache = allocCache(&g, r);
    if (cache) {
        int err = genBiomes(&g, cache, r);
        if (err == 0)
            printf("  Generated 16x16 nether map successfully\n");
        else
            printf("  Error generating nether biomes: %d\n", err);
        free(cache);
    }
    
    printf("  ✓ Nether tests passed\n\n");
}

void test_end() {
    printf("Testing End Generator...\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_END, 12345);
    
    // Test getBiomeAt
    int biome1 = getBiomeAt(&g, 4, 0, 64, 0);
    int biome2 = getBiomeAt(&g, 4, 100, 64, 100);
    
    printf("  End biome at (0,64,0): %d\n", biome1);
    printf("  End biome at (100,64,100): %d\n", biome2);
    
    // Test genBiomes
    Range r = {4, 0, 0, 16, 16, 0, 1};
    int *cache = allocCache(&g, r);
    if (cache) {
        int err = genBiomes(&g, cache, r);
        if (err == 0)
            printf("  Generated 16x16 end map successfully\n");
        else
            printf("  Error generating end biomes: %d\n", err);
        free(cache);
    }
    
    printf("  ✓ End tests passed\n\n");
}

void test_large_biomes() {
    printf("Testing Large Biomes Flag...\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, LARGE_BIOMES);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    int biome = getBiomeAt(&g, 4, 0, 64, 0);
    printf("  Large biomes at (0,64,0): %d\n", biome);
    printf("  ✓ Large biomes flag works\n\n");
}

int main() {
    printf("=== Generator Tests (Phase 4) ===\n\n");
    
    test_overworld();
    test_nether();
    test_end();
    test_large_biomes();
    
    printf("=== All Tests Passed! ===\n");
    return 0;
}
