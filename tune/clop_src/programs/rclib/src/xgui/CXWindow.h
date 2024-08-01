///////////////////////////////////////////////////////////////////////////
//
// CXWindow.h
//
// Remi Coulom
//
// April, 1999
//
///////////////////////////////////////////////////////////////////////////
#ifndef CXWindow_Declared
#define CXWindow_Declared

#include <X11/Xlib.h>
#include <X11/Xutil.h>

class CXWindow // xwin
{
 protected: /////////////////////////////////////////////////////////////////
  Display *display;
  Window win;
  int screen;
  GC gc;
  Visual *pvis;

  unsigned RMult, RMax;
  unsigned GMult, GMax;
  unsigned BMult, BMax;

  Colormap map;
  XColor White;
  XColor Black;
  XColor LightGrey;
  XColor DimGrey;

  Atom wm_delete_window;

  static Bool Predicate(Display *disp, XEvent *e, XPointer arg);

 public: ///////////////////////////////////////////////////////////////////
  CXWindow(int SizeX, int SizeY);
  virtual ~CXWindow();
};

#endif
