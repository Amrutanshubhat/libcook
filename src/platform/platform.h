#pragma once

#include <stdint.h>

typedef struct {
	long sec;
	long nsec;
} _platform_time;

extern void PlatformCreateWindow(const uint32_t width, const uint32_t height, const char* title);
extern void PlatformCloseWindow(void);
extern bool PlatformShouldWindowClose(void);
extern void PlatformCloseWindow(void);
extern void PlatformPollEvents(void);
extern void PlatformSwapBuffers(void);
extern void PlatformSetWindowTitle(const char* title);
extern void PlatformGetCurrentTime(_platform_time* t);
extern void PlatformSetSwapInterval(uint8_t interval);
