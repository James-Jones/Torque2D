
#include "platformNaCl/platformNaCl.h"
#include "io/zip/zipArchive.h"
#include "io/memstream.h"
#include "console/console.h"

#include <string>
#include <vector>

#include "naclLocalFileSystem.h"

static void Wait()
{
    struct timespec timeToSleep;
    timeToSleep.tv_sec = 1;
    timeToSleep.tv_nsec = 1000;
    nanosleep(&timeToSleep, NULL);
}

class LocalRead
{
public:
    LocalRead(PP_Resource inFileIO, U32 offset, U32 inSize, char* inBuffer) : fileIO(inFileIO), sizeToDownload(inSize),
        bufferOffset(offset), bytesLeft(inSize), buffer(inBuffer+offset)
    {
    }

    ~LocalRead()
    {
    }

    bool done() const {
        if(bytesLeft == 0)
            return true;
        return false;
    }

    char* buffer;
    U32 bufferOffset;

    U32 sizeToDownload;
    U32 bytesLeft;

    PP_Resource fileIO;
};

void LocalReadCallback(void* data, int32_t bytes_read) {
    if (bytes_read < 0)
        return;  // error
    LocalRead* dload = static_cast<LocalRead*>(data);
    dload->bytesLeft -= bytes_read;
    
    if (dload->bytesLeft !=  0)
    {
        dload->bufferOffset += bytes_read;

        PP_CompletionCallback cb;
        cb.func = LocalReadCallback;
        cb.user_data = data;
        cb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

        naclState.psFileIO->Read(dload->fileIO,
                                dload->bufferOffset,
                                &dload->buffer[dload->bufferOffset],
                                dload->bytesLeft,
                                cb);
    }
}


class NaClLocalFileSystem;

void NaClLocalFile_FileOpenCallback(void*data, int32_t result);


NaClLocalFile::NaClLocalFile(
    PP_Resource fs,
    const char* path,
    const File::AccessMode openMode) : mFileOffset(0), mFileBody(NULL) {

    PP_Resource fileRef = naclState.psFileRef->Create(fs, path);
    
    mFile = naclState.psFileIO->Create(naclState.hModule);

    int32_t openFlags = 0;
    switch(openMode)
    {
    case File::Read:
        openFlags |= PP_FILEOPENFLAG_READ;
        break;
    case File::Write:
        openFlags |= PP_FILEOPENFLAG_WRITE;
        break;
    case File::ReadWrite:
        openFlags |= PP_FILEOPENFLAG_READ|PP_FILEOPENFLAG_WRITE;
        break;
    }

    PP_CompletionCallback fileOpenCallback;
    fileOpenCallback.func = NaClLocalFile_FileOpenCallback;
    fileOpenCallback.user_data = this;
    fileOpenCallback.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileIO->Open(mFile, fileRef, openFlags, fileOpenCallback);
}

NaClLocalFile::~NaClLocalFile()
{
    delete [] mFileBody;
}

void NaClLocalFile::Read(U32 size, char *dst, U32 *bytesRead)
{
    U32 totalSize = mFileInfo.size;

    if(!mFileBody)
    {
        mFileBody = new char[totalSize];
    }

    LocalRead lread(mFile, mFileOffset, size, mFileBody);

    PP_CompletionCallback cb;
    cb.func = LocalReadCallback;
    cb.user_data = &lread;
    cb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileIO->Read(lread.fileIO,
                            lread.bufferOffset,
                            &lread.buffer[lread.bufferOffset],
                            lread.bytesLeft,
                            cb);

    while(lread.done() == false)
    {
        Wait();
    }
}

void NaClLocalFile::setPosition(S32 position, bool absolutePos)
{
    if(absolutePos)
    {
        mFileOffset = position;
    }
    else
    {
        mFileOffset += position;
    }
}
U32 NaClLocalFile::getPosition() const {
    return mFileOffset;
}


void NaClLocalFile::ReadFile(U32 size, char *dst, U32 *bytesRead)
{
    while(!mReady)
    {
        Wait();
    }

    dMemcpy(dst, getContents(), size);
    *bytesRead = size;
}

void NaClLocalFile::CloseFile()
{
    naclState.psFileIO->Close(mFile);
}

PP_Resource NaClLocalFile::getFile() const {
    return mFile;
}
PP_FileInfo* NaClLocalFile::getFileInfo() {
    return &mFileInfo;
}
const char* NaClLocalFile::getContents() const {
    return mFileBody;
}

void NaClLocalFile_FileOpenCallback(void*data, int32_t result) {
    if (result != PP_OK) {
    return;
    }
    NaClLocalFile* file = static_cast<NaClLocalFile*>(data);

    file->mReady = true;

    PP_CompletionCallback queryCb;
    queryCb.func = NULL;
    queryCb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;
    queryCb.user_data = data;

    // Query the file in order to get the file size.
    naclState.psFileIO->Query(file->getFile(), file->getFileInfo(), queryCb);
}

void IngoreCallback(void* data, int32_t result);


NaClLocalFileSystem::NaClLocalFileSystem() : mFileSystemOpen(false), mFileSystem(0){}

void NaClLocalFileSystem::Open(int64_t sizeInBytes, PP_CompletionCallback_Func callback)
{
    mFileSystem = naclState.psFileSys->Create(naclState.hModule, PP_FILESYSTEMTYPE_LOCALPERSISTENT);

    PP_CompletionCallback fileSystemOpenCallback;
    fileSystemOpenCallback.func = callback;
    fileSystemOpenCallback.user_data = this;
    fileSystemOpenCallback.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    int32_t result = naclState.psFileSys->Open(mFileSystem, sizeInBytes, fileSystemOpenCallback);
    AssertFatal(result == PP_OK_COMPLETIONPENDING, "File system open failed");
}

void NaClLocalFileSystem::MakeDirectory(const char* path)
{
    PP_Resource fileRef = naclState.psFileRef->Create(mFileSystem, path);

    PP_CompletionCallback mkdirCallback;
    mkdirCallback.func = IngoreCallback;
    mkdirCallback.user_data = this;
    mkdirCallback.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileRef->MakeDirectory(fileRef, PP_TRUE, mkdirCallback);
}

void NaClLocalFileSystem::DeleteFile(const char* path)
{
    PP_Resource fileRef = naclState.psFileRef->Create(mFileSystem, path);

    PP_CompletionCallback cb;
    cb.func = IngoreCallback;
    cb.user_data = this;
    cb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileRef->Delete(fileRef, cb);
}

void NaClLocalFileSystem::RenameFile(const char* path, const char* newPath)
{
    PP_Resource fileRef = naclState.psFileRef->Create(mFileSystem, path);
    PP_Resource newFileRef = naclState.psFileRef->Create(mFileSystem, newPath);

    PP_CompletionCallback cb;
    cb.func = IngoreCallback;
    cb.user_data = this;
    cb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileRef->Rename(fileRef, newFileRef, cb);
}

NaClLocalFile* NaClLocalFileSystem::OpenFile(const char* path, const File::AccessMode openMode)
{
    return new NaClLocalFile(mFileSystem, path, openMode);
}


void IngoreCallback(void* data, int32_t result) {
}
