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
#include "audio/load.h"
#include "ending/ceremony_and_credits.h"
#include "ending/code_80280000.h"
#include "crash_screen.h"
#include "ending/code_80281C40.h"
#include "ending/code_80281780.h"
#include "ending/podium_ceremony_actors.h"
#include "menu_items.h"
#include "racing/memory.h"
// Save lives in expansion RAM, ending at 0x80800000.
// Requires the expansion pak; save/load do nothing on 4MB.
// The snapshot is tagged with the course it was taken on and is dropped
// when a different course loads. Retry keeps it.

#define SAVE_STATE_RAM_END 0x80800000
#define gSaveState ((SaveStatePayload*) (SAVE_STATE_RAM_END - ALIGN16(sizeof(SaveStatePayload))))

SaveStatePayload* save = gSaveState;

static s16 sSaveCourseId = -1;
static s32 sModeSelection = -1; 
static s8 sPlayerCount = -1;

//SaveState will take the current data in RAM and memcpy it to the above

bool sBool_globals[BOOL_COUNT]; 
bool* sBool_global_ptrs[BOOL_COUNT]; 

bool8 sBool8_globals[BOOL8_COUNT]; 
bool8* sBool8_global_ptrs[BOOL8_COUNT]; 

f32 sF32_globals[F32_COUNT];
f32* sF32_global_ptrs[F32_COUNT];

s16 sS16_globals[S16_COUNT];
s16* sS16_global_ptrs[S16_COUNT];

s32 sS32_globals[S32_COUNT];
s32* sS32_global_ptrs[S32_COUNT];

void SaveState(void) {
    //SaveStatePayload* save = gSaveState;

    if (gExpansionPAK == 0) {
        return;
    }
    memcpy(save->actors, gActorList, sizeof(save->actors));
    sSaveCourseId = gCurrentCourseId;
    sModeSelection = gModeSelection;
    sPlayerCount = gPlayerCount;

    memcpy(save->actors, gActorList, sizeof(save->actors));
    memcpy(save->objects, gObjectList, sizeof(save->objects));
    memcpy(save->players, gPlayers, sizeof(save->players));
    memcpy(save->cameras, cameras, sizeof(save->cameras));
    memcpy(save->hud, playerHUD, sizeof(save->hud));
    memcpy(save->lapCount, gLapCountByPlayerId, sizeof(save->lapCount));
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
    memcpy(save->recoil, gRecoil, sizeof(save->recoil));
    memcpy(save->finished, gFinished, sizeof(save->finished));
    memcpy(save->finishTime, gFinishTime, sizeof(save->finishTime));
    memcpy(save->versusResults2P, nmi_gVersusResults2P, SAVE_STATE_VERSUS_2P_LEN * sizeof(nmi_gVersusResults2P[0]));
    memcpy(save->versusResults3P, nmi_gVersusResults3P, SAVE_STATE_VERSUS_3P_LEN * sizeof(nmi_gVersusResults3P[0]));
    memcpy(save->versusResults4P, nmi_gVersusResults4P, SAVE_STATE_VERSUS_4P_LEN * sizeof(nmi_gVersusResults4P[0]));
    memcpy(save->gasFrame, D_801652E0, sizeof(save->gasFrame));
    memcpy(save->spinRemain, D_8018D920, sizeof(save->spinRemain));
    memcpy(save->playerBalloonCount, gPlayerBalloonCount, sizeof(save->playerBalloonCount));
    memcpy(save->playerBalloonStatus, gPlayerBalloonStatus, sizeof(save->playerBalloonStatus));
    memcpy(save->balloonPhysics, D_8018D650, sizeof(save->balloonPhysics));
    memcpy(save->s32_ptrs, sS32_global_ptrs, sizeof(sS32_global_ptrs));
    memcpy(save->s32_vals, sS32_globals, sizeof(sS32_globals));

    memcpy(save->f32_ptrs, sF32_global_ptrs, sizeof(sF32_global_ptrs));
    memcpy(save->f32_vals, sF32_globals, sizeof(sF32_globals));

    memcpy(save->s16_ptrs, sS16_global_ptrs, sizeof(sS16_global_ptrs));
    memcpy(save->s16_vals, sS16_globals, sizeof(sS16_globals));

    memcpy(save->bool8_ptrs, sBool8_global_ptrs, sizeof(sBool8_global_ptrs));
    memcpy(save->bool8_vals, sBool8_globals, sizeof(sBool8_globals));

    memcpy(save->bool_ptrs, sBool_global_ptrs, sizeof(sBool_global_ptrs));
    memcpy(save->bool_vals, sBool_globals, sizeof(sBool_globals));
    SaveStateGetReplayCursor(&save->replayCursor);
}

//do memcpy to restore all the variables to their locations in RAM.
void LoadState(void) {
    s16 i;
    //SaveStatePayload* save = gSaveState;

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
    memcpy(gRecoil, save->recoil, sizeof(save->recoil));
    memcpy(gFinished, save->finished, sizeof(save->finished));
    memcpy(gFinishTime, save->finishTime, sizeof(save->finishTime));
    memcpy(nmi_gVersusResults2P, save->versusResults2P, sizeof(save->versusResults2P));
    memcpy(nmi_gVersusResults3P, save->versusResults3P, sizeof(save->versusResults3P));
    memcpy(nmi_gVersusResults4P, save->versusResults4P, sizeof(save->versusResults4P));
    memcpy(D_801652E0, save->gasFrame, sizeof(save->gasFrame));
    memcpy(D_8018D920, save->spinRemain, sizeof(save->spinRemain));
    memcpy(gPlayerBalloonCount, save->playerBalloonCount, sizeof(save->playerBalloonCount));
    memcpy(gPlayerBalloonStatus, save->playerBalloonStatus, sizeof(save->playerBalloonStatus));
    memcpy(D_8018D650, save->balloonPhysics, sizeof(save->balloonPhysics));
    for (i = 0; i < S32_COUNT; i++){
        *save->s32_ptrs[i] = save->s32_vals[i];
    }
    for (i = 0; i < F32_COUNT; i++){
        *save->f32_ptrs[i] = save->f32_vals[i];
    }
    for (i = 0; i < S16_COUNT; i++){
        *save->s16_ptrs[i] = save->s16_vals[i];
    }
    for (i = 0; i < BOOL8_COUNT; i++){
        *save->bool8_ptrs[i] = save->bool8_vals[i];
    }
    for (i = 0; i < BOOL_COUNT; i++){
        *save->bool_ptrs[i] = save->bool_vals[i];
    }

    SaveStateSetReplayCursor(&save->replayCursor);
}

void TakeSnapshots(void){
    TakeSnapshot_bool();
    TakeSnapshot_bool8();
    TakeSnapshot_f32();
    TakeSnapshot_s16();
    TakeSnapshot_s32();
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
    typ_idx++;

    //4
    rtn_idx = CheckSnapshot_s32();
    if (rtn_idx != -1){
        rtn[0] = typ_idx;
        rtn[1] = rtn_idx;
        return;
    }

    rtn[0] = -1;
    rtn[1] = -1;
    return;
}
//4
void TakeSnapshot_s32(void){
    sS32_global_ptrs[  0] = &gAudioTaskIndex;
    sS32_global_ptrs[  1] = &gCurrAiBufferIndex;
    sS32_global_ptrs[  2] = &gRefreshRate;
    sS32_global_ptrs[  3] = &gAudioErrorFlags;
    sS32_global_ptrs[  4] = &gAudioResetFadeOutFramesLeft;
    sS32_global_ptrs[  5] = &D_803B6E60;
    sS32_global_ptrs[  6] = &load_bss_pad;
    sS32_global_ptrs[  7] = &gMaxAudioCmds;
    sS32_global_ptrs[  8] = &gMaxSimultaneousNotes;
    sS32_global_ptrs[  9] = &gCurrAudioFrameDmaCount;
    sS32_global_ptrs[ 10] = &D_80164A28;
    sS32_global_ptrs[ 11] = &D_80164A2C;
    sS32_global_ptrs[ 12] = &D_8015F59C;
    sS32_global_ptrs[ 13] = &D_8015F5A0;
    sS32_global_ptrs[ 14] = &D_8015F5A4;
    sS32_global_ptrs[ 15] = &D_8015F788;
    sS32_global_ptrs[ 16] = &D_8015F8DC;
    sS32_global_ptrs[ 17] = &D_8015F8E0;
    sS32_global_ptrs[ 18] = &D_801625EC;
    sS32_global_ptrs[ 19] = &D_801625F0;
    sS32_global_ptrs[ 20] = &D_801625F4;
    sS32_global_ptrs[ 21] = &D_801625F8;
    sS32_global_ptrs[ 22] = &D_80165590;
    sS32_global_ptrs[ 23] = &D_80165594;
    sS32_global_ptrs[ 24] = &D_80165598;
    sS32_global_ptrs[ 25] = &D_8016559C;
    sS32_global_ptrs[ 26] = &D_801655A4;
    sS32_global_ptrs[ 27] = &D_801655AC;
    sS32_global_ptrs[ 28] = &D_801655B4;
    sS32_global_ptrs[ 29] = &D_801655BC;
    sS32_global_ptrs[ 30] = &D_801655C0;
    sS32_global_ptrs[ 31] = &D_801655C4;
    sS32_global_ptrs[ 32] = &D_801655C8;
    sS32_global_ptrs[ 33] = &D_801655CC;
    sS32_global_ptrs[ 34] = &D_801655D8;
    sS32_global_ptrs[ 35] = &D_801655E8;
    sS32_global_ptrs[ 36] = &D_801655F0;
    sS32_global_ptrs[ 37] = &D_801655F8;
    sS32_global_ptrs[ 38] = &D_80165608;
    sS32_global_ptrs[ 39] = &D_80165618;
    sS32_global_ptrs[ 40] = &D_80165628;
    sS32_global_ptrs[ 41] = &D_80165678;
    sS32_global_ptrs[ 42] = &D_80165754;
    sS32_global_ptrs[ 43] = &D_801657FC;
    sS32_global_ptrs[ 44] = &D_80165804;
    sS32_global_ptrs[ 45] = &D_8016580C;
    sS32_global_ptrs[ 46] = &D_80165814;
    sS32_global_ptrs[ 47] = &D_8016581C;
    sS32_global_ptrs[ 48] = &D_80165860;
    sS32_global_ptrs[ 49] = &D_8016586C;
    sS32_global_ptrs[ 50] = &D_80165878;
    sS32_global_ptrs[ 51] = &D_8016587C;
    sS32_global_ptrs[ 52] = &D_8016589C;
    sS32_global_ptrs[ 53] = &objectListSize;
    sS32_global_ptrs[ 54] = &D_80183DA0;
    sS32_global_ptrs[ 55] = &gNextFreeObjectParticle1;
    sS32_global_ptrs[ 56] = &gNextFreeObjectParticle2;
    sS32_global_ptrs[ 57] = &gNextFreeObjectParticle3;
    sS32_global_ptrs[ 58] = &gNextFreeObjectParticle4;
    sS32_global_ptrs[ 59] = &gNextFreeLeafParticle;
    sS32_global_ptrs[ 60] = &D_8018CF10;
    sS32_global_ptrs[ 61] = &D_8018D114;
    sS32_global_ptrs[ 62] = &gMatrixHudCount;
    sS32_global_ptrs[ 63] = &D_8018D140;
    sS32_global_ptrs[ 64] = &D_8018D150;
    sS32_global_ptrs[ 65] = &D_8018D158;
    sS32_global_ptrs[ 66] = &D_8018D160;
    sS32_global_ptrs[ 67] = &D_8018D168;
    sS32_global_ptrs[ 68] = &D_8018D170;
    sS32_global_ptrs[ 69] = &D_8018D178;
    sS32_global_ptrs[ 70] = &D_8018D180;
    sS32_global_ptrs[ 71] = &gIsHUDVisible;
    sS32_global_ptrs[ 72] = &D_8018D190;
    sS32_global_ptrs[ 73] = &D_8018D1A0;
    sS32_global_ptrs[ 74] = &D_8018D1B4;
    sS32_global_ptrs[ 75] = &D_8018D1C4;
    sS32_global_ptrs[ 76] = &D_8018D1C8;
    sS32_global_ptrs[ 77] = &D_8018D1CC;
    sS32_global_ptrs[ 78] = &D_8018D1D0;
    sS32_global_ptrs[ 79] = &D_8018D1D4;
    sS32_global_ptrs[ 80] = &D_8018D1D8;
    sS32_global_ptrs[ 81] = &D_8018D1DC;
    sS32_global_ptrs[ 82] = &D_8018D1EC;
    sS32_global_ptrs[ 83] = &D_8018D1F0;
    sS32_global_ptrs[ 84] = &D_8018D1F8;
    sS32_global_ptrs[ 85] = &D_8018D1FC;
    sS32_global_ptrs[ 86] = &D_8018D204;
    sS32_global_ptrs[ 87] = &D_8018D20C;
    sS32_global_ptrs[ 88] = &D_8018D21C;
    sS32_global_ptrs[ 89] = &D_8018D224;
    sS32_global_ptrs[ 90] = &D_8018D22C;
    sS32_global_ptrs[ 91] = &D_8018D240;
    sS32_global_ptrs[ 92] = &D_8018D2A4;
    sS32_global_ptrs[ 93] = &D_8018D2AC;
    sS32_global_ptrs[ 94] = &D_8018D2B4;
    sS32_global_ptrs[ 95] = &D_8018D2BC;
    sS32_global_ptrs[ 96] = &D_8018D314;
    sS32_global_ptrs[ 97] = &D_8018D320;
    sS32_global_ptrs[ 98] = &D_8018D380;
    sS32_global_ptrs[ 99] = &D_8018D384;
    sS32_global_ptrs[100] = &D_8018D388;
    sS32_global_ptrs[101] = &D_8018D3BC;
    sS32_global_ptrs[102] = &D_8018D3C0;
    sS32_global_ptrs[103] = &D_8018D3C4;
    sS32_global_ptrs[104] = &D_8018D3D4;
    sS32_global_ptrs[105] = &D_8018D3D8;
    sS32_global_ptrs[106] = &D_8018D3DC;
    sS32_global_ptrs[107] = &D_8018D3E0;
    sS32_global_ptrs[108] = &D_8018D3E4;
    sS32_global_ptrs[109] = &D_8018D3E8;
    sS32_global_ptrs[110] = &D_8018D3EC;
    sS32_global_ptrs[111] = &D_8018D3F0;
    sS32_global_ptrs[112] = &D_8018D3F4;
    sS32_global_ptrs[113] = &D_8018D3F8;
    sS32_global_ptrs[114] = &gRaceFrameCounter;
    sS32_global_ptrs[115] = &D_8018D400;
    sS32_global_ptrs[116] = &D_8018D40C;
    sS32_global_ptrs[117] = &D_8018D410;
    sS32_global_ptrs[118] = &D_801631CC;
    sS32_global_ptrs[119] = &D_80163238;
    sS32_global_ptrs[120] = &gIncrementUpdatePlayer;
    sS32_global_ptrs[121] = &D_8016337C;
    sS32_global_ptrs[122] = &gPlayerPathIndex;
    sS32_global_ptrs[123] = &D_80163480;
    sS32_global_ptrs[124] = &D_80163484;
    sS32_global_ptrs[125] = &D_80163488;
    sS32_global_ptrs[126] = &D_801634F0;
    sS32_global_ptrs[127] = &D_801634F4;
    sS32_global_ptrs[128] = &gVehicle2DPathLength;
    sS32_global_ptrs[129] = &D_8016448C;
    sS32_global_ptrs[130] = &D_801646A4;
    sS32_global_ptrs[131] = &D_801646A8;
    sS32_global_ptrs[132] = &D_801646AC;
    sS32_global_ptrs[133] = &D_801646B0;
    sS32_global_ptrs[134] = &D_801646B4;
    sS32_global_ptrs[135] = &D_801646B8;
    sS32_global_ptrs[136] = &D_801646BC;
    sS32_global_ptrs[137] = &sButtonSequenceIndex;
    sS32_global_ptrs[138] = &D_802876D4;
    sS32_global_ptrs[139] = &D_802876D8;
    sS32_global_ptrs[140] = &D_802876DC;
    sS32_global_ptrs[141] = &D_802874A0;
    sS32_global_ptrs[142] = &gGotoMenu;
    sS32_global_ptrs[143] = &D_80287554;
    sS32_global_ptrs[144] = &D_802874FC;
    sS32_global_ptrs[145] = &gTickSpeed;
    sS32_global_ptrs[146] = &D_80150120;
    sS32_global_ptrs[147] = &gGotoMode;
    sS32_global_ptrs[148] = &D_801502A0;
    sS32_global_ptrs[149] = &D_801502A4;
    sS32_global_ptrs[150] = &gIntroModelZEye;
    sS32_global_ptrs[151] = &gMenuFadeType;
    sS32_global_ptrs[152] = &gMenuTimingCounter;
    sS32_global_ptrs[153] = &gMenuDelayTimer;
    sS32_global_ptrs[154] = &sMenuTextureBufferIndex;
    sS32_global_ptrs[155] = &sMenuTextureEntries;
    sS32_global_ptrs[156] = &gNumD_8018E768Entries;
    sS32_global_ptrs[157] = &gCycleFlashMenu;
    sS32_global_ptrs[158] = &D_8018E7E0;
    sS32_global_ptrs[159] = &D_8018E83C;
    sS32_global_ptrs[160] = &D_8018E864_pad;
    sS32_global_ptrs[161] = &gControllerPak1NumFilesUsed;
    sS32_global_ptrs[162] = &gControllerPak1MaxWriteableFiles;
    sS32_global_ptrs[163] = &gControllerPak1NumPagesFree;
    sS32_global_ptrs[164] = &gControllerPak1FileNote;
    sS32_global_ptrs[165] = &gControllerPak2FileNote;
    sS32_global_ptrs[166] = &menu_item_bss_pad2;
    sS32_global_ptrs[167] = &sIntroModelTimer;
    sS32_global_ptrs[168] = &sGfxSeekPosition;
    sS32_global_ptrs[169] = &sPackedSeekPosition;
    sS32_global_ptrs[170] = &gDemoTimer;
    sS32_global_ptrs[171] = &D_80162DC8;
    sS32_global_ptrs[172] = &D_80162DCC;
    sS32_global_ptrs[173] = &D_80162DD0;
    sS32_global_ptrs[174] = &D_80162DDC;
    sS32_global_ptrs[175] = &D_80162DE0;
    sS32_global_ptrs[176] = &D_80162DE4;
    sS32_global_ptrs[177] = &D_80162DE8;
    sS32_global_ptrs[178] = &gPauseTriggered;
    sS32_global_ptrs[179] = &gPostTimeTrialReplayCannotSave;
    sS32_global_ptrs[180] = &D_80162DFC;
    sS32_global_ptrs[181] = &D_80162E00;    


    sS32_globals[  0] =  gAudioTaskIndex;
    sS32_globals[  1] = gCurrAiBufferIndex;
    sS32_globals[  2] = gRefreshRate;
    sS32_globals[  3] = gAudioErrorFlags;
    sS32_globals[  4] = gAudioResetFadeOutFramesLeft;
    sS32_globals[  5] = D_803B6E60;
    sS32_globals[  6] = load_bss_pad;
    sS32_globals[  7] = gMaxAudioCmds;
    sS32_globals[  8] = gMaxSimultaneousNotes;
    sS32_globals[  9] = gCurrAudioFrameDmaCount;
    sS32_globals[ 10] = D_80164A28;
    sS32_globals[ 11] = D_80164A2C;
    sS32_globals[ 12] = D_8015F59C;
    sS32_globals[ 13] = D_8015F5A0;
    sS32_globals[ 14] = D_8015F5A4;
    sS32_globals[ 15] = D_8015F788;
    sS32_globals[ 16] = D_8015F8DC;
    sS32_globals[ 17] = D_8015F8E0;
    sS32_globals[ 18] = D_801625EC;
    sS32_globals[ 19] = D_801625F0;
    sS32_globals[ 20] = D_801625F4;
    sS32_globals[ 21] = D_801625F8;
    sS32_globals[ 22] = D_80165590;
    sS32_globals[ 23] = D_80165594;
    sS32_globals[ 24] = D_80165598;
    sS32_globals[ 25] = D_8016559C;
    sS32_globals[ 26] = D_801655A4;
    sS32_globals[ 27] = D_801655AC;
    sS32_globals[ 28] = D_801655B4;
    sS32_globals[ 29] = D_801655BC;
    sS32_globals[ 30] = D_801655C0;
    sS32_globals[ 31] = D_801655C4;
    sS32_globals[ 32] = D_801655C8;
    sS32_globals[ 33] = D_801655CC;
    sS32_globals[ 34] = D_801655D8;
    sS32_globals[ 35] = D_801655E8;
    sS32_globals[ 36] = D_801655F0;
    sS32_globals[ 37] = D_801655F8;
    sS32_globals[ 38] = D_80165608;
    sS32_globals[ 39] = D_80165618;
    sS32_globals[ 40] = D_80165628;
    sS32_globals[ 41] = D_80165678;
    sS32_globals[ 42] = D_80165754;
    sS32_globals[ 43] = D_801657FC;
    sS32_globals[ 44] = D_80165804;
    sS32_globals[ 45] = D_8016580C;
    sS32_globals[ 46] = D_80165814;
    sS32_globals[ 47] = D_8016581C;
    sS32_globals[ 48] = D_80165860;
    sS32_globals[ 49] = D_8016586C;
    sS32_globals[ 50] = D_80165878;
    sS32_globals[ 51] = D_8016587C;
    sS32_globals[ 52] = D_8016589C;
    sS32_globals[ 53] = objectListSize;
    sS32_globals[ 54] = D_80183DA0;
    sS32_globals[ 55] = gNextFreeObjectParticle1;
    sS32_globals[ 56] = gNextFreeObjectParticle2;
    sS32_globals[ 57] = gNextFreeObjectParticle3;
    sS32_globals[ 58] = gNextFreeObjectParticle4;
    sS32_globals[ 59] = gNextFreeLeafParticle;
    sS32_globals[ 60] = D_8018CF10;
    sS32_globals[ 61] = D_8018D114;
    sS32_globals[ 62] = gMatrixHudCount;
    sS32_globals[ 63] = D_8018D140;
    sS32_globals[ 64] = D_8018D150;
    sS32_globals[ 65] = D_8018D158;
    sS32_globals[ 66] = D_8018D160;
    sS32_globals[ 67] = D_8018D168;
    sS32_globals[ 68] = D_8018D170;
    sS32_globals[ 69] = D_8018D178;
    sS32_globals[ 70] = D_8018D180;
    sS32_globals[ 71] = gIsHUDVisible;
    sS32_globals[ 72] = D_8018D190;
    sS32_globals[ 73] = D_8018D1A0;
    sS32_globals[ 74] = D_8018D1B4;
    sS32_globals[ 75] = D_8018D1C4;
    sS32_globals[ 76] = D_8018D1C8;
    sS32_globals[ 77] = D_8018D1CC;
    sS32_globals[ 78] = D_8018D1D0;
    sS32_globals[ 79] = D_8018D1D4;
    sS32_globals[ 80] = D_8018D1D8;
    sS32_globals[ 81] = D_8018D1DC;
    sS32_globals[ 82] = D_8018D1EC;
    sS32_globals[ 83] = D_8018D1F0;
    sS32_globals[ 84] = D_8018D1F8;
    sS32_globals[ 85] = D_8018D1FC;
    sS32_globals[ 86] = D_8018D204;
    sS32_globals[ 87] = D_8018D20C;
    sS32_globals[ 88] = D_8018D21C;
    sS32_globals[ 89] = D_8018D224;
    sS32_globals[ 90] = D_8018D22C;
    sS32_globals[ 91] = D_8018D240;
    sS32_globals[ 92] = D_8018D2A4;
    sS32_globals[ 93] = D_8018D2AC;
    sS32_globals[ 94] = D_8018D2B4;
    sS32_globals[ 95] = D_8018D2BC;
    sS32_globals[ 96] = D_8018D314;
    sS32_globals[ 97] = D_8018D320;
    sS32_globals[ 98] = D_8018D380;
    sS32_globals[ 99] = D_8018D384;
    sS32_globals[100] = D_8018D388;
    sS32_globals[101] = D_8018D3BC;
    sS32_globals[102] = D_8018D3C0;
    sS32_globals[103] = D_8018D3C4;
    sS32_globals[104] = D_8018D3D4;
    sS32_globals[105] = D_8018D3D8;
    sS32_globals[106] = D_8018D3DC;
    sS32_globals[107] = D_8018D3E0;
    sS32_globals[108] = D_8018D3E4;
    sS32_globals[109] = D_8018D3E8;
    sS32_globals[110] = D_8018D3EC;
    sS32_globals[111] = D_8018D3F0;
    sS32_globals[112] = D_8018D3F4;
    sS32_globals[113] = D_8018D3F8;
    sS32_globals[114] = gRaceFrameCounter;
    sS32_globals[115] = D_8018D400;
    sS32_globals[116] = D_8018D40C;
    sS32_globals[117] = D_8018D410;
    sS32_globals[118] = D_801631CC;
    sS32_globals[119] = D_80163238;
    sS32_globals[120] = gIncrementUpdatePlayer;
    sS32_globals[121] = D_8016337C;
    sS32_globals[122] = gPlayerPathIndex;
    sS32_globals[123] = D_80163480;
    sS32_globals[124] = D_80163484;
    sS32_globals[125] = D_80163488;
    sS32_globals[126] = D_801634F0;
    sS32_globals[127] = D_801634F4;
    sS32_globals[128] = gVehicle2DPathLength;
    sS32_globals[129] = D_8016448C;
    sS32_globals[130] = D_801646A4;
    sS32_globals[131] = D_801646A8;
    sS32_globals[132] = D_801646AC;
    sS32_globals[133] = D_801646B0;
    sS32_globals[134] = D_801646B4;
    sS32_globals[135] = D_801646B8;
    sS32_globals[136] = D_801646BC;
    sS32_globals[137] = sButtonSequenceIndex;
    sS32_globals[138] = D_802876D4;
    sS32_globals[139] = D_802876D8;
    sS32_globals[140] = D_802876DC;
    sS32_globals[141] = D_802874A0;
    sS32_globals[142] = gGotoMenu;
    sS32_globals[143] = D_80287554;
    sS32_globals[144] = D_802874FC;
    sS32_globals[145] = gTickSpeed;
    sS32_globals[146] = D_80150120;
    sS32_globals[147] = gGotoMode;
    sS32_globals[148] = D_801502A0;
    sS32_globals[149] = D_801502A4;
    sS32_globals[150] = gIntroModelZEye;
    sS32_globals[151] = gMenuFadeType;
    sS32_globals[152] = gMenuTimingCounter;
    sS32_globals[153] = gMenuDelayTimer;
    sS32_globals[154] = sMenuTextureBufferIndex;
    sS32_globals[155] = sMenuTextureEntries;
    sS32_globals[156] = gNumD_8018E768Entries;
    sS32_globals[157] = gCycleFlashMenu;
    sS32_globals[158] = D_8018E7E0;
    sS32_globals[159] = D_8018E83C;
    sS32_globals[160] = D_8018E864_pad;
    sS32_globals[161] = gControllerPak1NumFilesUsed;
    sS32_globals[162] = gControllerPak1MaxWriteableFiles;
    sS32_globals[163] = gControllerPak1NumPagesFree;
    sS32_globals[164] = gControllerPak1FileNote;
    sS32_globals[165] = gControllerPak2FileNote;
    sS32_globals[166] = menu_item_bss_pad2;
    sS32_globals[167] = sIntroModelTimer;
    sS32_globals[168] = sGfxSeekPosition;
    sS32_globals[169] = sPackedSeekPosition;
    sS32_globals[170] = gDemoTimer;
    sS32_globals[171] = D_80162DC8;
    sS32_globals[172] = D_80162DCC;
    sS32_globals[173] = D_80162DD0;
    sS32_globals[174] = D_80162DDC;
    sS32_globals[175] = D_80162DE0;
    sS32_globals[176] = D_80162DE4;
    sS32_globals[177] = D_80162DE8;
    sS32_globals[178] = gPauseTriggered;
    sS32_globals[179] = gPostTimeTrialReplayCannotSave;
    sS32_globals[180] = D_80162DFC;
    sS32_globals[181] = D_80162E00;
}

s16 CheckSnapshot_s32(void){
    s16 i;
    s32 s32_now[S32_COUNT];

    s32_now[  0] =  gAudioTaskIndex;
    s32_now[  1] = gCurrAiBufferIndex;
    s32_now[  2] = gRefreshRate;
    s32_now[  3] = gAudioErrorFlags;
    s32_now[  4] = gAudioResetFadeOutFramesLeft;
    s32_now[  5] = D_803B6E60;
    s32_now[  6] = load_bss_pad;
    s32_now[  7] = gMaxAudioCmds;
    s32_now[  8] = gMaxSimultaneousNotes;
    s32_now[  9] = gCurrAudioFrameDmaCount;
    s32_now[ 10] = D_80164A28;
    s32_now[ 11] = D_80164A2C;
    s32_now[ 12] = D_8015F59C;
    s32_now[ 13] = D_8015F5A0;
    s32_now[ 14] = D_8015F5A4;
    s32_now[ 15] = D_8015F788;
    s32_now[ 16] = D_8015F8DC;
    s32_now[ 17] = D_8015F8E0;
    s32_now[ 18] = D_801625EC;
    s32_now[ 19] = D_801625F0;
    s32_now[ 20] = D_801625F4;
    s32_now[ 21] = D_801625F8;
    s32_now[ 22] = D_80165590;
    s32_now[ 23] = D_80165594;
    s32_now[ 24] = D_80165598;
    s32_now[ 25] = D_8016559C;
    s32_now[ 26] = D_801655A4;
    s32_now[ 27] = D_801655AC;
    s32_now[ 28] = D_801655B4;
    s32_now[ 29] = D_801655BC;
    s32_now[ 30] = D_801655C0;
    s32_now[ 31] = D_801655C4;
    s32_now[ 32] = D_801655C8;
    s32_now[ 33] = D_801655CC;
    s32_now[ 34] = D_801655D8;
    s32_now[ 35] = D_801655E8;
    s32_now[ 36] = D_801655F0;
    s32_now[ 37] = D_801655F8;
    s32_now[ 38] = D_80165608;
    s32_now[ 39] = D_80165618;
    s32_now[ 40] = D_80165628;
    s32_now[ 41] = D_80165678;
    s32_now[ 42] = D_80165754;
    s32_now[ 43] = D_801657FC;
    s32_now[ 44] = D_80165804;
    s32_now[ 45] = D_8016580C;
    s32_now[ 46] = D_80165814;
    s32_now[ 47] = D_8016581C;
    s32_now[ 48] = D_80165860;
    s32_now[ 49] = D_8016586C;
    s32_now[ 50] = D_80165878;
    s32_now[ 51] = D_8016587C;
    s32_now[ 52] = D_8016589C;
    s32_now[ 53] = objectListSize;
    s32_now[ 54] = D_80183DA0;
    s32_now[ 55] = gNextFreeObjectParticle1;
    s32_now[ 56] = gNextFreeObjectParticle2;
    s32_now[ 57] = gNextFreeObjectParticle3;
    s32_now[ 58] = gNextFreeObjectParticle4;
    s32_now[ 59] = gNextFreeLeafParticle;
    s32_now[ 60] = D_8018CF10;
    s32_now[ 61] = D_8018D114;
    s32_now[ 62] = gMatrixHudCount;
    s32_now[ 63] = D_8018D140;
    s32_now[ 64] = D_8018D150;
    s32_now[ 65] = D_8018D158;
    s32_now[ 66] = D_8018D160;
    s32_now[ 67] = D_8018D168;
    s32_now[ 68] = D_8018D170;
    s32_now[ 69] = D_8018D178;
    s32_now[ 70] = D_8018D180;
    s32_now[ 71] = gIsHUDVisible;
    s32_now[ 72] = D_8018D190;
    s32_now[ 73] = D_8018D1A0;
    s32_now[ 74] = D_8018D1B4;
    s32_now[ 75] = D_8018D1C4;
    s32_now[ 76] = D_8018D1C8;
    s32_now[ 77] = D_8018D1CC;
    s32_now[ 78] = D_8018D1D0;
    s32_now[ 79] = D_8018D1D4;
    s32_now[ 80] = D_8018D1D8;
    s32_now[ 81] = D_8018D1DC;
    s32_now[ 82] = D_8018D1EC;
    s32_now[ 83] = D_8018D1F0;
    s32_now[ 84] = D_8018D1F8;
    s32_now[ 85] = D_8018D1FC;
    s32_now[ 86] = D_8018D204;
    s32_now[ 87] = D_8018D20C;
    s32_now[ 88] = D_8018D21C;
    s32_now[ 89] = D_8018D224;
    s32_now[ 90] = D_8018D22C;
    s32_now[ 91] = D_8018D240;
    s32_now[ 92] = D_8018D2A4;
    s32_now[ 93] = D_8018D2AC;
    s32_now[ 94] = D_8018D2B4;
    s32_now[ 95] = D_8018D2BC;
    s32_now[ 96] = D_8018D314;
    s32_now[ 97] = D_8018D320;
    s32_now[ 98] = D_8018D380;
    s32_now[ 99] = D_8018D384;
    s32_now[100] = D_8018D388;
    s32_now[101] = D_8018D3BC;
    s32_now[102] = D_8018D3C0;
    s32_now[103] = D_8018D3C4;
    s32_now[104] = D_8018D3D4;
    s32_now[105] = D_8018D3D8;
    s32_now[106] = D_8018D3DC;
    s32_now[107] = D_8018D3E0;
    s32_now[108] = D_8018D3E4;
    s32_now[109] = D_8018D3E8;
    s32_now[110] = D_8018D3EC;
    s32_now[111] = D_8018D3F0;
    s32_now[112] = D_8018D3F4;
    s32_now[113] = D_8018D3F8;
    s32_now[114] = gRaceFrameCounter;
    s32_now[115] = D_8018D400;
    s32_now[116] = D_8018D40C;
    s32_now[117] = D_8018D410;
    s32_now[118] = D_801631CC;
    s32_now[119] = D_80163238;
    s32_now[120] = gIncrementUpdatePlayer;
    s32_now[121] = D_8016337C;
    s32_now[122] = gPlayerPathIndex;
    s32_now[123] = D_80163480;
    s32_now[124] = D_80163484;
    s32_now[125] = D_80163488;
    s32_now[126] = D_801634F0;
    s32_now[127] = D_801634F4;
    s32_now[128] = gVehicle2DPathLength;
    s32_now[129] = D_8016448C;
    s32_now[130] = D_801646A4;
    s32_now[131] = D_801646A8;
    s32_now[132] = D_801646AC;
    s32_now[133] = D_801646B0;
    s32_now[134] = D_801646B4;
    s32_now[135] = D_801646B8;
    s32_now[136] = D_801646BC;
    s32_now[137] = sButtonSequenceIndex;
    s32_now[138] = D_802876D4;
    s32_now[139] = D_802876D8;
    s32_now[140] = D_802876DC;
    s32_now[141] = D_802874A0;
    s32_now[142] = gGotoMenu;
    s32_now[143] = D_80287554;
    s32_now[144] = D_802874FC;
    s32_now[145] = gTickSpeed;
    s32_now[146] = D_80150120;
    s32_now[147] = gGotoMode;
    s32_now[148] = D_801502A0;
    s32_now[149] = D_801502A4;
    s32_now[150] = gIntroModelZEye;
    s32_now[151] = gMenuFadeType;
    s32_now[152] = gMenuTimingCounter;
    s32_now[153] = gMenuDelayTimer;
    s32_now[154] = sMenuTextureBufferIndex;
    s32_now[155] = sMenuTextureEntries;
    s32_now[156] = gNumD_8018E768Entries;
    s32_now[157] = gCycleFlashMenu;
    s32_now[158] = D_8018E7E0;
    s32_now[159] = D_8018E83C;
    s32_now[160] = D_8018E864_pad;
    s32_now[161] = gControllerPak1NumFilesUsed;
    s32_now[162] = gControllerPak1MaxWriteableFiles;
    s32_now[163] = gControllerPak1NumPagesFree;
    s32_now[164] = gControllerPak1FileNote;
    s32_now[165] = gControllerPak2FileNote;
    s32_now[166] = menu_item_bss_pad2;
    s32_now[167] = sIntroModelTimer;
    s32_now[168] = sGfxSeekPosition;
    s32_now[169] = sPackedSeekPosition;
    s32_now[170] = gDemoTimer;
    s32_now[171] = D_80162DC8;
    s32_now[172] = D_80162DCC;
    s32_now[173] = D_80162DD0;
    s32_now[174] = D_80162DDC;
    s32_now[175] = D_80162DE0;
    s32_now[176] = D_80162DE4;
    s32_now[177] = D_80162DE8;
    s32_now[178] = gPauseTriggered;
    s32_now[179] = gPostTimeTrialReplayCannotSave;
    s32_now[180] = D_80162DFC;
    s32_now[181] = D_80162E00;
    for (i = 0; i < S32_COUNT; i++){
        if (s32_now[i] != sS32_globals[i]){
            return(i);
        }
    }
    return(-1);
}
//3
void TakeSnapshot_s16(void){
    sS16_global_ptrs[  0] = &isNotTheFirst;
    sS16_global_ptrs[  1] = &sMatrixShouldNotPop;
    sS16_global_ptrs[  2] = &sMatrixStackSize;
    sS16_global_ptrs[  3] = &gVolume;
    sS16_global_ptrs[  4] = &gTempoInternalToExternal;
    sS16_global_ptrs[  5] = &gCourseMaxX;
    sS16_global_ptrs[  6] = &gCourseMinX;
    sS16_global_ptrs[  7] = &gCourseMaxY;
    sS16_global_ptrs[  8] = &gCourseMinY;
    sS16_global_ptrs[  9] = &gCourseMaxZ;
    sS16_global_ptrs[ 10] = &gCourseMinZ;
    sS16_global_ptrs[ 11] = &D_8015F6F4;
    sS16_global_ptrs[ 12] = &D_8015F6F6;
    sS16_global_ptrs[ 13] = &D_8015F6FA;
    sS16_global_ptrs[ 14] = &D_8015F6FC;
    sS16_global_ptrs[ 15] = &gDebugPathCount;
    sS16_global_ptrs[ 16] = &sIsController1Unplugged;
    sS16_global_ptrs[ 17] = &D_801656F0;
    sS16_global_ptrs[ 18] = &D_80165708;
    sS16_global_ptrs[ 19] = &D_80165710;
    sS16_global_ptrs[ 20] = &D_80165718;
    sS16_global_ptrs[ 21] = &D_80165720;
    sS16_global_ptrs[ 22] = &D_80165728;
    sS16_global_ptrs[ 23] = &D_80165730;
    sS16_global_ptrs[ 24] = &D_80165738;
    sS16_global_ptrs[ 25] = &D_80165740;
    sS16_global_ptrs[ 26] = &D_80165748;
    sS16_global_ptrs[ 27] = &gNumActiveThwomps;
    sS16_global_ptrs[ 28] = &D_8016578C;
    sS16_global_ptrs[ 29] = &D_80165790;
    sS16_global_ptrs[ 30] = &D_80165794;
    sS16_global_ptrs[ 31] = &D_80165900;
    sS16_global_ptrs[ 32] = &D_8018CF18;
    sS16_global_ptrs[ 33] = &D_8018CF20;
    sS16_global_ptrs[ 34] = &D_8018CF48;
    sS16_global_ptrs[ 35] = &D_8018CF60;
    sS16_global_ptrs[ 36] = &D_8018CF78;
    sS16_global_ptrs[ 37] = &D_8018CF90;
    sS16_global_ptrs[ 38] = &D_8018CFA8;
    sS16_global_ptrs[ 39] = &D_8018CFB0;
    sS16_global_ptrs[ 40] = &D_8018CFB8;
    sS16_global_ptrs[ 41] = &D_8018CFC0;
    sS16_global_ptrs[ 42] = &D_8018CFC8;
    sS16_global_ptrs[ 43] = &D_8018CFD0;
    sS16_global_ptrs[ 44] = &D_8018CFD8;
    sS16_global_ptrs[ 45] = &D_8018CFE0;
    sS16_global_ptrs[ 46] = &D_8018CFE8;
    sS16_global_ptrs[ 47] = &D_8018CFF0;
    sS16_global_ptrs[ 48] = &D_8018CFF8;
    sS16_global_ptrs[ 49] = &D_8018D000;
    sS16_global_ptrs[ 50] = &D_8018D008;
    sS16_global_ptrs[ 51] = &D_8018D010;
    sS16_global_ptrs[ 52] = &D_8018D018;
    sS16_global_ptrs[ 53] = &D_8018D020;
    sS16_global_ptrs[ 54] = &D_8018D048;
    sS16_global_ptrs[ 55] = &D_8018D070;
    sS16_global_ptrs[ 56] = &D_8018D098;
    sS16_global_ptrs[ 57] = &D_8018D0C0;
    sS16_global_ptrs[ 58] = &D_8018D0E8;
    sS16_global_ptrs[ 59] = &D_8018D110;
    sS16_global_ptrs[ 60] = &D_8018D16C;
    sS16_global_ptrs[ 61] = &D_8018D174;
    sS16_global_ptrs[ 62] = &D_8018D17C;
    sS16_global_ptrs[ 63] = &D_8018D184;
    sS16_global_ptrs[ 64] = &D_8018D18C;
    sS16_global_ptrs[ 65] = &D_8018D200;
    sS16_global_ptrs[ 66] = &D_8018D208;
    sS16_global_ptrs[ 67] = &D_8018D210;
    sS16_global_ptrs[ 68] = &D_8018D218;
    sS16_global_ptrs[ 69] = &D_8018D2B0;
    sS16_global_ptrs[ 70] = &D_8018D2B8;
    sS16_global_ptrs[ 71] = &gMiniMapX;
    sS16_global_ptrs[ 72] = &gMiniMapY;
    sS16_global_ptrs[ 73] = &D_8018D2F0;
    sS16_global_ptrs[ 74] = &D_8018D2F8;
    sS16_global_ptrs[ 75] = &D_8018EDB0;
    sS16_global_ptrs[ 76] = &D_8018EDB2;
    sS16_global_ptrs[ 77] = &D_8018EDB4;
    sS16_global_ptrs[ 78] = &D_80162EB0;
    sS16_global_ptrs[ 79] = &D_80162EB2;
    sS16_global_ptrs[ 80] = &gTrainSmokeTimer;
    sS16_global_ptrs[ 81] = &sSomeNearestPathPoint;
    sS16_global_ptrs[ 82] = &D_80162FD0;
    sS16_global_ptrs[ 83] = &gIsPlayerNewPathPoint;
    sS16_global_ptrs[ 84] = &gFerrySmokeTimer;
    sS16_global_ptrs[ 85] = &sVehicleSoundRenderCounter;
    sS16_global_ptrs[ 86] = &gBestRankedHumanPlayer;
    sS16_global_ptrs[ 87] = &gIsInExtra;
    sS16_global_ptrs[ 88] = &D_8016347C;
    sS16_global_ptrs[ 89] = &D_8016347E;
    sS16_global_ptrs[ 90] = &D_8016348C;
    sS16_global_ptrs[ 91] = &D_801634EC;
    sS16_global_ptrs[ 92] = &D_80164358;
    sS16_global_ptrs[ 93] = &D_8016435A;
    sS16_global_ptrs[ 94] = &D_8016435C;
    sS16_global_ptrs[ 95] = &sCutsceneShot;
    sS16_global_ptrs[ 96] = &gCutsceneShotTimer;
    sS16_global_ptrs[ 97] = &sCutsceneSplineSegment;
    sS16_global_ptrs[ 98] = &D_80287B1E;
    sS16_global_ptrs[ 99] = &gMatrixEffectCount;
    sS16_global_ptrs[100] = &sReplayGhostBufferSize;
    sS16_global_ptrs[101] = &D_80162D86;
    sS16_global_ptrs[102] = &D_80165340;
    sS16_global_ptrs[103] = &D_8016556E;
    sS16_global_ptrs[104] = &D_80165570;
    sS16_global_ptrs[105] = &D_80165572;
    sS16_global_ptrs[106] = &D_80165574;
    sS16_global_ptrs[107] = &D_80165576;
    sS16_global_ptrs[108] = &D_80165578;
    sS16_global_ptrs[109] = &D_8016557A;
    sS16_global_ptrs[110] = &D_8016557C;
    sS16_global_ptrs[111] = &D_8016557E;
    sS16_global_ptrs[112] = &D_80165580;
    sS16_global_ptrs[113] = &D_80165582;

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

//2
void TakeSnapshot_f32(void){
    sF32_global_ptrs[  0] = &D_803B7178;
    sF32_global_ptrs[  1] = &D_80164A30;
    sF32_global_ptrs[  2] = &D_8015F704;
    sF32_global_ptrs[  3] = &gCourseDirection;
    sF32_global_ptrs[  4] = &D_8015F8E4;
    sF32_global_ptrs[  5] = &D_8015F8E8;
    sF32_global_ptrs[  6] = &D_801625FC;
    sF32_global_ptrs[  7] = &gWinnerTime;
    sF32_global_ptrs[  8] = &D_8018CFCC;
    sF32_global_ptrs[  9] = &D_8018CFD4;
    sF32_global_ptrs[ 10] = &D_8018CFE4;
    sF32_global_ptrs[ 11] = &D_8018CFEC;
    sF32_global_ptrs[ 12] = &D_8018CFF4;
    sF32_global_ptrs[ 13] = &D_8018D00C;
    sF32_global_ptrs[ 14] = &xOrientation;
    sF32_global_ptrs[ 15] = &D_8018D1E8;
    sF32_global_ptrs[ 16] = &gMiniMapMarkerScale;
    sF32_global_ptrs[ 17] = &gCurrentCpuTargetSpeed;
    sF32_global_ptrs[ 18] = &gPathStartZ;
    sF32_global_ptrs[ 19] = &D_80164698;
    sF32_global_ptrs[ 20] = &D_8016469C;
    sF32_global_ptrs[ 21] = &D_801646A0;
    sF32_global_ptrs[ 22] = &sCutsceneSplineSegmentProgress;
    sF32_global_ptrs[ 23] = &D_80150118;
    sF32_global_ptrs[ 24] = &gScreenAspect;
    sF32_global_ptrs[ 25] = &gCourseFarPersp;
    sF32_global_ptrs[ 26] = &gCourseNearPersp;
    sF32_global_ptrs[ 27] = &gIntroModelScale;
    sF32_global_ptrs[ 28] = &gIntroModelRotX;
    sF32_global_ptrs[ 29] = &gIntroModelRotY;
    sF32_global_ptrs[ 30] = &gIntroModelRotZ;
    sF32_global_ptrs[ 31] = &gIntroModelPosX;
    sF32_global_ptrs[ 32] = &gIntroModelPosY;
    sF32_global_ptrs[ 33] = &gIntroModelPosZ;
    sF32_global_ptrs[ 34] = &D_802BA034;

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

//1
void TakeSnapshot_bool8(void){
    sBool8_global_ptrs[  0] = &D_801657E6;
    sBool8_global_ptrs[  1] = &D_801657E8;
    sBool8_global_ptrs[  2] = &D_801657F0;
    sBool8_global_ptrs[  3] = &D_801657F8;
    sBool8_global_ptrs[  4] = &D_80165810;
    sBool8_global_ptrs[  5] = &D_80165818;
    sBool8_global_ptrs[  6] = &gGhostPlayerInit;
    sBool8_global_ptrs[  7] = &gCourseMapInit;


    sBool8_globals[0] = D_801657E6;
    sBool8_globals[1] = D_801657E8;
    sBool8_globals[2] = D_801657F0;
    sBool8_globals[3] = D_801657F8;
    sBool8_globals[4] = D_80165810;
    sBool8_globals[5] = D_80165818;
    sBool8_globals[6] = gGhostPlayerInit;
    sBool8_globals[7] = gCourseMapInit;
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

//0
void TakeSnapshot_bool(void){
     sBool_globals[ 0] = D_8018D214;
     sBool_global_ptrs[  0] = &D_8018D214;
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

