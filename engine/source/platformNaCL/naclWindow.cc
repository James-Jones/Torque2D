#include "platformNaCL/platformNaCL.h"
#include "console/console.h"

NaCLPlatState naclState;

void Platform::init()
{
   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "nacl" );
}

void Platform::shutdown()
{
}


void Platform::initWindow(const Point2I &initialSize, const char *name)
{
}

void Platform::setWindowTitle( const char* title )
{
   if( !title || !title[0] )
      return;
}

F32 Platform::getRandom()
{
    return 0.0f;
}

bool Platform::openWebBrowser( const char* webAddress )
{
    return false;
}

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_graphics_3d.h"

#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/ppp_input_event.h"

#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_input_event.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_view.h"

#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/c/ppp_graphics_3d.h"
#include "ppapi/c/ppb_url_request_info.h"
#include "ppapi/c/ppb_url_loader.h"

PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser)
{
    naclState.psMessagingInterface =
      (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));

    naclState.psVarInterface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));

    if(!glInitializePPAPI(get_browser))
    {
      return PP_ERROR_FAILED;
    }

    naclState.psGL = (PPB_OpenGLES2*)get_browser(PPB_OPENGLES2_INTERFACE);

    naclState.psG3D = (PPB_Graphics3D*)get_browser(PPB_GRAPHICS_3D_INTERFACE);

    naclState.psInstanceInterface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);

    naclState.psView = (PPB_View*)get_browser(PPB_VIEW_INTERFACE);

    naclState.psCore = (PPB_Core*)get_browser(PPB_CORE_INTERFACE);

    naclState.psVar = (PPB_Var*)get_browser(PPB_VAR_INTERFACE);

    naclState.psInputEventInterface = (PPB_InputEvent*)get_browser(PPB_INPUT_EVENT_INTERFACE);

    naclState.psKeyboard = (PPB_KeyboardInputEvent*)get_browser(PPB_KEYBOARD_INPUT_EVENT_INTERFACE);

    naclState.psURLRequest = (PPB_URLRequestInfo*) get_browser(PPB_URLREQUESTINFO_INTERFACE);

    naclState.psURLLoader = (PPB_URLLoader*) get_browser(PPB_URLLOADER_INTERFACE);

    naclState.psWebSocket = (PPB_WebSocket*) get_browser(PPB_WEBSOCKET_INTERFACE);

    return PP_OK;
}
PP_EXPORT void PPP_ShutdownModule()
{
}
