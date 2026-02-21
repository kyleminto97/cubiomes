// Test suite for utility functions

#include "../cubiomes-rebuild/util.h"
#include "../cubiomes-rebuild/finders.h"
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

void test_version_conversion()
{
    // Test mc2str
    if (strcmp(mc2str(MC_1_18), "1.18") == 0)
        test_pass("mc2str - MC_1_18");
    else
        test_fail("mc2str - MC_1_18", "Wrong string");
    
    if (strcmp(mc2str(MC_1_21_WD), "1.21 WD") == 0)
        test_pass("mc2str - MC_1_21_WD");
    else
        test_fail("mc2str - MC_1_21_WD", "Wrong string");
    
    // Test str2mc
    if (str2mc("1.18") == MC_1_18)
        test_pass("str2mc - 1.18");
    else
        test_fail("str2mc - 1.18", "Wrong enum");
    
    if (str2mc("1.21") == MC_1_21)
        test_pass("str2mc - 1.21");
    else
        test_fail("str2mc - 1.21", "Wrong enum");
    
    if (str2mc("invalid") == MC_UNDEF)
        test_pass("str2mc - invalid returns MC_UNDEF");
    else
        test_fail("str2mc - invalid", "Should return MC_UNDEF");
}

void test_biome_names()
{
    if (strcmp(biome2str(MC_1_18, plains), "plains") == 0)
        test_pass("biome2str - plains");
    else
        test_fail("biome2str - plains", "Wrong name");
    
    if (strcmp(biome2str(MC_1_18, ocean), "ocean") == 0)
        test_pass("biome2str - ocean");
    else
        test_fail("biome2str - ocean", "Wrong name");
    
    if (strcmp(biome2str(MC_1_18, mountains), "windswept_hills") == 0)
        test_pass("biome2str - windswept_hills (renamed)");
    else
        test_fail("biome2str - windswept_hills", "Wrong name");
    
    if (strcmp(biome2str(MC_1_18, snowy_tundra), "snowy_plains") == 0)
        test_pass("biome2str - snowy_plains (renamed)");
    else
        test_fail("biome2str - snowy_plains", "Wrong name");
    
    if (strcmp(biome2str(MC_1_18, cherry_grove), "cherry_grove") == 0)
        test_pass("biome2str - cherry_grove (1.20+)");
    else
        test_fail("biome2str - cherry_grove", "Wrong name");
    
    if (strcmp(biome2str(MC_1_18, pale_garden), "pale_garden") == 0)
        test_pass("biome2str - pale_garden (1.21+)");
    else
        test_fail("biome2str - pale_garden", "Wrong name");
}

void test_structure_names()
{
    if (strcmp(struct2str(Desert_Pyramid), "desert_pyramid") == 0)
        test_pass("struct2str - desert_pyramid");
    else
        test_fail("struct2str - desert_pyramid", "Wrong name");
    
    if (strcmp(struct2str(Village), "village") == 0)
        test_pass("struct2str - village");
    else
        test_fail("struct2str - village", "Wrong name");
    
    if (strcmp(struct2str(Ancient_City), "ancient_city") == 0)
        test_pass("struct2str - ancient_city");
    else
        test_fail("struct2str - ancient_city", "Wrong name");
    
    if (strcmp(struct2str(Trial_Chambers), "trial_chambers") == 0)
        test_pass("struct2str - trial_chambers");
    else
        test_fail("struct2str - trial_chambers", "Wrong name");
}

void test_biome_colors()
{
    unsigned char colors[256][3];
    initBiomeColors(colors);
    
    // Check that some colors are set
    if (colors[plains][0] != 0 || colors[plains][1] != 0 || colors[plains][2] != 0)
        test_pass("initBiomeColors - plains has color");
    else
        test_fail("initBiomeColors - plains", "Color is black");
    
    if (colors[ocean][0] != 0 || colors[ocean][1] != 0 || colors[ocean][2] != 0)
        test_pass("initBiomeColors - ocean has color");
    else
        test_fail("initBiomeColors - ocean", "Color is black");
    
    // Test getBiomeColor
    const unsigned char *color = getBiomeColor(plains);
    if (color[0] == colors[plains][0] && 
        color[1] == colors[plains][1] && 
        color[2] == colors[plains][2])
        test_pass("getBiomeColor - plains matches");
    else
        test_fail("getBiomeColor - plains", "Color mismatch");
    
    // Test invalid biome ID
    color = getBiomeColor(999);
    if (color != NULL)
        test_pass("getBiomeColor - invalid ID returns fallback");
    else
        test_fail("getBiomeColor - invalid ID", "Returned NULL");
    
    // Check specific colors are reasonable (RGB values 0-255)
    int valid_colors = 1;
    for (int i = 0; i < 256; i++)
    {
        if (colors[i][0] > 255 || colors[i][1] > 255 || colors[i][2] > 255)
        {
            valid_colors = 0;
            break;
        }
    }
    
    if (valid_colors)
        test_pass("initBiomeColors - all colors in valid range");
    else
        test_fail("initBiomeColors - colors", "Some colors out of range");
}

void test_color_variety()
{
    // Check that different biomes have different colors
    const unsigned char *plains_color = getBiomeColor(plains);
    const unsigned char *ocean_color = getBiomeColor(ocean);
    const unsigned char *desert_color = getBiomeColor(desert);
    
    int all_different = 1;
    if (plains_color[0] == ocean_color[0] && 
        plains_color[1] == ocean_color[1] && 
        plains_color[2] == ocean_color[2])
        all_different = 0;
    
    if (plains_color[0] == desert_color[0] && 
        plains_color[1] == desert_color[1] && 
        plains_color[2] == desert_color[2])
        all_different = 0;
    
    if (ocean_color[0] == desert_color[0] && 
        ocean_color[1] == desert_color[1] && 
        ocean_color[2] == desert_color[2])
        all_different = 0;
    
    if (all_different)
        test_pass("Color variety - different biomes have different colors");
    else
        test_fail("Color variety", "Some biomes have identical colors");
}

int main()
{
    printf("=== Utility Function Tests ===\n\n");
    
    test_version_conversion();
    test_biome_names();
    test_structure_names();
    test_biome_colors();
    test_color_variety();
    
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
