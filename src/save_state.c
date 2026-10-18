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

#define SAVE_STATE_CAMERA_COUNT 4
#define SAVE_STATE_RANK_LEN 10
#define SAVE_STATE_PATH_LEN 12
#define SAVE_STATE_FINISH_LEN 8
#define SAVE_STATE_UNEXPIRED_LEN 8

// 2 player only saves wins/losses, while 3p & 4p save 1st-3rd
#define SAVE_STATE_VERSUS_2P_LEN 2 * 1
#define SAVE_STATE_VERSUS_3P_LEN 3 * 3
#define SAVE_STATE_VERSUS_4P_LEN 4 * 3


//this struct is a layout of RAM containing a copy of all relevant race data
//the arrays are sized so that a memcpy can fill them from current race data. 
typedef struct SaveStatePayload {
    struct Actor actors[ACTOR_LIST_SIZE];
    Object objects[OBJECT_LIST_SIZE];
    Player players[NUM_PLAYERS];
    Camera cameras[SAVE_STATE_CAMERA_COUNT];
    hud_player hud[HUD_PLAYERS_SIZE];
    s32 lapCount[SAVE_STATE_RANK_LEN];
    s32 vehicle2DPathLength;
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
    s32 objectListSize;
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
    f32 courseTimer;
    f32 waterHeight;
    f32 waterVelocity;
    SaveStateReplayCursor replayCursor;
    struct_gRecoil recoil[8];
    s32 raceState;
    bool finished[8];
    f32 finishTime[8];
    s16 startCheck; //details unknown
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

    f32 cloudMaybe; // D_8018D1E8
    s16 isPlayerNewPathPoint; //gIsPlayerNewPathPoint
    s16 someNearestPathPoint; // sSomeNearestPathPoint;
    s16 matrixEffectCount; //gMatrixEffectCount
    s16 d_8018D200; //D_8018D200
    s16 d_8018D208; //D_8018D208
} SaveStatePayload;

#define SAVE_STATE_RAM_END 0x80800000
#define gSaveState ((SaveStatePayload*) (SAVE_STATE_RAM_END - ALIGN16(sizeof(SaveStatePayload))))

static s16 sSaveCourseId = -1;
static s32 sModeSelection = -1; 
static s8 sPlayerCount = -1;

//SaveState will take the current data in RAM and memcpy it to the above

void SaveState(void) {
    SaveStatePayload* save = gSaveState;

    if (gExpansionPAK == 0) {
        return;
    }
    sSaveCourseId = gCurrentCourseId;
    sModeSelection = gModeSelection;
    sPlayerCount = gPlayerCount;

    memcpy(save->actors, gActorList, sizeof(save->actors));
    memcpy(save->objects, gObjectList, sizeof(save->objects));
    memcpy(save->players, gPlayers, sizeof(save->players));
    memcpy(save->cameras, cameras, sizeof(save->cameras));
    memcpy(save->hud, playerHUD, sizeof(save->hud));
    memcpy(save->lapCount, gLapCountByPlayerId, sizeof(save->lapCount));
    save->vehicle2DPathLength = gVehicle2DPathLength;
    memcpy(save->trains, gTrainList, sizeof(save->trains));
    memcpy(save->crossingTriggered, isCrossingTriggeredByIndex, sizeof(save->crossingTriggered));
    memcpy(save->crossingTimer, sCrossingActiveTimer, sizeof(save->crossingTimer));
    memcpy(save->paddleBoats, gPaddleBoats, sizeof(save->paddleBoats));
    memcpy(save->boxTrucks, gBoxTruckList, sizeof(save->boxTrucks));
    memcpy(save->schoolBuses, gSchoolBusList, sizeof(save->schoolBuses));
    memcpy(save->tankerTrucks, gTankerTruckList, sizeof(save->tankerTrucks));
    memcpy(save->cars, gCarList, sizeof(save->cars));
    memcpy(save->vehiclePad, D_80163DD8, sizeof(save->vehiclePad));
    memcpy(save->bombKarts, gBombKarts, sizeof(save->bombKarts));
    memcpy(save->unexpiredActors, gUnexpiredActorsList, sizeof(save->unexpiredActors));
    memcpy(save->cpuItems, cpu_ItemStrategy, sizeof(save->cpuItems));
    save->objectListSize = objectListSize;
    memcpy(save->objectIndex1, indexObjectList1, sizeof(save->objectIndex1));
    memcpy(save->objectIndex2, indexObjectList2, sizeof(save->objectIndex2));
    memcpy(save->objectIndex3, indexObjectList3, sizeof(save->objectIndex3));
    memcpy(save->objectIndex4, indexObjectList4, sizeof(save->objectIndex4));
    memcpy(save->lakituIndex, gIndexLakituList, sizeof(save->lakituIndex));
    memcpy(save->itemWindowIndex, gItemWindowObjectByPlayerId, sizeof(save->itemWindowIndex));
    memcpy(save->bombKartObjectIndex, gIndexObjectBombKart, sizeof(save->bombKartObjectIndex));
    save->numActors = gNumActors;
    save->numPermanentActors = gNumPermanentActors;
    memcpy(save->nearestPathPoint, gNearestPathPointByPlayerId, sizeof(save->nearestPathPoint));
    memcpy(save->trackSection, gPlayersTrackSectionId, sizeof(save->trackSection));
    memcpy(save->pathIndex, gPathIndexByPlayerId, sizeof(save->pathIndex));
    memcpy(save->crossedFinishLine, gCrossedFinishLine, sizeof(save->crossedFinishLine));
    memcpy(save->wrongDirectionCounter, gWrongDirectionCounter, sizeof(save->wrongDirectionCounter));
    memcpy(save->isWrongDirection, gIsPlayerWrongDirection, sizeof(save->isWrongDirection));
    memcpy(save->pathPointsTraversed, gNumPathPointsTraversed, sizeof(save->pathPointsTraversed));
    memcpy(save->lapCompletion, gLapCompletionPercentByPlayerId, sizeof(save->lapCompletion));
    memcpy(save->courseCompletion, gCourseCompletionPercentByPlayerId, sizeof(save->courseCompletion));
    memcpy(save->raceRank, gGPCurrentRaceRankByPlayerId, sizeof(save->raceRank));
    memcpy(save->previousRaceRank, gPreviousGPCurrentRaceRankByPlayerId, sizeof(save->previousRaceRank));
    memcpy(save->raceRankDup, gGPCurrentRaceRankByPlayerIdDup, sizeof(save->raceRankDup));
    memcpy(save->finishLineTime, gTimePlayerLastTouchedFinishLine, sizeof(save->finishLineTime));
    memcpy(save->positionLut, gPlayerPositionLUT, sizeof(save->positionLut));
    memcpy(save->starEffectStartTime, gPlayerStarEffectStartTime, sizeof(save->starEffectStartTime));
    memcpy(save->cameraPathPoint, gNearestPathPointByCameraId, sizeof(save->cameraPathPoint));
    memcpy(save->previousPlayerZ, gPreviousPlayerZ, sizeof(save->previousPlayerZ));
    memcpy(save->laneData, gPlayerTrackPositionFactorInstruction, sizeof(save->laneData));
    save->courseTimer = gCourseTimer;
    save->waterHeight = D_8015F8E4;
    save->waterVelocity = D_8015F8E8;
    memcpy(save->recoil, gRecoil, sizeof(save->recoil));
    save->raceState = gRaceState;
    memcpy(save->finished, gFinished, sizeof(save->finished));
    memcpy(save->finishTime, gFinishTime, sizeof(save->finishTime));
    memcpy(save->versusResults2P, nmi_gVersusResults2P, SAVE_STATE_VERSUS_2P_LEN * sizeof(nmi_gVersusResults2P[0]));
    memcpy(save->versusResults3P, nmi_gVersusResults3P, SAVE_STATE_VERSUS_3P_LEN * sizeof(nmi_gVersusResults3P[0]));
    memcpy(save->versusResults4P, nmi_gVersusResults4P, SAVE_STATE_VERSUS_4P_LEN * sizeof(nmi_gVersusResults4P[0]));
    memcpy(save->gasFrame, D_801652E0, sizeof(save->gasFrame));
    memcpy(save->spinRemain, D_8018D920, sizeof(save->spinRemain));
    save->startCheck = D_801656F0;
    memcpy(save->playerBalloonCount, gPlayerBalloonCount, sizeof(save->playerBalloonCount));
    memcpy(save->playerBalloonStatus, gPlayerBalloonStatus, sizeof(save->playerBalloonStatus));
    memcpy(save->balloonPhysics, D_8018D650, sizeof(save->balloonPhysics));
    save->cloudMaybe = D_8018D1E8;
    save->isPlayerNewPathPoint = gIsPlayerNewPathPoint;
    save->someNearestPathPoint = sSomeNearestPathPoint;
    save->matrixEffectCount = gMatrixEffectCount;
    save->d_8018D200 = D_8018D200;
    save->d_8018D208 = D_8018D208;
    SaveStateGetReplayCursor(&save->replayCursor);
}

//do memcpy to restore all the variables to their locations in RAM.
void LoadState(void) {
    SaveStatePayload* save = gSaveState;

    if ((gExpansionPAK == 0) || (sSaveCourseId != gCurrentCourseId) || (sModeSelection != gModeSelection) || (sPlayerCount != gPlayerCount)) {
        return;
    }
    gPostTimeTrialReplayCannotSave = 1;

    memcpy(gActorList, save->actors, sizeof(save->actors));
    memcpy(gObjectList, save->objects, sizeof(save->objects));
    memcpy(gPlayers, save->players, sizeof(save->players));
    memcpy(cameras, save->cameras, sizeof(save->cameras));
    memcpy(playerHUD, save->hud, sizeof(save->hud));
    memcpy(gLapCountByPlayerId, save->lapCount, sizeof(save->lapCount));
    gVehicle2DPathLength = save->vehicle2DPathLength;
    memcpy(gTrainList, save->trains, sizeof(save->trains));
    memcpy(isCrossingTriggeredByIndex, save->crossingTriggered, sizeof(save->crossingTriggered));
    memcpy(sCrossingActiveTimer, save->crossingTimer, sizeof(save->crossingTimer));
    memcpy(gPaddleBoats, save->paddleBoats, sizeof(save->paddleBoats));
    memcpy(gBoxTruckList, save->boxTrucks, sizeof(save->boxTrucks));
    memcpy(gSchoolBusList, save->schoolBuses, sizeof(save->schoolBuses));
    memcpy(gTankerTruckList, save->tankerTrucks, sizeof(save->tankerTrucks));
    memcpy(gCarList, save->cars, sizeof(save->cars));
    memcpy(D_80163DD8, save->vehiclePad, sizeof(save->vehiclePad));
    memcpy(gBombKarts, save->bombKarts, sizeof(save->bombKarts));
    memcpy(gUnexpiredActorsList, save->unexpiredActors, sizeof(save->unexpiredActors));
    memcpy(cpu_ItemStrategy, save->cpuItems, sizeof(save->cpuItems));
    objectListSize = save->objectListSize;
    memcpy(indexObjectList1, save->objectIndex1, sizeof(save->objectIndex1));
    memcpy(indexObjectList2, save->objectIndex2, sizeof(save->objectIndex2));
    memcpy(indexObjectList3, save->objectIndex3, sizeof(save->objectIndex3));
    memcpy(indexObjectList4, save->objectIndex4, sizeof(save->objectIndex4));
    memcpy(gIndexLakituList, save->lakituIndex, sizeof(save->lakituIndex));
    memcpy(gItemWindowObjectByPlayerId, save->itemWindowIndex, sizeof(save->itemWindowIndex));
    memcpy(gIndexObjectBombKart, save->bombKartObjectIndex, sizeof(save->bombKartObjectIndex));
    gNumActors = save->numActors;
    gNumPermanentActors = save->numPermanentActors;
    memcpy(gNearestPathPointByPlayerId, save->nearestPathPoint, sizeof(save->nearestPathPoint));
    memcpy(gPlayersTrackSectionId, save->trackSection, sizeof(save->trackSection));
    memcpy(gPathIndexByPlayerId, save->pathIndex, sizeof(save->pathIndex));
    memcpy(gCrossedFinishLine, save->crossedFinishLine, sizeof(save->crossedFinishLine));
    memcpy(gWrongDirectionCounter, save->wrongDirectionCounter, sizeof(save->wrongDirectionCounter));
    memcpy(gIsPlayerWrongDirection, save->isWrongDirection, sizeof(save->isWrongDirection));
    memcpy(gNumPathPointsTraversed, save->pathPointsTraversed, sizeof(save->pathPointsTraversed));
    memcpy(gLapCompletionPercentByPlayerId, save->lapCompletion, sizeof(save->lapCompletion));
    memcpy(gCourseCompletionPercentByPlayerId, save->courseCompletion, sizeof(save->courseCompletion));
    memcpy(gGPCurrentRaceRankByPlayerId, save->raceRank, sizeof(save->raceRank));
    memcpy(gPreviousGPCurrentRaceRankByPlayerId, save->previousRaceRank, sizeof(save->previousRaceRank));
    memcpy(gGPCurrentRaceRankByPlayerIdDup, save->raceRankDup, sizeof(save->raceRankDup));
    memcpy(gTimePlayerLastTouchedFinishLine, save->finishLineTime, sizeof(save->finishLineTime));
    memcpy(gPlayerPositionLUT, save->positionLut, sizeof(save->positionLut));
    memcpy(gPlayerStarEffectStartTime, save->starEffectStartTime, sizeof(save->starEffectStartTime));
    memcpy(gNearestPathPointByCameraId, save->cameraPathPoint, sizeof(save->cameraPathPoint));
    memcpy(gPreviousPlayerZ, save->previousPlayerZ, sizeof(save->previousPlayerZ));
    memcpy(gPlayerTrackPositionFactorInstruction, save->laneData, sizeof(save->laneData));
    gCourseTimer = save->courseTimer;
    D_8015F8E4 = save->waterHeight;
    D_8015F8E8 = save->waterVelocity;
    memcpy(gRecoil, save->recoil, sizeof(save->recoil));
    gRaceState = save->raceState;
    memcpy(gFinished, save->finished, sizeof(save->finished));
    memcpy(gFinishTime, save->finishTime, sizeof(save->finishTime));
    memcpy(nmi_gVersusResults2P, save->versusResults2P, sizeof(save->versusResults2P));
    memcpy(nmi_gVersusResults3P, save->versusResults3P, sizeof(save->versusResults3P));
    memcpy(nmi_gVersusResults4P, save->versusResults4P, sizeof(save->versusResults4P));
    D_801656F0 = save->startCheck;
    memcpy(D_801652E0, save->gasFrame, sizeof(save->gasFrame));
    memcpy(D_8018D920, save->spinRemain, sizeof(save->spinRemain));
    memcpy(gPlayerBalloonCount, save->playerBalloonCount, sizeof(save->playerBalloonCount));
    memcpy(gPlayerBalloonStatus, save->playerBalloonStatus, sizeof(save->playerBalloonStatus));
    memcpy(D_8018D650, save->balloonPhysics, sizeof(save->balloonPhysics));
    D_8018D1E8 = save->cloudMaybe;
    gIsPlayerNewPathPoint = save->isPlayerNewPathPoint;
    sSomeNearestPathPoint = save->someNearestPathPoint;
    D_8018D200 = save->d_8018D200;
    D_8018D208 = save->d_8018D208;
    SaveStateSetReplayCursor(&save->replayCursor);
}

#define BOOL_COUNT 1
#define BOOL8_COUNT 8
#define F32_COUNT 35
#define S16_COUNT 114

bool sBool_globals[BOOL_COUNT]; 
bool8 sBool8_globals[BOOL8_COUNT]; 
f32 sF32_globals[F32_COUNT];
s16 sS16_globals[S16_COUNT];

void TakeSnapshots(void){
    TakeSnapshot_bool();
    TakeSnapshot_bool8();
    TakeSnapshot_f32();
    TakeSnapshot_s16();
}

void CheckSnapshots(s16* rtn){
    s16 typ_idx;
    s16 rtn_idx;

    typ_idx = 0;

    //0
    rtn_idx = CheckSnapshot_bool();
    if (rtn_idx != -1){
        rtn[0] = typ_idx;
        rtn[1] = rtn_idx;
        return;
    }
    typ_idx++;

    //1
    rtn_idx = CheckSnapshot_bool8();
    if (rtn_idx != -1){
        rtn[0] = typ_idx;
        rtn[1] = rtn_idx;
        return;
    }
    typ_idx++;

    //2
    rtn_idx = CheckSnapshot_f32();
    if (rtn_idx != -1){
        rtn[0] = typ_idx;
        rtn[1] = rtn_idx;
        return;
    }
    typ_idx++;

    //3
    rtn_idx = CheckSnapshot_s16();
    if (rtn_idx != -1){
        rtn[0] = typ_idx;
        rtn[1] = rtn_idx;
        return;
    }
    rtn[0] = -1;
    rtn[1] = -1;
    return;
}

void TakeSnapshot_s16(void){
    sS16_globals[  0] = isNotTheFirst;
    sS16_globals[  1] = sMatrixShouldNotPop;
    sS16_globals[  2] = sMatrixStackSize;
    sS16_globals[  3] = gVolume;
    sS16_globals[  4] = gTempoInternalToExternal;
    sS16_globals[  5] = gCourseMaxX;
    sS16_globals[  6] = gCourseMinX;
    sS16_globals[  7] = gCourseMaxY;
    sS16_globals[  8] = gCourseMinY;
    sS16_globals[  9] = gCourseMaxZ;
    sS16_globals[ 10] = gCourseMinZ;
    sS16_globals[ 11] = D_8015F6F4;
    sS16_globals[ 12] = D_8015F6F6;
    sS16_globals[ 13] = D_8015F6FA;
    sS16_globals[ 14] = D_8015F6FC;
    sS16_globals[ 15] = gDebugPathCount;
    sS16_globals[ 16] = sIsController1Unplugged;
    sS16_globals[ 17] = D_801656F0;
    sS16_globals[ 18] = D_80165708;
    sS16_globals[ 19] = D_80165710;
    sS16_globals[ 20] = D_80165718;
    sS16_globals[ 21] = D_80165720;
    sS16_globals[ 22] = D_80165728;
    sS16_globals[ 23] = D_80165730;
    sS16_globals[ 24] = D_80165738;
    sS16_globals[ 25] = D_80165740;
    sS16_globals[ 26] = D_80165748;
    sS16_globals[ 27] = gNumActiveThwomps;
    sS16_globals[ 28] = D_8016578C;
    sS16_globals[ 29] = D_80165790;
    sS16_globals[ 30] = D_80165794;
    sS16_globals[ 31] = D_80165900;
    sS16_globals[ 32] = D_8018CF18;
    sS16_globals[ 33] = D_8018CF20;
    sS16_globals[ 34] = D_8018CF48;
    sS16_globals[ 35] = D_8018CF60;
    sS16_globals[ 36] = D_8018CF78;
    sS16_globals[ 37] = D_8018CF90;
    sS16_globals[ 38] = D_8018CFA8;
    sS16_globals[ 39] = D_8018CFB0;
    sS16_globals[ 40] = D_8018CFB8;
    sS16_globals[ 41] = D_8018CFC0;
    sS16_globals[ 42] = D_8018CFC8;
    sS16_globals[ 43] = D_8018CFD0;
    sS16_globals[ 44] = D_8018CFD8;
    sS16_globals[ 45] = D_8018CFE0;
    sS16_globals[ 46] = D_8018CFE8;
    sS16_globals[ 47] = D_8018CFF0;
    sS16_globals[ 48] = D_8018CFF8;
    sS16_globals[ 49] = D_8018D000;
    sS16_globals[ 50] = D_8018D008;
    sS16_globals[ 51] = D_8018D010;
    sS16_globals[ 52] = D_8018D018;
    sS16_globals[ 53] = D_8018D020;
    sS16_globals[ 54] = D_8018D048;
    sS16_globals[ 55] = D_8018D070;
    sS16_globals[ 56] = D_8018D098;
    sS16_globals[ 57] = D_8018D0C0;
    sS16_globals[ 58] = D_8018D0E8;
    sS16_globals[ 59] = D_8018D110;
    sS16_globals[ 60] = D_8018D16C;
    sS16_globals[ 61] = D_8018D174;
    sS16_globals[ 62] = D_8018D17C;
    sS16_globals[ 63] = D_8018D184;
    sS16_globals[ 64] = D_8018D18C;
    sS16_globals[ 65] = D_8018D200;
    sS16_globals[ 66] = D_8018D208;
    sS16_globals[ 67] = D_8018D210;
    sS16_globals[ 68] = D_8018D218;
    sS16_globals[ 69] = D_8018D2B0;
    sS16_globals[ 70] = D_8018D2B8;
    sS16_globals[ 71] = gMiniMapX;
    sS16_globals[ 72] = gMiniMapY;
    sS16_globals[ 73] = D_8018D2F0;
    sS16_globals[ 74] = D_8018D2F8;
    sS16_globals[ 75] = D_8018EDB0;
    sS16_globals[ 76] = D_8018EDB2;
    sS16_globals[ 77] = D_8018EDB4;
    sS16_globals[ 78] = D_80162EB0;
    sS16_globals[ 79] = D_80162EB2;
    sS16_globals[ 80] = gTrainSmokeTimer;
    sS16_globals[ 81] = sSomeNearestPathPoint;
    sS16_globals[ 82] = D_80162FD0;
    sS16_globals[ 83] = gIsPlayerNewPathPoint;
    sS16_globals[ 84] = gFerrySmokeTimer;
    sS16_globals[ 85] = sVehicleSoundRenderCounter;
    sS16_globals[ 86] = gBestRankedHumanPlayer;
    sS16_globals[ 87] = gIsInExtra;
    sS16_globals[ 88] = D_8016347C;
    sS16_globals[ 89] = D_8016347E;
    sS16_globals[ 90] = D_8016348C;
    sS16_globals[ 91] = D_801634EC;
    sS16_globals[ 92] = D_80164358;
    sS16_globals[ 93] = D_8016435A;
    sS16_globals[ 94] = D_8016435C;
    sS16_globals[ 95] = sCutsceneShot;
    sS16_globals[ 96] = gCutsceneShotTimer;
    sS16_globals[ 97] = sCutsceneSplineSegment;
    sS16_globals[ 98] = D_80287B1E;
    sS16_globals[ 99] = gMatrixEffectCount;
    sS16_globals[100] = sReplayGhostBufferSize;
    sS16_globals[101] = D_80162D86;
    sS16_globals[102] = D_80165340;
    sS16_globals[103] = D_8016556E;
    sS16_globals[104] = D_80165570;
    sS16_globals[105] = D_80165572;
    sS16_globals[106] = D_80165574;
    sS16_globals[107] = D_80165576;
    sS16_globals[108] = D_80165578;
    sS16_globals[109] = D_8016557A;
    sS16_globals[110] = D_8016557C;
    sS16_globals[111] = D_8016557E;
    sS16_globals[112] = D_80165580;
    sS16_globals[113] = D_80165582;
}

s16 CheckSnapshot_s16(void){
    s16 i;
    s16 s16_now[S16_COUNT];

    s16_now[  0] = isNotTheFirst;
    s16_now[  1] = sMatrixShouldNotPop;
    s16_now[  2] = sMatrixStackSize;
    s16_now[  3] = gVolume;
    s16_now[  4] = gTempoInternalToExternal;
    s16_now[  5] = gCourseMaxX;
    s16_now[  6] = gCourseMinX;
    s16_now[  7] = gCourseMaxY;
    s16_now[  8] = gCourseMinY;
    s16_now[  9] = gCourseMaxZ;
    s16_now[ 10] = gCourseMinZ;
    s16_now[ 11] = D_8015F6F4;
    s16_now[ 12] = D_8015F6F6;
    s16_now[ 13] = D_8015F6FA;
    s16_now[ 14] = D_8015F6FC;
    s16_now[ 15] = gDebugPathCount;
    s16_now[ 16] = sIsController1Unplugged;
    s16_now[ 17] = D_801656F0;
    s16_now[ 18] = D_80165708;
    s16_now[ 19] = D_80165710;
    s16_now[ 20] = D_80165718;
    s16_now[ 21] = D_80165720;
    s16_now[ 22] = D_80165728;
    s16_now[ 23] = D_80165730;
    s16_now[ 24] = D_80165738;
    s16_now[ 25] = D_80165740;
    s16_now[ 26] = D_80165748;
    s16_now[ 27] = gNumActiveThwomps;
    s16_now[ 28] = D_8016578C;
    s16_now[ 29] = D_80165790;
    s16_now[ 30] = D_80165794;
    s16_now[ 31] = D_80165900;
    s16_now[ 32] = D_8018CF18;
    s16_now[ 33] = D_8018CF20;
    s16_now[ 34] = D_8018CF48;
    s16_now[ 35] = D_8018CF60;
    s16_now[ 36] = D_8018CF78;
    s16_now[ 37] = D_8018CF90;
    s16_now[ 38] = D_8018CFA8;
    s16_now[ 39] = D_8018CFB0;
    s16_now[ 40] = D_8018CFB8;
    s16_now[ 41] = D_8018CFC0;
    s16_now[ 42] = D_8018CFC8;
    s16_now[ 43] = D_8018CFD0;
    s16_now[ 44] = D_8018CFD8;
    s16_now[ 45] = D_8018CFE0;
    s16_now[ 46] = D_8018CFE8;
    s16_now[ 47] = D_8018CFF0;
    s16_now[ 48] = D_8018CFF8;
    s16_now[ 49] = D_8018D000;
    s16_now[ 50] = D_8018D008;
    s16_now[ 51] = D_8018D010;
    s16_now[ 52] = D_8018D018;
    s16_now[ 53] = D_8018D020;
    s16_now[ 54] = D_8018D048;
    s16_now[ 55] = D_8018D070;
    s16_now[ 56] = D_8018D098;
    s16_now[ 57] = D_8018D0C0;
    s16_now[ 58] = D_8018D0E8;
    s16_now[ 59] = D_8018D110;
    s16_now[ 60] = D_8018D16C;
    s16_now[ 61] = D_8018D174;
    s16_now[ 62] = D_8018D17C;
    s16_now[ 63] = D_8018D184;
    s16_now[ 64] = D_8018D18C;
    s16_now[ 65] = D_8018D200;
    s16_now[ 66] = D_8018D208;
    s16_now[ 67] = D_8018D210;
    s16_now[ 68] = D_8018D218;
    s16_now[ 69] = D_8018D2B0;
    s16_now[ 70] = D_8018D2B8;
    s16_now[ 71] = gMiniMapX;
    s16_now[ 72] = gMiniMapY;
    s16_now[ 73] = D_8018D2F0;
    s16_now[ 74] = D_8018D2F8;
    s16_now[ 75] = D_8018EDB0;
    s16_now[ 76] = D_8018EDB2;
    s16_now[ 77] = D_8018EDB4;
    s16_now[ 78] = D_80162EB0;
    s16_now[ 79] = D_80162EB2;
    s16_now[ 80] = gTrainSmokeTimer;
    s16_now[ 81] = sSomeNearestPathPoint;
    s16_now[ 82] = D_80162FD0;
    s16_now[ 83] = gIsPlayerNewPathPoint;
    s16_now[ 84] = gFerrySmokeTimer;
    s16_now[ 85] = sVehicleSoundRenderCounter;
    s16_now[ 86] = gBestRankedHumanPlayer;
    s16_now[ 87] = gIsInExtra;
    s16_now[ 88] = D_8016347C;
    s16_now[ 89] = D_8016347E;
    s16_now[ 90] = D_8016348C;
    s16_now[ 91] = D_801634EC;
    s16_now[ 92] = D_80164358;
    s16_now[ 93] = D_8016435A;
    s16_now[ 94] = D_8016435C;
    s16_now[ 95] = sCutsceneShot;
    s16_now[ 96] = gCutsceneShotTimer;
    s16_now[ 97] = sCutsceneSplineSegment;
    s16_now[ 98] = D_80287B1E;
    s16_now[ 99] = gMatrixEffectCount;
    s16_now[100] = sReplayGhostBufferSize;
    s16_now[101] = D_80162D86;
    s16_now[102] = D_80165340;
    s16_now[103] = D_8016556E;
    s16_now[104] = D_80165570;
    s16_now[105] = D_80165572;
    s16_now[106] = D_80165574;
    s16_now[107] = D_80165576;
    s16_now[108] = D_80165578;
    s16_now[109] = D_8016557A;
    s16_now[110] = D_8016557C;
    s16_now[111] = D_8016557E;
    s16_now[112] = D_80165580;
    s16_now[113] = D_80165582;

    for (i = 0; i < S16_COUNT; i++){
        if (s16_now[i] != sS16_globals[i]){
            return(i);
        }
    }
    return(-1);
}


void TakeSnapshot_f32(void){
     sF32_globals[ 0] = D_803B7178;
     sF32_globals[ 1] = D_80164A30;
     sF32_globals[ 2] = D_8015F704;
     sF32_globals[ 3] = gCourseDirection;
     sF32_globals[ 4] = D_8015F8E4;
     sF32_globals[ 5] = D_8015F8E8;
     sF32_globals[ 6] = D_801625FC;
     sF32_globals[ 7] = gWinnerTime;
     sF32_globals[ 8] = D_8018CFCC;
     sF32_globals[ 9] = D_8018CFD4;
     sF32_globals[10] = D_8018CFE4;
     sF32_globals[11] = D_8018CFEC;
     sF32_globals[12] = D_8018CFF4;
     sF32_globals[13] = D_8018D00C;
     sF32_globals[14] = xOrientation;
     sF32_globals[15] = D_8018D1E8;
     sF32_globals[16] = gMiniMapMarkerScale;
     sF32_globals[17] = gCurrentCpuTargetSpeed;
     sF32_globals[18] = gPathStartZ;
     sF32_globals[19] = D_80164698;
     sF32_globals[20] = D_8016469C;
     sF32_globals[21] = D_801646A0;
     sF32_globals[22] = sCutsceneSplineSegmentProgress;
     sF32_globals[23] = D_80150118;
     sF32_globals[24] = gScreenAspect;
     sF32_globals[25] = gCourseFarPersp;
     sF32_globals[26] = gCourseNearPersp;
     sF32_globals[27] = gIntroModelScale;
     sF32_globals[28] = gIntroModelRotX;
     sF32_globals[29] = gIntroModelRotY;
     sF32_globals[30] = gIntroModelRotZ;
     sF32_globals[31] = gIntroModelPosX;
     sF32_globals[32] = gIntroModelPosY;
     sF32_globals[33] = gIntroModelPosZ;
     sF32_globals[34] = D_802BA034;
}

s16 CheckSnapshot_f32(void){
    s16 i;
    f32 f32_now[F32_COUNT];
    
    f32_now[ 0] = D_803B7178;
    f32_now[ 1] = D_80164A30;
    f32_now[ 2] = D_8015F704;
    f32_now[ 3] = gCourseDirection;
    f32_now[ 4] = D_8015F8E4;
    f32_now[ 5] = D_8015F8E8;
    f32_now[ 6] = D_801625FC;
    f32_now[ 7] = gWinnerTime;
    f32_now[ 8] = D_8018CFCC;
    f32_now[ 9] = D_8018CFD4;
    f32_now[10] = D_8018CFE4;
    f32_now[11] = D_8018CFEC;
    f32_now[12] = D_8018CFF4;
    f32_now[13] = D_8018D00C;
    f32_now[14] = xOrientation;
    f32_now[15] = D_8018D1E8;
    f32_now[16] = gMiniMapMarkerScale;
    f32_now[17] = gCurrentCpuTargetSpeed;
    f32_now[18] = gPathStartZ;
    f32_now[19] = D_80164698;
    f32_now[20] = D_8016469C;
    f32_now[21] = D_801646A0;
    f32_now[22] = sCutsceneSplineSegmentProgress;
    f32_now[23] = D_80150118;
    f32_now[24] = gScreenAspect;
    f32_now[25] = gCourseFarPersp;
    f32_now[26] = gCourseNearPersp;
    f32_now[27] = gIntroModelScale;
    f32_now[28] = gIntroModelRotX;
    f32_now[29] = gIntroModelRotY;
    f32_now[30] = gIntroModelRotZ;
    f32_now[31] = gIntroModelPosX;
    f32_now[32] = gIntroModelPosY;
    f32_now[33] = gIntroModelPosZ;
    f32_now[34] = D_802BA034;

    for (i = 0; i < F32_COUNT; i++){
        if (f32_now[i] != sF32_globals[i]){
            return(i);
        }
    }
    return(-1);
}

void TakeSnapshot_bool(void){
     sBool_globals[ 0] = D_8018D214;
}

s16 CheckSnapshot_bool(void){
    s16 i;
    bool bool_now[1];
    
    bool_now[ 0] = D_8018D214;

    for (i = 0; i < BOOL_COUNT; i++){
        if (bool_now[i] != sBool_globals[i]){
            return(i);
        }
    }
    return(-1);
}

void TakeSnapshot_bool8(void){
    bool8 bool8_globals[8]; 
    
    bool8_globals[0] = D_801657E6;
    bool8_globals[1] = D_801657E8;
    bool8_globals[2] = D_801657F0;
    bool8_globals[3] = D_801657F8;
    bool8_globals[4] = D_80165810;
    bool8_globals[5] = D_80165818;
    bool8_globals[6] = gGhostPlayerInit;
    bool8_globals[7] = gCourseMapInit;
    memcpy(sBool8_globals, bool8_globals, sizeof(bool8_globals));
}

s16 CheckSnapshot_bool8(void){
    s16 i;
    bool8 bool8_now[8];
    
    bool8_now[0] = D_801657E6;
    bool8_now[1] = D_801657E8;
    bool8_now[2] = D_801657F0;
    bool8_now[3] = D_801657F8;
    bool8_now[4] = D_80165810;
    bool8_now[5] = D_80165818;
    bool8_now[6] = gGhostPlayerInit;
    bool8_now[7] = gCourseMapInit;

    for (i = 0; i < BOOL8_COUNT; i++){
        if (bool8_now[i] != sBool8_globals[i]){
            return(i);
        }
    }
    return(-1);
}

