#ifndef BIOMES_REBUILD_H_
#define BIOMES_REBUILD_H_

#include <stdint.h>

/* Minecraft versions - 1.21+ ONLY */
enum MCVersion
{
    MC_UNDEF,
    MC_1_18,     // 1.18 - 1.18.2
    MC_1_19_2,   // 1.19.2 - 1.19.3
    MC_1_19_4,   // 1.19.4 - 1.20.5
    MC_1_20_6,   // 1.20.6 - 1.21.0
    MC_1_21_1,
    MC_1_21_3,
    MC_1_21_WD,  // Winter Drop
    MC_1_21 = MC_1_21_WD,
    MC_NEWEST = MC_1_21,
};

/* Dimensions */
enum Dimension
{
    DIM_NETHER      = -1,
    DIM_OVERWORLD   =  0,
    DIM_END         = +1,
    DIM_UNDEF       = 1000,
};

/* Biome IDs - 1.18+ ONLY (no legacy variants) */
enum BiomeID
{
    none = -1,
    
    // Base biomes (0-39)
    ocean = 0,
    plains,
    desert,
    mountains,                  // windswept_hills
    forest,
    taiga,
    swamp,
    river,
    nether_wastes,
    the_end,
    // 10
    frozen_ocean,
    frozen_river,
    snowy_tundra,               // snowy_plains
    snowy_mountains,            // removed in 1.18, kept for ID continuity
    mushroom_fields,
    mushroom_field_shore,       // removed in 1.18, kept for ID continuity
    beach,
    desert_hills,               // removed in 1.18, kept for ID continuity
    wooded_hills,               // removed in 1.18, kept for ID continuity
    taiga_hills,                // removed in 1.18, kept for ID continuity
    // 20
    mountain_edge,              // removed in 1.18, kept for ID continuity
    jungle,
    jungle_hills,               // removed in 1.18, kept for ID continuity
    jungle_edge,                // sparse_jungle
    deep_ocean,
    stone_shore,                // stony_shore
    snowy_beach,
    birch_forest,
    birch_forest_hills,         // removed in 1.18, kept for ID continuity
    dark_forest,
    // 30
    snowy_taiga,
    snowy_taiga_hills,          // removed in 1.18, kept for ID continuity
    giant_tree_taiga,           // old_growth_pine_taiga
    giant_tree_taiga_hills,     // removed in 1.18, kept for ID continuity
    wooded_mountains,           // windswept_forest
    savanna,
    savanna_plateau,
    badlands,                   // mesa
    wooded_badlands_plateau,    // wooded_badlands
    badlands_plateau,           // removed in 1.18, kept for ID continuity
    
    // 1.13+ ocean variants (40-50)
    small_end_islands = 40,
    end_midlands,
    end_highlands,
    end_barrens,
    warm_ocean,
    lukewarm_ocean,
    cold_ocean,
    deep_warm_ocean,
    deep_lukewarm_ocean,
    deep_cold_ocean,
    // 50
    deep_frozen_ocean,
    
    the_void = 127,
    
    // Mutated variants (128+)
    sunflower_plains                = plains + 128,
    desert_lakes                    = desert + 128,
    gravelly_mountains              = mountains + 128,         // windswept_gravelly_hills
    flower_forest                   = forest + 128,
    taiga_mountains                 = taiga + 128,
    swamp_hills                     = swamp + 128,
    ice_spikes                      = snowy_tundra + 128,
    modified_jungle                 = jungle + 128,
    modified_jungle_edge            = jungle_edge + 128,
    tall_birch_forest               = birch_forest + 128,      // old_growth_birch_forest
    tall_birch_hills                = birch_forest_hills + 128,
    dark_forest_hills               = dark_forest + 128,
    snowy_taiga_mountains           = snowy_taiga + 128,
    giant_spruce_taiga              = giant_tree_taiga + 128,  // old_growth_spruce_taiga
    giant_spruce_taiga_hills        = giant_tree_taiga_hills + 128,
    modified_gravelly_mountains     = wooded_mountains + 128,
    shattered_savanna               = savanna + 128,           // windswept_savanna
    shattered_savanna_plateau       = savanna_plateau + 128,
    eroded_badlands                 = badlands + 128,
    modified_wooded_badlands_plateau = wooded_badlands_plateau + 128,
    modified_badlands_plateau       = badlands_plateau + 128,
    
    // 1.14+
    bamboo_jungle                   = 168,
    bamboo_jungle_hills             = 169,
    
    // 1.16+ Nether
    soul_sand_valley                = 170,
    crimson_forest                  = 171,
    warped_forest                   = 172,
    basalt_deltas                   = 173,
    
    // 1.17+ Caves
    dripstone_caves                 = 174,
    lush_caves                      = 175,
    
    // 1.18+ Mountains
    meadow                          = 177,
    grove                           = 178,
    snowy_slopes                    = 179,
    jagged_peaks                    = 180,
    frozen_peaks                    = 181,
    stony_peaks                     = 182,
    
    // 1.19+
    deep_dark                       = 183,
    mangrove_swamp                  = 184,
    
    // 1.20+
    cherry_grove                    = 185,
    
    // 1.21+
    pale_garden                     = 186,
};

#ifdef __cplusplus
extern "C"
{
#endif

/* Biome helper functions */
int biomeExists(int mc, int id);
int isOverworld(int mc, int id);
int getDimension(int id);
int isOceanic(int id);
int isShallowOcean(int id);
int isDeepOcean(int id);
int isSnowy(int id);
int isMesa(int id);
int areSimilar(int mc, int id1, int id2);

#ifdef __cplusplus
}
#endif

#endif /* BIOMES_REBUILD_H_ */
