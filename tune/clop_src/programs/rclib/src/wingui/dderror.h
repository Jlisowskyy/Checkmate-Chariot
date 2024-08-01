////////////////////////////////////////////////////////////////////////////
//
// dderror.h
//
// DDError
//
// Remi Coulom
//
// October, 1999
//
////////////////////////////////////////////////////////////////////////////
#ifndef DDERROR_H
#define DDERROR_H

#ifndef STRICT
#define STRICT
#endif
#include <windows.h>

void DDErrorFunction(HRESULT ddrval, const char *pszFile, int Line);
#define DDError(x) DDErrorFunction((x), __FILE__, __LINE__)

#endif
