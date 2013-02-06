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

