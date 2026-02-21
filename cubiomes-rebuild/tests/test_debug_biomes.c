// Debug test to see what biomes are being generated

#include "../cubiomes-rebuild/generator.h"
#include <stdio.h>

int main()
{
    Generator g;
    setupGenerator(&g, MC_1_18, 0);
    applySeed(&g, DIM_OVERWORLD, 12345);
    
    printf("Sampling biomes in a 20x20 grid:\n\n");
    
    for (int z = 0; z < 20; z++)
    {
        for (int x = 0; x < 20; x++)
        {
            int biome = getBiomeAt(&g, 1, x, 63 >> 2, z);
            printf("%3d ", biome);
        }
        printf("\n");
    }
    
    printf("\nBiome counts:\n");
    int counts[256] = {0};
    for (int z = 0; z < 100; z++)
    {
        for (int x = 0; x < 100; x++)
        {
            int biome = getBiomeAt(&g, 1, x, 63 >> 2, z);
            if (biome >= 0 && biome < 256)
                counts[biome]++;
        }
    }
    
    for (int i = 0; i < 256; i++)
    {
        if (counts[i] > 0)
            printf("Biome %d: %d occurrences\n", i, counts[i]);
    }
    
    return 0;
}
