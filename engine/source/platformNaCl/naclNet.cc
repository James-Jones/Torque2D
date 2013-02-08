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

#include "platformNaCL/platformNaCL.h"
#include "platform/event.h"
#include "console/console.h"
#include "game/gameInterface.h"
#include "io/fileStream.h"

#include <stdlib.h>

bool Net::init()
{
   naclState.hNetworkConnetion = naclState.psWebSocket->Create(naclState.hModule);

   if(naclState.psWebSocket->IsWebSocket(naclState.hNetworkConnetion) == PP_FALSE)
   {
      return(false);
   }
   return(true);
}

void Net::shutdown()
{
}

NetSocket Net::openListenPort(U16 port)
{
   return InvalidSocket;
}

NetSocket Net::openConnectTo(const char *addressString)
{
    return InvalidSocket;
}

void Net::closeConnectTo(NetSocket sock)
{
}

Net::Error Net::sendtoSocket(NetSocket socket, const U8 *buffer, int bufferSize)
{
}

void Net::closePort()
{
}

Net::Error Net::sendto(const NetAddress *address, const U8 *buffer, S32 bufferSize)
{
    return NoError;
}

void Net::process()
{
}
                 
NetSocket Net::openSocket()
{
    return NoError;
}

Net::Error Net::closeSocket(NetSocket socket)
{
    return NoError;
}

Net::Error Net::connect(NetSocket socket, const NetAddress *address)
{
    return NoError;
}

Net::Error Net::listen(NetSocket socket, S32 backlog)
{
    return NoError;
}

NetSocket Net::accept(NetSocket acceptSocket, NetAddress *remoteAddress)
{
   return InvalidSocket;
}

Net::Error Net::bind(NetSocket socket, U16 port)
{
    return NoError;
}

Net::Error Net::setBufferSize(NetSocket socket, S32 bufferSize)
{
    return NoError;
}

Net::Error Net::setBroadcast(NetSocket socket, bool broadcast)
{
    return NoError;  
}

Net::Error Net::setBlocking(NetSocket socket, bool blockingIO)
{
    return NoError; 
}

Net::Error Net::send(NetSocket socket, const U8 *buffer, S32 bufferSize)
{
    return NoError;
}

Net::Error Net::recv(NetSocket socket, U8 *buffer, S32 bufferSize, S32 *bytesRead)
{
    return NoError;
}

bool Net::compareAddresses(const NetAddress *a1, const NetAddress *a2)
{
   return true;
}

bool Net::stringToAddress(const char *addressString, NetAddress *address)
{
    return false;
}

void Net::addressToString(const NetAddress *address, char addressString[256])
{
}

Net::Error getLastError()
{
   return Net::UnknownError;
}


