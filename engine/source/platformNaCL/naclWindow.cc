#include "platform/platform.h"
#include "console/console.h"

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
}
PP_EXPORT void PPP_ShutdownModule()
{
}
