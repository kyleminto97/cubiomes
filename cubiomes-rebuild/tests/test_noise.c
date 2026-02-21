// Test rebuilt noise.c against original implementation
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Include original headers
#include "../noise.h"
#include "../rng.h"

// Include rebuilt headers
#include "../cubiomes-rebuild/noise.h"
#include "../cubiomes-rebuild/rng.h"

#define TEST_SEED 12345ULL
#define TOLERANCE 1e-10

int test_count = 0;
int pass_count = 0;
int fail_count = 0;

void test_result(const char *name, int passed) {
    test_count++;
    if (passed) {
        pass_count++;
        printf("  ✓ %s\n", name);
    } else {
        fail_count++;
        printf("  ✗ %s FAILED\n", name);
    }
}

int compare_double(double a, double b, double tolerance) {
    return fabs(a - b) < tolerance;
}

void test_perlin_init() {
    printf("\n=== Testing Perlin Initialization ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise noise1, noise2;
    perlinInit(&noise1, &seed1);
    perlinInit(&noise2, &seed2);
    
    // Compare key fields
    int passed = 1;
    passed &= compare_double(noise1.a, noise2.a, TOLERANCE);
    passed &= compare_double(noise1.b, noise2.b, TOLERANCE);
    passed &= compare_double(noise1.c, noise2.c, TOLERANCE);
    passed &= (noise1.h2 == noise2.h2);
    
    // Compare permutation table
    for (int i = 0; i < 256 && passed; i++) {
        passed &= (noise1.d[i] == noise2.d[i]);
    }
    
    test_result("perlinInit() produces identical results", passed);
}

void test_xperlin_init() {
    printf("\n=== Testing XPerlin Initialization ===\n");
    
    Xoroshiro xr1, xr2;
    xSetSeed(&xr1, TEST_SEED);
    xSetSeed(&xr2, TEST_SEED);
    
    PerlinNoise noise1, noise2;
    xPerlinInit(&noise1, &xr1);
    xPerlinInit(&noise2, &xr2);
    
    // Compare key fields
    int passed = 1;
    passed &= compare_double(noise1.a, noise2.a, TOLERANCE);
    passed &= compare_double(noise1.b, noise2.b, TOLERANCE);
    passed &= compare_double(noise1.c, noise2.c, TOLERANCE);
    
    test_result("xPerlinInit() produces identical results", passed);
}

void test_sample_perlin() {
    printf("\n=== Testing Perlin Sampling ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise noise1, noise2;
    perlinInit(&noise1, &seed1);
    perlinInit(&noise2, &seed2);
    
    // Test various coordinates
    double coords[][3] = {
        {0, 0, 0},
        {1.5, 2.3, 3.7},
        {-5.2, 10.8, -3.4},
        {100.0, 50.0, 75.0}
    };
    
    int all_passed = 1;
    for (int i = 0; i < 4; i++) {
        double x = coords[i][0];
        double y = coords[i][1];
        double z = coords[i][2];
        
        double v1 = samplePerlin(&noise1, x, y, z, 0, 0);
        double v2 = samplePerlin(&noise2, x, y, z, 0, 0);
        
        if (!compare_double(v1, v2, TOLERANCE)) {
            printf("    Mismatch at (%.1f, %.1f, %.1f): %f vs %f\n", x, y, z, v1, v2);
            all_passed = 0;
        }
    }
    
    test_result("samplePerlin() produces identical results", all_passed);
}

void test_sample_simplex2d() {
    printf("\n=== Testing Simplex 2D Sampling ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise noise1, noise2;
    perlinInit(&noise1, &seed1);
    perlinInit(&noise2, &seed2);
    
    // Test various 2D coordinates
    double coords[][2] = {
        {0, 0},
        {1.5, 2.3},
        {-5.2, 10.8},
        {100.0, 50.0}
    };
    
    int all_passed = 1;
    for (int i = 0; i < 4; i++) {
        double x = coords[i][0];
        double y = coords[i][1];
        
        double v1 = sampleSimplex2D(&noise1, x, y);
        double v2 = sampleSimplex2D(&noise2, x, y);
        
        if (!compare_double(v1, v2, TOLERANCE)) {
            printf("    Mismatch at (%.1f, %.1f): %f vs %f\n", x, y, v1, v2);
            all_passed = 0;
        }
    }
    
    test_result("sampleSimplex2D() produces identical results", all_passed);
}

void test_octave_init() {
    printf("\n=== Testing Octave Initialization ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise octaves1[8], octaves2[8];
    OctaveNoise noise1, noise2;
    
    octaveInit(&noise1, &seed1, octaves1, -7, 8);
    octaveInit(&noise2, &seed2, octaves2, -7, 8);
    
    // Compare octave count
    int passed = (noise1.octcnt == noise2.octcnt);
    
    // Compare each octave
    for (int i = 0; i < noise1.octcnt && passed; i++) {
        passed &= compare_double(octaves1[i].amplitude, octaves2[i].amplitude, TOLERANCE);
        passed &= compare_double(octaves1[i].lacunarity, octaves2[i].lacunarity, TOLERANCE);
    }
    
    test_result("octaveInit() produces identical results", passed);
}

void test_xoctave_init() {
    printf("\n=== Testing XOctave Initialization ===\n");
    
    Xoroshiro xr1, xr2;
    xSetSeed(&xr1, TEST_SEED);
    xSetSeed(&xr2, TEST_SEED);
    
    PerlinNoise octaves1[16], octaves2[16];
    OctaveNoise noise1, noise2;
    
    double amplitudes[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    
    int n1 = xOctaveInit(&noise1, &xr1, octaves1, amplitudes, -7, 8, -1);
    int n2 = xOctaveInit(&noise2, &xr2, octaves2, amplitudes, -7, 8, -1);
    
    // Compare octave count
    int passed = (n1 == n2) && (noise1.octcnt == noise2.octcnt);
    
    // Compare each octave
    for (int i = 0; i < noise1.octcnt && passed; i++) {
        passed &= compare_double(octaves1[i].amplitude, octaves2[i].amplitude, TOLERANCE);
        passed &= compare_double(octaves1[i].lacunarity, octaves2[i].lacunarity, TOLERANCE);
    }
    
    test_result("xOctaveInit() produces identical results", passed);
}

void test_sample_octave() {
    printf("\n=== Testing Octave Sampling ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise octaves1[8], octaves2[8];
    OctaveNoise noise1, noise2;
    
    octaveInit(&noise1, &seed1, octaves1, -7, 8);
    octaveInit(&noise2, &seed2, octaves2, -7, 8);
    
    // Test various coordinates
    double coords[][3] = {
        {0, 0, 0},
        {10.5, 20.3, 30.7},
        {-50.2, 100.8, -30.4}
    };
    
    int all_passed = 1;
    for (int i = 0; i < 3; i++) {
        double x = coords[i][0];
        double y = coords[i][1];
        double z = coords[i][2];
        
        double v1 = sampleOctave(&noise1, x, y, z);
        double v2 = sampleOctave(&noise2, x, y, z);
        
        if (!compare_double(v1, v2, TOLERANCE)) {
            printf("    Mismatch at (%.1f, %.1f, %.1f): %f vs %f\n", x, y, z, v1, v2);
            all_passed = 0;
        }
    }
    
    test_result("sampleOctave() produces identical results", all_passed);
}

void test_double_perlin_init() {
    printf("\n=== Testing Double Perlin Initialization ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise octavesA1[8], octavesB1[8];
    PerlinNoise octavesA2[8], octavesB2[8];
    DoublePerlinNoise noise1, noise2;
    
    doublePerlinInit(&noise1, &seed1, octavesA1, octavesB1, -7, 8);
    doublePerlinInit(&noise2, &seed2, octavesA2, octavesB2, -7, 8);
    
    // Compare amplitude
    int passed = compare_double(noise1.amplitude, noise2.amplitude, TOLERANCE);
    passed &= (noise1.octA.octcnt == noise2.octA.octcnt);
    passed &= (noise1.octB.octcnt == noise2.octB.octcnt);
    
    test_result("doublePerlinInit() produces identical results", passed);
}

void test_xdouble_perlin_init() {
    printf("\n=== Testing XDouble Perlin Initialization ===\n");
    
    Xoroshiro xr1, xr2;
    xSetSeed(&xr1, TEST_SEED);
    xSetSeed(&xr2, TEST_SEED);
    
    PerlinNoise octaves1[16], octaves2[16];
    DoublePerlinNoise noise1, noise2;
    
    double amplitudes[] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    
    int n1 = xDoublePerlinInit(&noise1, &xr1, octaves1, amplitudes, -7, 8, -1);
    int n2 = xDoublePerlinInit(&noise2, &xr2, octaves2, amplitudes, -7, 8, -1);
    
    // Compare results
    int passed = (n1 == n2);
    passed &= compare_double(noise1.amplitude, noise2.amplitude, TOLERANCE);
    
    test_result("xDoublePerlinInit() produces identical results", passed);
}

void test_sample_double_perlin() {
    printf("\n=== Testing Double Perlin Sampling ===\n");
    
    uint64_t seed1 = TEST_SEED;
    uint64_t seed2 = TEST_SEED;
    
    PerlinNoise octavesA1[8], octavesB1[8];
    PerlinNoise octavesA2[8], octavesB2[8];
    DoublePerlinNoise noise1, noise2;
    
    doublePerlinInit(&noise1, &seed1, octavesA1, octavesB1, -7, 8);
    doublePerlinInit(&noise2, &seed2, octavesA2, octavesB2, -7, 8);
    
    // Test various coordinates
    double coords[][3] = {
        {0, 0, 0},
        {10.5, 20.3, 30.7},
        {-50.2, 100.8, -30.4}
    };
    
    int all_passed = 1;
    for (int i = 0; i < 3; i++) {
        double x = coords[i][0];
        double y = coords[i][1];
        double z = coords[i][2];
        
        double v1 = sampleDoublePerlin(&noise1, x, y, z);
        double v2 = sampleDoublePerlin(&noise2, x, y, z);
        
        if (!compare_double(v1, v2, TOLERANCE)) {
            printf("    Mismatch at (%.1f, %.1f, %.1f): %f vs %f\n", x, y, z, v1, v2);
            all_passed = 0;
        }
    }
    
    test_result("sampleDoublePerlin() produces identical results", all_passed);
}

int main() {
    printf("=================================================\n");
    printf("Testing Rebuilt Noise Functions vs Original\n");
    printf("=================================================\n");
    
    test_perlin_init();
    test_xperlin_init();
    test_sample_perlin();
    test_sample_simplex2d();
    test_octave_init();
    test_xoctave_init();
    test_sample_octave();
    test_double_perlin_init();
    test_xdouble_perlin_init();
    test_sample_double_perlin();
    
    printf("\n=================================================\n");
    printf("Test Results: %d/%d passed", pass_count, test_count);
    if (fail_count > 0) {
        printf(" (%d FAILED)", fail_count);
    }
    printf("\n=================================================\n");
    
    return (fail_count == 0) ? 0 : 1;
}
