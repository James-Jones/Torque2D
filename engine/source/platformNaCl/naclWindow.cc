#include "platformNaCl/platformNaCl.h"
#include "platformNaCl/platformGL.h"
#include "platform/platformVideo.h"
#include "console/console.h"
#include "game/gameInterface.h"
#include "math/mPoint.h"
#include "al/alc.h"
#include <string>

#include "io/zip/zipArchive.h"

NaClPlatState naclState;

void Platform::init()
{
   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "nacl" );

   //Unpack Zip archive of game assets.
   NaClLocalFileSystem::Unpack();
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

void Platform::restartInstance()
{
}

void Platform::AlertOK(const char *windowTitle, const char *message)
{
}

bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
    return false;
}

bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
    return false;
}

bool Platform::AlertYesNo(const char *windowTitle, const char *message)
{
    return false;
}

void Platform::enableKeyboardTranslation(void)
{
}

void Platform::disableKeyboardTranslation(void)
{
}

void Platform::setMouseLock(bool locked)
{
}

void Platform::minimizeWindow()
{
}

void Platform::restoreWindow()
{
}

static Point2I windowSize;

const Point2I &Platform::getWindowSize()
{
   return windowSize;
}

void Platform::setWindowSize( U32 newWidth, U32 newHeight )
{
   windowSize.set( newWidth, newHeight );
}

Resolution Video::getDesktopResolution()
{
   Resolution  Result;
   PP_Size size;

   naclState.psFullscreen->GetScreenSize(naclState.hModule, &size);

   // Retrieve Resolution Information.
   Result.bpp  = 32;
   Result.w    = size.width;
   Result.h    = size.height;

   // Return Result;
   return Result;
}

void Platform::process()
{
}

void TimeManager::process()
{
}

S32 Platform::messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons, MBIcons icon)
{
}

NaClPlatState::NaClPlatState() : currentTime(0)
{
}

void SwapBuffersCallback(void* user_data, int32_t result)
{
    Semaphore* sem = static_cast<Semaphore*>(user_data);
    sem->release();
}

//Simple test
void TestRendering(void* user_data, int32_t result)
{
    naclState.psGL->ClearColor(naclState.hRenderContext, 1, 0, 1, 1);
    naclState.psGL->Clear(naclState.hRenderContext, GL_COLOR_BUFFER_BIT);
}

void SetContextZero(void* user_data, int32_t result)
{
    glSetCurrentContextPPAPI(0);
}

void SetContextMain(void* user_data, int32_t result)
{
    glSetCurrentContextPPAPI(naclState.hRenderContext);
}

void SwapBuffers(void* user_data, int32_t result)
{
    PP_CompletionCallback cc = PP_MakeCompletionCallback(SwapBuffersCallback, user_data);
    naclState.psG3D->SwapBuffers(naclState.hRenderContext, cc);
}

void SetupFullscreenMode(void* user_data, int32_t result)
{
    bool fullScreen = Con::getBoolVariable( "$pref::Video::fullScreen" );
    naclState.psFullscreen->SetFullscreen(naclState.hModule, fullScreen ? PP_TRUE : PP_FALSE);
}

void LogicThread(void* data)
{
    bool initOK;
    const char* argv[] = {"Torque2D"};

    AssertFatal(naclState.psCore->IsMainThread() == PP_FALSE, "Running on the wrong thread");

    initOK = Game->mainInitialize(1, argv);

    if(initOK)
    {
        PP_CompletionCallback callback = PP_MakeCompletionCallback(SetupFullscreenMode, 0);
        naclState.psCore->CallOnMainThread(0, callback, PP_OK);

        Semaphore swapBuffersWaiter;

        while(Game->isRunning())
        {
            //glSetCurrentContextPPAPI(naclState.hRenderContext)
            PP_CompletionCallback callback = PP_MakeCompletionCallback(SetContextMain, 0);
            naclState.psCore->CallOnMainThread(0, callback, PP_OK);

    #if 0
            Game->mainLoop();
    #else
            //Simple test
            callback = PP_MakeCompletionCallback(TestRendering, 0);
            naclState.psCore->CallOnMainThread(0, callback, PP_OK);
    #endif

            //glSetCurrentContextPPAPI(0)
            callback = PP_MakeCompletionCallback(SetContextZero, 0);
            naclState.psCore->CallOnMainThread(0, callback, PP_OK);

            //SwapBuffers
            swapBuffersWaiter.acquire();//Wait for any pending SwapBuffers operation to finish.
            callback = PP_MakeCompletionCallback(SwapBuffers, &swapBuffersWaiter);
            naclState.psCore->CallOnMainThread(0, callback, PP_OK);
        }
    }
    else
    {
        Platform::outputDebugString("fatal:mainInitialize() returned false");
    }
}

void StartLogicThread(void* user_data, int32_t result)
{
    if(naclState.psCore->IsMainThread())
    {
        naclState.logicThread = new Thread(LogicThread);
    }
}

void NaClInit(void* user_data, int32_t result)
{
    //Open the file system asynchronously.
    //Once completed, call ScheduleInitGame.
    naclState.localFileSys.Open("/", 128*1024*1024, StartLogicThread);
}

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[])
{
    naclState.hRenderContext = 0;
    naclState.hModule = instance;
    naclState.psInputEventInterface->RequestInputEvents(instance, PP_INPUTEVENT_CLASS_KEYBOARD | PP_INPUTEVENT_CLASS_MOUSE);
    return PP_TRUE;
}

static void Instance_DidDestroy(PP_Instance instance) {
    delete naclState.logicThread;
    Game->mainShutdown();
    naclState.psCore->ReleaseResource(naclState.hRenderContext);
    naclState.hRenderContext = 0;
    naclState.hModule = 0;
}
static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) {
	struct PP_Rect pos;

	naclState.psView->GetRect(view_resource, &pos);

	if (pos.size.width == 0 || pos.size.height == 0)
	{
		return;
	}

	naclState.i32PluginWidth = pos.size.width;
	naclState.i32PluginHeight = pos.size.height;

    Platform::setWindowSize( naclState.i32PluginWidth, naclState.i32PluginHeight );

    if(naclState.hRenderContext == 0)
    {
	    int32_t attribs[] = {
	    PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
	    PP_GRAPHICS3DATTRIB_BLUE_SIZE, 8,
	    PP_GRAPHICS3DATTRIB_GREEN_SIZE, 8,
	    PP_GRAPHICS3DATTRIB_RED_SIZE, 8,
	    PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 16,
	    PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 0,
	    PP_GRAPHICS3DATTRIB_SAMPLES, 4,
	    PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 1,
	    PP_GRAPHICS3DATTRIB_WIDTH, naclState.i32PluginWidth,
	    PP_GRAPHICS3DATTRIB_HEIGHT, naclState.i32PluginHeight,
	    PP_GRAPHICS3DATTRIB_NONE,
	    };

	    naclState.hRenderContext = naclState.psG3D->Create(naclState.hModule, NULL, attribs);
	    naclState.psInstanceInterface->BindGraphics(naclState.hModule, naclState.hRenderContext);

        PP_CompletionCallback cc = PP_MakeCompletionCallback(NaClInit, 0);
        naclState.psCore->CallOnMainThread(0, cc, PP_OK);
    }
    else
    {
        naclState.psG3D->ResizeBuffers(naclState.hRenderContext,
            naclState.i32PluginWidth,
            naclState.i32PluginHeight);

    }
}

static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}

PP_Bool InputEvent_HandleEvent(PP_Instance instance_id, PP_Resource input_event)
{
    uint32_t ui32KeyCode;

    PP_InputEvent_Type eInputEventType = 
        naclState.psInputEventInterface->GetType(input_event);

    if(eInputEventType == PP_INPUTEVENT_TYPE_KEYDOWN)
    {
        ui32KeyCode = naclState.psKeyboard->GetKeyCode(input_event);
    }
    else
    if(eInputEventType == PP_INPUTEVENT_TYPE_KEYUP)
    {
        ui32KeyCode = naclState.psKeyboard->GetKeyCode(input_event);
    }

    return PP_TRUE;
}

void Messaging_HandleMessage(PP_Instance instance, struct PP_Var message)
{
    uint32_t ui32MessageLength = 0;
    const char* pszMessage = 0;
    
    switch(message.type)
    {
        case PP_VARTYPE_STRING:
        {
            pszMessage = naclState.psVar->VarToUtf8(message, &ui32MessageLength);
            break;
        }
        case PP_VARTYPE_NULL:
        case PP_VARTYPE_BOOL:
        case PP_VARTYPE_INT32:
        case PP_VARTYPE_DOUBLE:
        case PP_VARTYPE_OBJECT:
        case PP_VARTYPE_ARRAY:
        case PP_VARTYPE_DICTIONARY:
        case PP_VARTYPE_ARRAY_BUFFER:
        case PP_VARTYPE_UNDEFINED:
        default:
        {
            break;
        }
    }
}

PP_EXPORT const void* PPP_GetInterface(const char* interface_name)
{
    static PPP_Instance instance_interface = 
    {
        &Instance_DidCreate,
        &Instance_DidDestroy,
        &Instance_DidChangeView,
        &Instance_DidChangeFocus,
        &Instance_HandleDocumentLoad
    };

    static PPP_InputEvent input_event_interface = 
    {
        &InputEvent_HandleEvent
    };

    static PPP_Messaging messaging_interface =
    {
        &Messaging_HandleMessage
    };

    if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0)
    {
        return &instance_interface;
    }

    if (strcmp(interface_name, PPP_INPUT_EVENT_INTERFACE) == 0)
    {
        return &input_event_interface;
    }

    if(strcmp(interface_name, PPP_MESSAGING_INTERFACE) == 0)
    {
        return &messaging_interface;
    }

    return NULL;
}

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

    alSetPpapiInfo(a_module_id, get_browser);

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

    naclState.psGamepad = (PPB_Gamepad*) get_browser(PPB_GAMEPAD_INTERFACE);

    naclState.psFullscreen = (PPB_Fullscreen*) get_browser(PPB_FULLSCREEN_INTERFACE);

    naclState.psMouseLock = (PPB_MouseLock*) get_browser(PPB_MOUSELOCK_INTERFACE);

    naclState.psFileIO = (PPB_FileIO*) get_browser(PPB_FILEIO_INTERFACE);

    naclState.psFileRef = (PPB_FileRef*) get_browser(PPB_FILEREF_INTERFACE);

    naclState.psFileSys = (PPB_FileSystem*) get_browser(PPB_FILESYSTEM_INTERFACE);

    naclState.psURLResponseInfo = (PPB_URLResponseInfo*) get_browser(PPB_URLRESPONSEINFO_INTERFACE);

    naclState.psConsole = (PPB_Console*) get_browser(PPB_CONSOLE_INTERFACE);

    naclState.psMessageLoop = (PPB_MessageLoop*) get_browser(PPB_MESSAGELOOP_INTERFACE);

    return PP_OK;
}
PP_EXPORT void PPP_ShutdownModule()
{
    glTerminatePPAPI();
}
