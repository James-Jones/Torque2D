#ifndef _PLATFORMNACL_H_
#define _PLATFORMNACL_H_

#include "platform/platform.h"

#include <string.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_graphics_3d.h"

#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/ppp_input_event.h"
#include "ppapi/c/ppp_graphics_3d.h"

#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_input_event.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_view.h"
#include "ppapi/c/ppb_websocket.h"
#include "ppapi/c/ppb_gamepad.h"
#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/c/ppb_url_request_info.h"
#include "ppapi/c/ppb_url_loader.h"
#include "ppapi/c/ppb_fullscreen.h"

#include "ppapi/gles2/gl2ext_ppapi.h"

struct NaClPlatState
{
    S32 i32PluginWidth;
    S32 i32PluginHeight;

    //Handles
    PP_Resource hRenderContext;
    PP_Instance hModule;
    PP_Resource hNetworkConnetion;

    //Interfaces to pepper systems.
    PPB_Messaging* psMessagingInterface;
    PPB_Var* psVarInterface;
    PPB_OpenGLES2* psGL;
    PPB_Graphics3D* psG3D;
    PPB_Instance* psInstanceInterface;
    PPB_View* psView;
    PPB_Core* psCore;
    PPB_Var* psVar;
    PPB_InputEvent* psInputEventInterface;
    PPB_KeyboardInputEvent* psKeyboard;
    PPB_URLRequestInfo* psURLRequest;
    PPB_URLLoader* psURLLoader;
    PPB_WebSocket* psWebSocket;
    PPB_Gamepad* psGamepad;
    PPB_Fullscreen* psFullscreen;

    U32 currentTime;

    NaClPlatState();
};

extern NaClPlatState naclState;

#endif
