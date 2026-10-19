#ifndef SAVE_STATE_H
#define SAVE_STATE_H

#include <ultra64.h>
#include <macros.h>
#include <common_structs.h>
#include <defines.h>
#include <actor_types.h>
#include <objects.h>
#include <vehicles.h>
#include <bomb_kart.h>
#include <path.h>
#include <course.h>
#include <string.h>

#include "main.h"
#include "camera.h"
#include "code_800029B0.h"
#include "code_80057C60.h"
#include "cpu_vehicles_camera_path.h"
#include "effects.h"
#include "replays.h"
#include "save_state.h"
#include "spawn_players.h"
#include "menus.h"

#include "audio/data.h"
#include "ending/ceremony_and_credits.h"
// Save lives in expansion RAM, ending at 0x80800000.
// Requires the expansion pak; save/load do nothing on 4MB.
// The snapshot is tagged with the course it was taken on and is dropped
// when a different course loads. Retry keeps it.

#include "animation.h"
#include "audio/heap.h"
#include "code_800AF9B0.h"
#include "render_player.h"
#include "replays.h"
#include "racing/race_logic.h"

void SaveState(void);
void LoadState(void);
void TakeSnapshots(void);
void CheckSnapshots(s16*);

void TakeSnapshot_f32(void);
s16 CheckSnapshot_f32(void);

void TakeSnapshot_bool(void);
s16 CheckSnapshot_bool(void);

void TakeSnapshot_bool8(void);
s16 CheckSnapshot_bool8(void);

void TakeSnapshot_s16(void);
s16 CheckSnapshot_s16(void);

void TakeSnapshot_s32(void);
s16 CheckSnapshot_s32(void);

#define SAVE_STATE_CAMERA_COUNT 4
#define SAVE_STATE_RANK_LEN 10
#define SAVE_STATE_PATH_LEN 12
#define SAVE_STATE_FINISH_LEN 8
#define SAVE_STATE_UNEXPIRED_LEN 8

// 2 player only saves wins/losses, while 3p & 4p save 1st-3rd
#define SAVE_STATE_VERSUS_2P_LEN 2 * 1
#define SAVE_STATE_VERSUS_3P_LEN 3 * 3
#define SAVE_STATE_VERSUS_4P_LEN 4 * 3

#define BOOL_COUNT 1
#define BOOL8_COUNT 8
#define F32_COUNT 35
#define S16_COUNT 114
#define S32_COUNT 182

//this struct is a layout of RAM containing a copy of all relevant race data
//the arrays are sized so that a memcpy can fill them from current race data. 
typedef struct SaveStatePayload {
    struct Actor actors[ACTOR_LIST_SIZE];
    Object objects[OBJECT_LIST_SIZE];
    Player players[NUM_PLAYERS];
    Camera cameras[SAVE_STATE_CAMERA_COUNT];
    hud_player hud[HUD_PLAYERS_SIZE];
    s32 lapCount[SAVE_STATE_RANK_LEN];
    TrainStuff trains[NUM_TRAINS];
    u16 crossingTriggered[NUM_CROSSINGS];
    u16 crossingTimer[NUM_CROSSINGS];
    PaddleBoatStuff paddleBoats[NUM_PADDLE_BOATS];
    VehicleStuff boxTrucks[NUM_RACE_BOX_TRUCKS];
    VehicleStuff schoolBuses[NUM_RACE_SCHOOL_BUSES];
    VehicleStuff tankerTrucks[NUM_RACE_TANKER_TRUCKS];
    VehicleStuff cars[NUM_RACE_CARS];
    s32 vehiclePad[4];
    BombKart bombKarts[NUM_BOMB_KARTS_MAX];
    struct unexpiredActors unexpiredActors[SAVE_STATE_UNEXPIRED_LEN];
    CpuItemStrategyData cpuItems[NUM_PLAYERS];
    s32 objectIndex1[SOME_OBJECT_INDEX_LIST_SIZE];
    s32 objectIndex2[SOME_OBJECT_INDEX_LIST_SIZE];
    s32 objectIndex3[SOME_OBJECT_INDEX_LIST_SIZE];
    s32 objectIndex4[SOME_OBJECT_INDEX_LIST_SIZE];
    s32 lakituIndex[4];
    s32 itemWindowIndex[4];
    s32 bombKartObjectIndex[NUM_BOMB_KARTS_MAX];
    u16 numActors;
    u16 numPermanentActors;
    u16 nearestPathPoint[SAVE_STATE_PATH_LEN];
    u16 trackSection[SAVE_STATE_PATH_LEN];
    u16 pathIndex[SAVE_STATE_PATH_LEN];
    u16 crossedFinishLine[SAVE_STATE_PATH_LEN];
    u16 wrongDirectionCounter[SAVE_STATE_PATH_LEN];
    u16 isWrongDirection[SAVE_STATE_PATH_LEN];
    s32 pathPointsTraversed[SAVE_STATE_RANK_LEN];
    f32 lapCompletion[SAVE_STATE_RANK_LEN];
    f32 courseCompletion[SAVE_STATE_RANK_LEN];
    s32 raceRank[SAVE_STATE_RANK_LEN];
    s32 previousRaceRank[SAVE_STATE_RANK_LEN];
    s32 raceRankDup[SAVE_STATE_RANK_LEN];
    f32 finishLineTime[SAVE_STATE_FINISH_LEN];
    s16 positionLut[SAVE_STATE_FINISH_LEN];
    s32 starEffectStartTime[NUM_PLAYERS];
    s16 cameraPathPoint[SAVE_STATE_CAMERA_COUNT];
    f32 previousPlayerZ[SAVE_STATE_RANK_LEN];
    TrackPositionFactorInstruction laneData[SAVE_STATE_RANK_LEN];
    SaveStateReplayCursor replayCursor;
    struct_gRecoil recoil[8];
    bool finished[8];
    f32 finishTime[8];
    s32 gasFrame[8];
    s32 spinRemain[8];
    s16 playerBalloonCount[8];
    u16 playerBalloonStatus[8][3]; //gPlayerBalloonStatus
    Vec3f balloonPhysics[8];

    // special handling, versus and battle results for all player counts are stored in the same
    // larger array and accessed through pointers
    u8 versusResults2P[SAVE_STATE_VERSUS_2P_LEN];
    u8 versusResults3P[SAVE_STATE_VERSUS_3P_LEN];
    u8 versusResults4P[SAVE_STATE_VERSUS_4P_LEN];
    // no need to save battle, as you can't load a savestate after a player has won the battle

    bool* bool_ptrs[BOOL_COUNT];
    bool bool_vals[BOOL_COUNT];
    
    bool8* bool8_ptrs[BOOL8_COUNT];
    bool8 bool8_vals[BOOL8_COUNT];

    s16* s16_ptrs[S16_COUNT];
    s16 s16_vals[S16_COUNT];

    f32* f32_ptrs[F32_COUNT];
    f32 f32_vals[F32_COUNT];

    s32* s32_ptrs[S32_COUNT];
    s32 s32_vals[S32_COUNT];
} SaveStatePayload;

extern SaveStatePayload* save;
extern s16 newMatrixEffectCount;
#endif
