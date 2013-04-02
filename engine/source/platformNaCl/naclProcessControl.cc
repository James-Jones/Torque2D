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

#include "platformNaCl/platformNaCl.h"
#include <ppapi\cpp\logging.h>
#include<cstring>

/* Fix link failure about missing kill function */
extern "C" {
    #define __MYPID 1

    int
    kill (int pid,
          int sig)
    {
      if(pid == __MYPID)
        _exit(sig);
      return 0;
    }
}

void Platform::postQuitMessage(const U32 in_quitVal)
{
   PP_DCHECK(in_quitVal);
}

void Platform::debugBreak()
{
    PP_NOTREACHED();
}

void Platform::forceShutdown(S32 returnValue)
{
   exit(returnValue);
}

/**
 * Creates new string PP_Var from C string. The resulting object will be a
 * refcounted string object. It will be AddRef()ed for the caller. When the
 * caller is done with it, it should be Release()d.
 * @param[in] str C string to be converted to PP_Var
 * @return PP_Var containing string.
 */
static struct PP_Var CStrToVar(const char* str) {
  if (naclState.psVarInterface != NULL) {
	return naclState.psVarInterface->VarFromUtf8(str, strlen(str));
  }
  return PP_MakeUndefined();
}

static void SendStringMainThreadCallback(void* user_data, int32_t result)
{
    std::string* str = (std::string*) (user_data);
    PP_Var naclString = CStrToVar(str->c_str());
	naclState.psConsole->Log(naclState.hModule, PP_LOGLEVEL_LOG, naclString);
    naclState.psVarInterface->Release(naclString);
    delete str;
}

void Platform::outputDebugString( const char *string )
{
    if(naclState.psCore->IsMainThread())
    {
        PP_Var naclString = CStrToVar(string);
        naclState.psConsole->Log(naclState.hModule, PP_LOGLEVEL_LOG, naclString);
        naclState.psVarInterface->Release(naclString);
    }
    else
    {
        std::string* msg = new std::string(string);

        PP_CompletionCallback callback = PP_MakeCompletionCallback(SendStringMainThreadCallback, msg);
        naclState.psCore->CallOnMainThread(0, callback, PP_OK);
    }
}
