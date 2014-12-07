#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

#ifdef USE_MOTION_SENSOR
extern void systemUpdateMotionSensor (void);
extern int  systemGetSensorX (void);
extern int  systemGetSensorY (void);
#endif

extern void systemMessage(const char *, ...);

extern void systemOnWriteDataToSoundBuffer(s16* finalWave, int length);

extern void* systemAlloc(u32 size);
extern void systemFree(void* ptr);

extern u32* systemGetPixels();
extern u16 systemGetScreenWidth();
extern u16 systemGetScreenHeight();
extern u16 systemGetScreenDepth();
extern void systemPrepareDraw();
extern void systemDrawPixel(int x, int y, u32 color);
extern void systemFlushScreen();
#endif // SYSTEM_H
