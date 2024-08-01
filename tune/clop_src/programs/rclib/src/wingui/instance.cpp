#define STRICT
#include <windows.h>

#include "instance.h"

HINSTANCE GetInstance()
{
#ifdef NOMPROGRAMME
 return GetModuleHandle(NOMPROGRAMME);
#else
 return GetModuleHandle(0);
#endif
}

