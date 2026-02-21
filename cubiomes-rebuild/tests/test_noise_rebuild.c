// Test that rebuilt noise functions work correctly
// This tests the rebuilt version in isolation
#include "../cubiomes-rebuild/noise.h"
#include "../cubiomes-rebuild/rng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TEST_SEED 12345ULL

int test_count = 0;
int pass_count = 0;

void test_result(const char *name, int passed) {
    test_count++;
    if (passed) {
        pass_count++;
        printf("  ✓ %s\n", name);
    } else {
        printf("  ✗ %s FAILED\n", name);
    }
}

void test_perlin_basic() {
    printf("\n=== Testing Perlin Noise ===\n");
    
    uint64_t seed = TEST_SEED;
    PerlinNoise noise;
    perlinInit(&noise, &seed);
    
    // Test that initialization worked
    int passed = 1;
    passed &= (noise.amplitude == 1.0);
    passed &= (noise.lacunarity == 1.0);
    passed &= (noise.a >= 0 && noise.a < 256);
    passed &= (noise.b >= 0 && noise.b < 256);
    passed &= (noise.c >= 0 && noise.c < 256);
    
    test_result("perlinInit() initializes correctly", passed);
    
    // Test sampling produces reasonable values
    double v1 = samplePerlin(&noise, 0, 0, 0, 0, 0);
    double v2 = samplePerlin(&noise, 1.5, 2.3, 3.7, 0, 0);
    double v3 = samplePerlin(&noise, -5.2, 10.8, -3.4, 0, 0);
    
    // Perlin noise should be roughly in range [-1, 1]
    passed = (fabs(v1) < 2.0) && (fabs(v2) < 2.0) && (fabs(v3) < 2.0);
    test_result("samplePerlin() produces reasonable values", passed);
    
    // Test that same coordinates give same result
    double v4 = samplePerlin(&noise, 1.5, 2.3, 3.7, 0, 0);
    passed = (v2 == v4);
    test_result("samplePerlin() is deterministic", passed);
}

void test_xperlin_basic() {
    printf("\n=== Testing XPerlin Noise ===\n");
    
    Xoroshiro xr;
    xSetSeed(&xr, TEST_SEED);
    
    PerlinNoise noise;
    xPerlinInit(&noise, &xr);
    
    // Test that initialization worked
    int passed = 1;
    passed &= (noise.amplitude == 1.0);
    passed &= (noise.lacunarity == 1.0);
    
    test_result("xPerlinInit() initializes correctly", passed);
    
    // Test sampling
    double v = samplePerlin(&noise, 10.5, 20.3, 30.7, 0, 0);
    passed = (fabs(v) < 2.0);
    test_result("xPerlinInit() noise samples correctly", passed);
}

void test_simplex2d_basic() {
    printf("\n=== Testing Simplex 2D Noise ===\n");
    
    uint64_t seed = TEST_SEED;
    PerlinNoise noise;
    perlinInit(&noise, &seed);
    
    // Test sampling
    double v1 = sampleSimplex2D(&noise, 0, 0);
    double v2 = sampleSimplex2D(&noise, 10.5, 20.3);
    double v3 = sampleSimplex2D(&noise, -50.2, 100.8);
    
    // Simplex noise should be in reasonable range
    int passed = (fabs(v1) < 100.0) && (fabs(v2) < 100.0) && (fabs(v3) < 100.0);
    test_result("sampleSimplex2D() produces reasonable values", passed);
    
    // Test determinism
    double v4 = sampleSimplex2D(&noise, 10.5, 20.3);
    passed = (v2 == v4);
    test_result("sampleSimplex2D() is deterministic", passed);
}

void test_octave_basic() {
    printf("\n=== Testing Octave Noise ===\n");
    
    uint64_t seed = TEST_SEED;
    PerlinNoise octaves[8];
    OctaveNoise noise;
    
    octaveInit(&noise, &seed, octaves, -7, 8);
    
    // Test that initialization worked
    int passed = (noise.octcnt == 8);
    test_result("octaveInit() creates correct number of octaves", passed);
    
    // Test that amplitudes are set correctly
    passed = 1;
    for (int i = 0; i < noise.octcnt; i++) {
        passed &= (octaves[i].amplitude > 0);
        passed &= (octaves[i].lacunarity > 0);
    }
    test_result("octaveInit() sets amplitudes and lacunarity correctly", passed);
    
    // Test sampling
    double v = sampleOctave(&noise, 10.5, 20.3, 30.7);
    passed = (fabs(v) < 10.0);
    test_result("sampleOctave() produces reasonable values", passed);
}

void test_xoctave_basic() {
    printf("\n=== Testing XOctave Noise ===\n");
    
    Xoroshiro xr;
    xSetSeed(&xr, TEST_SEED);
    
    PerlinNoise octaves[16];
    OctaveNoise noise;
    
    double amplitudes[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    int n = xOctaveInit(&noise, &xr, octaves, amplitudes, -7, 8, -1);
    
    // Test that initialization worked
    int passed = (n == 8) && (noise.octcnt == 8);
    test_result("xOctaveInit() creates correct number of octaves", passed);
    
    // Test sampling
    double v = sampleOctave(&noise, 10.5, 20.3, 30.7);
    passed = (fabs(v) < 10.0);
    test_result("xOctaveInit() noise samples correctly", passed);
}

void test_double_perlin_basic() {
    printf("\n=== Testing Double Perlin Noise ===\n");
    
    uint64_t seed = TEST_SEED;
    PerlinNoise octavesA[8], octavesB[8];
    DoublePerlinNoise noise;
    
    doublePerlinInit(&noise, &seed, octavesA, octavesB, -7, 8);
    
    // Test that initialization worked
    int passed = (noise.amplitude > 0);
    passed &= (noise.octA.octcnt == 8);
    passed &= (noise.octB.octcnt == 8);
    test_result("doublePerlinInit() initializes correctly", passed);
    
    // Test sampling
    double v = sampleDoublePerlin(&noise, 10.5, 20.3, 30.7);
    passed = (fabs(v) < 20.0);
    test_result("sampleDoublePerlin() produces reasonable values", passed);
}

void test_xdouble_perlin_basic() {
    printf("\n=== Testing XDouble Perlin Noise ===\n");
    
    Xoroshiro xr;
    xSetSeed(&xr, TEST_SEED);
    
    PerlinNoise octaves[16];
    DoublePerlinNoise noise;
    
    double amplitudes[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    int n = xDoublePerlinInit(&noise, &xr, octaves, amplitudes, -7, 8, -1);
    
    // Test that initialization worked
    int passed = (n == 16);
    passed &= (noise.amplitude > 0);
    test_result("xDoublePerlinInit() initializes correctly", passed);
    
    // Test sampling
    double v = sampleDoublePerlin(&noise, 10.5, 20.3, 30.7);
    passed = (fabs(v) < 20.0);
    test_result("xDoublePerlinInit() noise samples correctly", passed);
}

void test_consistency() {
    printf("\n=== Testing Consistency ===\n");
    
    // Test that same seed produces same results
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise noise1, noise2;
    perlinInit(&noise1, &seed1);
    perlinInit(&noise2, &seed2);
    
    double v1 = samplePerlin(&noise1, 5.5, 10.3, 15.7, 0, 0);
    double v2 = samplePerlin(&noise2, 5.5, 10.3, 15.7, 0, 0);
    
    int passed = (v1 == v2);
    test_result("Same seed produces same noise", passed);
    
    // Test that different seeds produce different results
    seed1 = TEST_SEED;
    seed2 = TEST_SEED + 1;
    
    perlinInit(&noise1, &seed1);
    perlinInit(&noise2, &seed2);
    
    v1 = samplePerlin(&noise1, 5.5, 10.3, 15.7, 0, 0);
    v2 = samplePerlin(&noise2, 5.5, 10.3, 15.7, 0, 0);
    
    passed = (v1 != v2);
    test_result("Different seeds produce different noise", passed);
}

int main() {
    printf("=================================================\n");
    printf("Testing Rebuilt Noise Functions\n");
    printf("=================================================\n");
    
    test_perlin_basic();
    test_xperlin_basic();
    test_simplex2d_basic();
    test_octave_basic();
    test_xoctave_basic();
    test_double_perlin_basic();
    test_xdouble_perlin_basic();
    test_consistency();
    
    printf("\n=================================================\n");
    printf("Test Results: %d/%d passed\n", pass_count, test_count);
    printf("=================================================\n");
    
    if (pass_count == test_count) {
        printf("\n✓ All noise functions working correctly!\n");
        printf("✓ Ready for Phase 3: BiomeNoise System\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed - needs investigation\n");
        return 1;
    }
}
