#include <stdlib.h>
#include <string.h>
#include "asm.h"
#include "processor.h"
#include "irq.h"
#include "gx.h"
#include "lwp.h"
#include "video.h"
#include "video_types.h"

//#define _VIDEO_DEBUG

#define VIDEO_MQ					1

/*+----------------------------------------------------------------------------------------------+*/
#define MEM_VIDEO_BASE               0xCC002000           ///< Memory address of Video Interface
#define MEM_VIDEO_BASE_PTR           (u32*)MEM_VIDEO_BASE   ///< Pointer to Video Interface
/*+----------------------------------------------------------------------------------------------+*/
#define R_VIDEO_STATUS1				 *((vu16*)(MEM_VIDEO_BASE+0x02))   ///< Status? register location. Includes typecasting for direct c writes.
#define R_VIDEO_FRAMEBUFFER_1        *((vu32*)(MEM_VIDEO_BASE+0x1C))   ///< Framebuffer1 register location. Includes typecasting for direct c writes.
#define R_VIDEO_FRAMEBUFFER_2        *((vu32*)(MEM_VIDEO_BASE+0x24))   ///< Framebuffer2 register location. Includes typecasting for direct c writes.
#define R_VIDEO_HALFLINE_1           *((vu16*)(MEM_VIDEO_BASE+0x2C))   ///< HalfLine1 register location. Includes typecasting for direct c writes.
#define R_VIDEO_HALFLINE_2           *((vu16*)(MEM_VIDEO_BASE+0x2E))   ///< HalfLine2 register location. Includes typecasting for direct c writes.
#define R_VIDEO_STATUS               *((vu16*)(MEM_VIDEO_BASE+0x6C))   ///< VideoStatus register location. Includes typecasting for direct c writes.

static const u32 VIDEO_Mode640X480YUV16[3][32] = {
	// NTSC 60Hz
	{
		0x0F060001, 0x476901AD, 0x02EA5140, 0x00030018,
		0x00020019, 0x410C410C, 0x40ED40ED, 0x00435A4E,
		0x00000000, 0x00435A4E, 0x00000000, 0x00000000,
		0x110701AE, 0x10010001, 0x00010001, 0x00010001,
		0x00000000, 0x00000000, 0x28500100, 0x1AE771F0,
		0x0DB4A574, 0x00C1188E, 0xC4C0CBE2, 0xFCECDECF,
		0x13130F08, 0x00080C0F, 0x00FF0000, 0x00000000,
		0x02800000, 0x000000FF, 0x00FF00FF, 0x00FF00FF
	},
	//PAL 50Hz
	{
		0x11F50101, 0x4B6A01B0, 0x02F85640, 0x00010023,
		0x00000024, 0x4D2B4D6D, 0x4D8A4D4C, 0x00435A4E,
		0x00000000, 0x00435A4E, 0x00000000, 0x013C0144,
		0x113901B1, 0x10010001, 0x00010001, 0x00010001,
		0x00000000, 0x00000000, 0x28500100, 0x1AE771F0,
		0x0DB4A574, 0x00C1188E, 0xC4C0CBE2, 0xFCECDECF,
		0x13130F08, 0x00080C0F, 0x00FF0000, 0x00000000,
		0x02800000, 0x000000FF, 0x00FF00FF, 0x00FF00FF	
	},
	//PAL 60Hz
	{
		0x0F060001, 0x476901AD, 0x02EA5140, 0x00030018,
		0x00020019, 0x410C410C, 0x40ED40ED, 0x00435A4E,
		0x00000000, 0x00435A4E, 0x00000000, 0x00050176,
		0x110701AE, 0x10010001, 0x00010001, 0x00010001,
		0x00000000, 0x00000000, 0x28500100, 0x1AE771F0,
		0x0DB4A574, 0x00C1188E, 0xC4C0CBE2, 0xFCECDECF,
		0x13130F08, 0x00080C0F, 0x00FF0000, 0x00000000,
		0x02800000, 0x000000FF, 0x00FF00FF, 0x00FF00FF
	}
};

GXRModeObj TVPal524IntAa = 
{
	VI_TVMODE_PAL_INT,
	640,
	264,
	524,
	(VI_MAX_WIDTH_PAL-640)/2,
	(VI_MAX_HEIGHT_PAL-528)/2,
	640,
	524,
	VI_XFBMODE_DF,
	GX_FALSE,
	GX_TRUE,
	
    // sample points arranged in increasing Y order
	 3,  2,  9,  6,  3, 10,  // pix 0, 3 sample points, 1/12 units, 4 bits each
	 3,  2,  9,  6,  3, 10,  // pix 1
	 9,  2,  3,  6,  9, 10,  // pix 2
	 9,  2,  3,  6,  9, 10,  // pix 3
 
	 // vertical filter[7], 1/64 units, 6 bits each
	 4,         // line n-1
	 8,         // line n-1
	12,         // line n
	16,         // line n
	12,         // line n
	 8,         // line n+1
	 4          // line n+1
};

GXRModeObj TVPal528IntDf = 
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    528,             // efbHeight
    528,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 528)/2,        // viYOrigin
    640,             // viWidth
    528,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
     6,  6,  6,  6,  6,  6,  // pix 0, 3 sample points, 1/12 units, 4 bits each
     6,  6,  6,  6,  6,  6,  // pix 1
     6,  6,  6,  6,  6,  6,  // pix 2
     6,  6,  6,  6,  6,  6,  // pix 3

    // vertical filter[7], 1/64 units, 6 bits each
     8,         // line n-1
     8,         // line n-1
    10,         // line n
    12,         // line n
    10,         // line n
     8,         // line n+1
     8          // line n+1
};

GXRModeObj TVPal574IntDfScale = 
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    574,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 574)/2,        // viYOrigin
    640,             // viWidth
    574,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
     6,  6,  6,  6,  6,  6,  // pix 0, 3 sample points, 1/12 units, 4 bits each
     6,  6,  6,  6,  6,  6,  // pix 1
     6,  6,  6,  6,  6,  6,  // pix 2
     6,  6,  6,  6,  6,  6,  // pix 3

    // vertical filter[7], 1/64 units, 6 bits each
     8,         // line n-1
     8,         // line n-1
    10,         // line n
    12,         // line n
    10,         // line n
     8,         // line n+1
     8          // line n+1
};

static lwpq_t video_queue;

static u32 videoMode = -1;
static vu32 retraceCount = 0;
static const u32 *currTiming = NULL;
static VIRetraceCallback preRetraceCB = NULL;
static VIRetraceCallback postRetraceCB = NULL;

extern void __UnmaskIrq(u32);
extern void __MaskIrq(u32);
extern u32 __video_clearif();

#ifdef _VIDEO_DEBUG
extern int printf(const char *fmt,...);
#endif

static void __VIRetraceHandler(u32 nIrq,void *pCtx)
{
	u32 vint = __video_clearif();

	if(vint&0xc) return;

	retraceCount++;
	if(preRetraceCB)
		preRetraceCB(retraceCount);

	if(postRetraceCB)
		postRetraceCB(retraceCount);
#ifdef _VIDEO_DEBUG
	printf("__VIRetraceHandler(%d)\n",retraceCount);
#endif
	LWP_WakeThread(video_queue);
}
/*
static void __VIInit(u8 mode)
{
	int i;
	*((u32*)0x800000CC) = mode;
	currTiming = VIDEO_Mode640X480YUV16[mode];

	R_VIDEO_STATUS1 = 2;
	for(i=0;i<1000;i++) {}
	R_VIDEO_STATUS1 = 2;
}

static u32 getCurrentHalfLine()
{
	u32 tmp;
	u32 halfLine1 = 0;
	u32 halfLine2 = 0;

	tmp = R_VIDEO_HALFLINE_1;
	do {
		halfLine1 = R_VIDEO_HALFLINE_1;
		halfLine2 = R_VIDEO_HALFLINE_2;
	} while(halfLine1!=tmp);
	
	halfLine1--;
	halfLine2--;
	halfLine1 <<= 1;
	tmp = ((u16*)currTiming)[3];

	return ((halfLine2/tmp)+halfLine1);	
}
*/

void __vi_init()
{
	retraceCount = 0;
	
	LWP_InitQueue(&video_queue);

	IRQ_Request(IRQ_PI_VI,__VIRetraceHandler,NULL);
	__UnmaskIrq(IRQMASK(IRQ_PI_VI));
}

void VIDEO_Init(u32 VideoMode)
{
	u32 *pDstAddr=MEM_VIDEO_BASE_PTR;

	videoMode = VideoMode;
	switch(VideoMode) {
		case VIDEO_640X480_NTSC_YUV16:
		case VIDEO_640X480_PAL50_YUV16:
		case VIDEO_640X480_PAL60_YUV16:
			currTiming = VIDEO_Mode640X480YUV16[videoMode];
			break;
		default:
			videoMode = VIDEO_GetCurrentMode();
			currTiming = VIDEO_Mode640X480YUV16[videoMode];
			break;
	}
	memcpy(pDstAddr,currTiming,sizeof(u32)*32);
}

void VIDEO_SetFrameBuffer(u32 Which, u32 FrameBufferAddr)
{
   switch(Which)
   {
      // Framebuffer 1 
      case VIDEO_FRAMEBUFFER_1:
               R_VIDEO_FRAMEBUFFER_1 = (0x10000000|(FrameBufferAddr>>5));
               break;

      // Framebuffer 2 
      case VIDEO_FRAMEBUFFER_2:
               R_VIDEO_FRAMEBUFFER_2 = (0x10000000|(FrameBufferAddr>>5));
               break;

      // Set both Framebuffers 
      case VIDEO_FRAMEBUFFER_BOTH:
               R_VIDEO_FRAMEBUFFER_1 = (0x10000000|(FrameBufferAddr>>5));
               R_VIDEO_FRAMEBUFFER_2 = (0x10000000|(FrameBufferAddr>>5));
               break;
   }
}

void VIDEO_SetBlack(bool btrue)
{
	if(btrue==TRUE) {
		((vu16*)MEM_VIDEO_BASE)[0] = 0x0006;
		((vu32*)MEM_VIDEO_BASE)[3] = 0x001501E6;
		((vu32*)MEM_VIDEO_BASE)[4] = 0x001401E7;
	} else {
		switch(videoMode) {
			case VIDEO_640X480_NTSC_YUV16:
				((vu16*)MEM_VIDEO_BASE)[0] = 0x0F06;
				((vu32*)MEM_VIDEO_BASE)[3] = 0x00030018;
				((vu32*)MEM_VIDEO_BASE)[4] = 0x00020019;
				break;
			case VIDEO_640X480_PAL50_YUV16:
				((vu16*)MEM_VIDEO_BASE)[0] = 0x11F5;
				((vu32*)MEM_VIDEO_BASE)[3] = 0x00010023;
				((vu32*)MEM_VIDEO_BASE)[4] = 0x00000024;
				break;
			case VIDEO_640X480_PAL60_YUV16:
				((vu16*)MEM_VIDEO_BASE)[0] = 0x0F06;
				((vu32*)MEM_VIDEO_BASE)[3] = 0x00030018;
				((vu32*)MEM_VIDEO_BASE)[4] = 0x00020019;
				break;
		}
	}
}

void VIDEO_WaitVSync(void)
{
	u32 retcnt = retraceCount;

	while(retraceCount==retcnt) {
#ifdef _VIDEO_DEBUG
		printf("VIDEO_WaitVSync(%d,%d)\n",retraceCount,retcnt);
#endif
		LWP_SleepThread(video_queue);
	}
}

#define CLAMP(x,l,h) ((x>h)?h:((x<l)?l:x))

u32 VIDEO_GetCurrentMode()
{
	return (R_VIDEO_STATUS1>>8)&3;
}

u32 VIDEO_GetYCbCr(u8 r,u8 g,u8 b)
{
	float Y,Cb,Cr;

	Y  =  0.257 * (float)r + 0.504 * (float)g + 0.098 * (float)b +  16.0 + 0.5;
	Cb = -0.148 * (float)r - 0.291 * (float)g + 0.439 * (float)b + 128.0 + 0.5;
	Cr =  0.439 * (float)r - 0.368 * (float)g - 0.071 * (float)b + 128.0 + 0.5;

	Y  = CLAMP(Y,16,235);
	Cb = CLAMP(Y,16,240);
	Cr = CLAMP(Y,16,240);

	return (u32)((u8)Y<<24|(u8)Cb<<16|(u8)Y<<8|(u8)Cr);
}

VIRetraceCallback VIDEO_SetPreRetraceCallback(VIRetraceCallback callback)
{
	u32 level = 0;
	VIRetraceCallback ret = preRetraceCB;
	_CPU_ISR_Disable(level);
	preRetraceCB = callback;
	_CPU_ISR_Restore(level);
	return ret;
}

VIRetraceCallback VIDEO_SetPostRetraceCallback(VIRetraceCallback callback)
{
	u32 level = 0;
	VIRetraceCallback ret = postRetraceCB;
	_CPU_ISR_Disable(level);
	postRetraceCB = callback;
	_CPU_ISR_Restore(level);
	return ret;
}
