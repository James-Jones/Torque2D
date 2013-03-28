//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "platform//platform.h"
#include "platformNaCl/platformNaCl.h"
#include "platform/platformFileIO.h"
#include "collection/vector.h"
#include "string/stringTable.h"
#include "console/console.h"
#include "io/resource/resourceManager.h"
#include "string/unicode.h"

#include <time.h>
#include <sys/utime.h>

#include <string>

//------------------------------------------------------------------------------

enum DriveType
{
   DRIVETYPE_FIXED = 0,
   DRIVETYPE_REMOVABLE = 1,
   DRIVETYPE_REMOTE = 2,
   DRIVETYPE_CDROM = 3,
   DRIVETYPE_RAMDISK = 4,
   DRIVETYPE_UNKNOWN = 5
};

//-------------------------------------- Helper Functions
static void forwardslash(char *str)
{
   while(*str)
   {
      if(*str == '\\')
         *str = '/';
      str++;
   }
}

static void backslash(char *str)
{
   while(*str)
   {
      if(*str == '/')
         *str = '\\';
      str++;
   }
}

//-----------------------------------------------------------------------------
bool Platform::fileDelete(const char * name)
{
    return(false);
}

bool Platform::fileRename(const char *oldName, const char *newName)
{
    return false;
}

bool Platform::fileTouch(const char * name)
{
   return(false);
};

bool Platform::pathCopy(const char *fromName, const char *toName, bool nooverwrite)
{
    return false;
}

//-----------------------------------------------------------------------------
// Constructors & Destructor
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// After construction, the currentStatus will be Closed and the capabilities
// will be 0.
//-----------------------------------------------------------------------------
File::File()
: currentStatus(Closed), capability(0)
{
    handle = NULL;
}

//-----------------------------------------------------------------------------
// insert a copy constructor here... (currently disabled)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
File::~File()
{
    close();
    delete (NaClLocalFile*)handle;
    handle = (void *)NULL;
}

void OpenFileFromMainThread(void* user_data, int32_t result)
{
    OpenFileParams* params = static_cast<OpenFileParams*>(user_data);
    params->openedFile = naclState.localFileSys.OpenFile(params);
}

//-----------------------------------------------------------------------------
// Open a file in the mode specified by openMode (Read, Write, or ReadWrite).
// Truncate the file if the mode is either Write or ReadWrite and truncate is
// true.
//
// Sets capability appropriate to the openMode.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::open(const char *filename, const AccessMode openMode)
{
    OpenFileParams* params = new OpenFileParams;
    params->filename = filename;
    params->openMode = openMode;

    params->openedFile = NULL;

    PP_CompletionCallback callback = PP_MakeCompletionCallback(OpenFileFromMainThread, params);
    naclState.psCore->CallOnMainThread(0, callback, PP_OK);
    params->_Waiter.acquire();

    AssertFatal(params->openedFile != NULL, "Failed to open file");

    handle = params->openedFile;

    currentStatus = Ok;

    delete params;

    return currentStatus;
}

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
    const NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;
    return psLocalFile->getPosition();
}

//-----------------------------------------------------------------------------
// Set the position of the file pointer.
// Absolute and relative positioning is supported via the absolutePos
// parameter.
//
// If positioning absolutely, position MUST be positive - an IOError results if
// position is negative.
// Position can be negative if positioning relatively, however positioning
// before the start of the file is an IOError.
//
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::setPosition(S32 position, bool absolutePos)
{
    NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;

    psLocalFile->setPosition(position, absolutePos);

    if(psLocalFile->getPosition() >= getSize())
    {
        return currentStatus = EOS;
    }
    return currentStatus = Ok;
}

//-----------------------------------------------------------------------------
// Get the size of the file in bytes.
// It is an error to query the file size for a Closed file, or for one with an
// error status.
//-----------------------------------------------------------------------------
U32 File::getSize() const
{
    NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;

    return psLocalFile->getFileInfo()->size;
}

//-----------------------------------------------------------------------------
// Flush the file.
// It is an error to flush a read-only file.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::flush()
{
    return currentStatus = Ok;
}

void CloseFileFromMainThread(void* user_data, int32_t result)
{
    NaClLocalFile* psLocalFile = (NaClLocalFile*)user_data;

    psLocalFile->CloseFile();
}

//-----------------------------------------------------------------------------
// Close the File.
//
// Returns the currentStatus
//-----------------------------------------------------------------------------
File::Status File::close()
{
    NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;

    PP_CompletionCallback callback = PP_MakeCompletionCallback(CloseFileFromMainThread, psLocalFile);
    naclState.psCore->CallOnMainThread(0, callback, PP_OK);

    return currentStatus = Closed;
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
File::Status File::getStatus() const
{
    return currentStatus;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus when an error has been encountered.
//-----------------------------------------------------------------------------
File::Status File::setStatus()
{
    return Ok;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus to status.
//-----------------------------------------------------------------------------
File::Status File::setStatus(File::Status status)
{
    return currentStatus = status;
}

void ReadFileFromMainThread(void* user_data, int32_t result)
{
    ReadFileParams* params = (ReadFileParams*)user_data;

    params->file->ReadFile(params);
}

//-----------------------------------------------------------------------------
// Read from a file.
// The number of bytes to read is passed in size, the data is returned in src.
// The number of bytes read is available in bytesRead if a non-Null pointer is
// provided.
//-----------------------------------------------------------------------------
File::Status File::read(U32 size, char *dst, U32 *bytesRead)
{
    NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;

    ReadFileParams* params = new ReadFileParams;
    params->size = size;
    params->dst = dst;
    params->file = psLocalFile;

    PP_CompletionCallback callback = PP_MakeCompletionCallback(ReadFileFromMainThread, params);
    naclState.psCore->CallOnMainThread(0, callback, PP_OK);

    //Wait for the read to complete.
    params->_Waiter.acquire();

    dMemcpy(dst, psLocalFile->getContents()+psLocalFile->getPosition(), psLocalFile->getBytesRead());
    *bytesRead = psLocalFile->getBytesRead();

    return Ok;
}

//-----------------------------------------------------------------------------
// Write to a file.
// The number of bytes to write is passed in size, the data is passed in src.
// The number of bytes written is available in bytesWritten if a non-Null
// pointer is provided.
//-----------------------------------------------------------------------------
File::Status File::write(U32 size, const char *src, U32 *bytesWritten)
{
    return IOError;
}

//-----------------------------------------------------------------------------
// Self-explanatory.
//-----------------------------------------------------------------------------
bool File::hasCapability(Capability cap) const
{
    return (0 != (U32(cap) & capability));
}

S32 Platform::compareFileTimes(const FileTime &a, const FileTime &b)
{
   return 0;
}

//--------------------------------------

bool Platform::getFileTimes(const char *filePath, FileTime *createTime, FileTime *modifyTime)
{
   return false;
}

bool Platform::createPath(const char *file)
{
    return false;
}

void Platform::openFolder(const char* path )
{
}

//--------------------------------------
bool Platform::dumpPath(const char *path, Vector<Platform::FileInfo> &fileVector, S32 recurseDepth)
{
   return false;
}


//--------------------------------------

StringTableEntry Platform::getCurrentDirectory()
{
   return StringTable->insert("");
}

bool Platform::setCurrentDirectory(StringTableEntry newDir)
{
    return false;
}

StringTableEntry Platform::getExecutableName()
{
    return StringTable->insert("");
}

StringTableEntry Platform::getExecutablePath()
{
    return StringTable->insert("");
}

//--------------------------------------
bool Platform::isFile(const char *pFilePath)
{
    return false;
}

//--------------------------------------
S32 Platform::getFileSize(const char *pFilePath)
{
    return -1;
}


//--------------------------------------
bool Platform::isDirectory(const char *pDirPath)
{
   return false;
}



//--------------------------------------
bool Platform::isSubDirectory(const char *pParent, const char *pDir)
{
   return false;
}

bool Platform::hasSubDirectory(const char *pPath)
{
   return false;
}

bool Platform::dumpDirectories( const char *path, Vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath )
{
   ResourceManager->initExcludedDirectories();

   bool retVal = false;

   return retVal;
}

//-----------------------------------------------------------------------------

StringTableEntry Platform::osGetTemporaryDirectory()
{
    //TODO: PP_FILESYSTEMTYPE_LOCALTEMPORARY
   return StringTable->insert("");
}

const char *Platform::getUserDataDirectory() 
{
   return StringTable->insert("data/");
}

const char *Platform::getUserHomeDirectory() 
{
   return StringTable->insert("home/");
}

