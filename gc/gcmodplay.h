#ifndef __GCMODPLAY_H__
#define __GCMODPLAY_H__

#include <gctypes.h>
#include <modplay.h>

#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

typedef struct _modsndbuf {
	u32 freq;
	u16 fmt;
	u32 chans;
	f32 samples;
	u32 data_len;
	void *usr_data;
	void (*callback)(void *,u8 *,u32);
} MODSNDBUF;

typedef struct _modplay {
	MOD mod;
	BOOL playing,paused;
	BOOL bits,stereo,manual_polling;
	u32 playfreq,numSFXChans;
	MODSNDBUF soundBuf;
} MODPlay;

void MODPlay_Init(MODPlay *mod);
u32 MODPlay_SetFrequency(MODPlay *mod,u32 freq);
void MODPlay_SetStereo(MODPlay *mod,BOOL stereo);
u32 MODPlay_SetMOD(MODPlay *mod,u8 *mem);
void MODPlay_Unload(MODPlay *mod);
u32 MODPlay_AllocSFXChannels(MODPlay *mod,u32 sfxchans);
u32 MODPlay_Start(MODPlay *mod);
u32 MODPlay_Stop(MODPlay *mod);
u32 MODPlay_TriggerNote(MODPlay *mod,u32 chan,u8 inst,u16 freq,u8 vol);
u32 MODPlay_Pause(MODPlay *mod,BOOL);

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif