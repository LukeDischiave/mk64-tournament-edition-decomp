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

// Save lives in expansion RAM, ending at 0x80800000.
// Requires the expansion pak; save/load do nothing on 4MB.
// The snapshot is tagged with the course it was taken on and is dropped
// when a different course loads. Retry keeps it.


#define SAVE_STATE_CAMERA_COUNT 4
#define SAVE_STATE_RANK_LEN 10
#define SAVE_STATE_PATH_LEN 12
#define SAVE_STATE_FINISH_LEN 8
#define SAVE_STATE_UNEXPIRED_LEN 8


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
} SaveStatePayload;

#define SAVE_STATE_RAM_END 0x80800000
#define gSaveState ((SaveStatePayload*) (SAVE_STATE_RAM_END - ALIGN16(sizeof(SaveStatePayload))))

static s16 sSaveCourseId = -1;

//SaveState will take the current data in RAM and memcpy it to the above

void SaveState(void) {
    SaveStatePayload* save = gSaveState;

    if (gExpansionPAK == 0) {
        return;
    }
    sSaveCourseId = gCurrentCourseId;

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
    SaveStateGetReplayCursor(&save->replayCursor);
}

//do memcpy to restore all the variables to their locations in RAM.
void LoadState(void) {
    SaveStatePayload* save = gSaveState;

    if ((gExpansionPAK == 0) || (sSaveCourseId != gCurrentCourseId)) {
        return;
    }

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
    SaveStateSetReplayCursor(&save->replayCursor);
}
