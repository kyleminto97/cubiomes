// Cubiomes Rebuild - Structure Finders (1.18+ only)
// Rebuilt from original cubiomes, keeping only 1.18+ functionality

#include "finders.h"
#include "biomes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159265358979323846

//==============================================================================
// Structure Configurations (1.18+ only)
//==============================================================================

int getStructureConfig(int structureType, int mc, StructureConfig *sconf)
{
    // Structure configs for 1.18+
    static const StructureConfig
    s_desert_pyramid        = { 14357617, 32, 24, Desert_Pyramid,   DIM_OVERWORLD, 0},
    s_igloo                 = { 14357618, 32, 24, Igloo,            DIM_OVERWORLD, 0},
    s_jungle_temple         = { 14357619, 32, 24, Jungle_Temple,    DIM_OVERWORLD, 0},
    s_swamp_hut             = { 14357620, 32, 24, Swamp_Hut,        DIM_OVERWORLD, 0},
    s_outpost               = {165745296, 32, 24, Outpost,          DIM_OVERWORLD, 0},
    s_village               = { 10387312, 34, 26, Village,          DIM_OVERWORLD, 0},
    s_ocean_ruin            = { 14357621, 20, 12, Ocean_Ruin,       DIM_OVERWORLD, 0},
    s_shipwreck             = {165745295, 24, 20, Shipwreck,        DIM_OVERWORLD, 0},
    s_monument              = { 10387313, 32, 27, Monument,         DIM_OVERWORLD, 0},
    s_mansion               = { 10387319, 80, 60, Mansion,          DIM_OVERWORLD, 0},
    s_ruined_portal         = { 34222645, 40, 25, Ruined_Portal,    DIM_OVERWORLD, 0},
    s_ruined_portal_n       = { 34222645, 40, 25, Ruined_Portal_N,  DIM_NETHER, 0},
    s_ancient_city          = { 20083232, 24, 16, Ancient_City,     DIM_OVERWORLD, 0},
    s_trail_ruins           = { 83469867, 34, 26, Trail_Ruins,      DIM_OVERWORLD, 0},
    s_trial_chambers        = { 94251327, 34, 22, Trial_Chambers,   DIM_OVERWORLD, 0},
    s_treasure              = { 10387320,  1,  1, Treasure,         DIM_OVERWORLD, 0},
    s_mineshaft             = {        0,  1,  1, Mineshaft,        DIM_OVERWORLD, 0},
    s_desert_well           = {    40002,  1,  1, Desert_Well,      DIM_OVERWORLD, 1.f/1000},
    s_geode                 = {    20002,  1,  1, Geode,            DIM_OVERWORLD, 1.f/24},
    s_fortress              = { 30084232, 27, 23, Fortress,         DIM_NETHER, 0},
    s_bastion               = { 30084232, 27, 23, Bastion,          DIM_NETHER, 0},
    s_end_city              = { 10387313, 20,  9, End_City,         DIM_END, 0},
    s_end_gateway           = {    40000,  1,  1, End_Gateway,      DIM_END, 1.f/700},
    s_end_island            = {        0,  1,  1, End_Island,       DIM_END, 1.f/14}
    ;

    // Only support 1.18+
    if (mc < MC_1_18)
    {
        memset(sconf, 0, sizeof(StructureConfig));
        return 0;
    }

    switch (structureType)
    {
    case Desert_Pyramid:
        *sconf = s_desert_pyramid;
        return 1;
    case Jungle_Temple:
        *sconf = s_jungle_temple;
        return 1;
    case Swamp_Hut:
        *sconf = s_swamp_hut;
        return 1;
    case Igloo:
        *sconf = s_igloo;
        return 1;
    case Village:
        *sconf = s_village;
        return 1;
    case Ocean_Ruin:
        *sconf = s_ocean_ruin;
        return 1;
    case Shipwreck:
        *sconf = s_shipwreck;
        return 1;
    case Ruined_Portal:
        *sconf = s_ruined_portal;
        return 1;
    case Ruined_Portal_N:
        *sconf = s_ruined_portal_n;
        return 1;
    case Monument:
        *sconf = s_monument;
        return 1;
    case End_City:
        *sconf = s_end_city;
        return 1;
    case Mansion:
        *sconf = s_mansion;
        return 1;
    case Outpost:
        *sconf = s_outpost;
        return 1;
    case Ancient_City:
        *sconf = s_ancient_city;
        return mc >= MC_1_19_2;
    case Treasure:
        *sconf = s_treasure;
        return 1;
    case Mineshaft:
        *sconf = s_mineshaft;
        return 1;
    case Fortress:
        *sconf = s_fortress;
        return 1;
    case Bastion:
        *sconf = s_bastion;
        return 1;
    case End_Gateway:
        *sconf = s_end_gateway;
        return 1;
    case End_Island:
        *sconf = s_end_island;
        return 1;
    case Desert_Well:
        *sconf = s_desert_well;
        return 1;
    case Geode:
        *sconf = s_geode;
        return 1;
    case Trail_Ruins:
        *sconf = s_trail_ruins;
        return mc >= MC_1_20_6;
    case Trial_Chambers:
        *sconf = s_trial_chambers;
        return mc >= MC_1_21_1;
    default:
        memset(sconf, 0, sizeof(StructureConfig));
        return 0;
    }
}

//==============================================================================
// Structure Position Finding
//==============================================================================

static uint64_t getPopulationSeed(int mc, uint64_t ws, int x, int z)
{
    Xoroshiro xr;
    uint64_t a, b;

    xSetSeed(&xr, ws);
    a = xNextLongJ(&xr) | 1;
    b = xNextLongJ(&xr) | 1;

    return (x * a + z * b) ^ ws;
}

int getStructurePos(int structureType, int mc, uint64_t seed, int regX, int regZ, Pos *pos)
{
    StructureConfig sconf;
    
    if (!getStructureConfig(structureType, mc, &sconf))
        return 0;

    switch (structureType)
    {
    case Fortress:
    case Bastion:
        *pos = getFeaturePos(sconf, seed, regX, regZ);
        return 1;

    case Monument:
    case Mansion:
        *pos = getLargeStructurePos(sconf, seed, regX, regZ);
        return 1;

    case End_Gateway:
    case End_Island:
    case Desert_Well:
    case Geode:
        // Decorator features - use population seed
        pos->x = regX * 16;
        pos->z = regZ * 16;
        seed = getPopulationSeed(mc, seed, pos->x, pos->z);
        setSeed(&seed, seed);
        if (nextFloat(&seed) >= sconf.rarity)
            return 0;
        pos->x += nextInt(&seed, 16);
        pos->z += nextInt(&seed, 16);
        return 1;

    default:
        // Standard feature structures
        *pos = getFeaturePos(sconf, seed, regX, regZ);
        return 1;
    }
}

//==============================================================================
// Mineshafts
//==============================================================================

int getMineshafts(int mc, uint64_t seed, int chunkX, int chunkZ,
        int chunkW, int chunkH, Pos *out, int nout)
{
    (void)mc; // unused for 1.18+
    
    int i, j, n = 0;
    uint64_t rnd;

    for (i = 0; i < chunkW; i++)
    {
        for (j = 0; j < chunkH; j++)
        {
            int cx = chunkX + i;
            int cz = chunkZ + j;
            
            setSeed(&rnd, seed);
            rnd = (nextLong(&rnd) * cx) ^ (nextLong(&rnd) * cz) ^ seed;
            setSeed(&rnd, rnd);
            
            if (nextDouble(&rnd) < 0.004)
            {
                if (out && n < nout)
                {
                    out[n].x = cx;
                    out[n].z = cz;
                }
                n++;
            }
        }
    }
    
    return n;
}

//==============================================================================
// End Islands
//==============================================================================

int getEndIslands(EndIsland islands[2], int mc, uint64_t seed, int chunkX, int chunkZ)
{
    uint64_t cs;
    int n = 0;
    
    // End island generation
    if (mc <= MC_1_18)
        setSeed(&cs, chunkX + chunkZ * 10387313ULL);
    else
        setSeed(&cs, seed + chunkX + chunkZ * 10387313ULL);
    
    skipNextN(&cs, 1);
    
    if (nextInt(&cs, 14) == 0)
    {
        int x = nextInt(&cs, 16);
        int z = nextInt(&cs, 16);
        int y = 55 + nextInt(&cs, 16);
        int r = 2 + nextInt(&cs, 4);
        
        if (islands)
        {
            islands[n].x = chunkX * 16 + x;
            islands[n].y = y;
            islands[n].z = chunkZ * 16 + z;
            islands[n].r = r;
        }
        n++;
        
        if (nextInt(&cs, 4) == 0)
        {
            x = nextInt(&cs, 16);
            z = nextInt(&cs, 16);
            y = 55 + nextInt(&cs, 16);
            r = 2 + nextInt(&cs, 4);
            
            if (islands)
            {
                islands[n].x = chunkX * 16 + x;
                islands[n].y = y;
                islands[n].z = chunkZ * 16 + z;
                islands[n].r = r;
            }
            n++;
        }
    }
    
    return n;
}

//==============================================================================
// Strongholds (1.18+)
//==============================================================================

Pos initFirstStronghold(StrongholdIter *sh, int mc, uint64_t s48)
{
    const double dist_1_18 = 32.0 * 1.5;
    
    if (sh)
    {
        memset(sh, 0, sizeof(StrongholdIter));
        sh->mc = mc;
        sh->rnds = s48;
        sh->ringnum = 0;
        sh->ringmax = 3;
        sh->ringidx = 0;
        sh->dist = dist_1_18;
        
        setSeed(&sh->rnds, s48);
        sh->angle = 2.0 * PI * nextDouble(&sh->rnds);
    }
    
    Pos p;
    uint64_t rnd = s48;
    setSeed(&rnd, s48);
    double angle = 2.0 * PI * nextDouble(&rnd);
    p.x = (int)round(cos(angle) * dist_1_18 * 16.0);
    p.z = (int)round(sin(angle) * dist_1_18 * 16.0);
    
    return p;
}

int nextStronghold(StrongholdIter *sh, const Generator *g)
{
    if (sh->index >= 128)
        return 0;
    
    sh->pos.x = (int)round(cos(sh->angle) * sh->dist * 16.0);
    sh->pos.z = (int)round(sin(sh->angle) * sh->dist * 16.0);
    
    // Biome check for 1.18+
    if (g)
    {
        int cx = sh->pos.x >> 4;
        int cz = sh->pos.z >> 4;
        int id = getBiomeAt(g, 1, cx, 0, cz);
        int attempts = 0;
        
        // Strongholds avoid ocean biomes (bounded to prevent infinite loop)
        while (isOceanic(id) && attempts < 128)
        {
            sh->angle += 2.0 * PI / sh->ringmax;
            sh->pos.x = (int)round(cos(sh->angle) * sh->dist * 16.0);
            sh->pos.z = (int)round(sin(sh->angle) * sh->dist * 16.0);
            cx = sh->pos.x >> 4;
            cz = sh->pos.z >> 4;
            id = getBiomeAt(g, 1, cx, 0, cz);
            attempts++;
        }
    }
    
    sh->index++;
    sh->ringidx++;
    
    if (sh->ringidx >= sh->ringmax)
    {
        sh->ringnum++;
        sh->ringidx = 0;
        sh->ringmax = (sh->ringnum < 2) ? 6 : 10 + 10 * sh->ringnum;
        sh->dist += 32.0 * 3.0;
    }
    
    sh->angle += 2.0 * PI / sh->ringmax;
    sh->nextapprox.x = (int)round(cos(sh->angle) * sh->dist * 16.0);
    sh->nextapprox.z = (int)round(sin(sh->angle) * sh->dist * 16.0);
    
    return 128 - sh->index;
}

//==============================================================================
// Spawn
//==============================================================================

Pos estimateSpawn(const Generator *g, uint64_t *rng)
{
    uint64_t r = g->seed;
    setSeed(&r, r);
    
    Pos p = {0, 0};
    
    if (rng)
        *rng = r;
    
    return p;
}

Pos getSpawn(const Generator *g)
{
    // Simplified spawn - just return origin
    // Full implementation would require grass block checks
    Pos p = {0, 0};
    return p;
}

//==============================================================================
// Biome Viability Checks
//==============================================================================

int isViableFeatureBiome(int mc, int structureType, int biomeID)
{
    if (!biomeExists(mc, biomeID))
        return 0;

    switch (structureType)
    {
    case Desert_Pyramid:
        return biomeID == desert;
        
    case Jungle_Temple:
        return biomeID == jungle || biomeID == bamboo_jungle;
        
    case Swamp_Hut:
        return biomeID == swamp || (mc >= MC_1_19_2 && biomeID == mangrove_swamp);
        
    case Igloo:
        return biomeID == snowy_tundra || biomeID == snowy_taiga ||
               (mc >= MC_1_18 && biomeID == grove);
        
    case Village:
        return biomeID == plains || biomeID == desert || biomeID == savanna ||
               biomeID == taiga || biomeID == snowy_tundra ||
               (mc >= MC_1_18 && biomeID == meadow);
        
    case Ocean_Ruin:
    case Shipwreck:
    case Monument:
        return isOceanic(biomeID);
        
    case Mansion:
        return biomeID == dark_forest;
        
    case Outpost:
        if (mc >= MC_1_18)
        {
            switch (biomeID)
            {
            case desert:
            case plains:
            case savanna:
            case snowy_tundra:
            case taiga:
            case grove:
            case meadow:
                return 1;
            }
        }
        return 0;
        
    case Ancient_City:
        return biomeID == deep_dark;
        
    case Trail_Ruins:
        switch (biomeID)
        {
        case taiga:
        case snowy_taiga:
        case jungle:
        case birch_forest:
        case tall_birch_forest:
        case giant_tree_taiga:
        case giant_spruce_taiga:
            return 1;
        }
        return 0;
        
    case Trial_Chambers:
        return biomeID != deep_dark && isOverworld(mc, biomeID);
        
    case Fortress:
        return getDimension(biomeID) == DIM_NETHER;
        
    case Bastion:
        return biomeID == nether_wastes || biomeID == crimson_forest ||
               biomeID == warped_forest || biomeID == soul_sand_valley;
        
    case End_City:
        return biomeID == end_highlands || biomeID == end_midlands;
        
    default:
        return 0;
    }
}

int isViableStructurePos(int structType, Generator *g, int blockX, int blockZ, uint32_t flags)
{
    (void)flags;
    
    int64_t chunkX = blockX >> 4;
    int64_t chunkZ = blockZ >> 4;

    if (g->dim == DIM_NETHER)
    {
        if (structType == Fortress)
        {
            // Fortresses generate everywhere bastions don't in 1.18+
            StructureConfig sc;
            if (!getStructureConfig(Fortress, g->mc, &sc))
                return 0;
            int rpx = floordiv(blockX, sc.regionSize << 4);
            int rpz = floordiv(blockZ, sc.regionSize << 4);
            Pos rp;
            if (!getStructurePos(Bastion, g->mc, g->seed, rpx, rpz, &rp))
                return 1;
            return !isViableStructurePos(Bastion, g, blockX, blockZ, flags);
        }
        int sampleX = (int)(chunkX * 4 + 2);
        int sampleZ = (int)(chunkZ * 4 + 2);
        int id = getBiomeAt(g, 4, sampleX, 0, sampleZ);
        return isViableFeatureBiome(g->mc, structType, id);
    }
    else if (g->dim == DIM_END)
    {
        int id = getBiomeAt(g, 16, (int)chunkX, 0, (int)chunkZ);
        return isViableFeatureBiome(g->mc, structType, id);
    }

    // Overworld (1.18+): sample biome at chunk center in biome coordinates
    int sampleX = (int)(chunkX * 4 + 2);
    int sampleZ = (int)(chunkZ * 4 + 2);
    int sampleY = 319 >> 2; // surface Y in biome coords
    int id = getBiomeAt(g, 4, sampleX, sampleY, sampleZ);
    
    if (id < 0)
        return 0;
    
    return isViableFeatureBiome(g->mc, structType, id);
}

//==============================================================================
// Structure Variants
//==============================================================================

int getVariant(StructureVariant *sv, int structType, int mc, uint64_t seed,
        int blockX, int blockZ, int biomeID)
{
    (void)structType;
    (void)mc;
    (void)seed;
    (void)blockX;
    (void)blockZ;
    (void)biomeID;
    
    // Simplified - just zero out the structure
    memset(sv, 0, sizeof(StructureVariant));
    return 0;
}
