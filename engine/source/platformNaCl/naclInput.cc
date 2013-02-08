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
#include "platform/platformInput.h"
#include "platform/platformVideo.h"
#include "platform/event.h"
#include "console/console.h"

#include "platform/platformInput_ScriptBinding.h"

// Static class variables:
InputManager*  Input::smManager;
bool           Input::smActive;
CursorManager* Input::smCursorManager = 0; //*** DAW: Added
U8             Input::smModifierKeys; //*** DAW: Added
bool           Input::smLastKeyboardActivated;
bool           Input::smLastMouseActivated;
bool           Input::smLastJoystickActivated;

static void fillAsciiTable();

//------------------------------------------------------------------------------
//
// This function gets the standard ASCII code corresponding to our key code
// and the existing modifier key state.
//
//------------------------------------------------------------------------------
struct AsciiData
{
   struct KeyData
   {
      U16   ascii;
      bool  isDeadChar;
   };

   KeyData upper;
   KeyData lower;
   KeyData goofy;
};


#define NUM_KEYS ( KEY_OEM_102 + 1 )
#define KEY_FIRST KEY_ESCAPE
static AsciiData AsciiTable[NUM_KEYS];

//------------------------------------------------------------------------------
void Input::init()
{

}

//------------------------------------------------------------------------------
ConsoleFunction( isJoystickDetected, bool, 1, 1, "() Use the isJoystickDetected function to determine if one or more joysticks are connected to the system.\n"
																"This doesn't tell us how many joysticks there are, just that there are joysticks. It is our job to find out how many and to attach them.\n"
																"@return Returns true if one or more joysticks are attached and detected, false otherwise.\n"
																"@sa disableJoystick, enableJoystick, getJoystickAxes")
{
   U32 i;
   bool connected = false;
   argc; argv;
   PP_GamepadsSampleData sample;
   naclState.psGamepad->Sample(naclState.hModule, &sample);

   for(i=0; i < sample.length; ++i)
   {
       if(sample.items[i].connected)
       {
           connected = true;
           break;
       }
   }
   return( connected );
}

//------------------------------------------------------------------------------
ConsoleFunction( getJoystickAxes, const char*, 2, 2, "( instance ) Use the getJoystickAxes function to get the current axes position (x and y ) of any intance of a joystick.\n"
																"@param instance A non-negative number value selecting a specific joystick instance attached to this computer.\n"
																"@return Returns a string containing the \"x y\" position of the joystick.\n"
																"@sa disableJoystick, enableJoystick, isJoystickDetected")
{
   argc;
   return( "" );
}

//------------------------------------------------------------------------------
U16 Input::getKeyCode( U16 asciiCode )
{
   U16 keyCode = 0;
   U16 i;

   return( keyCode );
}

//------------------------------------------------------------------------------
U16 Input::getAscii( U16 keyCode, KEY_STATE keyState )
{
    return 0;
}

//------------------------------------------------------------------------------
void Input::destroy()
{
   if ( smManager && smManager->isEnabled() )
   {
      smManager->disable();
      delete smManager;
      smManager = NULL;
   }
}

//------------------------------------------------------------------------------
bool Input::enable()
{
   if ( smManager && !smManager->isEnabled() )
      return( smManager->enable() );

   return( false );
}

//------------------------------------------------------------------------------
void Input::disable()
{
   if ( smManager && smManager->isEnabled() )
      smManager->disable();
}

//------------------------------------------------------------------------------

void Input::activate()
{

}

//------------------------------------------------------------------------------
void Input::deactivate()
{

}

//------------------------------------------------------------------------------
void Input::reactivate()
{

}

//------------------------------------------------------------------------------
bool Input::isEnabled()
{
   return false;
}

//------------------------------------------------------------------------------
bool Input::isActive()
{
   return smActive;
}

//------------------------------------------------------------------------------
void Input::process()
{
}

//------------------------------------------------------------------------------
// Accesses the global input manager to see if its mouse is enabled
bool Input::isMouseEnabled()
{
    return false;
}

//------------------------------------------------------------------------------
// Accesses the global input manager to see if its keyboard is enabled
bool Input::isKeyboardEnabled()
{
    return false;
}

//------------------------------------------------------------------------------
// Access the global input manager and enables its mouse
void Input::enableMouse()
{
}

//------------------------------------------------------------------------------
// Access the global input manager and disables its mouse
void Input::disableMouse()
{
}

//------------------------------------------------------------------------------
// Access the global input manager and enables its keyboard
void Input::enableKeyboard()
{
}

//------------------------------------------------------------------------------
// Access the global input manager and enables its keyboard
void Input::disableKeyboard()
{
}

//------------------------------------------------------------------------------
bool Input::activateKeyboard()
{
   return( false );
}

//------------------------------------------------------------------------------
void Input::deactivateKeyboard()
{

}

//------------------------------------------------------------------------------
bool Input::enableJoystick()
{
}

//------------------------------------------------------------------------------
void Input::disableJoystick()
{
}

//------------------------------------------------------------------------------
void Input::echoInputState()
{

}

//------------------------------------------------------------------------------
void Input::setCursorPos(S32 x, S32 y)
{   

}

static void NaClMouseLocked(void* user_data, int32_t result)
{
}

//------------------------------------------------------------------------------
// Set the cursor to draw (true) or not (false)
void Input::setCursorState(bool on)
{
    if(on)
    {
        naclState.psMouseLock->UnlockMouse(naclState.hModule);
    }
    else
    {
        //Only works if the tab is in fullscreen mode
        PP_CompletionCallback cc = PP_MakeCompletionCallback(NaClMouseLocked, 0);
        naclState.psMouseLock->LockMouse(naclState.hModule, cc);
    }
}

//------------------------------------------------------------------------------


void Input::setCursorShape(U32 cursorID)
{

}

//------------------------------------------------------------------------------
// Functions to change the cursor shape using the Input class.
void Input::pushCursor(S32 cursorID)
{

}

void Input::popCursor()
{

}

void Input::refreshCursor()
{

}

//------------------------------------------------------------------------------

U32 Input::getDoubleClickTime()
{

}

S32 Input::getDoubleClickWidth()
{

}

S32 Input::getDoubleClickHeight()
{

}

//------------------------------------------------------------------------------
InputManager* Input::getManager()
{
   return( smManager );
}

U8 TranslateOSKeyCode(U8 vcode)
{
   return 0;
}

//-----------------------------------------------------------------------------
// Clipboard functions
const char* Platform::getClipboard()
{
    return "";
}

//-----------------------------------------------------------------------------
bool Platform::setClipboard(const char *text)
{
    return false;
}

