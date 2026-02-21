// Cubiomes Rebuild Viewer
// Left: controls & biome info | Centre: map | Right: structures & biomes browser

#include "raylib/include/raylib.h"

#ifdef _WIN32
#define NOGDI
#define NOUSER
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef DrawText
#undef ShowCursor
#undef CloseWindow
#undef Rectangle
#endif

#include "../../generator.h"
#include "../../finders.h"
#include "../../util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#ifndef _WIN32
#include <pthread.h>
#include <unistd.h>
#endif

//==============================================================================
// Layout
//==============================================================================

#define WIN_W         1500
#define WIN_H         860
#define LEFT_W        270
#define RIGHT_W       270
#define MAP_X         LEFT_W
#define MAP_W         (WIN_W - LEFT_W - RIGHT_W)
#define MAP_H         WIN_H
#define TILE_PX       128
#define MAX_TILES     1024
#define SEED_BUF_LEN  24
#define QUEUE_SIZE    512
#define MAX_WORKERS   32

//==============================================================================
// Light theme
//==============================================================================

#define C_BG          (Color){245, 245, 248, 255}
#define C_SIDEBAR     (Color){252, 252, 254, 255}
#define C_PANEL       (Color){238, 238, 242, 255}
#define C_PANEL_HI    (Color){228, 232, 245, 255}
#define C_ACCENT      (Color){55, 120, 220, 255}
#define C_ACCENT_HI   (Color){70, 140, 240, 255}
#define C_ACCENT_SOFT (Color){220, 232, 250, 255}
#define C_TEXT         (Color){30, 30, 40, 255}
#define C_TEXT_SEC     (Color){100, 100, 115, 255}
#define C_TEXT_DIM     (Color){150, 150, 165, 255}
#define C_BORDER      (Color){210, 212, 220, 255}
#define C_BTN         (Color){235, 236, 242, 255}
#define C_BTN_HI      (Color){220, 225, 238, 255}
#define C_BTN_ACTIVE  (Color){55, 120, 220, 255}
#define C_SECTION_BG  (Color){242, 243, 248, 255}
#define C_SECTION_HDR (Color){70, 75, 90, 255}
#define C_INPUT_BG    (Color){255, 255, 255, 255}
#define C_INPUT_FOCUS (Color){240, 245, 255, 255}
#define C_ROW_HI      (Color){225, 235, 252, 255}
#define C_MAP_BG      (Color){30, 30, 40, 255}

//==============================================================================
// Display name enums — clean human-readable names
//==============================================================================

static const char *biomeDisplayName(int id) {
    switch (id) {
    case ocean:                 return "Ocean";
    case plains:                return "Plains";
    case desert:                return "Desert";
    case mountains:             return "Windswept Hills";
    case forest:                return "Forest";
    case taiga:                 return "Taiga";
    case swamp:                 return "Swamp";
    case river:                 return "River";
    case nether_wastes:         return "Nether Wastes";
    case the_end:               return "The End";
    case frozen_ocean:          return "Frozen Ocean";
    case frozen_river:          return "Frozen River";
    case snowy_tundra:          return "Snowy Plains";
    case mushroom_fields:       return "Mushroom Fields";
    case beach:                 return "Beach";
    case jungle:                return "Jungle";
    case jungle_edge:           return "Sparse Jungle";
    case deep_ocean:            return "Deep Ocean";
    case stone_shore:           return "Stony Shore";
    case snowy_beach:           return "Snowy Beach";
    case birch_forest:          return "Birch Forest";
    case dark_forest:           return "Dark Forest";
    case snowy_taiga:           return "Snowy Taiga";
    case giant_tree_taiga:      return "Old Growth Pine Taiga";
    case wooded_mountains:      return "Windswept Forest";
    case savanna:               return "Savanna";
    case savanna_plateau:       return "Savanna Plateau";
    case badlands:              return "Badlands";
    case wooded_badlands_plateau: return "Wooded Badlands";
    case small_end_islands:     return "Small End Islands";
    case end_midlands:          return "End Midlands";
    case end_highlands:         return "End Highlands";
    case end_barrens:           return "End Barrens";
    case warm_ocean:            return "Warm Ocean";
    case lukewarm_ocean:        return "Lukewarm Ocean";
    case cold_ocean:            return "Cold Ocean";
    case deep_warm_ocean:       return "Deep Warm Ocean";
    case deep_lukewarm_ocean:   return "Deep Lukewarm Ocean";
    case deep_cold_ocean:       return "Deep Cold Ocean";
    case deep_frozen_ocean:     return "Deep Frozen Ocean";
    case sunflower_plains:      return "Sunflower Plains";
    case gravelly_mountains:    return "Windswept Gravelly Hills";
    case flower_forest:         return "Flower Forest";
    case ice_spikes:            return "Ice Spikes";
    case tall_birch_forest:     return "Old Growth Birch Forest";
    case giant_spruce_taiga:    return "Old Growth Spruce Taiga";
    case shattered_savanna:     return "Windswept Savanna";
    case eroded_badlands:       return "Eroded Badlands";
    case bamboo_jungle:         return "Bamboo Jungle";
    case soul_sand_valley:      return "Soul Sand Valley";
    case crimson_forest:        return "Crimson Forest";
    case warped_forest:         return "Warped Forest";
    case basalt_deltas:         return "Basalt Deltas";
    case dripstone_caves:       return "Dripstone Caves";
    case lush_caves:            return "Lush Caves";
    case meadow:                return "Meadow";
    case grove:                 return "Grove";
    case snowy_slopes:          return "Snowy Slopes";
    case jagged_peaks:          return "Jagged Peaks";
    case frozen_peaks:          return "Frozen Peaks";
    case stony_peaks:           return "Stony Peaks";
    case deep_dark:             return "Deep Dark";
    case mangrove_swamp:        return "Mangrove Swamp";
    case cherry_grove:          return "Cherry Grove";
    case pale_garden:           return "Pale Garden";
    default:                    return "Unknown";
    }
}

static const char *structDisplayName(int stype) {
    switch (stype) {
    case Desert_Pyramid:  return "Desert Pyramid";
    case Jungle_Temple:   return "Jungle Temple";
    case Swamp_Hut:       return "Swamp Hut";
    case Igloo:           return "Igloo";
    case Village:         return "Village";
    case Ocean_Ruin:      return "Ocean Ruin";
    case Shipwreck:       return "Shipwreck";
    case Monument:        return "Ocean Monument";
    case Mansion:         return "Woodland Mansion";
    case Outpost:         return "Pillager Outpost";
    case Ruined_Portal:   return "Ruined Portal";
    case Ruined_Portal_N: return "Ruined Portal (Nether)";
    case Ancient_City:    return "Ancient City";
    case Treasure:        return "Buried Treasure";
    case Mineshaft:       return "Mineshaft";
    case Fortress:        return "Nether Fortress";
    case Bastion:         return "Bastion Remnant";
    case End_City:        return "End City";
    case End_Gateway:     return "End Gateway";
    case Trail_Ruins:     return "Trail Ruins";
    case Trial_Chambers:  return "Trial Chambers";
    default:              return "Unknown";
    }
}

//==============================================================================
// Thread abstraction
//==============================================================================

#ifdef _WIN32
typedef HANDLE Thread;
typedef CRITICAL_SECTION Mutex;
typedef CONDITION_VARIABLE CondVar;
static void mutexInit(Mutex *m)     { InitializeCriticalSection(m); }
static void mutexLock(Mutex *m)     { EnterCriticalSection(m); }
static void mutexUnlock(Mutex *m)   { LeaveCriticalSection(m); }
static void mutexDestroy(Mutex *m)  { DeleteCriticalSection(m); }
static void condInit(CondVar *c)    { InitializeConditionVariable(c); }
static void condSignal(CondVar *c)  { WakeConditionVariable(c); }
static void condBroadcast(CondVar *c){ WakeAllConditionVariable(c); }
static void condWait(CondVar *c, Mutex *m) { SleepConditionVariableCS(c, m, INFINITE); }
static void condDestroy(CondVar *c) { (void)c; }
typedef DWORD (WINAPI *ThreadFunc)(LPVOID);
static Thread threadCreate(ThreadFunc fn, void *arg) {
    return CreateThread(NULL, 0, fn, arg, 0, NULL);
}
static void threadJoin(Thread t) { WaitForSingleObject(t, INFINITE); CloseHandle(t); }
static int getNumCores(void) { SYSTEM_INFO si; GetSystemInfo(&si); return (int)si.dwNumberOfProcessors; }
#else
typedef pthread_t Thread;
typedef pthread_mutex_t Mutex;
typedef pthread_cond_t CondVar;
static void mutexInit(Mutex *m)     { pthread_mutex_init(m, NULL); }
static void mutexLock(Mutex *m)     { pthread_mutex_lock(m); }
static void mutexUnlock(Mutex *m)   { pthread_mutex_unlock(m); }
static void mutexDestroy(Mutex *m)  { pthread_mutex_destroy(m); }
static void condInit(CondVar *c)    { pthread_cond_init(c, NULL); }
static void condSignal(CondVar *c)  { pthread_cond_signal(c); }
static void condBroadcast(CondVar *c){ pthread_cond_broadcast(c); }
static void condWait(CondVar *c, Mutex *m) { pthread_cond_wait(c, m); }
static void condDestroy(CondVar *c) { pthread_cond_destroy(c); }
typedef void *(*ThreadFunc)(void *);
static Thread threadCreate(ThreadFunc fn, void *arg) {
    Thread t; pthread_create(&t, NULL, fn, arg); return t;
}
static void threadJoin(Thread t) { pthread_join(t, NULL); }
static int getNumCores(void) { return (int)sysconf(_SC_NPROCESSORS_ONLN); }
#endif

//==============================================================================
// Tile types & shared state
//==============================================================================

typedef struct { int tx, tz, scale; uint64_t seed; int dim; Texture2D tex; int valid, lastUsedFrame; } Tile;
typedef struct { int tx, tz, scale; uint64_t seed; int dim; Color *pixels; } ReadyTile;
typedef struct { int tx, tz, scale; uint64_t seed; int dim, mc; uint32_t flags; } TileRequest;

static Tile         tileCache[MAX_TILES];
static int          tileCount = 0;
static TileRequest  requestQueue[QUEUE_SIZE];
static int          reqHead=0, reqTail=0, reqCount=0;
static ReadyTile    readyQueue[QUEUE_SIZE];
static int          rdyHead=0, rdyTail=0, rdyCount=0;
static Mutex        reqMutex, rdyMutex;
static CondVar      reqCond;
static volatile int workerRunning = 1;
static int          frameCounter = 0;
static int          numWorkers = 1;

// Global font
static Font appFont;
static int  fontLoaded = 0;

//==============================================================================
// Font-aware text drawing
//==============================================================================

static void DrawTextF(const char *text, int x, int y, int size, Color col) {
    if (fontLoaded)
        DrawTextEx(appFont, text, (Vector2){(float)x, (float)y}, (float)size, 1, col);
    else
        DrawText(text, x, y, size, col);
}

static int MeasureTextF(const char *text, int size) {
    if (fontLoaded)
        return (int)MeasureTextEx(appFont, text, (float)size, 1).x;
    else
        return MeasureText(text, size);
}

//==============================================================================
// Scale, tile cache, queues
//==============================================================================

static int pickScale(float zoom) {
    float m = 4.0f / zoom;
    if (m >= 256) return 256;
    if (m >= 64)  return 64;
    if (m >= 16)  return 16;
    return 4;
}
static inline int tileWorldSize(int s) { return TILE_PX * s; }

static Tile *findTile(int tx, int tz, int scale, uint64_t seed, int dim) {
    for (int i = 0; i < tileCount; i++) {
        Tile *t = &tileCache[i];
        if (t->valid && t->tx==tx && t->tz==tz && t->scale==scale && t->seed==seed && t->dim==dim) return t;
    }
    return NULL;
}

static Tile *findFallback(int wx, int wz, int ww, int wh, int exScale, uint64_t seed, int dim) {
    Tile *best = NULL; int bs = -999999;
    for (int i = 0; i < tileCount; i++) {
        Tile *t = &tileCache[i];
        if (!t->valid || t->seed!=seed || t->dim!=dim || t->scale==exScale) continue;
        int tw = tileWorldSize(t->scale), ox = t->tx*tw, oz = t->tz*tw;
        if (ox > wx || oz > wz || ox+tw < wx+ww || oz+tw < wz+wh) continue;
        int sc = -t->scale*1000 + t->lastUsedFrame;
        if (sc > bs) { best = t; bs = sc; }
    }
    return best;
}

static int evictLRU(int cs) {
    int v=-1, of=frameCounter+1, vc=1;
    for (int i = 0; i < tileCount; i++) {
        if (!tileCache[i].valid) return i;
        int ic = (tileCache[i].scale == cs);
        if (vc && !ic) { v=i; of=tileCache[i].lastUsedFrame; vc=0; }
        else if (ic==vc && tileCache[i].lastUsedFrame < of) { v=i; of=tileCache[i].lastUsedFrame; vc=ic; }
    }
    if (v >= 0 && tileCache[v].valid) { UnloadTexture(tileCache[v].tex); tileCache[v].valid = 0; }
    return v;
}

static Tile *insertTile(int tx, int tz, int scale, uint64_t seed, int dim, Texture2D tex, int cs) {
    int idx = -1;
    for (int i = 0; i < tileCount; i++) if (!tileCache[i].valid) { idx = i; break; }
    if (idx < 0 && tileCount < MAX_TILES) idx = tileCount++;
    if (idx < 0) idx = evictLRU(cs);
    if (idx < 0) return NULL;
    Tile *t = &tileCache[idx];
    t->tx=tx; t->tz=tz; t->scale=scale; t->seed=seed; t->dim=dim;
    t->tex=tex; t->valid=1; t->lastUsedFrame=frameCounter;
    return t;
}

static void flushRequests(void) {
    mutexLock(&reqMutex); reqHead=reqTail=reqCount=0; mutexUnlock(&reqMutex);
}

static void invalidateAll(void) {
    for (int i = 0; i < tileCount; i++)
        if (tileCache[i].valid) { UnloadTexture(tileCache[i].tex); tileCache[i].valid = 0; }
    tileCount = 0;
    flushRequests();
    mutexLock(&rdyMutex);
    for (int i = 0; i < rdyCount; i++) free(readyQueue[(rdyHead+i)%QUEUE_SIZE].pixels);
    rdyHead=rdyTail=rdyCount=0;
    mutexUnlock(&rdyMutex);
}

static void enqueueRequest(TileRequest req) {
    mutexLock(&reqMutex);
    if (reqCount < QUEUE_SIZE) {
        requestQueue[reqTail] = req; reqTail = (reqTail+1)%QUEUE_SIZE; reqCount++;
        condSignal(&reqCond);
    }
    mutexUnlock(&reqMutex);
}

static int dequeueRequest(TileRequest *out) {
    mutexLock(&reqMutex);
    while (reqCount == 0 && workerRunning) condWait(&reqCond, &reqMutex);
    if (!workerRunning && reqCount == 0) { mutexUnlock(&reqMutex); return 0; }
    *out = requestQueue[reqHead]; reqHead = (reqHead+1)%QUEUE_SIZE; reqCount--;
    mutexUnlock(&reqMutex); return 1;
}

static void enqueueReady(ReadyTile rt) {
    mutexLock(&rdyMutex);
    if (rdyCount < QUEUE_SIZE) { readyQueue[rdyTail]=rt; rdyTail=(rdyTail+1)%QUEUE_SIZE; rdyCount++; }
    else free(rt.pixels);
    mutexUnlock(&rdyMutex);
}

static int dequeueReady(ReadyTile *out) {
    mutexLock(&rdyMutex);
    if (rdyCount == 0) { mutexUnlock(&rdyMutex); return 0; }
    *out = readyQueue[rdyHead]; rdyHead=(rdyHead+1)%QUEUE_SIZE; rdyCount--;
    mutexUnlock(&rdyMutex); return 1;
}

static int isRequested(int tx, int tz, int scale, uint64_t seed, int dim) {
    mutexLock(&reqMutex);
    for (int i = 0; i < reqCount; i++) {
        TileRequest *r = &requestQueue[(reqHead+i)%QUEUE_SIZE];
        if (r->tx==tx && r->tz==tz && r->scale==scale && r->seed==seed && r->dim==dim)
        { mutexUnlock(&reqMutex); return 1; }
    }
    mutexUnlock(&reqMutex);
    mutexLock(&rdyMutex);
    for (int i = 0; i < rdyCount; i++) {
        ReadyTile *r = &readyQueue[(rdyHead+i)%QUEUE_SIZE];
        if (r->tx==tx && r->tz==tz && r->scale==scale && r->seed==seed && r->dim==dim)
        { mutexUnlock(&rdyMutex); return 1; }
    }
    mutexUnlock(&rdyMutex); return 0;
}

//==============================================================================
// Worker pool
//==============================================================================

static
#ifdef _WIN32
DWORD WINAPI
#else
void *
#endif
workerFunc(void *arg) {
    (void)arg; omp_set_num_threads(1);
    unsigned char bc[256][3]; initBiomeColors(bc);
    Generator g; setupGenerator(&g, MC_1_21, 0);
    uint64_t ls = 0; int ld = DIM_UNDEF;
    while (workerRunning) {
        TileRequest req; if (!dequeueRequest(&req)) break;
        if (req.seed != ls || req.dim != ld) { applySeed(&g, req.dim, req.seed); ls=req.seed; ld=req.dim; }
        Range r; r.scale=req.scale; r.x=req.tx*TILE_PX; r.z=req.tz*TILE_PX;
        r.sx=TILE_PX; r.sz=TILE_PX; r.y=319>>2; r.sy=0;
        int *ids = allocCache(&g, r); if (!ids) continue;
        genBiomes(&g, ids, r);
        Color *px = (Color*)malloc(TILE_PX*TILE_PX*sizeof(Color));
        if (!px) { free(ids); continue; }
        for (int j = 0; j < TILE_PX*TILE_PX; j++) {
            int id = ids[j];
            px[j] = (id < 0 || id >= 256) ? (Color){0,0,0,255}
                : (Color){bc[id][0], bc[id][1], bc[id][2], 255};
        }
        free(ids);
        ReadyTile rt = {req.tx, req.tz, req.scale, req.seed, req.dim, px};
        enqueueReady(rt);
    }
    return 0;
}

//==============================================================================
// Structure drawing on map
//==============================================================================

static void drawStructures(uint64_t seed, int dim, int mc,
    double camX, double camZ, float zoom, float mapCX, float mapCY)
{
    if (zoom < 0.3f) return;
    double hw = (MAP_W/2.0)/zoom, hh = (MAP_H/2.0)/zoom;
    double mnX = camX-hw, mxX = camX+hw, mnZ = camZ-hh, mxZ = camZ+hh;

    int stypes[] = {Village,Desert_Pyramid,Jungle_Temple,Swamp_Hut,Igloo,Monument,
                    Mansion,Outpost,Ancient_City,Trail_Ruins,Trial_Chambers};
    Color scolors[] = {{255,255,255,220},{255,200,50,220},{50,180,50,220},{120,50,200,220},
        {130,200,255,220},{40,40,200,220},{150,100,50,220},{180,180,180,220},
        {40,40,90,220},{180,120,60,220},{200,70,70,220}};
    int ns = 11;
    if (dim == DIM_NETHER) {
        stypes[0]=Fortress; stypes[1]=Bastion; ns=2;
        scolors[0]=(Color){200,50,50,220}; scolors[1]=(Color){100,100,100,220};
    } else if (dim == DIM_END) {
        stypes[0]=End_City; ns=1; scolors[0]=(Color){180,160,240,220};
    }

    Generator g; setupGenerator(&g, mc, 0); applySeed(&g, dim, seed);
    Vector2 mouse = GetMousePosition();
    for (int si = 0; si < ns; si++) {
        StructureConfig sc; if (!getStructureConfig(stypes[si], mc, &sc)) continue;
        int rs = sc.regionSize*16;
        for (int rz = (int)floor(mnZ/rs)-1; rz <= (int)floor(mxZ/rs)+1; rz++)
        for (int rx = (int)floor(mnX/rs)-1; rx <= (int)floor(mxX/rs)+1; rx++) {
            Pos p; if (!getStructurePos(stypes[si], mc, seed, rx, rz, &p)) continue;
            if (p.x<mnX||p.x>mxX||p.z<mnZ||p.z>mxZ) continue;
            float sx = (float)((p.x-camX)*zoom)+mapCX;
            float sy = (float)((p.z-camZ)*zoom)+mapCY;
            float rd = (zoom > 1.0f) ? 6.0f : 4.0f;
            DrawCircle((int)sx, (int)sy, rd+1, (Color){0,0,0,80});
            DrawCircle((int)sx, (int)sy, rd, scolors[si]);
            // Structure tooltip on hover
            float dx = mouse.x-sx, dy = mouse.y-sy;
            if (dx*dx+dy*dy < (rd+5)*(rd+5)) {
                const char *nm = structDisplayName(stypes[si]);
                char tip[80]; snprintf(tip, 80, "%s (%d, %d)", nm, p.x, p.z);
                int tw2 = MeasureTextF(tip, 13) + 12;
                int tipX = (int)sx+12, tipY = (int)sy-12;
                if (tipX+tw2 > WIN_W-RIGHT_W) tipX = (int)sx-tw2-4;
                DrawRectangle(tipX, tipY, tw2, 22, (Color){255,255,255,235});
                DrawRectangleLines(tipX, tipY, tw2, 22, C_BORDER);
                DrawTextF(tip, tipX+6, tipY+4, 13, C_TEXT);
            }
        }
    }
}

//==============================================================================
// Right sidebar data tables (alphabetical, by dimension)
//==============================================================================

typedef struct { const char *name; int id; Color col; } Entry;

static const Entry owStructs[] = {
    {"Ancient City",      Ancient_City,    {40,40,90,255}},
    {"Buried Treasure",   Treasure,        {200,170,50,255}},
    {"Desert Pyramid",    Desert_Pyramid,  {255,200,50,255}},
    {"Igloo",             Igloo,           {130,200,255,255}},
    {"Jungle Temple",     Jungle_Temple,   {50,180,50,255}},
    {"Mineshaft",         Mineshaft,       {130,100,70,255}},
    {"Ocean Monument",    Monument,        {40,40,200,255}},
    {"Ocean Ruin",        Ocean_Ruin,      {40,120,140,255}},
    {"Pillager Outpost",  Outpost,         {180,180,180,255}},
    {"Ruined Portal",     Ruined_Portal,   {180,80,220,255}},
    {"Shipwreck",         Shipwreck,       {100,80,60,255}},
    {"Swamp Hut",         Swamp_Hut,       {120,50,200,255}},
    {"Trail Ruins",       Trail_Ruins,     {180,120,60,255}},
    {"Trial Chambers",    Trial_Chambers,  {200,70,70,255}},
    {"Village",           Village,         {120,120,120,255}},
    {"Woodland Mansion",  Mansion,         {150,100,50,255}},
};
#define N_OW_STRUCTS (int)(sizeof(owStructs)/sizeof(owStructs[0]))

static const Entry netherStructs[] = {
    {"Bastion Remnant",   Bastion,         {100,100,100,255}},
    {"Nether Fortress",   Fortress,        {200,50,50,255}},
    {"Ruined Portal",     Ruined_Portal_N, {180,80,220,255}},
};
#define N_NETHER_STRUCTS (int)(sizeof(netherStructs)/sizeof(netherStructs[0]))

static const Entry endStructs[] = {
    {"End City",          End_City,        {180,160,240,255}},
    {"End Gateway",       End_Gateway,     {240,240,100,255}},
};
#define N_END_STRUCTS (int)(sizeof(endStructs)/sizeof(endStructs[0]))

// Overworld biomes (1.18+ that actually generate)
static const Entry owBiomes[] = {
    {"Badlands",                badlands,               {0}},
    {"Bamboo Jungle",           bamboo_jungle,          {0}},
    {"Beach",                   beach,                  {0}},
    {"Birch Forest",            birch_forest,           {0}},
    {"Cherry Grove",            cherry_grove,           {0}},
    {"Cold Ocean",              cold_ocean,             {0}},
    {"Dark Forest",             dark_forest,            {0}},
    {"Deep Cold Ocean",         deep_cold_ocean,        {0}},
    {"Deep Dark",               deep_dark,              {0}},
    {"Deep Frozen Ocean",       deep_frozen_ocean,      {0}},
    {"Deep Lukewarm Ocean",     deep_lukewarm_ocean,    {0}},
    {"Deep Ocean",              deep_ocean,             {0}},
    {"Desert",                  desert,                 {0}},
    {"Dripstone Caves",         dripstone_caves,        {0}},
    {"Eroded Badlands",         eroded_badlands,        {0}},
    {"Flower Forest",           flower_forest,          {0}},
    {"Forest",                  forest,                 {0}},
    {"Frozen Ocean",            frozen_ocean,           {0}},
    {"Frozen Peaks",            frozen_peaks,           {0}},
    {"Frozen River",            frozen_river,           {0}},
    {"Grove",                   grove,                  {0}},
    {"Ice Spikes",              ice_spikes,             {0}},
    {"Jagged Peaks",            jagged_peaks,           {0}},
    {"Jungle",                  jungle,                 {0}},
    {"Lukewarm Ocean",          lukewarm_ocean,         {0}},
    {"Lush Caves",              lush_caves,             {0}},
    {"Mangrove Swamp",          mangrove_swamp,         {0}},
    {"Meadow",                  meadow,                 {0}},
    {"Mushroom Fields",         mushroom_fields,        {0}},
    {"Ocean",                   ocean,                  {0}},
    {"Old Growth Birch Forest", tall_birch_forest,      {0}},
    {"Old Growth Pine Taiga",   giant_tree_taiga,       {0}},
    {"Old Growth Spruce Taiga", giant_spruce_taiga,     {0}},
    {"Pale Garden",             pale_garden,            {0}},
    {"Plains",                  plains,                 {0}},
    {"River",                   river,                  {0}},
    {"Savanna",                 savanna,                {0}},
    {"Savanna Plateau",         savanna_plateau,        {0}},
    {"Snowy Beach",             snowy_beach,            {0}},
    {"Snowy Plains",            snowy_tundra,           {0}},
    {"Snowy Slopes",            snowy_slopes,           {0}},
    {"Snowy Taiga",             snowy_taiga,            {0}},
    {"Sparse Jungle",           jungle_edge,            {0}},
    {"Stony Peaks",             stony_peaks,            {0}},
    {"Stony Shore",             stone_shore,            {0}},
    {"Sunflower Plains",        sunflower_plains,       {0}},
    {"Swamp",                   swamp,                  {0}},
    {"Taiga",                   taiga,                  {0}},
    {"Warm Ocean",              warm_ocean,             {0}},
    {"Windswept Forest",        wooded_mountains,       {0}},
    {"Windswept Gravelly Hills",gravelly_mountains,     {0}},
    {"Windswept Hills",         mountains,              {0}},
    {"Windswept Savanna",       shattered_savanna,      {0}},
    {"Wooded Badlands",         wooded_badlands_plateau,{0}},
};
#define N_OW_BIOMES (int)(sizeof(owBiomes)/sizeof(owBiomes[0]))

static const Entry netherBiomes[] = {
    {"Basalt Deltas",       basalt_deltas,      {0}},
    {"Crimson Forest",      crimson_forest,     {0}},
    {"Nether Wastes",       nether_wastes,      {0}},
    {"Soul Sand Valley",    soul_sand_valley,   {0}},
    {"Warped Forest",       warped_forest,      {0}},
};
#define N_NETHER_BIOMES (int)(sizeof(netherBiomes)/sizeof(netherBiomes[0]))

static const Entry endBiomes[] = {
    {"End Barrens",         end_barrens,        {0}},
    {"End Highlands",       end_highlands,      {0}},
    {"End Midlands",        end_midlands,       {0}},
    {"Small End Islands",   small_end_islands,  {0}},
    {"The End",             the_end,            {0}},
};
#define N_END_BIOMES (int)(sizeof(endBiomes)/sizeof(endBiomes[0]))

//==============================================================================
// UI helpers
//==============================================================================

static int uiButton(Rectangle r, const char *label, int fs) {
    int hov = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRec(r, hov ? C_BTN_HI : C_BTN);
    DrawRectangleLinesEx(r, 1, C_BORDER);
    int tw = MeasureTextF(label, fs);
    DrawTextF(label, (int)(r.x+(r.width-tw)/2), (int)(r.y+(r.height-fs)/2), fs, C_TEXT);
    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static int uiToggle(Rectangle r, const char *label, int active, int fs) {
    int hov = CheckCollisionPointRec(GetMousePosition(), r);
    Color bg = active ? C_BTN_ACTIVE : (hov ? C_BTN_HI : C_BTN);
    Color fg = active ? WHITE : C_TEXT;
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 1, active ? C_ACCENT_HI : C_BORDER);
    int tw = MeasureTextF(label, fs);
    DrawTextF(label, (int)(r.x+(r.width-tw)/2), (int)(r.y+(r.height-fs)/2), fs, fg);
    return hov && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

static void uiSectionHeader(int x, int y, int w, const char *label) {
    DrawRectangle(x, y, w, 26, C_SECTION_BG);
    DrawLine(x, y+26, x+w, y+26, C_BORDER);
    DrawTextF(label, x+10, y+6, 13, C_SECTION_HDR);
}

// Seed box
typedef struct { char buf[SEED_BUF_LEN]; int len, active; } SeedBox;

static int updateSeedBox(SeedBox *sb, Rectangle r, uint64_t *out) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        sb->active = CheckCollisionPointRec(GetMousePosition(), r);
    if (!sb->active) return 0;
    int key;
    while ((key = GetCharPressed()) != 0)
        if (sb->len < SEED_BUF_LEN-1 && ((key>='0'&&key<='9')||(key=='-'&&sb->len==0)))
        { sb->buf[sb->len++]=(char)key; sb->buf[sb->len]='\0'; }
    if (IsKeyPressed(KEY_BACKSPACE) && sb->len > 0) sb->buf[--sb->len]='\0';
    if (IsKeyPressed(KEY_ENTER) && sb->len > 0) {
        *out = (uint64_t)strtoll(sb->buf, NULL, 10); sb->active = 0; return 1;
    }
    return 0;
}

static void drawSeedBox(SeedBox *sb, Rectangle r) {
    DrawRectangleRec(r, sb->active ? C_INPUT_FOCUS : C_INPUT_BG);
    DrawRectangleLinesEx(r, 1, sb->active ? C_ACCENT : C_BORDER);
    const char *txt = sb->len > 0 ? sb->buf : "Enter seed...";
    DrawTextF(txt, (int)r.x+10, (int)r.y+8, 14, sb->len > 0 ? C_TEXT : C_TEXT_DIM);
    if (sb->active && ((int)(GetTime()*2)%2==0)) {
        int cx = (int)r.x + 10 + MeasureTextF(sb->buf, 14);
        DrawTextF("|", cx, (int)r.y+8, 14, C_ACCENT);
    }
}

// Scrollable entry list
static float drawEntryList(const Entry *entries, int count,
    unsigned char biomeColors[256][3], int isStruct,
    int x, int y, int w, int h, float scrollY, int hoverBiomeId)
{
    int rowH = 24;
    int contentH = count * rowH;
    float maxScroll = (contentH > h) ? (float)(contentH - h) : 0;
    if (scrollY < 0) scrollY = 0;
    if (scrollY > maxScroll) scrollY = maxScroll;

    BeginScissorMode(x, y, w, h);
    for (int i = 0; i < count; i++) {
        int ry = y + i * rowH - (int)scrollY;
        if (ry + rowH < y || ry > y + h) continue;

        int highlight = (!isStruct && entries[i].id == hoverBiomeId);
        if (highlight)
            DrawRectangle(x, ry, w, rowH, C_ROW_HI);
        else if (i % 2 == 0)
            DrawRectangle(x, ry, w, rowH, (Color){248,248,252,255});

        // Color swatch
        Color sc;
        if (isStruct) {
            sc = entries[i].col;
        } else {
            int id = entries[i].id;
            sc = (id >= 0 && id < 256)
                ? (Color){biomeColors[id][0], biomeColors[id][1], biomeColors[id][2], 255}
                : (Color){200, 200, 200, 255};
        }
        DrawRectangleRounded((Rectangle){(float)(x+8), (float)(ry+5), 14, 14}, 0.3f, 4, sc);
        DrawRectangleRoundedLinesEx((Rectangle){(float)(x+8), (float)(ry+5), 14, 14}, 0.3f, 4, 1, C_BORDER);

        DrawTextF(entries[i].name, x+28, ry+5, 13, highlight ? C_ACCENT : C_TEXT);
    }
    EndScissorMode();

    // Scrollbar track + thumb
    if (contentH > h) {
        DrawRectangle(x+w-6, y, 6, h, (Color){230,230,235,255});
        float barH = (float)h * h / contentH;
        if (barH < 20) barH = 20;
        float barY = y + (scrollY / maxScroll) * (h - barH);
        DrawRectangleRounded((Rectangle){(float)(x+w-5), barY, 4, barH}, 0.5f, 4, (Color){180,180,195,255});
    }
    return scrollY;
}

//==============================================================================
// Main
//==============================================================================

int main(void)
{
    fprintf(stderr, "[LOG] main: start\n"); fflush(stderr);
    int cores = getNumCores();
    numWorkers = cores - 2;
    if (numWorkers < 2) numWorkers = 2;
    if (numWorkers > MAX_WORKERS) numWorkers = MAX_WORKERS;
    fprintf(stderr, "[LOG] cores=%d workers=%d\n", cores, numWorkers); fflush(stderr);

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(WIN_W, WIN_H, "Cubiomes Viewer");
    SetTargetFPS(60);
    fprintf(stderr, "[LOG] window created\n"); fflush(stderr);

    // Load system font (Segoe UI on Windows, fallback to default)
#ifdef _WIN32
    {
        const char *fontPath = "C:\\Windows\\Fonts\\segoeui.ttf";
        if (FileExists(fontPath)) {
            appFont = LoadFontEx(fontPath, 24, NULL, 0);
            if (appFont.glyphCount > 0) {
                SetTextureFilter(appFont.texture, TEXTURE_FILTER_BILINEAR);
                fontLoaded = 1;
            }
        }
    }
#endif

    fprintf(stderr, "[LOG] font loaded=%d\n", fontLoaded); fflush(stderr);

    uint64_t seed = 0;
    int dim = DIM_OVERWORLD;
    int mc = MC_1_21;
    uint32_t genFlags = 0;
    double camX = 0, camZ = 0;
    float zoom = 0.25f;
    int curScale = pickScale(zoom);

    int dragging = 0;
    Vector2 dragStart = {0};
    double dragCamX = 0, dragCamZ = 0;

    SeedBox seedBox = {0};

    // Hover biome
    Generator hoverGen;
    setupGenerator(&hoverGen, mc, 0);
    int hoverNeedApply = 1;
    uint64_t hoverSeed = ~seed;
    int hoverDim = DIM_UNDEF;
    int hoverBiomeId = -1;

    unsigned char biomeColors[256][3];
    initBiomeColors(biomeColors);

    float scrollStructs = 0, scrollBiomes = 0;

    mutexInit(&reqMutex); mutexInit(&rdyMutex); condInit(&reqCond);
    memset(tileCache, 0, sizeof(tileCache));
    fprintf(stderr, "[LOG] starting %d workers\n", numWorkers); fflush(stderr);
    Thread workers[MAX_WORKERS];
    for (int i = 0; i < numWorkers; i++)
        workers[i] = threadCreate(workerFunc, (void*)(intptr_t)i);
    fprintf(stderr, "[LOG] workers started\n"); fflush(stderr);

    Rectangle mapRect = {MAP_X, 0, MAP_W, MAP_H};
    Rectangle rightRect = {(float)(WIN_W-RIGHT_W), 0, RIGHT_W, (float)WIN_H};

    fprintf(stderr, "[LOG] entering main loop\n"); fflush(stderr);
    while (!WindowShouldClose())
    {
        frameCounter++;
        if (frameCounter <= 3) { fprintf(stderr, "[LOG] frame %d start\n", frameCounter); fflush(stderr); }
        Vector2 mouse = GetMousePosition();
        int mouseInRight = CheckCollisionPointRec(mouse, rightRect);
        float mapCX = MAP_X + MAP_W / 2.0f;
        float mapCY = MAP_H / 2.0f;

        // --- Seed ---
        Rectangle seedRect = {16, 64, LEFT_W-32, 32};
        uint64_t newSeed = seed;
        if (updateSeedBox(&seedBox, seedRect, &newSeed) && newSeed != seed) {
            seed = newSeed; invalidateAll(); hoverNeedApply = 1;
        }

        // --- Zoom ---
        float wheel = GetMouseWheelMove();
        if (wheel != 0 && !seedBox.active) {
            if (CheckCollisionPointRec(mouse, mapRect)) {
                double wmx = camX + (mouse.x - mapCX) / zoom;
                double wmz = camZ + (mouse.y - mapCY) / zoom;
                zoom *= (wheel > 0) ? 1.15f : (1.0f / 1.15f);
                if (zoom < 0.002f) zoom = 0.002f;
                if (zoom > 32.0f) zoom = 32.0f;
                camX = wmx - (mouse.x - mapCX) / zoom;
                camZ = wmz - (mouse.y - mapCY) / zoom;
                int ns = pickScale(zoom);
                if (ns != curScale) { curScale = ns; flushRequests(); }
            } else if (mouseInRight) {
                float mid = WIN_H / 2.0f;
                if (mouse.y < mid) scrollStructs -= wheel * 40;
                else scrollBiomes -= wheel * 40;
            }
        }

        // --- Pan ---
        if (!seedBox.active) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, mapRect)) {
                dragging = 1; dragStart = mouse; dragCamX = camX; dragCamZ = camZ;
            }
            if (dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                camX = dragCamX - (mouse.x - dragStart.x) / zoom;
                camZ = dragCamZ - (mouse.y - dragStart.y) / zoom;
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) dragging = 0;
        }

        // --- Biome hover (left sidebar only, no tooltip) ---
        if (CheckCollisionPointRec(mouse, mapRect) && (frameCounter % 3 == 0)) {
            double wx = camX + (mouse.x - mapCX) / zoom;
            double wz = camZ + (mouse.y - mapCY) / zoom;
            if (hoverNeedApply || hoverSeed != seed || hoverDim != dim) {
                applySeed(&hoverGen, dim, seed); hoverSeed = seed; hoverDim = dim; hoverNeedApply = 0;
            }
            hoverBiomeId = getBiomeAt(&hoverGen, 4, (int)wx >> 2, 319 >> 2, (int)wz >> 2);
        }

        // --- Upload tiles ---
        { ReadyTile rt; int up = 0;
          while (dequeueReady(&rt) && up < 32) {
            if (rt.seed != seed || rt.dim != dim) { free(rt.pixels); continue; }
            Image img = {.data=rt.pixels, .width=TILE_PX, .height=TILE_PX, .mipmaps=1, .format=PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
            Texture2D tex = LoadTextureFromImage(img);
            SetTextureFilter(tex, TEXTURE_FILTER_POINT);
            free(rt.pixels);
            insertTile(rt.tx, rt.tz, rt.scale, rt.seed, rt.dim, tex, curScale);
            up++;
        }}

        // --- Visible tiles ---
        int tw = tileWorldSize(curScale);
        double hw = (MAP_W/2.0)/zoom, hh = (MAP_H/2.0)/zoom;
        int mnTX = (int)floor((camX-hw)/tw), mxTX = (int)floor((camX+hw)/tw);
        int mnTZ = (int)floor((camZ-hh)/tw), mxTZ = (int)floor((camZ+hh)/tw);
        for (int tz = mnTZ; tz <= mxTZ; tz++)
            for (int tx = mnTX; tx <= mxTX; tx++)
                if (!findTile(tx,tz,curScale,seed,dim) && !isRequested(tx,tz,curScale,seed,dim))
                { TileRequest rq = {tx,tz,curScale,seed,dim,mc,genFlags}; enqueueRequest(rq); }

        //==============================================================
        // DRAW
        //==============================================================
        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d BeginDrawing\n", frameCounter); fflush(stderr); }
        BeginDrawing();
        ClearBackground(C_BG);

        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d MAP\n", frameCounter); fflush(stderr); }
        // ---- MAP ----
        BeginScissorMode(MAP_X, 0, MAP_W, MAP_H);
        DrawRectangle(MAP_X, 0, MAP_W, MAP_H, C_MAP_BG);

        for (int tz = mnTZ; tz <= mxTZ; tz++) for (int tx = mnTX; tx <= mxTX; tx++) {
            int wx = tx*tw, wz = tz*tw;
            float sx = (float)((wx-camX)*zoom)+mapCX, sy = (float)((wz-camZ)*zoom)+mapCY;
            float sw = (float)(tw*zoom);
            Tile *t = findTile(tx, tz, curScale, seed, dim);
            if (t) { t->lastUsedFrame = frameCounter;
                DrawTexturePro(t->tex, (Rectangle){0,0,(float)TILE_PX,(float)TILE_PX},
                    (Rectangle){sx,sy,sw,sw}, (Vector2){0,0}, 0, WHITE);
            } else {
                Tile *fb = findFallback(wx, wz, tw, tw, curScale, seed, dim);
                if (fb) { fb->lastUsedFrame = frameCounter;
                    int fbw = tileWorldSize(fb->scale), fbox = fb->tx*fbw, fboz = fb->tz*fbw;
                    float srcX = (float)(wx-fbox)/fbw*TILE_PX, srcY = (float)(wz-fboz)/fbw*TILE_PX;
                    float srcW = (float)tw/fbw*TILE_PX;
                    DrawTexturePro(fb->tex, (Rectangle){srcX,srcY,srcW,srcW},
                        (Rectangle){sx,sy,sw,sw}, (Vector2){0,0}, 0, WHITE);
                }
            }
        }

        // Origin cross
        { float ox = (float)(-camX*zoom)+mapCX, oy = (float)(-camZ*zoom)+mapCY;
          DrawLine((int)ox-12,(int)oy,(int)ox+12,(int)oy,(Color){255,255,255,100});
          DrawLine((int)ox,(int)oy-12,(int)ox,(int)oy+12,(Color){255,255,255,100}); }

        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d drawStructures\n", frameCounter); fflush(stderr); }
        drawStructures(seed, dim, mc, camX, camZ, zoom, mapCX, mapCY);
        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d drawStructures done\n", frameCounter); fflush(stderr); }
        EndScissorMode();

        // Map overlay buttons (bottom-right of map area)
        { int bx = WIN_W-RIGHT_W-54, by = MAP_H-170;
          if (uiButton((Rectangle){(float)bx,(float)by,42,42},"+",20))
          { zoom*=1.5f; if(zoom>32)zoom=32; int ns=pickScale(zoom); if(ns!=curScale){curScale=ns;flushRequests();} }
          if (uiButton((Rectangle){(float)bx,(float)(by+50),42,42},"-",20))
          { zoom/=1.5f; if(zoom<0.002f)zoom=0.002f; int ns=pickScale(zoom); if(ns!=curScale){curScale=ns;flushRequests();} }
          if (uiButton((Rectangle){(float)bx,(float)(by+100),42,42},"C",15))
          { camX=0; camZ=0; }
        }

        // Map status bar
        { double wx2 = camX+(mouse.x-mapCX)/zoom, wz2 = camZ+(mouse.y-mapCY)/zoom;
          char bar[140];
          snprintf(bar, 140, "X: %d   Z: %d   |   Scale 1:%d   |   Zoom x%.2f   |   %d FPS   |   %d threads",
              (int)wx2, (int)wz2, curScale, zoom, GetFPS(), numWorkers);
          DrawRectangle(MAP_X, MAP_H-26, MAP_W, 26, (Color){255,255,255,200});
          DrawLine(MAP_X, MAP_H-26, MAP_X+MAP_W, MAP_H-26, C_BORDER);
          DrawTextF(bar, MAP_X+12, MAP_H-21, 13, C_TEXT_SEC);
        }

        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d LEFT\n", frameCounter); fflush(stderr); }
        // ---- LEFT SIDEBAR ----
        DrawRectangle(0, 0, LEFT_W, WIN_H, C_SIDEBAR);
        DrawLine(LEFT_W-1, 0, LEFT_W-1, WIN_H, C_BORDER);

        // Title
        DrawTextF("Cubiomes Viewer", 16, 16, 18, C_ACCENT);
        DrawTextF("MC 1.21", 16+MeasureTextF("Cubiomes Viewer",18)+8, 20, 12, C_TEXT_DIM);

        // Seed
        DrawTextF("Seed", 16, 48, 13, C_TEXT_SEC);
        drawSeedBox(&seedBox, seedRect);

        // Dimension
        DrawTextF("Dimension", 16, 110, 13, C_TEXT_SEC);
        { float bw = (LEFT_W - 44) / 3.0f; int y = 130;
          if (uiToggle((Rectangle){16,(float)y,bw,28},"Overworld",dim==DIM_OVERWORLD,12))
          { dim=DIM_OVERWORLD; invalidateAll(); hoverNeedApply=1; scrollStructs=0; scrollBiomes=0; }
          if (uiToggle((Rectangle){16+bw+6,(float)y,bw,28},"Nether",dim==DIM_NETHER,12))
          { dim=DIM_NETHER; invalidateAll(); hoverNeedApply=1; scrollStructs=0; scrollBiomes=0; }
          if (uiToggle((Rectangle){16+2*(bw+6),(float)y,bw,28},"End",dim==DIM_END,12))
          { dim=DIM_END; invalidateAll(); hoverNeedApply=1; scrollStructs=0; scrollBiomes=0; }
        }

        DrawLine(16, 172, LEFT_W-16, 172, C_BORDER);

        // Biome under cursor
        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d biome card\n", frameCounter); fflush(stderr); }
        DrawTextF("Biome Under Cursor", 16, 182, 13, C_TEXT_SEC);
        if (CheckCollisionPointRec(mouse, mapRect) && hoverBiomeId >= 0 && hoverBiomeId < 256) {
            Color bc = {biomeColors[hoverBiomeId][0], biomeColors[hoverBiomeId][1], biomeColors[hoverBiomeId][2], 255};
            // Card-style display
            DrawRectangleRounded((Rectangle){16, 202, LEFT_W-32, 52}, 0.15f, 4, C_PANEL);
            DrawRectangleRoundedLinesEx((Rectangle){16, 202, LEFT_W-32, 52}, 0.15f, 4, 1, C_BORDER);
            DrawRectangleRounded((Rectangle){26, 212, 20, 20}, 0.3f, 4, bc);
            DrawRectangleRoundedLinesEx((Rectangle){26, 212, 20, 20}, 0.3f, 4, 1, C_BORDER);
            const char *bname = biomeDisplayName(hoverBiomeId);
            DrawTextF(bname, 54, 210, 14, C_TEXT);
            char ids[24]; snprintf(ids, 24, "ID: %d", hoverBiomeId);
            DrawTextF(ids, 54, 228, 12, C_TEXT_DIM);
        } else {
            DrawRectangleRounded((Rectangle){16, 202, LEFT_W-32, 52}, 0.15f, 4, C_PANEL);
            DrawRectangleRoundedLinesEx((Rectangle){16, 202, LEFT_W-32, 52}, 0.15f, 4, 1, C_BORDER);
            DrawTextF("Hover over the map", 26, 220, 13, C_TEXT_DIM);
        }

        DrawLine(16, 268, LEFT_W-16, 268, C_BORDER);

        // Seed Filters placeholder
        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d seed filters\n", frameCounter); fflush(stderr); }
        DrawTextF("Seed Filters", 16, 278, 13, C_TEXT_SEC);
        DrawRectangleRounded((Rectangle){16, 300, LEFT_W-32, 260}, 0.1f, 4, C_PANEL);
        DrawRectangleRoundedLinesEx((Rectangle){16, 300, LEFT_W-32, 260}, 0.1f, 4, 1, C_BORDER);
        DrawTextF("Add filter conditions to", 28, 320, 13, C_TEXT_DIM);
        DrawTextF("search for seeds matching", 28, 338, 13, C_TEXT_DIM);
        DrawTextF("specific criteria.", 28, 356, 13, C_TEXT_DIM);
        DrawTextF("e.g. 3 villages within", 28, 386, 12, C_TEXT_DIM);
        DrawTextF("300 blocks of spawn,", 28, 402, 12, C_TEXT_DIM);
        DrawTextF("mushroom island nearby...", 28, 418, 12, C_TEXT_DIM);

        DrawLine(16, 574, LEFT_W-16, 574, C_BORDER);

        // Controls
        DrawTextF("Controls", 16, 584, 13, C_TEXT_SEC);
        DrawTextF("Drag         Pan map", 16, 606, 12, C_TEXT_DIM);
        DrawTextF("Scroll       Zoom in/out", 16, 622, 12, C_TEXT_DIM);
        DrawTextF("1 / 2 / 3    Switch dimension", 16, 638, 12, C_TEXT_DIM);

        // Bottom
        { char info[80]; snprintf(info, 80, "%d workers  |  %d cores", numWorkers, cores);
          DrawTextF(info, 16, WIN_H-24, 11, C_TEXT_DIM); }

        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d RIGHT\n", frameCounter); fflush(stderr); }
        // ---- RIGHT SIDEBAR ----
        int rx = WIN_W - RIGHT_W;
        DrawRectangle(rx, 0, RIGHT_W, WIN_H, C_SIDEBAR);
        DrawLine(rx, 0, rx, WIN_H, C_BORDER);

        const Entry *curStructs; int nCS;
        const Entry *curBiomesArr; int nCB;
        const char *dimLabel;
        if (dim == DIM_NETHER) {
            curStructs=netherStructs; nCS=N_NETHER_STRUCTS;
            curBiomesArr=netherBiomes; nCB=N_NETHER_BIOMES; dimLabel="Nether";
        } else if (dim == DIM_END) {
            curStructs=endStructs; nCS=N_END_STRUCTS;
            curBiomesArr=endBiomes; nCB=N_END_BIOMES; dimLabel="End";
        } else {
            curStructs=owStructs; nCS=N_OW_STRUCTS;
            curBiomesArr=owBiomes; nCB=N_OW_BIOMES; dimLabel="Overworld";
        }

        // Structures
        { char hdr[64]; snprintf(hdr, 64, "Structures - %s", dimLabel);
          uiSectionHeader(rx, 0, RIGHT_W, hdr); }
        int structH = WIN_H / 2 - 26;
        scrollStructs = drawEntryList(curStructs, nCS, biomeColors, 1,
            rx, 26, RIGHT_W, structH, scrollStructs, hoverBiomeId);

        // Biomes
        int biomeY = WIN_H / 2;
        { char hdr[64]; snprintf(hdr, 64, "Biomes - %s", dimLabel);
          uiSectionHeader(rx, biomeY, RIGHT_W, hdr); }
        int biomeH = WIN_H - biomeY - 26;
        scrollBiomes = drawEntryList(curBiomesArr, nCB, biomeColors, 0,
            rx, biomeY+26, RIGHT_W, biomeH, scrollBiomes, hoverBiomeId);

        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d EndDrawing\n", frameCounter); fflush(stderr); }
        EndDrawing();
        if (frameCounter <= 3) { fprintf(stderr, "[LOG] f%d done\n", frameCounter); fflush(stderr); }
    }

    // Shutdown
    workerRunning = 0;
    mutexLock(&reqMutex); condBroadcast(&reqCond); mutexUnlock(&reqMutex);
    for (int i = 0; i < numWorkers; i++) threadJoin(workers[i]);
    for (int i = 0; i < tileCount; i++) if (tileCache[i].valid) UnloadTexture(tileCache[i].tex);
    if (fontLoaded) UnloadFont(appFont);
    mutexDestroy(&reqMutex); mutexDestroy(&rdyMutex); condDestroy(&reqCond);
    CloseWindow();
    return 0;
}
