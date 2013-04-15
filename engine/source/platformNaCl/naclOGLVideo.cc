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

#include "platformNaCl/platformGL.h"
#include "platformNaCl/platformNaCl.h"
#include "platformNaCl/naclOGLVideo.h"
#include "console/console.h"
#include "math/mPoint.h"
#include "platform/event.h"
#include "game/gameInterface.h"
#include "console/consoleInternal.h"
#include "console/ast.h"
#include "io/fileStream.h"

//------------------------------------------------------------------------------
OpenGLDevice::OpenGLDevice()
{
   initDevice();
}


//------------------------------------------------------------------------------
void OpenGLDevice::initDevice()
{
   // Set the device name:
   mDeviceName = "OpenGL";

   // Let Parent enumerate valid resolutions.
   //Parent::initDevice();
}


//------------------------------------------------------------------------------
bool OpenGLDevice::activate( U32 width, U32 height, U32 bpp, bool fullScreen )
{
   Con::printf( "Activating the OpenGL display device..." );

   return true;
}


//------------------------------------------------------------------------------
void OpenGLDevice::shutdown()
{
   Con::printf( "Shutting down the OpenGL display device..." );

}


//------------------------------------------------------------------------------
// This is the real workhorse function of the DisplayDevice...
//
bool OpenGLDevice::setScreenMode( U32 width, U32 height, U32 bpp, bool fullScreen, bool forceIt, bool repaint )
{
   return true;
}


//------------------------------------------------------------------------------
void OpenGLDevice::swapBuffers()
{

}


//------------------------------------------------------------------------------
const char* OpenGLDevice::getDriverInfo()
{
   // Output some driver info to the console:
   const char* vendorString   = "Dummy Vendor";//(const char*) glGetString( GL_VENDOR );
   const char* rendererString = "Dummy Renderer";//(const char*) glGetString( GL_RENDERER );
   const char* versionString  = "Dummy Version";//(const char*) glGetString( GL_VERSION );
   const char* extensionsString = "Dummy Extensions";//(const char*) glGetString( GL_EXTENSIONS );

   U32 bufferLen = ( vendorString ? dStrlen( vendorString ) : 0 )
                 + ( rendererString ? dStrlen( rendererString ) : 0 )
                 + ( versionString  ? dStrlen( versionString ) : 0 )
                 + ( extensionsString ? dStrlen( extensionsString ) : 0 )
                 + 4;

   char* returnString = Con::getReturnBuffer( bufferLen );
   dSprintf( returnString, bufferLen, "%s\t%s\t%s\t%s",
         ( vendorString ? vendorString : "" ),
         ( rendererString ? rendererString : "" ),
         ( versionString ? versionString : "" ),
         ( extensionsString ? extensionsString : "" ) );

   return( returnString );
}


//------------------------------------------------------------------------------
bool OpenGLDevice::getGammaCorrection(F32 &g)
{
   return true;
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setGammaCorrection(F32 g)
{
    return true;
}

//------------------------------------------------------------------------------
bool OpenGLDevice::setVerticalSync( bool on )
{
    return true;
}

//------------------------------------------------------------------------------
DisplayDevice* OpenGLDevice::create()
{
    OpenGLDevice* newOGLDevice = new OpenGLDevice();

    return (DisplayDevice*) newOGLDevice;
}
