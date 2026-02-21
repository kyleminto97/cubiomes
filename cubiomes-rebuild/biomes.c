// Cubiomes Rebuild - Biome Helper Functions (1.18+ only)
// Rebuilt from original cubiomes, keeping only 1.18+ functionality

#include "biomes.h"

//==============================================================================
// Biome Existence and Dimension
//==============================================================================

int biomeExists(int mc, int id)
{
    // Version-specific biomes
    if (id == pale_garden) return mc >= MC_1_21_WD;
    if (id == cherry_grove) return mc >= MC_1_21_1;
    if (id == deep_dark || id == mangrove_swamp) return mc >= MC_1_19_2;
    
    // Nether biomes (1.16+)
    if (id >= nether_wastes && id <= basalt_deltas) return 1;
    
    // End biomes (1.9+)
    if (id >= small_end_islands && id <= end_barrens) return 1;
    if (id == the_end) return 1;
    
    // 1.18+ overworld biomes
    switch (id)
    {
    case ocean:
    case plains:
    case desert:
    case mountains:
    case forest:
    case taiga:
    case swamp:
    case river:
    case frozen_ocean:
    case frozen_river:
    case snowy_tundra:
    case snowy_mountains:
    case mushroom_fields:
    case mushroom_field_shore:
    case beach:
    case desert_hills:
    case wooded_hills:
    case taiga_hills:
    case mountain_edge:
    case jungle:
    case jungle_hills:
    case jungle_edge:
    case deep_ocean:
    case stone_shore:
    case snowy_beach:
    case birch_forest:
    case birch_forest_hills:
    case dark_forest:
    case snowy_taiga:
    case snowy_taiga_hills:
    case giant_tree_taiga:
    case giant_tree_taiga_hills:
    case wooded_mountains:
    case savanna:
    case savanna_plateau:
    case badlands:
    case wooded_badlands_plateau:
    case badlands_plateau:
    case warm_ocean:
    case lukewarm_ocean:
    case cold_ocean:
    case deep_warm_ocean:
    case deep_lukewarm_ocean:
    case deep_cold_ocean:
    case deep_frozen_ocean:
    case sunflower_plains:
    case gravelly_mountains:
    case flower_forest:
    case ice_spikes:
    case tall_birch_forest:
    case giant_spruce_taiga:
    case shattered_savanna:
    case modified_wooded_badlands_plateau:
    case eroded_badlands:
    case bamboo_jungle:
    case soul_sand_valley:
    case crimson_forest:
    case warped_forest:
    case basalt_deltas:
    case dripstone_caves:
    case lush_caves:
    case meadow:
    case grove:
    case snowy_slopes:
    case frozen_peaks:
    case jagged_peaks:
    case stony_peaks:
        return 1;
    default:
        return 0;
    }
}

int isOverworld(int mc, int id)
{
    if (!biomeExists(mc, id))
        return 0;
    return getDimension(id) == DIM_OVERWORLD;
}

int getDimension(int id)
{
    if (id >= small_end_islands && id <= end_barrens) return DIM_END;
    if (id == the_end) return DIM_END;
    if (id >= nether_wastes && id <= basalt_deltas) return DIM_NETHER;
    return DIM_OVERWORLD;
}

//==============================================================================
// Biome Categories
//==============================================================================

int isOceanic(int id)
{
    switch (id)
    {
    case ocean:
    case frozen_ocean:
    case deep_ocean:
    case warm_ocean:
    case lukewarm_ocean:
    case cold_ocean:
    case deep_warm_ocean:
    case deep_lukewarm_ocean:
    case deep_cold_ocean:
    case deep_frozen_ocean:
        return 1;
    default:
        return 0;
    }
}

int isShallowOcean(int id)
{
    switch (id)
    {
    case ocean:
    case frozen_ocean:
    case warm_ocean:
    case lukewarm_ocean:
    case cold_ocean:
        return 1;
    default:
        return 0;
    }
}

int isDeepOcean(int id)
{
    switch (id)
    {
    case deep_ocean:
    case deep_warm_ocean:
    case deep_lukewarm_ocean:
    case deep_cold_ocean:
    case deep_frozen_ocean:
        return 1;
    default:
        return 0;
    }
}

int isSnowy(int id)
{
    switch (id)
    {
    case frozen_ocean:
    case frozen_river:
    case snowy_tundra:
    case snowy_mountains:
    case snowy_beach:
    case snowy_taiga:
    case snowy_taiga_hills:
    case ice_spikes:
    case cold_ocean:
    case deep_cold_ocean:
    case deep_frozen_ocean:
    case snowy_slopes:
    case frozen_peaks:
    case jagged_peaks:
    case grove:
        return 1;
    default:
        return 0;
    }
}

int isMesa(int id)
{
    switch (id)
    {
    case badlands:
    case wooded_badlands_plateau:
    case badlands_plateau:
    case eroded_badlands:
    case modified_wooded_badlands_plateau:
        return 1;
    default:
        return 0;
    }
}

//==============================================================================
// Biome Similarity
//==============================================================================

int areSimilar(int mc, int id1, int id2)
{
    if (id1 == id2)
        return 1;
    
    if (!biomeExists(mc, id1) || !biomeExists(mc, id2))
        return 0;
    
    // Oceanic biomes are similar to each other
    if (isOceanic(id1) && isOceanic(id2))
        return 1;
    
    // Mesa biomes are similar to each other
    if (isMesa(id1) && isMesa(id2))
        return 1;
    
    return 0;
}
