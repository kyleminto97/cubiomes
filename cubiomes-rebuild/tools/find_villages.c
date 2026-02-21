// find_villages.c - Find seeds with 3+ villages within 350 blocks of spawn
// Usage: find_villages.exe
//
// Optimized two-stage pipeline:
//   Stage 1: Structure positions (cheap, RNG-only, no biome noise)
//            Filters ~99% of seeds using just the 48-bit structure seed.
//   Stage 2: Biome viability (expensive, full noise init)
//            Only runs on seeds that passed stage 1.
//
// The outer loop is parallelized with OpenMP so all cores are used.

#include "../finders.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define RADIUS        300
#define MIN_VILLAGES  5
#define MAX_RESULTS   10
#define MC_VERSION    MC_1_21

// Batch size per thread — large enough to amortize overhead,
// small enough to stop promptly once we hit MAX_RESULTS.
#define BATCH_SIZE    50000

int main(void)
{
    StructureConfig sconf;
    if (!getStructureConfig(Village, MC_VERSION, &sconf))
    {
        fprintf(stderr, "Failed to get village config\n");
        return 1;
    }

    int regionSize = sconf.regionSize;
    int regRange = (RADIUS / 16) / regionSize + 1;
    int64_t radiusSq = (int64_t)RADIUS * RADIUS;

    // Shared result counter (atomic)
    volatile int found = 0;
    uint64_t globalSeed = 0;

    printf("Searching for seeds with %d+ villages within %d blocks of spawn...\n",
           MIN_VILLAGES, RADIUS);
    printf("Using %d thread(s)\n\n",
#ifdef _OPENMP
           omp_get_max_threads()
#else
           1
#endif
    );

    while (found < MAX_RESULTS)
    {
        uint64_t batchStart = globalSeed;
        globalSeed += BATCH_SIZE;

        #pragma omp parallel
        {
            // Each thread gets its own Generator (they're big, avoid false sharing)
            Generator g;
            setupGenerator(&g, MC_VERSION, 0);

            #pragma omp for schedule(dynamic, 256)
            for (int64_t i = 0; i < BATCH_SIZE; i++)
            {
                if (found >= MAX_RESULTS)
                    continue; // can't break out of omp for, but skip work

                uint64_t seed = batchStart + (uint64_t)i;

                // ============================================================
                // STAGE 1: Cheap position-only filter (no biome noise)
                // getStructurePos only uses the Java LCG — extremely fast.
                // ============================================================
                Pos candidates[8]; // max possible in our small region scan
                int nCandidates = 0;

                for (int rz = -regRange; rz <= regRange; rz++)
                {
                    for (int rx = -regRange; rx <= regRange; rx++)
                    {
                        Pos pos;
                        if (!getStructurePos(Village, MC_VERSION, seed, rx, rz, &pos))
                            continue;

                        // Integer distance check — avoid sqrt entirely
                        int64_t dsq = (int64_t)pos.x * pos.x + (int64_t)pos.z * pos.z;
                        if (dsq > radiusSq)
                            continue;

                        if (nCandidates < 8)
                            candidates[nCandidates] = pos;
                        nCandidates++;
                    }
                }

                // Early reject: not enough positions even before biome check
                if (nCandidates < MIN_VILLAGES)
                    continue;

                // ============================================================
                // STAGE 2: Expensive biome viability (full noise init)
                // Only reached by ~1% of seeds that passed stage 1.
                // ============================================================
                applySeed(&g, DIM_OVERWORLD, seed);

                int confirmed = 0;
                int remaining = (nCandidates < 8) ? nCandidates : 8;

                for (int c = 0; c < remaining; c++)
                {
                    if (isViableStructurePos(Village, &g, candidates[c].x, candidates[c].z, 0))
                        confirmed++;

                    // Early exit: can't possibly reach MIN_VILLAGES
                    if (confirmed + (remaining - c - 1) < MIN_VILLAGES)
                        break;
                }

                if (confirmed >= MIN_VILLAGES)
                {
                    int mySlot;
                    #pragma omp atomic capture
                    mySlot = found++;

                    if (mySlot < MAX_RESULTS)
                    {
                        // Re-scan for display (only on hits, negligible cost)
                        #pragma omp critical
                        {
                            printf("[%d] Seed %-20lld | Villages within %d blocks:\n",
                                   mySlot + 1, (long long)seed, RADIUS);
                            for (int c = 0; c < remaining; c++)
                            {
                                if (!isViableStructurePos(Village, &g,
                                        candidates[c].x, candidates[c].z, 0))
                                    continue;
                                double dist = sqrt((double)candidates[c].x * candidates[c].x +
                                                   (double)candidates[c].z * candidates[c].z);
                                printf("      Village at (%d, %d)  dist=%.0f\n",
                                       candidates[c].x, candidates[c].z, dist);
                            }
                            printf("\n");
                        }
                    }
                }
            }
        } // end parallel
    }

    printf("Done. Found %d seeds.\n", found);
    return 0;
}
