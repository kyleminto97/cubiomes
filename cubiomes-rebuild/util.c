// Cubiomes Rebuild - Utilities (1.18+ only)
// Minimal utility functions for string conversion and biome colors

#include "util.h"
#include "finders.h"
#include <string.h>

//==============================================================================
// Version String Conversion
//==============================================================================

const char* mc2str(int mc)
{
    switch (mc)
    {
    case MC_1_18:    return "1.18";
    case MC_1_19_2:  return "1.19.2";
    case MC_1_19_4:  return "1.19.4";
    case MC_1_20_6:  return "1.20.6";
    case MC_1_21_1:  return "1.21.1";
    case MC_1_21_3:  return "1.21.3";
    case MC_1_21_WD: return "1.21 WD";
    default:         return "Unknown";
    }
}

int str2mc(const char *s)
{
    if (!strcmp(s, "1.21"))     return MC_1_21;
    if (!strcmp(s, "1.21 WD"))  return MC_1_21_WD;
    if (!strcmp(s, "1.21.3"))   return MC_1_21_3;
    if (!strcmp(s, "1.21.2"))   return MC_1_21_3;
    if (!strcmp(s, "1.21.1"))   return MC_1_21_1;
    if (!strcmp(s, "1.20"))     return MC_1_20_6;
    if (!strcmp(s, "1.20.6"))   return MC_1_20_6;
    if (!strcmp(s, "1.19"))     return MC_1_19_4;
    if (!strcmp(s, "1.19.4"))   return MC_1_19_4;
    if (!strcmp(s, "1.19.2"))   return MC_1_19_2;
    if (!strcmp(s, "1.18"))     return MC_1_18;
    if (!strcmp(s, "1.18.2"))   return MC_1_18;
    return MC_UNDEF;
}

//==============================================================================
// Biome String Conversion (1.18+ naming)
//==============================================================================

const char *biome2str(int mc, int id)
{
    (void)mc; // All 1.18+ use same names
    
    switch (id)
    {
    case ocean: return "ocean";
    case plains: return "plains";
    case desert: return "desert";
    case mountains: return "windswept_hills";
    case forest: return "forest";
    case taiga: return "taiga";
    case swamp: return "swamp";
    case river: return "river";
    case nether_wastes: return "nether_wastes";
    case the_end: return "the_end";
    case frozen_ocean: return "frozen_ocean";
    case frozen_river: return "frozen_river";
    case snowy_tundra: return "snowy_plains";
    case mushroom_fields: return "mushroom_fields";
    case beach: return "beach";
    case jungle: return "jungle";
    case jungle_edge: return "sparse_jungle";
    case deep_ocean: return "deep_ocean";
    case stone_shore: return "stony_shore";
    case snowy_beach: return "snowy_beach";
    case birch_forest: return "birch_forest";
    case dark_forest: return "dark_forest";
    case snowy_taiga: return "snowy_taiga";
    case giant_tree_taiga: return "old_growth_pine_taiga";
    case wooded_mountains: return "windswept_forest";
    case savanna: return "savanna";
    case savanna_plateau: return "savanna_plateau";
    case badlands: return "badlands";
    case wooded_badlands_plateau: return "wooded_badlands";
    case small_end_islands: return "small_end_islands";
    case end_midlands: return "end_midlands";
    case end_highlands: return "end_highlands";
    case end_barrens: return "end_barrens";
    case warm_ocean: return "warm_ocean";
    case lukewarm_ocean: return "lukewarm_ocean";
    case cold_ocean: return "cold_ocean";
    case deep_warm_ocean: return "deep_warm_ocean";
    case deep_lukewarm_ocean: return "deep_lukewarm_ocean";
    case deep_cold_ocean: return "deep_cold_ocean";
    case deep_frozen_ocean: return "deep_frozen_ocean";
    case the_void: return "the_void";
    case sunflower_plains: return "sunflower_plains";
    case gravelly_mountains: return "windswept_gravelly_hills";
    case flower_forest: return "flower_forest";
    case ice_spikes: return "ice_spikes";
    case tall_birch_forest: return "old_growth_birch_forest";
    case giant_spruce_taiga: return "old_growth_spruce_taiga";
    case shattered_savanna: return "windswept_savanna";
    case eroded_badlands: return "eroded_badlands";
    case bamboo_jungle: return "bamboo_jungle";
    case soul_sand_valley: return "soul_sand_valley";
    case crimson_forest: return "crimson_forest";
    case warped_forest: return "warped_forest";
    case basalt_deltas: return "basalt_deltas";
    case dripstone_caves: return "dripstone_caves";
    case lush_caves: return "lush_caves";
    case meadow: return "meadow";
    case grove: return "grove";
    case snowy_slopes: return "snowy_slopes";
    case jagged_peaks: return "jagged_peaks";
    case frozen_peaks: return "frozen_peaks";
    case stony_peaks: return "stony_peaks";
    case deep_dark: return "deep_dark";
    case mangrove_swamp: return "mangrove_swamp";
    case cherry_grove: return "cherry_grove";
    case pale_garden: return "pale_garden";
    default: return "unknown";
    }
}

//==============================================================================
// Structure String Conversion
//==============================================================================

const char *struct2str(int stype)
{
    switch (stype)
    {
    case Desert_Pyramid: return "desert_pyramid";
    case Jungle_Temple: return "jungle_temple";
    case Swamp_Hut: return "swamp_hut";
    case Igloo: return "igloo";
    case Village: return "village";
    case Ocean_Ruin: return "ocean_ruin";
    case Shipwreck: return "shipwreck";
    case Monument: return "monument";
    case Mansion: return "mansion";
    case Outpost: return "outpost";
    case Ruined_Portal: return "ruined_portal";
    case Ruined_Portal_N: return "ruined_portal_nether";
    case Ancient_City: return "ancient_city";
    case Treasure: return "buried_treasure";
    case Mineshaft: return "mineshaft";
    case Desert_Well: return "desert_well";
    case Geode: return "geode";
    case Fortress: return "fortress";
    case Bastion: return "bastion_remnant";
    case End_City: return "end_city";
    case End_Gateway: return "end_gateway";
    case End_Island: return "end_island";
    case Trail_Ruins: return "trail_ruins";
    case Trial_Chambers: return "trial_chambers";
    default: return "unknown";
    }
}

//==============================================================================
// Biome Colors (Default Minecraft-like palette)
//==============================================================================

static const unsigned char g_biome_colors[256][3] = {
    [ocean]                 = {  0,   0, 112},
    [plains]                = {141, 179,  96},
    [desert]                = {250, 148,  24},
    [mountains]             = { 96,  96,  96},
    [forest]                = {  5, 102,  33},
    [taiga]                 = { 11, 102,  89},
    [swamp]                 = {  7, 249, 178},
    [river]                 = {  0,   0, 255},
    [nether_wastes]         = {191,  59,  59},
    [the_end]               = {128, 128, 255},
    [frozen_ocean]          = {112, 112, 214},
    [frozen_river]          = {160, 160, 255},
    [snowy_tundra]          = {255, 255, 255},
    [snowy_mountains]       = {160, 160, 160},
    [mushroom_fields]       = {255,   0, 255},
    [mushroom_field_shore]  = {160,   0, 255},
    [beach]                 = {250, 222,  85},
    [desert_hills]          = {210,  95,  18},
    [wooded_hills]          = { 34,  85,  28},
    [taiga_hills]           = { 22,  57,  51},
    [mountain_edge]         = {114, 120, 154},
    [jungle]                = { 83, 123,   9},
    [jungle_hills]          = { 44,  66,   5},
    [jungle_edge]           = { 98, 139,  23},
    [deep_ocean]            = {  0,   0,  48},
    [stone_shore]           = {162, 162, 132},
    [snowy_beach]           = {250, 240, 192},
    [birch_forest]          = { 48, 116,  68},
    [birch_forest_hills]    = { 31,  95,  50},
    [dark_forest]           = { 64,  81,  26},
    [snowy_taiga]           = { 49,  85,  74},
    [snowy_taiga_hills]     = { 36,  63,  54},
    [giant_tree_taiga]      = { 89, 102,  81},
    [giant_tree_taiga_hills]= { 69,  79,  62},
    [wooded_mountains]      = { 80, 112,  80},
    [savanna]               = {189, 178,  95},
    [savanna_plateau]       = {167, 157, 100},
    [badlands]              = {217,  69,  21},
    [wooded_badlands_plateau]={176,  151,  101},
    [badlands_plateau]      = {202,  140,  101},
    [small_end_islands]     = {128, 128, 255},
    [end_midlands]          = {128, 128, 200},
    [end_highlands]         = {128, 128, 150},
    [end_barrens]           = {128, 128, 100},
    [warm_ocean]            = {  0,   0, 172},
    [lukewarm_ocean]        = {  0,   0, 144},
    [cold_ocean]            = { 32,  32, 112},
    [deep_warm_ocean]       = {  0,   0,  80},
    [deep_lukewarm_ocean]   = {  0,   0,  64},
    [deep_cold_ocean]       = { 32,  32,  56},
    [deep_frozen_ocean]     = { 64,  64, 144},
    [the_void]              = {  0,   0,   0},
    [sunflower_plains]      = {181, 219, 136},
    [desert_lakes]          = {255, 188,  64},
    [gravelly_mountains]    = {136, 136, 136},
    [flower_forest]         = { 45, 142,  73},
    [taiga_mountains]       = { 51, 142, 129},
    [swamp_hills]           = { 47, 255, 218},
    [ice_spikes]            = {180, 220, 220},
    [modified_jungle]       = {123, 163,  49},
    [modified_jungle_edge]  = {138, 179,  63},
    [tall_birch_forest]     = { 88, 156, 108},
    [tall_birch_hills]      = { 71, 135,  90},
    [dark_forest_hills]     = {104, 121,  66},
    [snowy_taiga_mountains] = { 89, 125, 114},
    [giant_spruce_taiga]    = {129, 142, 121},
    [giant_spruce_taiga_hills]={109, 119,  102},
    [modified_gravelly_mountains]={120, 152, 120},
    [shattered_savanna]     = {229, 218, 135},
    [shattered_savanna_plateau]={207, 197, 140},
    [eroded_badlands]       = {255, 109,  61},
    [modified_wooded_badlands_plateau]={216, 191, 141},
    [modified_badlands_plateau]={242, 180, 141},
    [bamboo_jungle]         = {118, 142,  20},
    [bamboo_jungle_hills]   = { 59,  71,  10},
    [soul_sand_valley]      = { 94,  56,  48},
    [crimson_forest]        = {221,   8,   8},
    [warped_forest]         = { 73, 144, 123},
    [basalt_deltas]         = {104,  95, 112},
    [dripstone_caves]       = {124,  93,  67},
    [lush_caves]            = {172, 225, 119},
    [meadow]                = {131, 204, 139},
    [grove]                 = {139, 213, 204},
    [snowy_slopes]          = {226, 234, 241},
    [jagged_peaks]          = {232, 245, 253},
    [frozen_peaks]          = {250, 250, 250},
    [stony_peaks]           = {149, 155, 159},
    [deep_dark]             = {  0,   0,   0},
    [mangrove_swamp]        = {  7, 249, 178},
    [cherry_grove]          = {255, 192, 203},
    [pale_garden]           = {200, 220, 200},
};

void initBiomeColors(unsigned char biomeColors[256][3])
{
    memcpy(biomeColors, g_biome_colors, sizeof(g_biome_colors));
}

const unsigned char* getBiomeColor(int id)
{
    if (id < 0 || id >= 256)
        return g_biome_colors[the_void];
    return g_biome_colors[id];
}
