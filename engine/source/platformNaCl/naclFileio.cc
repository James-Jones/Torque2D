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

const int READ_BUFFER_SIZE = 1048576;//Stream in 1MB chunks.
typedef void (*OnDownloaded)(void* pvUserData, bool bSuccessful);

class NaCLFile
{
public:
    NaCLFile() : bSuccessful(0), bDownloaded(false) {
    }
    std::string filebody;
    char buffer[READ_BUFFER_SIZE];
    PP_Resource loader;
    PP_Resource request;
    OnDownloaded pfnOnDownloaded;
    void* pvUserData;

    bool bDownloaded;
    bool bSuccessful;
};

class DownloadContext
{
public:
    DownloadContext() : id(0), iDownloaded(0) {
    }

    int id;
    std::string data;
    int iDownloaded;
};


int StartDownload(NaCLFile* psFile,
            const char* url,
            void* pvUserData,
            OnDownloaded pfnOnDownloaded);

void FileDownloaded(void* pvUserData, bool bSuccessful)
{
    NaCLFile* psFile = (NaCLFile*)pvUserData;
    psFile->bDownloaded = true;
    psFile->bSuccessful = bSuccessful;
}

bool BlockOnFile(NaCLFile* psNaClFile)
{
    while(!psNaClFile->bDownloaded)
    {
        struct timespec timeToSleep;
        timeToSleep.tv_sec = 1;
        timeToSleep.tv_nsec = 500;
        nanosleep(&timeToSleep, NULL);
    }
    return psNaClFile->bSuccessful;
}

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
    handle = new NaCLFile();
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
    delete (NaCLFile*)handle;
    handle = (void *)NULL;
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
    NaCLFile* psNaClFile = (NaCLFile*)handle;

    StartDownload(psNaClFile, filename, psNaClFile, FileDownloaded);
    return IOError;
}

//-----------------------------------------------------------------------------
// Get the current position of the file pointer.
//-----------------------------------------------------------------------------
U32 File::getPosition() const
{
    return 0;
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
    return IOError;
}

//-----------------------------------------------------------------------------
// Get the size of the file in bytes.
// It is an error to query the file size for a Closed file, or for one with an
// error status.
//-----------------------------------------------------------------------------
U32 File::getSize() const
{
    NaCLFile* psNaClFile = (NaCLFile*)handle;

    if(BlockOnFile(psNaClFile))
    {
        return psNaClFile->filebody.size();
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Flush the file.
// It is an error to flush a read-only file.
// Returns the currentStatus of the file.
//-----------------------------------------------------------------------------
File::Status File::flush()
{
    return IOError;
}

//-----------------------------------------------------------------------------
// Close the File.
//
// Returns the currentStatus
//-----------------------------------------------------------------------------
File::Status File::close()
{
    return IOError;
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
    return IOError;
}

//-----------------------------------------------------------------------------
// Sets and returns the currentStatus to status.
//-----------------------------------------------------------------------------
File::Status File::setStatus(File::Status status)
{
    return currentStatus = status;
}

//-----------------------------------------------------------------------------
// Read from a file.
// The number of bytes to read is passed in size, the data is returned in src.
// The number of bytes read is available in bytesRead if a non-Null pointer is
// provided.
//-----------------------------------------------------------------------------
File::Status File::read(U32 size, char *dst, U32 *bytesRead)
{
    NaCLFile* psNaClFile = (NaCLFile*)handle;
    if(BlockOnFile(psNaClFile))
    {
        dMemcpy(dst, psNaClFile->filebody.c_str(), size);
        *bytesRead = size;
        return Ok;
    }
    return IOError;
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

void OnRead(void* pvUserData, int32_t result);

void ReadBody(NaCLFile* psFile)
{
    PP_CompletionCallback fileReadCallback;
    fileReadCallback.func = OnRead;
    fileReadCallback.user_data = psFile;
    fileReadCallback.flags = PP_COMPLETIONCALLBACK_FLAG_OPTIONAL;

    int iResult = PP_OK;
    do
    {
        iResult = naclState.psURLLoader->ReadResponseBody(psFile->loader, psFile->buffer, READ_BUFFER_SIZE, fileReadCallback);
        
        // Handle streaming data directly. Note that we *don't* want to call
        // OnRead here, since in the case of result > 0 it will schedule
        // another call to this function. If the network is very fast, we could
        // end up with a deeply recursive stack.
        if (iResult > 0)
        {
            //AppendDataBytes(buffer, result);

            // Make sure we don't get a buffer overrun.
            iResult = std::min(READ_BUFFER_SIZE, iResult);
            // Note that we do *not* try to minimally increase the amount of allocated
            // memory here by calling url_response_body_.reserve().  Doing so causes a
            // lot of string reallocations that kills performance for large files.
            psFile->filebody.insert(psFile->filebody.end(),
                                    psFile->buffer,
                                    psFile->buffer + iResult);
        }
    } while (iResult > 0);

    if(iResult != PP_OK_COMPLETIONPENDING)
    {
        OnRead(psFile, iResult);
    }
}

void OnRead(void* pvUserData, int32_t result)
{
    NaCLFile* psFile = (NaCLFile*)pvUserData;

    if (result == PP_OK)
    {
        // Streaming the file is complete.
        psFile->pfnOnDownloaded(psFile->pvUserData, true);
        delete psFile;

    }
    else if (result > 0)
    {
        // The URLLoader just filled "result" number of bytes into our buffer.
        // Save them and perform another read.

        // Make sure we don't get a buffer overrun.
        int iNumBytes = std::min(READ_BUFFER_SIZE, result);
        // Note that we do *not* try to minimally increase the amount of allocated
        // memory here by calling url_response_body_.reserve().  Doing so causes a
        // lot of string reallocations that kills performance for large files.
        psFile->filebody.insert(psFile->filebody.end(),
                                psFile->buffer,
                                psFile->buffer + iNumBytes);

        ReadBody(psFile);
    }
    else
    {
        // A read error occurred.
        psFile->pfnOnDownloaded(psFile->pvUserData, false);
        delete psFile;
    }
}

void OnOpen(void* user_data, int32_t result)
{
    int64_t bytes_received = 0;
    int64_t total_bytes_to_be_received = 0;
    NaCLFile* psFile = (NaCLFile*)user_data;

    //Try to pre-allocate memory for the response.
    if (naclState.psURLLoader->GetDownloadProgress(psFile->loader,
                                        &bytes_received,
                                        &total_bytes_to_be_received))
    {
        if (total_bytes_to_be_received > 0)
        {
            psFile->filebody.reserve(total_bytes_to_be_received);
        }
    }

    // We will not use the download progress anymore, so just disable it.
    naclState.psURLRequest->SetProperty(psFile->request,
        PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, PP_MakeBool(PP_FALSE));
    
    ReadBody(psFile);
}

int StartDownload(NaCLFile* psFile,
            const char* url,
            void* pvUserData,
            OnDownloaded pfnOnDownloaded)
{
    PP_CompletionCallback fileOpenCallback;

    psFile->loader = naclState.psURLLoader->Create(naclState.hModule);

    psFile->request = naclState.psURLRequest->Create(naclState.hModule);

    psFile->pfnOnDownloaded = pfnOnDownloaded;

    psFile->pvUserData = pvUserData;

    naclState.psURLRequest->SetProperty(psFile->request,
    PP_URLREQUESTPROPERTY_URL, naclState.psVar->VarFromUtf8(url, strlen(url)));

    naclState.psURLRequest->SetProperty(psFile->request,
        PP_URLREQUESTPROPERTY_METHOD, naclState.psVar->VarFromUtf8("GET", 3));

    naclState.psURLRequest->SetProperty(psFile->request,
        PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS, PP_MakeBool(PP_TRUE));

    fileOpenCallback.func = OnOpen;
    fileOpenCallback.user_data = psFile;
    fileOpenCallback.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    if(naclState.psURLLoader->Open(psFile->loader,
        psFile->request,
        fileOpenCallback) != PP_OK_COMPLETIONPENDING)
    {
        return 0;
    }

    return 1;
}
