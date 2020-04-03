#include "Misc_InternalPch.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#if !SHIP
#pragma comment(lib, "winmm.lib")
#undef GetClassInfo
#endif // SHIP
#elif defined(PLATFORM_MAC)
#include <CoreServices/CoreServices.h>
#import <Cocoa/Cocoa.h>
#include <unistd.h>
#include <sys/time.h>
#elif defined(PLATFORM_IOS)
#include <QuartzCore/QuartzCore.h>
#elif defined(PLATFORM_ANDROID)
#include <SDL.h>
#endif // PLATFORM_WINDOWS

#include <stdio.h>
#include <assert.h>
#include "Map.h"
#include "AnsiString.h"
#include <time.h>

namespace Misc {

bool gInAssert = false;	

uint32_t GetMilliseconds()
{
#if defined(PLATFORM_WINDOWS)
	return ::timeGetTime();
#endif
#if defined(PLATFORM_MAC)
	timeval time;
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000) + (time.tv_usec / 1000);
#endif // PLATFORM_WINDOWS
#if defined(PLATFORM_IOS)
	CFTimeInterval timeInterval = CACurrentMediaTime();
	return (uint32_t)(timeInterval * 1000);
#endif // PLATFORM_WINDOWS
#if defined(PLATFORM_ANDROID)
	return SDL_GetTicks();
#endif // PLATFORM_ANDROID
	CORE_ASSERT(0);
}


void SleepMilliseconds(unsigned int milliseconds)
{
#if defined(PLATFORM_WINDOWS)
	::Sleep(milliseconds);
#elif defined(PLATFORM_MAC)  ||  defined(PLATFORM_IOS)
	usleep(milliseconds * 1000);
#elif defined(PLATFORM_ANDROID)
	SDL_Delay(milliseconds);
#endif
}


} // namespace Misc
