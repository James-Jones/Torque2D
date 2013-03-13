#ifndef _NACLLOCALFILESYSTEM_H_
#define _NACLLOCALFILESYSTEM_H_

#include "platformNaCl/platformNaCl.h"
#include "platform/platformFileIO.h"

class NaClLocalFile
{
    friend class NaClLocalFileSystem;
    friend void NaClLocalFile_FileOpenCallback(void*data, int32_t result);
private:
    NaClLocalFile(
        PP_Resource fs,
        const char* path,
        const File::AccessMode openMode);

    void Read(U32 size, char *dst, U32 *bytesRead);

    //Data members
    PP_Resource mFile;
    PP_FileInfo mFileInfo;
    char* mFileBody;
    bool mReady;

    S32 mFileOffset;

public:
    ~NaClLocalFile();

    void ReadFile(U32 size, char *dst, U32 *bytesRead);

    void CloseFile();

    void setPosition(S32 position, bool absolutePos);

    U32 getPosition() const;

    PP_Resource getFile() const;
    PP_FileInfo* getFileInfo();
    const char* getContents() const;
};


class NaClLocalFileSystem
{
    friend void FileSystemOpenCallback(void* data, int32_t result);

public:
    NaClLocalFileSystem();

    void Open(int64_t sizeInBytes, PP_CompletionCallback_Func callback);

    void MakeDirectory(const char* path);

    void DeleteFile(const char* path);

    void RenameFile(const char* path, const char* newPath);

    NaClLocalFile* OpenFile(const char* path, const File::AccessMode openMode);

private:

    PP_Resource mFileSystem;
    
    bool mFileSystemOpen;
};

#endif //_NACLLOCALFILESYSTEM_H_
