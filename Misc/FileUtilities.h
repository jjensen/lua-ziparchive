#ifndef MISC_FILEUTILITIES_H
#define MISC_FILEUTILITIES_H

#include <time.h>

namespace Misc {

class AnsiString;

namespace FileUtilities
{
	// Creates nested directories. Can use either a '/' or '\' as a delineator.
	// Example: If 'inPath' == 'c:/test1/test2/test3", then "c:/test1" is created and then "c:/test1/test2" is created.
	//   In the above example, if 'inPath' == 'c:/test1/test2/test3/", then "c:/test1/test2/test3" is also created.
	bool PathCreate(const char* inPath);

	bool PathDestroy(const char* inDirName);

	bool FileExists(const char* fileName);

	time_t FileGetLastWriteTime(const char* fileName);
	void FileSetLastWriteTime(const char* fileName, time_t lastWriteTime);

	bool FileSetAttributes(const char* fileName, bool readOnly);

	bool FileErase(const char* fileName, bool eraseEvenIfReadOnly = false);

	bool FileCopy(const char* srcFileName, const char* dstFileName);
	bool FileMove(const char* srcFileName, const char* dstFileName);

	bool CopyDirectory(const char* srcDirectory, const char* dstDirectory);

	AnsiString PathCombine(const char* path);
} // namespace FileUtilities

} // namespace Misc

#endif // MISC_FILEUTILITIES_H
