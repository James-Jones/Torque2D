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

#include "platform/threads/mutex.h"
#include "platformNaCl/platformNaCl.h"
#include "memory/safeDelete.h"
#include <pthread.h>

//////////////////////////////////////////////////////////////////////////
// Mutex Data
//////////////////////////////////////////////////////////////////////////

struct PlatformMutexData
{
   pthread_mutex_t mMutex;

   PlatformMutexData()
   {
      mMutex = PTHREAD_MUTEX_INITIALIZER;
   }

   bool ValidMutex() const
   {
       return mMutex.mutex_handle != NC_INVALID_HANDLE;
   }
};

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

Mutex::Mutex()
{
   mData = new PlatformMutexData;

   pthread_mutex_init(&mData->mMutex, NULL);
}

Mutex::~Mutex()
{
   if(mData && mData->ValidMutex())
      pthread_mutex_destroy(&mData->mMutex);
   
   SAFE_DELETE(mData);
}

//////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////

bool Mutex::lock(bool block /* = true */)
{
   if(mData == NULL || !mData->ValidMutex())
      return false;

   if(block)
   {
        return (bool)pthread_mutex_lock(&mData->mMutex) == 0;
   }

   return (bool)pthread_mutex_trylock(&mData->mMutex) == 0;
}

void Mutex::unlock()
{
   if(mData == NULL || !mData->ValidMutex())
      return;

    pthread_mutex_unlock(&mData->mMutex);
}
