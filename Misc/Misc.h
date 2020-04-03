#ifndef MISC_H
#define MISC_H

#include <assert.h>
#include <stdint.h>
#include <new>

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#define MISC_CDECL __cdecl
#if defined(_MSC_VER) && _MSC_VER < 1300
#include "stdint.vc.h"
#endif
#elif defined(macintosh)  ||  defined(__APPLE__)
#if !defined(PLATFORM_IOS)
#define PLATFORM_MAC
#endif
#define MISC_CDECL
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID
#define MISC_CDECL
#elif defined(linux)
#define PLATFORM_LINUX
#define MISC_CDECL
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;

#if defined(PLATFORM_WINDOWS)
typedef signed __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;
#elif defined(PLATFORM_MAC)  ||  defined(PLATFORM_IOS)
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
#elif defined(PLATFORM_ANDROID)
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
#elif defined(PLATFORM_LINUX)
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
#endif

typedef unsigned short wchar;

#define array_countof(array) (sizeof(array)/sizeof(array[0]))

namespace Misc {

template <typename TYPE>
inline TYPE Min(TYPE num0, TYPE num1)
{
	return (num0 < num1) ? num0 : num1;
}


template <typename TYPE>
inline TYPE Max(TYPE num0, TYPE num1)
{
	return (num0 > num1) ? num0 : num1;
}


template <typename TYPE>
inline TYPE Clamp(TYPE num, TYPE minNum, TYPE maxNum)
{
	return (num < minNum) ? minNum : (num > maxNum) ? maxNum : num;
}

uint32_t GetMilliseconds();
void SleepMilliseconds(unsigned int milliseconds);

extern bool gInAssert;

#ifdef NDEBUG

#define CORE_ASSERT(exp)		((void)0)

#else

#define CORE_ASSERT(exp)		{ ::Misc::gInAssert = true; assert(exp); ::Misc::gInAssert = false; }

#endif // NDEBUG

class AnsiString;
class Color;
class DirectoryScanner;
class File;
class Image;
class IRCServer;
class ZipArchive;
class ZipEntryFile;
class ZipEntryFileHandle;

class WideString;


} // namespace Misc

#endif // MISC_H

