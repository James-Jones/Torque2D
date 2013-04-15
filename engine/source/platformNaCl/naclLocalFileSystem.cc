
#include "platformNaCl/platformNaCl.h"
#include "io/zip/zipArchive.h"
#include "io/memstream.h"
#include "console/console.h"
#include "math/mMathFn.h"

#include <string>
#include <vector>

#include "naclLocalFileSystem.h"

class LocalRead
{
public:
    LocalRead(Semaphore* sem, PP_Resource inFileIO, U32 offset, U32 inSize, char* inBuffer) : 
      semaphore(sem), fileIO(inFileIO), sizeToDownload(inSize),
        bufferOffset(offset), bytesLeft(inSize), buffer(inBuffer)
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

    LocalRead* dload = static_cast<LocalRead*>(data);

    if (bytes_read < 0)
    {
        dload->semaphore->release();
        return;  // error
    }

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
    OpenFileParams* params) : mFileOffset(0), mFileBody(NULL) {

    params->openedFile = this;

#ifdef TORQUE_DEBUG
    debugFilename = params->filename;
#endif

    mFileInfo.size = 0;
    mFileInfo.creation_time = 0;
    mFileInfo.last_access_time = 0;
    mFileInfo.last_modified_time = 0;
    mFileInfo.system_type = PP_FILESYSTEMTYPE_INVALID;

    PP_Resource fileRef = naclState.psFileRef->Create(fs, params->filename.c_str());
    
    mFile = naclState.psFileIO->Create(naclState.hModule);

    int32_t openFlags = 0;
    switch(params->openMode)
    {
    case File::Read:
        openFlags |= PP_FILEOPENFLAG_READ;
        break;
    case File::Write:
        openFlags |= PP_FILEOPENFLAG_WRITE|PP_FILEOPENFLAG_CREATE|PP_FILEOPENFLAG_TRUNCATE;
        break;
    case File::ReadWrite:
        openFlags |= PP_FILEOPENFLAG_READ|PP_FILEOPENFLAG_WRITE;
        break;
    case File::WriteAppend:
        openFlags |= PP_FILEOPENFLAG_WRITE;
        break;
    }

    PP_CompletionCallback fileOpenCallback;
    fileOpenCallback.func = NaClLocalFile_FileOpenCallback;
    fileOpenCallback.user_data = params;
    fileOpenCallback.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    int32_t result = naclState.psFileIO->Open(mFile, fileRef, openFlags, fileOpenCallback);

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
    AssertFatal(mOpened, "File not opened");

    U32 size = params->size;
    char *dst = params->dst;

    U32 totalSize = mFileInfo.size;

    if(!mFileBody)
    {
        mFileBody = new char[totalSize];
    }

    mBytesRead = getMin(size, totalSize-mFileOffset);

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

void LocalWriteCallback(void* data, int32_t bytes_written) {
    WriteFileParams* params = static_cast<WriteFileParams*>(data);

    if (bytes_written < 0)
    {
        params->_Waiter.release();
        return;  // error
    }

    params->bytesLeft -= bytes_written;
    params->totalBytesWritten += bytes_written;

    if(params->bytesLeft != 0)
    {
        params->bufferOffset += bytes_written;

        PP_CompletionCallback cb;
        cb = PP_MakeCompletionCallback(LocalWriteCallback, params);

        naclState.psFileIO->Write(params->file->getFile(), params->bufferOffset, params->src+params->bufferOffset, params->bytesLeft, cb);
    }
    else
    {
        params->_Waiter.release();
    }
}

void NaClLocalFile::WriteFile(WriteFileParams* params)
{
    params->bytesLeft = params->size;

    PP_CompletionCallback cb;
    cb.func = LocalWriteCallback;
    cb.user_data = params;
    cb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileIO->Write(mFile, mFileOffset, params->src, params->size, cb);
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
    OpenFileParams* params = static_cast<OpenFileParams*>(data);

    if (result == PP_OK) {
        params->openedFile->mOpened = true;

        if(params->openMode == File::WriteAppend)
        {
            params->openedFile->setPosition(params->openedFile->getFileInfo()->size, true);
        }
    }

    params->_Waiter.release();
}

void NaClLocalFile_FileOpenCallback(void*data, int32_t result) {
    OpenFileParams* params = static_cast<OpenFileParams*>(data);
    NaClLocalFile* file = params->openedFile;

    if (result != PP_OK) {
        params->_Waiter.release();
        return;
    }

    PP_CompletionCallback queryCb;
    queryCb.func = NaClLocalFile_FileQueryCallback;
    queryCb.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;
    queryCb.user_data = data;

    // Query the file in order to get the file size.
    S32 queryResult = naclState.psFileIO->Query(file->getFile(), file->getFileInfo(), queryCb);

    AssertFatal(queryResult == PP_OK_COMPLETIONPENDING, "Failed to submit file query");
}

void IngoreCallback(void* data, int32_t result);
void MakeDirectoryCallback(void* data, int32_t result);




NaClLocalFileSystem::NaClLocalFileSystem() : mFileSystemOpen(false), mFileSystem(0) {}

void NaClLocalFileSystem::Unpack()
{
    Zip::ZipArchive za;
    naclState.localFileSys.SetDirectory("/");
    if(za.openArchive("/zip/Torque2DNaClAssets.zip", Zip::ZipArchive::Read))
    {
        U32 count = za.numEntries();
        for(U32 fileIdx = 0; fileIdx < count; ++fileIdx)
        {
            std::string destFilename = "/main/";
            destFilename += za[fileIdx].mFilename;
            za.extractFile(za[fileIdx].mFilename, destFilename.c_str());
        }
    }
    za.closeArchive();

    //Will need to be run on main thread.
    //naclState.localFileSys.DeleteFile("/zip/Torque2DNaClAssets.zip");

    naclState.localFileSys.SetDirectory("/main/");
}


void NaClLocalFileSystem::Open(const char* root, int64_t sizeInBytes, PP_CompletionCallback_Func callback)
{
    mFileSystem = naclState.psFileSys->Create(naclState.hModule, PP_FILESYSTEMTYPE_LOCALPERSISTENT);

    PP_CompletionCallback fileSystemOpenCallback = PP_MakeCompletionCallback(callback, this);

    int32_t result = naclState.psFileSys->Open(mFileSystem, sizeInBytes, fileSystemOpenCallback);
    AssertFatal(result == PP_OK_COMPLETIONPENDING, "File system open failed");

    mRoot = std::string(root);
}

void NaClLocalFileSystem::MakeDirectory(MakeDirParams* params)
{
    PP_Resource fileRef = naclState.psFileRef->Create(mFileSystem, params->path);

    PP_CompletionCallback mkdirCallback;
    mkdirCallback.func = MakeDirectoryCallback;
    mkdirCallback.user_data = params;
    mkdirCallback.flags = PP_COMPLETIONCALLBACK_FLAG_NONE;

    naclState.psFileRef->MakeDirectory(fileRef, params->makeAncestors, mkdirCallback);
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

NaClLocalFile* NaClLocalFileSystem::OpenFile(OpenFileParams* params)
{
    params->filename = mRoot + params->filename;
    return new NaClLocalFile(mFileSystem, params);
}

void NaClLocalFileSystem::SetDirectory(const char* path)
{
    mRoot = path;
}

void IngoreCallback(void* data, int32_t result) {
}

void MakeDirectoryCallback(void* data, int32_t result) {
    MakeDirParams* params = static_cast<MakeDirParams*>(data);

    params->_Waiter.release();
}
