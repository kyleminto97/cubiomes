// Generate reference data from original cubiomes for testing rebuilt version
#include "../generator.h"
#include "../finders.h"
#include <stdio.h>
#include <stdlib.h>

// Stub for missing function
int getStructureConfig_override(int structureType, int mc, StructureConfig *sconf) {
    return 0; // Not used in our tests
}

void test_biome_generation() {
    printf("=== Testing Biome Generation ===\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    
    // Test multiple seeds
    uint64_t test_seeds[] = {12345, 67890, 123456789, 987654321, 0};
    
    for (int s = 0; s < 5; s++) {
        uint64_t seed = test_seeds[s];
        applySeed(&g, DIM_OVERWORLD, seed);
        
        printf("\nSeed: %llu\n", (unsigned long long)seed);
        
        // Test various coordinates
        int test_coords[][3] = {
            {0, 63, 0},
            {100, 63, 100},
            {-100, 63, -100},
            {1000, 63, 1000},
            {0, 0, 0},
            {0, 255, 0}
        };
        
        for (int i = 0; i < 6; i++) {
            int x = test_coords[i][0];
            int y = test_coords[i][1];
            int z = test_coords[i][2];
            
            int biome = getBiomeAt(&g, 1, x, y, z);
            printf("  Biome at (%d, %d, %d): %d\n", x, y, z, biome);
            
            // Save to reference file
            char filename[256];
            snprintf(filename, sizeof(filename), 
                     "tests/reference/biome_%llu_%d_%d_%d.ref",
                     (unsigned long long)seed, x, y, z);
            FILE *f = fopen(filename, "w");
            if (f) {
                fprintf(f, "%d\n", biome);
                fclose(f);
            }
        }
    }
}

void test_structure_positions() {
    printf("\n=== Testing Structure Positions ===\n");
    
    uint64_t seed = 12345;
    
    int structure_types[] = {
        Village, Desert_Pyramid, Jungle_Pyramid, Swamp_Hut,
        Igloo, Outpost, Monument, Mansion
    };
    const char *structure_names[] = {
        "Village", "Desert_Pyramid", "Jungle_Pyramid", "Swamp_Hut",
        "Igloo", "Outpost", "Monument", "Mansion"
    };
    
    for (int i = 0; i < 8; i++) {
        StructureConfig sconf;
        if (!getStructureConfig(structure_types[i], MC_1_21, &sconf)) {
            printf("  %s: Not available in 1.21\n", structure_names[i]);
            continue;
        }
        
        Pos pos;
        if (getStructurePos(structure_types[i], MC_1_21, seed, 0, 0, &pos)) {
            printf("  %s at region (0,0): (%d, %d)\n", 
                   structure_names[i], pos.x, pos.z);
            
            // Save to reference file
            char filename[256];
            snprintf(filename, sizeof(filename),
                     "tests/reference/struct_%s_%llu_0_0.ref",
                     structure_names[i], (unsigned long long)seed);
            FILE *f = fopen(filename, "w");
            if (f) {
                fprintf(f, "%d %d\n", pos.x, pos.z);
                fclose(f);
            }
        }
    }
}

void test_nether_biomes() {
    printf("\n=== Testing Nether Biomes ===\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_NETHER, 12345);
    
    int test_coords[][3] = {
        {0, 64, 0},
        {100, 64, 100},
        {-100, 64, -100}
    };
    
    for (int i = 0; i < 3; i++) {
        int x = test_coords[i][0];
        int y = test_coords[i][1];
        int z = test_coords[i][2];
        
        int biome = getBiomeAt(&g, 4, x >> 2, y >> 2, z >> 2);
        printf("  Nether biome at (%d, %d, %d): %d\n", x, y, z, biome);
        
        // Save to reference file
        char filename[256];
        snprintf(filename, sizeof(filename),
                 "tests/reference/nether_12345_%d_%d_%d.ref",
                 x, y, z);
        FILE *f = fopen(filename, "w");
        if (f) {
            fprintf(f, "%d\n", biome);
            fclose(f);
        }
    }
}

void test_end_biomes() {
    printf("\n=== Testing End Biomes ===\n");
    
    Generator g;
    setupGenerator(&g, MC_1_21, 0);
    applySeed(&g, DIM_END, 12345);
    
    int test_coords[][3] = {
        {0, 64, 0},
        {1000, 64, 1000},
        {-1000, 64, -1000}
    };
    
    for (int i = 0; i < 3; i++) {
        int x = test_coords[i][0];
        int y = test_coords[i][1];
        int z = test_coords[i][2];
        
        int biome = getBiomeAt(&g, 4, x >> 2, y >> 2, z >> 2);
        printf("  End biome at (%d, %d, %d): %d\n", x, y, z, biome);
        
        // Save to reference file
        char filename[256];
        snprintf(filename, sizeof(filename),
                 "tests/reference/end_12345_%d_%d_%d.ref",
                 x, y, z);
        FILE *f = fopen(filename, "w");
        if (f) {
            fprintf(f, "%d\n", biome);
            fclose(f);
        }
    }
}

int main() {
    printf("Generating reference data from original cubiomes...\n");
    printf("This data will be used to validate the rebuilt version.\n\n");
    
    test_biome_generation();
    test_structure_positions();
    test_nether_biomes();
    test_end_biomes();
    
    printf("\n=== Reference data generation complete ===\n");
    printf("Files saved to tests/reference/\n");
    
    return 0;
}
