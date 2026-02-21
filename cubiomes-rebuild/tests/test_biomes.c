// Test suite for biome helper functions (1.18+ only)

#include "../cubiomes-rebuild/biomes.h"
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

void test_biome_existence()
{
    if (biomeExists(MC_1_18, plains) && 
        biomeExists(MC_1_18, ocean) &&
        biomeExists(MC_1_18, meadow) &&
        biomeExists(MC_1_18, frozen_peaks))
        test_pass("biomeExists - 1.18 biomes");
    else
        test_fail("biomeExists - 1.18 biomes", "Basic 1.18 biomes should exist");
    
    if (biomeExists(MC_1_19_2, deep_dark) &&
        biomeExists(MC_1_19_2, mangrove_swamp))
        test_pass("biomeExists - 1.19 biomes");
    else
        test_fail("biomeExists - 1.19 biomes", "1.19 biomes should exist in 1.19+");
    
    if (biomeExists(MC_1_21_1, cherry_grove))
        test_pass("biomeExists - 1.20 biomes");
    else
        test_fail("biomeExists - 1.20 biomes", "Cherry grove should exist in 1.20+");
    
    if (biomeExists(MC_1_21_WD, pale_garden))
        test_pass("biomeExists - 1.21 biomes");
    else
        test_fail("biomeExists - 1.21 biomes", "Pale garden should exist in 1.21+");
    
    if (!biomeExists(MC_1_18, cherry_grove) &&
        !biomeExists(MC_1_18, pale_garden))
        test_pass("biomeExists - version gating");
    else
        test_fail("biomeExists - version gating", "Future biomes should not exist in older versions");
}

void test_dimensions()
{
    if (getDimension(plains) == DIM_OVERWORLD &&
        getDimension(ocean) == DIM_OVERWORLD &&
        getDimension(meadow) == DIM_OVERWORLD)
        test_pass("getDimension - overworld");
    else
        test_fail("getDimension - overworld", "Overworld biomes should return DIM_OVERWORLD");
    
    if (getDimension(nether_wastes) == DIM_NETHER &&
        getDimension(crimson_forest) == DIM_NETHER &&
        getDimension(basalt_deltas) == DIM_NETHER)
        test_pass("getDimension - nether");
    else
        test_fail("getDimension - nether", "Nether biomes should return DIM_NETHER");
    
    if (getDimension(the_end) == DIM_END &&
        getDimension(small_end_islands) == DIM_END &&
        getDimension(end_highlands) == DIM_END)
        test_pass("getDimension - end");
    else
        test_fail("getDimension - end", "End biomes should return DIM_END");
    
    if (isOverworld(MC_1_18, plains) &&
        isOverworld(MC_1_18, ocean) &&
        !isOverworld(MC_1_18, nether_wastes) &&
        !isOverworld(MC_1_18, the_end))
        test_pass("isOverworld");
    else
        test_fail("isOverworld", "isOverworld should correctly identify overworld biomes");
}

void test_oceanic()
{
    if (isOceanic(ocean) &&
        isOceanic(frozen_ocean) &&
        isOceanic(warm_ocean) &&
        isOceanic(lukewarm_ocean) &&
        isOceanic(cold_ocean))
        test_pass("isOceanic - shallow oceans");
    else
        test_fail("isOceanic - shallow oceans", "Shallow oceans should be oceanic");
    
    if (isOceanic(deep_ocean) &&
        isOceanic(deep_warm_ocean) &&
        isOceanic(deep_lukewarm_ocean) &&
        isOceanic(deep_cold_ocean) &&
        isOceanic(deep_frozen_ocean))
        test_pass("isOceanic - deep oceans");
    else
        test_fail("isOceanic - deep oceans", "Deep oceans should be oceanic");
    
    if (!isOceanic(plains) &&
        !isOceanic(desert) &&
        !isOceanic(river))
        test_pass("isOceanic - non-oceans");
    else
        test_fail("isOceanic - non-oceans", "Non-ocean biomes should not be oceanic");
    
    if (isShallowOcean(ocean) &&
        isShallowOcean(warm_ocean) &&
        !isShallowOcean(deep_ocean) &&
        !isShallowOcean(plains))
        test_pass("isShallowOcean");
    else
        test_fail("isShallowOcean", "isShallowOcean should only match shallow oceans");
    
    if (isDeepOcean(deep_ocean) &&
        isDeepOcean(deep_frozen_ocean) &&
        !isDeepOcean(ocean) &&
        !isDeepOcean(plains))
        test_pass("isDeepOcean");
    else
        test_fail("isDeepOcean", "isDeepOcean should only match deep oceans");
}

void test_snowy()
{
    if (isSnowy(frozen_ocean) &&
        isSnowy(snowy_tundra) &&
        isSnowy(snowy_taiga) &&
        isSnowy(ice_spikes) &&
        isSnowy(frozen_peaks) &&
        isSnowy(grove))
        test_pass("isSnowy - snowy biomes");
    else
        test_fail("isSnowy - snowy biomes", "Snowy biomes should be identified");
    
    if (!isSnowy(plains) &&
        !isSnowy(desert) &&
        !isSnowy(jungle))
        test_pass("isSnowy - non-snowy biomes");
    else
        test_fail("isSnowy - non-snowy biomes", "Non-snowy biomes should not be snowy");
}

void test_mesa()
{
    if (isMesa(badlands) &&
        isMesa(wooded_badlands_plateau) &&
        isMesa(eroded_badlands) &&
        isMesa(modified_wooded_badlands_plateau))
        test_pass("isMesa - badlands variants");
    else
        test_fail("isMesa - badlands variants", "All badlands variants should be mesa");
    
    if (!isMesa(plains) &&
        !isMesa(desert) &&
        !isMesa(savanna))
        test_pass("isMesa - non-mesa biomes");
    else
        test_fail("isMesa - non-mesa biomes", "Non-mesa biomes should not be mesa");
}

void test_similarity()
{
    if (areSimilar(MC_1_18, plains, plains) &&
        areSimilar(MC_1_18, ocean, ocean))
        test_pass("areSimilar - identical biomes");
    else
        test_fail("areSimilar - identical biomes", "Identical biomes should be similar");
    
    if (areSimilar(MC_1_18, ocean, deep_ocean) &&
        areSimilar(MC_1_18, warm_ocean, cold_ocean) &&
        areSimilar(MC_1_18, deep_warm_ocean, deep_frozen_ocean))
        test_pass("areSimilar - oceanic biomes");
    else
        test_fail("areSimilar - oceanic biomes", "All oceanic biomes should be similar");
    
    if (areSimilar(MC_1_18, badlands, eroded_badlands) &&
        areSimilar(MC_1_18, badlands, wooded_badlands_plateau))
        test_pass("areSimilar - mesa biomes");
    else
        test_fail("areSimilar - mesa biomes", "All mesa biomes should be similar");
    
    if (!areSimilar(MC_1_18, plains, ocean) &&
        !areSimilar(MC_1_18, desert, forest))
        test_pass("areSimilar - different biomes");
    else
        test_fail("areSimilar - different biomes", "Different biome types should not be similar");
}

int main()
{
    printf("=== Biome Helper Function Tests ===\n\n");
    
    test_biome_existence();
    test_dimensions();
    test_oceanic();
    test_snowy();
    test_mesa();
    test_similarity();
    
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
