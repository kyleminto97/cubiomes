// Comparison test: Rebuild vs Original Cubiomes
// Tests that our rebuild produces identical results to the original library

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include original cubiomes
#include "../generator.h"
#include "../finders.h"

// Include rebuild (with different names to avoid conflicts)
#define Generator GeneratorRebuild
#define setupGenerator setupGeneratorRebuild
#define applySeed applySeedRebuild
#define getBiomeAt getBiomeAtRebuild
#define genBiomes genBiomesRebuild
#define getStructureConfig getStructureConfigRebuild
#define getStructurePos getStructurePosRebuild
#define isViableFeatureBiome isViableFeatureBiomeRebuild
#define Pos PosRebuild
#define StructureConfig StructureConfigRebuild

#include "../cubiomes-rebuild/generator.h"
#include "../cubiomes-rebuild/finders.h"

#undef Generator
#undef setupGenerator
#undef applySeed
#undef getBiomeAt
#undef genBiomes
#undef getStructureConfig
#undef getStructurePos
#undef isViableFeatureBiome
#undef Pos
#undef StructureConfig

int test_count = 0;
int pass_count = 0;

void test_pass(const char *name)
{
    test_count++;
    pass_count++;
    printf("Test %d: %s... PASS\n", test_count, name);
}

void test_fail(const char *name, const char *msg)
{
    test_count++;
    printf("Test %d: %s... FAIL: %s\n", test_count, name, msg);
}

void compare_biomes_at_position(uint64_t seed, int mc, int x, int z)
{
    char test_name[256];
    snprintf(test_name, sizeof(test_name), "Biome at (%d, %d) seed=%llu mc=%d", 
             x, z, (unsigned long long)seed, mc);
    
    // Original
    Generator g_orig;
    setupGenerator(&g_orig, mc, 0);
    applySeed(&g_orig, DIM_OVERWORLD, seed);
    int biome_orig = getBiomeAt(&g_orig, 1, x >> 2, 63 >> 2, z >> 2);
    
    // Rebuild
    GeneratorRebuild g_rebuild;
    setupGeneratorRebuild(&g_rebuild, mc, 0);
    applySeedRebuild(&g_rebuild, DIM_OVERWORLD, seed);
    int biome_rebuild = getBiomeAtRebuild(&g_rebuild, 1, x >> 2, 63 >> 2, z >> 2);
    
    if (biome_orig == biome_rebuild)
        test_pass(test_name);
    else
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Original=%d, Rebuild=%d", biome_orig, biome_rebuild);
        test_fail(test_name, msg);
    }
}

void compare_biome_area(uint64_t seed, int mc, int x, int z, int w, int h)
{
    char test_name[256];
    snprintf(test_name, sizeof(test_name), "Biome area (%d,%d) %dx%d seed=%llu", 
             x, z, w, h, (unsigned long long)seed);
    
    // Original
    Generator g_orig;
    setupGenerator(&g_orig, mc, 0);
    applySeed(&g_orig, DIM_OVERWORLD, seed);
    
    int *cache_orig = (int*)malloc(w * h * sizeof(int));
    Range r = {1, x, z, w, h, 63, 1};
    genBiomes(&g_orig, cache_orig, r);
    
    // Rebuild
    GeneratorRebuild g_rebuild;
    setupGeneratorRebuild(&g_rebuild, mc, 0);
    applySeedRebuild(&g_rebuild, DIM_OVERWORLD, seed);
    
    int *cache_rebuild = (int*)malloc(w * h * sizeof(int));
    genBiomesRebuild(&g_rebuild, cache_rebuild, r);
    
    // Compare
    int matches = 1;
    int first_mismatch_idx = -1;
    for (int i = 0; i < w * h; i++)
    {
        if (cache_orig[i] != cache_rebuild[i])
        {
            matches = 0;
            if (first_mismatch_idx == -1)
                first_mismatch_idx = i;
        }
    }
    
    if (matches)
        test_pass(test_name);
    else
    {
        char msg[256];
        int idx = first_mismatch_idx;
        snprintf(msg, sizeof(msg), "Mismatch at index %d: Original=%d, Rebuild=%d", 
                 idx, cache_orig[idx], cache_rebuild[idx]);
        test_fail(test_name, msg);
    }
    
    free(cache_orig);
    free(cache_rebuild);
}

void compare_structure_positions(uint64_t seed, int mc, int structType, int regX, int regZ)
{
    char test_name[256];
    const char *struct_names[] = {
        "Desert_Pyramid", "Jungle_Temple", "Swamp_Hut", "Igloo", "Village",
        "Ocean_Ruin", "Shipwreck", "Monument", "Mansion", "Outpost",
        "Ruined_Portal", "Ruined_Portal_N", "Ancient_City", "Treasure", "Mineshaft"
    };
    const char *name = (structType < 15) ? struct_names[structType] : "Unknown";
    snprintf(test_name, sizeof(test_name), "%s position at region (%d,%d) seed=%llu", 
             name, regX, regZ, (unsigned long long)seed);
    
    // Original
    Pos pos_orig;
    int valid_orig = getStructurePos(structType, mc, seed, regX, regZ, &pos_orig);
    
    // Rebuild
    PosRebuild pos_rebuild;
    int valid_rebuild = getStructurePosRebuild(structType, mc, seed, regX, regZ, &pos_rebuild);
    
    if (valid_orig != valid_rebuild)
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Validity mismatch: Original=%d, Rebuild=%d", 
                 valid_orig, valid_rebuild);
        test_fail(test_name, msg);
        return;
    }
    
    if (!valid_orig)
    {
        test_pass(test_name);
        return;
    }
    
    if (pos_orig.x == pos_rebuild.x && pos_orig.z == pos_rebuild.z)
        test_pass(test_name);
    else
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Position mismatch: Original=(%d,%d), Rebuild=(%d,%d)", 
                 pos_orig.x, pos_orig.z, pos_rebuild.x, pos_rebuild.z);
        test_fail(test_name, msg);
    }
}

void test_biome_generation()
{
    printf("\n--- Biome Generation Comparison ---\n");
    
    // Test various seeds and positions
    uint64_t seeds[] = {0, 12345, 67890, 123456789, 987654321};
    int positions[][2] = {{0, 0}, {100, 100}, {-100, -100}, {1000, 1000}, {-1000, -1000}};
    
    for (int s = 0; s < 5; s++)
    {
        for (int p = 0; p < 5; p++)
        {
            compare_biomes_at_position(seeds[s], MC_1_18, positions[p][0], positions[p][1]);
        }
    }
    
    // Test area generation
    compare_biome_area(12345, MC_1_18, 0, 0, 64, 64);
    compare_biome_area(67890, MC_1_18, 100, 100, 32, 32);
}

void test_nether_generation()
{
    printf("\n--- Nether Biome Comparison ---\n");
    
    uint64_t seed = 12345;
    
    // Original
    Generator g_orig;
    setupGenerator(&g_orig, MC_1_18, 0);
    applySeed(&g_orig, DIM_NETHER, seed);
    int biome_orig = getBiomeAt(&g_orig, 1, 0, 64 >> 2, 0);
    
    // Rebuild
    GeneratorRebuild g_rebuild;
    setupGeneratorRebuild(&g_rebuild, MC_1_18, 0);
    applySeedRebuild(&g_rebuild, DIM_NETHER, seed);
    int biome_rebuild = getBiomeAtRebuild(&g_rebuild, 1, 0, 64 >> 2, 0);
    
    if (biome_orig == biome_rebuild)
        test_pass("Nether biome at (0,0)");
    else
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Original=%d, Rebuild=%d", biome_orig, biome_rebuild);
        test_fail("Nether biome at (0,0)", msg);
    }
}

void test_end_generation()
{
    printf("\n--- End Biome Comparison ---\n");
    
    uint64_t seed = 12345;
    
    // Original
    Generator g_orig;
    setupGenerator(&g_orig, MC_1_18, 0);
    applySeed(&g_orig, DIM_END, seed);
    int biome_orig = getBiomeAt(&g_orig, 1, 0, 64 >> 2, 0);
    
    // Rebuild
    GeneratorRebuild g_rebuild;
    setupGeneratorRebuild(&g_rebuild, MC_1_18, 0);
    applySeedRebuild(&g_rebuild, DIM_END, seed);
    int biome_rebuild = getBiomeAtRebuild(&g_rebuild, 1, 0, 64 >> 2, 0);
    
    if (biome_orig == biome_rebuild)
        test_pass("End biome at (0,0)");
    else
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "Original=%d, Rebuild=%d", biome_orig, biome_rebuild);
        test_fail("End biome at (0,0)", msg);
    }
}

void test_structure_positions()
{
    printf("\n--- Structure Position Comparison ---\n");
    
    uint64_t seeds[] = {0, 12345, 67890};
    int regions[][2] = {{0, 0}, {1, 1}, {-1, -1}, {10, 10}};
    
    // Test various structure types
    int structures[] = {
        Desert_Pyramid, Jungle_Temple, Swamp_Hut, Igloo, Village,
        Monument, Mansion, Outpost
    };
    
    for (int s = 0; s < 3; s++)
    {
        for (int r = 0; r < 4; r++)
        {
            for (int st = 0; st < 8; st++)
            {
                compare_structure_positions(seeds[s], MC_1_18, structures[st], 
                                          regions[r][0], regions[r][1]);
            }
        }
    }
}

void test_biome_checks()
{
    printf("\n--- Biome Viability Comparison ---\n");
    
    int structures[] = {Desert_Pyramid, Jungle_Temple, Swamp_Hut, Village, Monument};
    int biomes[] = {desert, jungle, swamp, plains, ocean, forest};
    
    for (int st = 0; st < 5; st++)
    {
        for (int b = 0; b < 6; b++)
        {
            int orig = isViableFeatureBiome(MC_1_18, structures[st], biomes[b]);
            int rebuild = isViableFeatureBiomeRebuild(MC_1_18, structures[st], biomes[b]);
            
            char test_name[256];
            snprintf(test_name, sizeof(test_name), "Viability check struct=%d biome=%d", 
                     structures[st], biomes[b]);
            
            if (orig == rebuild)
                test_pass(test_name);
            else
            {
                char msg[256];
                snprintf(msg, sizeof(msg), "Original=%d, Rebuild=%d", orig, rebuild);
                test_fail(test_name, msg);
            }
        }
    }
}

int main()
{
    printf("=== Cubiomes Rebuild vs Original Comparison ===\n");
    printf("This test validates that the rebuild produces identical results\n");
    printf("to the original cubiomes library for 1.18+ versions.\n");
    
    test_biome_generation();
    test_nether_generation();
    test_end_generation();
    test_structure_positions();
    test_biome_checks();
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d\n", pass_count, test_count);
    
    if (pass_count == test_count) {
        printf("\n✓ SUCCESS: Rebuild matches original cubiomes!\n");
        return 0;
    } else {
        printf("\n✗ FAILURE: Some tests did not match.\n");
        return 1;
    }
}
