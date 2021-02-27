#ifndef SOUND_H
#define SOUND_H
#include "main.h"
#include "cwav.h"

extern CWAV* sfx_sound;
extern CWAV* lag_sound;
extern CWAV* cred_sound;

#define PLAYBEEP() {cwavPlay(sfx_sound, 0, -1);}
#define PLAYBOOP() {cwavPlay(sfx_sound, 1, -1);}
#define PLAYCLICK() {cwavPlay(sfx_sound, 2, -1);}


#define STARTLAG() {cwavPlay(lag_sound, 0, -1);}
#define STOPLAG() {cwavStop(lag_sound, 0, -1);}

#define STARTCRED() {cwavPlay(cred_sound, 0, -1);}
#define STOPCRED() {cwavStop(cred_sound, 0, -1);}
#endif