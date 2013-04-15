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
#include "io/fileObject.h"

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

    if(params->openedFile == NULL || params->openedFile->IsOpened()==false)
    {
        handle = NULL;
        currentStatus = IOError;
        if(params->openedFile)
        {
            delete params->openedFile;
        }
    }
    else
    {
        switch (openMode)
        {
        case Read:
            capability = U32(FileRead);
            break;
        case Write:
        case WriteAppend:
            capability = U32(FileWrite);
            break;
        case ReadWrite:
            capability = U32(FileRead)  |
                         U32(FileWrite);
            break;
        default:
            AssertFatal(false, "File::open: bad access mode");
        }

        handle = params->openedFile;
        currentStatus = Ok;
    }

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
    if(handle)
    {
        NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;

        PP_CompletionCallback callback = PP_MakeCompletionCallback(CloseFileFromMainThread, psLocalFile);
        naclState.psCore->CallOnMainThread(0, callback, PP_OK);
    }

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
    AssertFatal(Closed != currentStatus, "File::read: file closed");
    AssertFatal(NULL != handle, "File::read: invalid file handle");
    AssertFatal(NULL != dst, "File::read: NULL destination pointer");
    AssertFatal(true == hasCapability(FileRead), "File::read: file lacks capability");
    AssertWarn(0 != size, "File::read: size of zero");

    if (Ok != currentStatus || 0 == size)
        return currentStatus;

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

    if(bytesRead)
    {
        *bytesRead = psLocalFile->getBytesRead();
    }

    psLocalFile->setPosition(psLocalFile->getBytesRead(), false);

    if(psLocalFile->getBytesRead() != size)
    {
        currentStatus = EOS;
    }
    else
    {
        currentStatus = Ok;
    }

    delete params;

    return currentStatus;
}

void WriteFileFromMainThread(void* user_data, int32_t result)
{
    WriteFileParams* params = (WriteFileParams*)user_data;

    params->file->WriteFile(params);
}


//-----------------------------------------------------------------------------
// Write to a file.
// The number of bytes to write is passed in size, the data is passed in src.
// The number of bytes written is available in bytesWritten if a non-Null
// pointer is provided.
//-----------------------------------------------------------------------------
File::Status File::write(U32 size, const char *src, U32 *bytesWritten)
{
    AssertFatal(Closed != currentStatus, "File::write: file closed");
    AssertFatal(NULL != handle, "File::write: invalid file handle");
    AssertFatal(NULL != src, "File::write: NULL source pointer");
    AssertFatal(true == hasCapability(FileWrite), "File::write: file lacks capability");
    AssertWarn(0 != size, "File::write: size of zero");

    if ((Ok != currentStatus && EOS != currentStatus) || 0 == size)
        return currentStatus;

    NaClLocalFile* psLocalFile = (NaClLocalFile*)handle;

    WriteFileParams* params = new WriteFileParams;
    params->size = size;
    params->src = src;
    params->file = psLocalFile;

    PP_CompletionCallback callback = PP_MakeCompletionCallback(WriteFileFromMainThread, params);
    naclState.psCore->CallOnMainThread(0, callback, PP_OK);

    //Wait for the write to complete.
    params->_Waiter.acquire();

    if(bytesWritten)
    {
        *bytesWritten = params->getTotalBytesWritten();
    }

    psLocalFile->setPosition(params->getTotalBytesWritten(), false);

    currentStatus = Ok;

    delete params;

    return currentStatus;
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

void MakeDirFromMainThread(void* user_data, int32_t result)
{
    MakeDirParams* params = static_cast<MakeDirParams*>(user_data);
    naclState.localFileSys.MakeDirectory(params);
}

bool Platform::createPath(const char *file)
{
   char pathbuf[1024];
   const char *dir;
   pathbuf[0] = 0;
   U32 pathLen = 0;

#if 0
   while((dir = dStrchr(file, '/')) != NULL)
   {
      dStrncpy(pathbuf + pathLen, file, dir - file);
      pathbuf[pathLen + dir-file] = 0;

      MakeDirParams params;
      params.path = pathbuf;
      PP_CompletionCallback callback = PP_MakeCompletionCallback(MakeDirFromMainThread, &params);
      naclState.psCore->CallOnMainThread(0, callback, PP_OK);

      params._Waiter.acquire();

      pathLen += dir - file;
      pathbuf[pathLen++] = '/';
      file = dir + 1;
   }
#else

      dir = dStrrchr(file, '/');

      if(dir != NULL)
      {
          const dsize_t len = dir - file;
          dStrncpy(pathbuf, file, len);
          pathbuf[len] = 0;

          MakeDirParams params;
          params.path = pathbuf;
          PP_CompletionCallback callback = PP_MakeCompletionCallback(MakeDirFromMainThread, &params);
          naclState.psCore->CallOnMainThread(0, callback, PP_OK);

          params._Waiter.acquire();
      }

#endif
   return true;
}

void Platform::openFolder(const char* path )
{
}

//--------------------------------------
static bool recurseDumpPath(const char *path, Vector<Platform::FileInfo> &fileVector, S32 recurseDepth )
{
    char fullFilePath[1024];

    //Platform::makeFullPathName(path, fullPath, sizeof(fullPath));

    dSprintf(fullFilePath, sizeof(fullFilePath), "dirdump/%s/files.txt", path);

    bool success = false;
    FileStream fileStream;

    success = fileStream.open( fullFilePath, FileStream::Read);

    //Add all files in this directory to the vector
    if(success == true)
    {
        const U32 size = fileStream.getStreamSize();
        char* pFileContents = new char[size + 1];
        // Read script.
        fileStream.read(size, pFileContents);
        fileStream.close();
        pFileContents[size] = 0;

        std::string fileList(pFileContents);
        delete pFileContents;
        pFileContents = NULL;
        size_t startPos = 0;
        size_t endPos;

        const char* lineEnding = "\r\n";
        const int lineEndingCharCount = 2;

        endPos = fileList.find(lineEnding, startPos);
        //Each line in the file
        while(endPos != std::string::npos)
        {
            char subFilePath[1024];
            std::string str = fileList.substr(startPos, endPos-startPos);
            const char* fileName = str.c_str();

            startPos = endPos+lineEndingCharCount;
            endPos = fileList.find(lineEnding, startPos);

             // add it to the list
             fileVector.increment();
             Platform::FileInfo& rInfo = fileVector.last();

             rInfo.pFullPath = StringTable->insert(path);
             rInfo.pFileName = StringTable->insert(fileName);

             dSprintf(subFilePath, sizeof(subFilePath), "%s/%s", path, fileName);

             success = fileStream.open( subFilePath, FileStream::Read);
             if(success == false)
             {
                 return false;
             }
             rInfo.fileSize = fileStream.getStreamSize();
             fileStream.close();
        }
    }

    //Move on the next directory
    if(recurseDepth > 0 || recurseDepth == -1)
    {
        char fullDirPath[1024];
        dSprintf(fullDirPath, sizeof(fullDirPath), "dirdump/%s/dirs.txt", path);

        success = fileStream.open( fullDirPath, FileStream::Read);

        if(success)
        {
            const U32 size = fileStream.getStreamSize();
            char* pFileContents = new char[size + 1];
            // Read script.
            fileStream.read(size, pFileContents);
            fileStream.close();
            pFileContents[size] = 0;

            std::string dirList(pFileContents);
            delete pFileContents;
            pFileContents = NULL;
            size_t startPos = 0;
            size_t endPos;

            const char* lineEnding = "\r\n";
            const int lineEndingCharCount = 2;

            endPos = dirList.find(lineEnding, startPos);
            //Each line in the file
            while(endPos != std::string::npos)
            {
                char subDirPath [1024];
                std::string str = dirList.substr(startPos, endPos-startPos);
                const char* childDir = str.c_str();

                startPos = endPos+lineEndingCharCount;
                endPos = dirList.find(lineEnding, startPos);

                // skip . and .. directories
                if (dStrcmp(childDir, ".") == 0 || dStrcmp(childDir, "..") == 0)
                   continue;

                // Skip excluded directores
                if(Platform::isExcludedDirectory(childDir))
                   continue;

                dSprintf(subDirPath, sizeof(subDirPath), "%s/%s", path, childDir);

                if( recurseDepth > 0 )
                {
                    recurseDumpPath(subDirPath, fileVector, recurseDepth - 1);
                }
                else if (recurseDepth == -1)
                {
                    recurseDumpPath(subDirPath, fileVector, -1);
                }
            }
        }
    }

    return true;
}
bool Platform::dumpPath(const char *path, Vector<Platform::FileInfo> &fileVector, S32 recurseDepth)
{
   return recurseDumpPath(path, fileVector, recurseDepth );
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

static bool recurseDumpDirectories(const char *basePath, const char *subPath, Vector<StringTableEntry> &directoryVector, S32 currentDepth, S32 recurseDepth, bool noBasePath)
{
   char search[1024];

   //-----------------------------------------------------------------------------
   // Compose our search string - Format : ([path]/[subpath]/)
   //-----------------------------------------------------------------------------

   char trail = basePath[ dStrlen(basePath) - 1 ];
   char subTrail;
   char subLead;
   if( subPath )
   {
       subTrail = subPath[ dStrlen(subPath) - 1 ];
       subLead = subPath[0];
   }


   if( trail == '/' )
   {
      // we have a sub path and it's not an empty string
      if(  subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) )
      {
         if( subTrail == '/' )
            dSprintf(search, 1024, "%s%s", basePath,subPath );
         else
            dSprintf(search, 1024, "%s%s/", basePath,subPath );
      }
      else
         dSprintf( search, 1024, "%s", basePath );
   }
   else
   {
      if(  subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) )
         if( subTrail == '/' )
            dSprintf(search, 1024, "%s%s", basePath,subPath );
         else
            dSprintf(search, 1024, "%s%s/", basePath,subPath );
      else
         dSprintf(search, 1024, "%s/", basePath );
   }


    //Open text file listing the subdirectories
    bool success = false;
    FileStream fileStream;

    {
        std::string dirPath("dirdump/");
        dirPath += std::string(search);
        dirPath += std::string("dirs.txt");

        success = fileStream.open( dirPath.c_str(), FileStream::Read);

        if(success == false)
        {
            //No subdirectories left to search.
            return true;
        }
    }

   //-----------------------------------------------------------------------------
   // add path to our return list ( provided it is valid )
   //-----------------------------------------------------------------------------
   if( !Platform::isExcludedDirectory( subPath ) )
   {

      if( noBasePath )
      {
         // We have a path and it's not an empty string or an excluded directory
         if( ( subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) ) )
            directoryVector.push_back( StringTable->insert( subPath ) );
      }
      else
      {
         if( ( subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) ) )
         {
            char szPath [ 1024 ];
            dMemset( szPath, 0, 1024 );
            if ( trail == '/' )
            {
                if ( subLead == '/' )
                   dSprintf( szPath, 1024, "%s%s", basePath, &subPath[1] );
                else
                   dSprintf( szPath, 1024, "%s%s", basePath, subPath );
            }
            else
            {
                if( subLead == '/' )
                   dSprintf( szPath, 1024, "%s%s", basePath, subPath );
                else
                   dSprintf( szPath, 1024, "%s/%s", basePath, subPath );
            }
            directoryVector.push_back( StringTable->insert( szPath ) );
         }
         else
            directoryVector.push_back( StringTable->insert( basePath ) );
      }
   }

    // Create a script buffer.
    const U32 size = fileStream.getStreamSize();
    char* pFileContents = new char[size + 1];
    // Read script.
    fileStream.read(size, pFileContents);
    fileStream.close();
    pFileContents[size] = 0;

    std::string dirList(pFileContents);
    delete pFileContents;
    pFileContents = NULL;
    size_t startPos = 0;
    size_t endPos;

    const char* lineEnding = "\r\n";
    const int lineEndingCharCount = 2;

    endPos = dirList.find(lineEnding, startPos);
    //Each line in the file
    while(endPos != std::string::npos)
    {
        std::string str = dirList.substr(startPos, endPos-startPos);
        const char* dirName = str.c_str();

        startPos = endPos+lineEndingCharCount;
        endPos = dirList.find(lineEnding, startPos);

         // skip . and .. directories
         if (dStrcmp((const char *)dirName, ".") == 0 || dStrcmp((const char *)dirName, "..") == 0)
            continue;

         // skip excluded directories
         if( Platform::isExcludedDirectory( (const char *)dirName ) )
            continue;

         if( ( subPath  && ( dStrncmp( subPath, "", 1 ) != 0 ) ))
         {
            char child[1024];

            if( subTrail == '/' )
               dSprintf(child, sizeof(child), "%s%s", subPath, dirName);
            else
               dSprintf(child, sizeof(child), "%s/%s", subPath, dirName);

            if( currentDepth < recurseDepth || recurseDepth == -1 )
               recurseDumpDirectories(basePath, child, directoryVector, currentDepth+1, recurseDepth, noBasePath );

         }
         else
         {
            char child[1024];

            if( trail == '/' )
               dStrcpy( child, (const char *)dirName );
            else
               dSprintf(child, sizeof(child), "/%s", dirName);

            if( currentDepth < recurseDepth || recurseDepth == -1 )
               recurseDumpDirectories(basePath, child, directoryVector, currentDepth+1, recurseDepth, noBasePath );
         }
    }

   return true;
}

bool Platform::dumpDirectories( const char *path, Vector<StringTableEntry> &directoryVector, S32 depth, bool noBasePath )
{
   ResourceManager->initExcludedDirectories();

   bool retVal =  recurseDumpDirectories( path, "", directoryVector, -1, depth, noBasePath );

   clearExcludedDirectories();

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

