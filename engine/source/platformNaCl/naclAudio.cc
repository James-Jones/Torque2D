#include "platformNaCl/platformNaCl.h"

namespace Audio
{

void OpenALDLLShutdown()
{
}

bool OpenALDLLInit()
{
   OpenALDLLShutdown();
    return(true);
}

} // end namespace Audio
