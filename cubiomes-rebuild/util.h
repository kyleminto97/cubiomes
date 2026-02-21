// Cubiomes Rebuild - Utilities (1.18+ only)
// Minimal utility functions for string conversion and biome colors

#ifndef UTIL_REBUILD_H_
#define UTIL_REBUILD_H_

#include "biomes.h"

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================================================
// Version String Conversion
//==============================================================================

/* Convert version enum to string */
const char* mc2str(int mc);

/* Convert string to version enum */
int str2mc(const char *s);

//==============================================================================
// Biome String Conversion
//==============================================================================

/* Get the resource id name for a biome (1.18+ naming) */
const char *biome2str(int mc, int id);

//==============================================================================
// Structure String Conversion
//==============================================================================

/* Get the resource id name for a structure */
const char *struct2str(int stype);

//==============================================================================
// Biome Colors
//==============================================================================

/* Initialize a biome colormap with default colors
 * Colors are stored as RGB triplets: biomeColors[biomeID][0/1/2] = R/G/B
 */
void initBiomeColors(unsigned char biomeColors[256][3]);

/* Get the default color for a specific biome
 * Returns pointer to RGB triplet [R, G, B]
 */
const unsigned char* getBiomeColor(int id);

#ifdef __cplusplus
}
#endif

#endif // UTIL_REBUILD_H_
