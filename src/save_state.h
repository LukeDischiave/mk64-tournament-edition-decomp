#ifndef SAVE_STATE_H
#define SAVE_STATE_H

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

#endif
