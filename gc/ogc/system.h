#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <gctypes.h>

#define R_RESET							*(vu32*)0xCC003024

#define SYS_RESTART						R_RESET = 0x00000000;
#define SYS_HOTRESET					R_RESET = 0x00000001; //1?
#define SYS_SHUTDOWN					R_RESET = 0x00000002;

#define SYS_PROTECTCHAN0				0
#define SYS_PROTECTCHAN1				1
#define SYS_PROTECTCHAN2				2
#define SYS_PROTECTCHAN3				3
#define SYS_PROTECTCHANMAX				4

#define SYS_PROTECTNONE					0x00000000
#define SYS_PROTECTREAD					0x00000001		//OK to read
#define SYS_PROTECTWRITE				0x00000002		//OK to write
#define SYS_PROTECTRDWR					(SYS_PROTECTREAD|SYS_PROTECTWRITE)

#define MEM_VIRTUAL_TO_PHYSICAL(x)		(((u32)(x))&~0xC0000000)
#define MEM_K0_TO_PHYSICAL(x)			(u32)((s8*)(x)-0x80000000)
#define MEM_K1_TO_PHYSICAL(x)			(u32)((s8*)(x)-0xC0000000)

#define MEM_PHYSICAL_TO_K0(x)			(void*)((u32)(x)+0x80000000)
#define MEM_PHYSICAL_TO_K1(x)			(void*)((u32)(x)+0xC0000000)

#define MEM_K0_TO_K1(x)					(void*)((u32)(x)+0x40000000)
#define MEM_K1_TO_K0(x)					(void*)((u32)(x)-0x40000000)


#ifdef __cplusplus
   extern "C" {
#endif /* __cplusplus */

typedef void (*resetcallback)(void);

void SYS_Init();
void SYS_SetArenaLo(void *newLo);
void* SYS_GetArenaLo();
void SYS_SetArenaHi(void *newHi);
void* SYS_GetArenaHi();
void SYS_ProtectRange(u32 chan,void *addr,u32 bytes,u32 cntrl);
resetcallback SYS_SetResetCallback(resetcallback cb);
void SYS_StartPMC(u32 mcr0val,u32 mcr1val);
void SYS_DumpPMC();
void SYS_StopPMC();

#ifdef __cplusplus
   }
#endif /* __cplusplus */

#endif