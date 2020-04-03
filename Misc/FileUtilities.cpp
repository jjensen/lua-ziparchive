#include "Misc_InternalPch.h"
#include "FileUtilities.h"
#include "AnsiString.h"
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <io.h>
#include <sys/stat.h>
#elif defined(PLATFORM_IOS)
#import <Foundation/Foundation.h>
#include <sys/stat.h>
#include <dirent.h>
#define MAX_MAC_PATH_LONG 2048
#define MAX_PATH MAX_MAC_PATH_LONG
#elif defined(PLATFORM_MAC)
#import <Foundation/Foundation.h>
#include <sys/stat.h>
#include <dirent.h>
#define MAX_MAC_PATH_LONG 2048
#define MAX_PATH MAX_MAC_PATH_LONG
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#define MAX_MAC_PATH_LONG 2048
#define MAX_PATH MAX_MAC_PATH_LONG
#endif
#include "DiskFile.h"

namespace Misc {
namespace FileUtilities {

AnsiString RemoveTrailingSlash(const AnsiString& theDirectory)
{
	size_t aLen = theDirectory.Length();
	
	if (((theDirectory[aLen-1] == '\\') || (theDirectory[aLen-1] == '/')))
		return theDirectory.Sub(0, aLen - 1);
	else
		return theDirectory;
}

AnsiString	AddTrailingSlash(const AnsiString& theDirectory, bool backSlash)
{
	if (!theDirectory.IsEmpty())
	{
		char aChar = theDirectory[theDirectory.Length()-1];
		if (aChar!='\\' && aChar!='/')
			return theDirectory + (backSlash?'\\':'/');
		else
			return theDirectory;
	}
	else
		return "";
}

bool PathCreate(const char* inPath)
{
	char path[MAX_PATH];
	char* pathPtr = path;

	if (*inPath == '/')
	{
		inPath++;			// Skip the initial /
		*pathPtr++ = '/';
	}
	
	while (char ch = *inPath++)
	{
		if (ch == '/'  ||  ch == '\\')
		{
			*pathPtr = 0;
#if defined(PLATFORM_WINDOWS)
			// Create the directory if it's not a drive letter.
			char* colonPtr = pathPtr - 1;
			bool isDriveLetter = colonPtr == (path + 1)  &&  *colonPtr == ':';
			if (!isDriveLetter)
			{
				if (!::CreateDirectoryA(path, NULL)  &&  ::GetLastError() != ERROR_ALREADY_EXISTS)
					return false;
			}
			*pathPtr++ = '\\';
#else
			if (mkdir(path, 0777)  &&  errno != EEXIST)
				return false;
			*pathPtr++ = '/';			
#endif
		}
		else
			*pathPtr++ = ch;
	}

	return true;
}


bool PathDestroy(const char* inDirName)
{
#if defined(WIN32)
	char dirName[MAX_PATH];
	char* dirNamePtr = dirName;
	char ch;
	WIN32_FIND_DATAA fd;
	HANDLE handle;
	
	if (*inDirName == 0)
		return true;
	
	while (ch = *inDirName++) {
		if (ch == '/'  ||  ch == '\\')
			*dirNamePtr++ = '\\';
		else
			*dirNamePtr++ = ch;
	}
	if (dirNamePtr[-1] != '\\')
		*dirNamePtr++ = '\\';
	*dirNamePtr = 0;
	
	strcpy(dirNamePtr, "*.*");
	
	handle = FindFirstFileA(dirName, &fd);
	if (handle == INVALID_HANDLE_VALUE)
		return false;
	
	*dirNamePtr = 0;
	
	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			int skipDir = fd.cFileName[0] == '.'  &&
			(fd.cFileName[1] == 0  ||  (fd.cFileName[1] == '.'  &&  fd.cFileName[2] == 0));
			if (!skipDir) {
				strcpy(dirNamePtr, fd.cFileName);
				strcat(dirNamePtr, "\\");
				if (!PathDestroy(dirName))
					return false;
			}
		} else {
			strcpy(dirNamePtr, fd.cFileName);
			SetFileAttributesA(dirName, FILE_ATTRIBUTE_ARCHIVE);
			if (!DeleteFileA(dirName)) {
				FindClose(handle);
				return false;
			}
		}
	} while (FindNextFileA(handle, &fd));
	
	FindClose(handle);
	
	*dirNamePtr = 0;
	if (!RemoveDirectoryA(dirName))
		return false;
#else
	char dirName[MAX_PATH];
	char* dirNamePtr;
	char ch;
	
	if (*inDirName == 0)
		return true;
	
	dirNamePtr = dirName;
	
	while ((ch = *inDirName++) != 0) {
		if (ch == '/'  ||  ch == '\\')
			*dirNamePtr++ = '/';
		else
			*dirNamePtr++ = ch;
	}
	if (dirNamePtr[-1] != '/')
		*dirNamePtr++ = '/';
	*dirNamePtr = 0;
	
	DIR* dirp = opendir(dirName);
	if (!dirp)
		return false;
	
	struct dirent* dp;
	while ((dp = readdir(dirp)) != NULL) {
		struct stat attr;
		strcpy(dirNamePtr, dp->d_name);
		if (stat(dirName, &attr) != -1  &&  (attr.st_mode & S_IFDIR)) {
			int skipDir = dp->d_name[0] == '.'  &&  (dp->d_name[1] == 0  ||  (dp->d_name[1] == '.'  &&  dp->d_name[2] == 0));
			if (!skipDir) {
				strcat(dirNamePtr, "\\");
				if (!PathDestroy(dirName))
					return false;
			}
		} else {
			//			::SetFileAttributes(dirName, FILE_ATTRIBUTE_ARCHIVE);
			//			if (!::DeleteFile(dirName))
			if (unlink(dirName) == -1) {
				closedir(dirp);
				return false;
			}
		}
	}
	
	closedir(dirp);
	
	*dirNamePtr = 0;
	if (rmdir(dirName) == -1)
		return false;
#endif
	
	return true;
}


bool FileExists(const char* fileName)
{
#if defined(PLATFORM_WINDOWS)
	return _access(fileName, 0) != -1;
#else
	return access(fileName, 0) != -1;
#endif
}


time_t FileGetLastWriteTime(const char* fileName)
{
	DiskFile file;
	return file.Open(fileName) ? file.GetLastWriteTime() : 0;
}


void FileSetLastWriteTime(const char* fileName, time_t lastWriteTime)
{
	DiskFile file;
	if (file.Open(fileName, File::MODE_READWRITE))
		file.SetLastWriteTime(lastWriteTime);
}


bool FileSetAttributes(const char* fileName, bool readOnly)
{
#if defined(PLATFORM_WINDOWS)
	int flags = readOnly ? 0 : _S_IREAD | _S_IWRITE;
	return _chmod(fileName, flags) == 0;
#elif defined(PLATFORM_ANDROID)
    int inmode = readOnly ? 0444 : 0666;
	int mode = (((inmode / 100) % 10) * 64) + (((inmode / 10) % 10) * 8) + (inmode % 10);
	return chmod(fileName, mode) == 0;
#else
	int flags = readOnly ? 0 : S_IREAD | S_IWRITE;
	return chmod(fileName, flags) == 0;
#endif
}


bool FileErase(const char* fileName, bool eraseEvenIfReadOnly)
{
	if (eraseEvenIfReadOnly)
	{
		FileSetAttributes(fileName, false);
	}
#if defined(PLATFORM_WINDOWS)
	return _unlink(fileName) == 0;
#else
	return unlink(fileName) == 0;
#endif
}

bool FileCopy(const char* srcFileName, const char* dstFileName)
{
#if defined(PLATFORM_WINDOWS)
	// NOTE: This may fail if the file already exists but is marked read-only.
	BOOL success = ::CopyFileA(srcFileName, dstFileName, FALSE);
	return success ? true : false;
#else
	//CORE_ASSERT(!"CopyFile not implemented for the Macintosh.");
	int inputFile;
	int outputFile;

	// Operate in 64k buffers.
	const size_t BUFFER_SIZE = 64 * 1024;
	unsigned char* buffer;
	
	ssize_t fileSize;

    inputFile = open(srcFileName, O_RDONLY);
	if (inputFile == -1) {
		return false;
	}
    outputFile = open(dstFileName, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (outputFile == -1) {
		close(inputFile);
		return false;
	}

	// Allocate the buffer space.
	buffer = (unsigned char*)malloc(BUFFER_SIZE);
	
	// Get the source file's size.
	fileSize = lseek(inputFile, 0, SEEK_END);
	lseek(inputFile, 0, SEEK_SET);
	
	// Keep copying until there is no more file left to copy.
	int ret = 1;
	while (fileSize > 0)
	{
		// Copy the minimum of BUFFER_SIZE or the fileSize.
        ssize_t readSize = BUFFER_SIZE < fileSize ? BUFFER_SIZE : fileSize;
		if (read(inputFile, buffer, readSize) != readSize) {
			ret = 0;
			break;
		}
		if (write(outputFile, buffer, readSize) != readSize) {
			ret = 0;
			break;
		}
		fileSize -= readSize;
	}
	
	close(outputFile);
	close(inputFile);

	return true;
#endif
}


bool FileMove(const char* srcFileName, const char* dstFileName)
{
	FileSetAttributes(dstFileName, false);

#if defined(PLATFORM_WINDOWS)
	DWORD fileAttributes = GetFileAttributes(dstFileName);
	if (fileAttributes != INVALID_FILE_ATTRIBUTES)
	{
		if (fileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			SetFileAttributes(dstFileName, fileAttributes & ~FILE_ATTRIBUTE_READONLY);
		}
	}
	if (!MoveFile(srcFileName, dstFileName))
	{
		CopyFile(srcFileName, dstFileName, FALSE);
		_unlink(srcFileName);		// Just in case.
		return true;
	}

	return false;

#elif defined(PLATFORM_MAC)
	NSFileManager *fileManager = [NSFileManager defaultManager];
	unlink(dstFileName);
	if ([fileManager movePath:[NSString stringWithUTF8String: srcFileName] toPath: [NSString stringWithUTF8String: dstFileName] handler:nil] == NO)
	{
		if ([fileManager copyPath:[NSString stringWithUTF8String: srcFileName] toPath: [NSString stringWithUTF8String: dstFileName] handler:nil] == NO)
		{
			int hi = 5;  (void)hi;
		}

		return false;
	}

	return true;

#elif defined(PLATFORM_IOS)
	NSFileManager *fileManager = [NSFileManager defaultManager];
	unlink(dstFileName);
	if ([fileManager moveItemAtPath:[NSString stringWithUTF8String: srcFileName] toPath: [NSString stringWithUTF8String: dstFileName] error:NULL] == NO)
	{
//		if ([fileManager copyPathAtPath:[NSString stringWithUTF8String: srcFileName] toPath: [NSString stringWithUTF8String: dstFileName] error:NULL] == NO)
		{
			int hi = 5;  (void)hi;
		}

		return false;
	}

	return true;

#elif defined(PLATFORM_ANDROID)
	unlink(dstFileName);
	rename(srcFileName, dstFileName);
	return true;

#else
	CORE_ASSERT(0);
	return false;
#endif
}


AnsiString PathCombine(const char* path) {
  const char *pathEnd = path + strlen(path);
  char *file;
  char *filestart, *fileorg;

  filestart = fileorg = (char*)malloc(pathEnd - path + 1);
  memcpy(fileorg, path, pathEnd - path);
  fileorg[pathEnd - path] = 0;

  {
    char *ptr = filestart;
    char *endptr = filestart + (pathEnd - path);
    file = filestart;
    while ( ptr != endptr ) {
      // Skip './'
      if ( *ptr == '.' ) {
        if ( ptr[1] == 0  ||  ptr[1] == '/'  ||  ptr[1] == '\\' ) {
          int add = ptr[1] ? 1 : 0;
          ptr += 1 + add;
          if ( file == filestart ) {
            file += 1 + add;
            filestart += 1 + add;
          }
        } else if ( ptr[1] == '.'  &&  ( ptr[2] == 0  ||  ptr[2] == '/'  ||  ptr[2] == '\\' ) ) {
          // Go up a subdirectory.
          int add = ptr[2] ? 1 : 0;
          ptr += 2 + add;
          if ( file != filestart ) {
	    file--;
	    file -= (*file == '/' ? 1 : 0);
	    if ( file - filestart == 1  &&  *file == ':' )
              file += 2;
            else {
              while ( file >= filestart  &&  ( *file != '/'  &&  *file != '\\' ) )
                file--;
              file++;
            }
          } else {
            file += 2 + add;
//            filestart += 2 + add;
          }
        } else {
          *file++ = *ptr++;
        }
      } else if ( *ptr == '\\'  ||  *ptr == '/' ) {
        if ( file > filestart  &&  ( file[-1] == '/'  ||  file[-1] == '\\' ) ) {
          ptr++;
        } else {
#if defined(WIN32)
          /* is it a unc path? */
          if ( file == filestart  &&  (file[0] == '\\'  ||  file[0] == '/')  &&  (file[1] == '\\'  ||  file[1] == '/')) {
            file += 2;
            ptr += 2;
          } else 
#endif /* WIN32 */
		  {
            *file++ = '/';
            ptr++;
          }
        }
      } else {
        *file++ = *ptr++;
      }
    }
  }
  *file = 0;
//	file--;
//	if ( *file == '/' )
//    *file = 0;

  AnsiString ret = filestart;
  free(fileorg);
  return ret;
}


} // namespace FileUtilities
} // namespace Misc
