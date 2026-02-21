// Test suite for structure finders (1.18+ only)

#include "../cubiomes-rebuild/finders.h"
#include "../cubiomes-rebuild/generator.h"
#include <stdio.h>
#include <string.h>

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

void test_structure_configs()
{
    StructureConfig sconf;
    
    if (getStructureConfig(Desert_Pyramid, MC_1_18, &sconf) &&
        sconf.salt == 14357617 &&
        sconf.regionSize == 32 &&
        sconf.chunkRange == 24)
        test_pass("getStructureConfig - Desert Pyramid");
    else
        test_fail("getStructureConfig - Desert Pyramid", "Config mismatch");
    
    if (getStructureConfig(Village, MC_1_18, &sconf) &&
        sconf.regionSize == 34 &&
        sconf.chunkRange == 26)
        test_pass("getStructureConfig - Village");
    else
        test_fail("getStructureConfig - Village", "Config mismatch");
    
    if (getStructureConfig(Ancient_City, MC_1_19_2, &sconf))
        test_pass("getStructureConfig - Ancient City (1.19+)");
    else
        test_fail("getStructureConfig - Ancient City (1.19+)", "Should exist in 1.19+");
    
    if (!getStructureConfig(Ancient_City, MC_1_18, &sconf))
        test_pass("getStructureConfig - Ancient City version gate");
    else
        test_fail("getStructureConfig - Ancient City version gate", "Should not exist in 1.18");
    
    if (getStructureConfig(Trail_Ruins, MC_1_21_1, &sconf))
        test_pass("getStructureConfig - Trail Ruins (1.20+)");
    else
        test_fail("getStructureConfig - Trail Ruins (1.20+)", "Should exist in 1.20+");
    
    if (getStructureConfig(Trial_Chambers, MC_1_21_1, &sconf))
        test_pass("getStructureConfig - Trial Chambers (1.21+)");
    else
        test_fail("getStructureConfig - Trial Chambers (1.21+)", "Should exist in 1.21+");
}

void test_structure_positions()
{
    uint64_t seed = 12345;
    Pos pos;
    
    if (getStructurePos(Desert_Pyramid, MC_1_18, seed, 0, 0, &pos))
        test_pass("getStructurePos - Desert Pyramid at (0,0)");
    else
        test_fail("getStructurePos - Desert Pyramid at (0,0)", "Failed to get position");
    
    if (getStructurePos(Village, MC_1_18, seed, 1, 1, &pos))
        test_pass("getStructurePos - Village at (1,1)");
    else
        test_fail("getStructurePos - Village at (1,1)", "Failed to get position");
    
    if (getStructurePos(Monument, MC_1_18, seed, 0, 0, &pos))
        test_pass("getStructurePos - Monument (large structure)");
    else
        test_fail("getStructurePos - Monument (large structure)", "Failed to get position");
    
    if (getStructurePos(Fortress, MC_1_18, seed, 0, 0, &pos))
        test_pass("getStructurePos - Fortress (Nether)");
    else
        test_fail("getStructurePos - Fortress (Nether)", "Failed to get position");
}

void test_inline_functions()
{
    StructureConfig sconf;
    getStructureConfig(Desert_Pyramid, MC_1_18, &sconf);
    
    Pos pos = getFeatureChunkInRegion(sconf, 12345, 0, 0);
    if (pos.x >= 0 && pos.x < 24 && pos.z >= 0 && pos.z < 24)
        test_pass("getFeatureChunkInRegion - in range");
    else
        test_fail("getFeatureChunkInRegion - in range", "Position out of range");
    
    pos = getFeaturePos(sconf, 12345, 0, 0);
    if (pos.x >= 0 && pos.z >= 0)
        test_pass("getFeaturePos - valid position");
    else
        test_fail("getFeaturePos - valid position", "Invalid position");
    
    getStructureConfig(Monument, MC_1_18, &sconf);
    pos = getLargeStructureChunkInRegion(sconf, 12345, 0, 0);
    if (pos.x >= 0 && pos.z >= 0)
        test_pass("getLargeStructureChunkInRegion - valid");
    else
        test_fail("getLargeStructureChunkInRegion - valid", "Invalid position");
    
    pos = getLargeStructurePos(sconf, 12345, 0, 0);
    if (pos.x >= 0 && pos.z >= 0)
        test_pass("getLargeStructurePos - valid");
    else
        test_fail("getLargeStructurePos - valid", "Invalid position");
}

void test_slime_chunks()
{
    uint64_t seed = 12345;
    
    // Test a few chunks
    int slime_count = 0;
    for (int x = 0; x < 10; x++)
    {
        for (int z = 0; z < 10; z++)
        {
            if (isSlimeChunk(seed, x, z))
                slime_count++;
        }
    }
    
    // Should be roughly 10% (10 out of 100)
    if (slime_count >= 5 && slime_count <= 15)
        test_pass("isSlimeChunk - distribution");
    else
        test_fail("isSlimeChunk - distribution", "Unexpected distribution");
}

void test_mineshafts()
{
    uint64_t seed = 12345;
    Pos out[100];
    
    int n = getMineshafts(MC_1_18, seed, 0, 0, 10, 10, out, 100);
    
    if (n >= 0 && n < 10)
        test_pass("getMineshafts - reasonable count");
    else
        test_fail("getMineshafts - reasonable count", "Unexpected count");
}

void test_end_islands()
{
    uint64_t seed = 12345;
    EndIsland islands[2];
    
    int n = getEndIslands(islands, MC_1_18, seed, 0, 0);
    
    if (n >= 0 && n <= 2)
        test_pass("getEndIslands - valid count");
    else
        test_fail("getEndIslands - valid count", "Invalid count");
}

void test_strongholds()
{
    uint64_t seed = 12345;
    StrongholdIter sh;
    
    Pos first = initFirstStronghold(&sh, MC_1_18, seed);
    
    if (first.x != 0 || first.z != 0)
        test_pass("initFirstStronghold - non-zero position");
    else
        test_fail("initFirstStronghold - non-zero position", "Position at origin");
    
    if (sh.mc == MC_1_18 && sh.index == 0)
        test_pass("initFirstStronghold - iterator initialized");
    else
        test_fail("initFirstStronghold - iterator initialized", "Iterator not initialized");
    
    // Test iteration without biome check
    int remaining = nextStronghold(&sh, NULL);
    if (remaining == 127 && sh.index == 1)
        test_pass("nextStronghold - iteration");
    else
        test_fail("nextStronghold - iteration", "Iteration failed");
}

void test_biome_viability()
{
    if (isViableFeatureBiome(MC_1_18, Desert_Pyramid, desert))
        test_pass("isViableFeatureBiome - Desert Pyramid in desert");
    else
        test_fail("isViableFeatureBiome - Desert Pyramid in desert", "Should be viable");
    
    if (!isViableFeatureBiome(MC_1_18, Desert_Pyramid, plains))
        test_pass("isViableFeatureBiome - Desert Pyramid not in plains");
    else
        test_fail("isViableFeatureBiome - Desert Pyramid not in plains", "Should not be viable");
    
    if (isViableFeatureBiome(MC_1_18, Jungle_Temple, jungle))
        test_pass("isViableFeatureBiome - Jungle Temple in jungle");
    else
        test_fail("isViableFeatureBiome - Jungle Temple in jungle", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_18, Swamp_Hut, swamp))
        test_pass("isViableFeatureBiome - Swamp Hut in swamp");
    else
        test_fail("isViableFeatureBiome - Swamp Hut in swamp", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_18, Igloo, snowy_tundra))
        test_pass("isViableFeatureBiome - Igloo in snowy tundra");
    else
        test_fail("isViableFeatureBiome - Igloo in snowy tundra", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_18, Village, plains))
        test_pass("isViableFeatureBiome - Village in plains");
    else
        test_fail("isViableFeatureBiome - Village in plains", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_18, Monument, ocean))
        test_pass("isViableFeatureBiome - Monument in ocean");
    else
        test_fail("isViableFeatureBiome - Monument in ocean", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_19_2, Ancient_City, deep_dark))
        test_pass("isViableFeatureBiome - Ancient City in deep dark");
    else
        test_fail("isViableFeatureBiome - Ancient City in deep dark", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_18, Fortress, nether_wastes))
        test_pass("isViableFeatureBiome - Fortress in nether");
    else
        test_fail("isViableFeatureBiome - Fortress in nether", "Should be viable");
    
    if (isViableFeatureBiome(MC_1_18, End_City, end_highlands))
        test_pass("isViableFeatureBiome - End City in end highlands");
    else
        test_fail("isViableFeatureBiome - End City in end highlands", "Should be viable");
}

void test_structure_viability()
{
    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    // Just test that the function runs without crashing
    // Actual biome checks would require full generator setup
    int viable = isViableStructurePos(Desert_Pyramid, &g, 0, 0, 0);
    (void)viable; // Result depends on actual biome at (0,0)
    
    test_pass("isViableStructurePos - runs without error");
}

int main()
{
    printf("=== Structure Finder Tests (1.18+) ===\n\n");
    
    test_structure_configs();
    test_structure_positions();
    test_inline_functions();
    test_slime_chunks();
    test_mineshafts();
    test_end_islands();
    test_strongholds();
    test_biome_viability();
    test_structure_viability();
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d\n", pass_count, test_count);
    
    if (pass_count == test_count) {
        printf("All tests passed!\n");
        return 0;
    } else {
        printf("Some tests failed.\n");
        return 1;
    }
}
