////////////////////////////////////////////////////////////////////////////
//
// CBrushCache.h
//
// Rémi Coulom
//
// September, 2001
//
////////////////////////////////////////////////////////////////////////////
#ifndef CBrushCache_Declared
#define CBrushCache_Declared

#include <windows.h>

class CBrushCache // brc
{
 private: //////////////////////////////////////////////////////////////////
  enum {CacheSize = 10};

  HBRUSH thbr[CacheSize];
  COLORREF tclr[CacheSize];
  int Index;

  int Creations;

 public: ///////////////////////////////////////////////////////////////////
  CBrushCache();
  int GetCreations() const {return Creations;}
  void ResetCreations() {Creations = 0;}
  HBRUSH GetBrush(COLORREF clr);
  ~CBrushCache();
};

#endif
