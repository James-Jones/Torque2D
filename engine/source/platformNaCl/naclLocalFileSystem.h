#ifndef _NACLLOCALFILESYSTEM_H_
#define _NACLLOCALFILESYSTEM_H_

#include "platformNaCl/platformNaCl.h"
#include "platform/platformFileIO.h"
#include "platform/platformSemaphore.h"
#include <string>

class NaClLocalFile;

class ReadFileParams
{
public:
    U32 size;
    char *dst;
    NaClLocalFile* file;
    Semaphore _Waiter;

    ReadFileParams() : _Waiter(0), file(NULL), dst(NULL), size(0)
    {
    }
};

class WriteFileParams
{
public:
    U32 size;
    const char *src;
    NaClLocalFile* file;
    Semaphore _Waiter;

    WriteFileParams() : _Waiter(0), file(NULL), src(NULL), size(0),
        bufferOffset(0), bytesLeft(0), totalBytesWritten(0)
    {
    }

    U32 getTotalBytesWritten () const
    {
        return totalBytesWritten;
    }

private:
//restrict access to NaClLocalFile
friend class NaClLocalFile;
friend void LocalWriteCallback(void* data, int32_t bytes_written);
    U32 bufferOffset;
    U32 bytesLeft;
    U32 totalBytesWritten;
};

class OpenFileParams
{
public:

    OpenFileParams() : _Waiter(0), filename(""), openedFile(NULL)
    {
    }

    //const char* filename;

    std::string filename;
    File::AccessMode openMode;
    NaClLocalFile* openedFile;
    Semaphore _Waiter;
};

class NaClLocalFile
{
    friend class NaClLocalFileSystem;
    friend void NaClLocalFile_FileOpenCallback(void*data, int32_t result);
    friend void NaClLocalFile_FileQueryCallback(void*data, int32_t result);
private:
    NaClLocalFile(
        PP_Resource fs,
        OpenFileParams* params);

    //Data members
    PP_Resource mFile;
    PP_FileInfo mFileInfo;
    char* mFileBody;
    bool mReady;

    S32 mFileOffset;
    U32 mBytesRead;

public:
    //Semaphore _Waiter;

public:
    ~NaClLocalFile();

    void ReadFile(ReadFileParams* params);

    void WriteFile(WriteFileParams* params);

    void CloseFile();

    void setPosition(S32 position, bool absolutePos);

    U32 getPosition() const;

    PP_Resource getFile() const;
    const PP_FileInfo* getFileInfo() const;
    PP_FileInfo* getFileInfo();
    const char* getContents() const;
    const U32 getBytesRead() const;
};


class NaClLocalFileSystem
{
    friend void FileSystemOpenCallback(void* data, int32_t result);

public:
    NaClLocalFileSystem();

    void Open(const char* root, int64_t sizeInBytes, PP_CompletionCallback_Func callback);

    void MakeDirectory(const char* path);

    void DeleteFile(const char* path);

    void RenameFile(const char* path, const char* newPath);

    NaClLocalFile* OpenFile(OpenFileParams* params);

private:

    PP_Resource mFileSystem;

    std::string mRoot;
    
    bool mFileSystemOpen;
};

#endif //_NACLLOCALFILESYSTEM_H_
