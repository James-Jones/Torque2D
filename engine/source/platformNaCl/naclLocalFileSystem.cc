
#include "platformNaCl/platformNaCl.h"
#include "io/zip/zipArchive.h"
#include "io/memstream.h"
#include "console/console.h"
#include "math/mMathFn.h";

#include <string>
#include <vector>

#include "naclLocalFileSystem.h"

class LocalRead
{
public:
    LocalRead(Semaphore* sem, PP_Resource inFileIO, U32 offset, U32 inSize, char* inBuffer) : 
      semaphore(sem), fileIO(inFileIO), sizeToDownload(inSize),
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

    Semaphore* semaphore;

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
    else
    {
        dload->semaphore->release();
    }
}


class NaClLocalFileSystem;

void NaClLocalFile_FileOpenCallback(void*data, int32_t result);


NaClLocalFile::NaClLocalFile(
    PP_Resource fs,
    const char* path,
    const File::AccessMode openMode) : mFileOffset(0), mFileBody(NULL) {

    mFileInfo.size = 0;

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

    int32_t result = naclState.psFileIO->Open(mFile, fileRef, openFlags, fileOpenCallback);
    _Waiter.acquire();

    AssertFatal(result == PP_OK_COMPLETIONPENDING, "File open failed");
}

NaClLocalFile::~NaClLocalFile()
{
    delete [] mFileBody;
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

void NaClLocalFile::ReadFile(ReadFileParams* params)
{
    AssertFatal(mReady, "File not opened");

    U32 size = params->size;
    char *dst = params->dst;

    U32 totalSize = mFileInfo.size;

    if(!mFileBody)
    {
        mFileBody = new char[totalSize];
    }

    mBytesRead = getMin(size-mFileOffset, totalSize-mFileOffset);

    LocalRead lread(&params->_Waiter, mFile, mFileOffset, mBytesRead, mFileBody);

    PP_CompletionCallback cb;
    cb.func = LocalReadCallback;
    cb.user_data = &lread;
    cb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileIO->Read(lread.fileIO,
                            lread.bufferOffset,
                            &lread.buffer[lread.bufferOffset],
                            lread.bytesLeft,
                            cb);
}

void NaClLocalFile::CloseFile()
{
    naclState.psFileIO->Close(mFile);
}

PP_Resource NaClLocalFile::getFile() const {
    return mFile;
}
const PP_FileInfo* NaClLocalFile::getFileInfo() const {
    return &mFileInfo;
}
PP_FileInfo* NaClLocalFile::getFileInfo() {
    return &mFileInfo;
}
const char* NaClLocalFile::getContents() const {
    return mFileBody;
}
const U32 NaClLocalFile::getBytesRead() const {
    return mBytesRead;
}

void NaClLocalFile_FileQueryCallback(void*data, int32_t result) {
    NaClLocalFile* file = static_cast<NaClLocalFile*>(data);
    file->_Waiter.release();
}

void NaClLocalFile_FileOpenCallback(void*data, int32_t result) {
    NaClLocalFile* file = static_cast<NaClLocalFile*>(data);

    if (result != PP_OK) {
        file->_Waiter.release();
        return;
    }

    file->mReady = true;

    PP_CompletionCallback queryCb;
    queryCb.func = NaClLocalFile_FileQueryCallback;
    queryCb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;
    queryCb.user_data = data;

    // Query the file in order to get the file size.
    S32 queryResult = naclState.psFileIO->Query(file->getFile(), file->getFileInfo(), queryCb);

    AssertFatal(queryResult == PP_OK_COMPLETIONPENDING, "Failed to submit file query");
}

void IngoreCallback(void* data, int32_t result);


NaClLocalFileSystem::NaClLocalFileSystem() : mFileSystemOpen(false), mFileSystem(0){}

void NaClLocalFileSystem::Open(const char* root, int64_t sizeInBytes, PP_CompletionCallback_Func callback)
{
    mFileSystem = naclState.psFileSys->Create(naclState.hModule, PP_FILESYSTEMTYPE_LOCALPERSISTENT);

    PP_CompletionCallback fileSystemOpenCallback = PP_MakeCompletionCallback(callback, this);

    int32_t result = naclState.psFileSys->Open(mFileSystem, sizeInBytes, fileSystemOpenCallback);
    AssertFatal(result == PP_OK_COMPLETIONPENDING, "File system open failed");

    mRoot = std::string(root);
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
    return new NaClLocalFile(mFileSystem, (mRoot+path).c_str(), openMode);
}


void IngoreCallback(void* data, int32_t result) {
}
