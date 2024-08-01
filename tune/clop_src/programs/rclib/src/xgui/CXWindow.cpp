///////////////////////////////////////////////////////////////////////////
//
// CXWindow.cpp
//
// Remi Coulom
//
// April, 1999
//
///////////////////////////////////////////////////////////////////////////
#include "CXWindow.h"
#include "debug.h"

#include <cmath>

static int MaxValue(unsigned m)
{
 if (!m)
  return 0;
 while(!(m & 1))
  m >>= 1;
 return (int)m;
}

static int MultValue(unsigned m)
{
 if (!m)
  return 0;

 int Result = 1;
 while (!(m & 1))
 {
  m >>= 1;
  Result <<= 1;
 }

 return Result;
}

///////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////
CXWindow::CXWindow(int SizeX, int SizeY)
{
 display = XOpenDisplay(0);
 FATAL(!display);
 win = XCreateSimpleWindow(display,
                           DefaultRootWindow(display),
                           0,
                           0,
                           SizeX,
                           SizeY,
                           0, // Border width
                           0,
                           0);
 {
  XGCValues gcv;
  gcv.graphics_exposures = False;
  gc = XCreateGC(display, win, GCGraphicsExposures, &gcv);
 }
 screen = XDefaultScreen(display);
 pvis = XDefaultVisual(display, screen);
 map = XDefaultColormap(display,DefaultScreen(display));
 XColor exact;
 XAllocNamedColor(display, map, "white", &White, &exact);
 XAllocNamedColor(display, map, "black", &Black, &exact);
 XAllocNamedColor(display, map, "grey41", &DimGrey, &exact);
 XAllocNamedColor(display, map, "grey82", &LightGrey, &exact);

 if (pvis->red_mask && pvis->green_mask && pvis->blue_mask)
 {
  RMult = MultValue(pvis->red_mask);
  RMax = MaxValue(pvis->red_mask);
  GMult = MultValue(pvis->green_mask);
  GMax = MaxValue(pvis->green_mask);
  BMult = MultValue(pvis->blue_mask);
  BMax = MaxValue(pvis->blue_mask);
 }
 else 
  RMult = RMax = GMult = GMax = BMult = BMax = 0;

 wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
 XSetWMProtocols(display, win, &wm_delete_window, 1);

 XMapRaised(display, win);
}

///////////////////////////////////////////////////////////////////////////
// Predicate
///////////////////////////////////////////////////////////////////////////
Bool CXWindow::Predicate(Display *display, XEvent *event, XPointer arg)
{
 return event->xany.window = (Window)arg;
}

///////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////
CXWindow::~CXWindow()
{
 if (display)
 {
  XFreeGC(display, gc);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
 }
}
