///////////////////////////////////////////////////////////////////////////
//
// subwin.h
//
// CSubWindow class declaration
//
// Rémi Coulom
//
// April, 1995
//
///////////////////////////////////////////////////////////////////////////
#ifndef SUBWIN_H
#define SUBWIN_H

#ifndef STRICT
#define STRICT
#endif
#include <windows.h>

#include "window.h"     // CWindow

LRESULT CALLBACK SubWindowMessage(HWND hwnd,
                                  UINT Message,
                                  WPARAM wParam,
                                  LPARAM lParam);

class CSubWindow : public CWindow // sw
{
 friend LRESULT CALLBACK SubWindowMessage(HWND hwnd,
                                          UINT Message,
                                          WPARAM wParam,
                                          LPARAM lParam);

 private: /////////////////////////////////////////////////////////////////
  HMENU hmenu;
  HWND hwndStatus;
  int fStatus;
  int fState;
  int fVisible;
  int fTitle;
  DWORD dwStyleWithoutTitle;
  DWORD dwStyleWithTitle;
  HMENU hMenu;
  UINT idItem;

  void Create(LPCSTR lpszTitre);

 protected: ///////////////////////////////////////////////////////////////
  HWND hwndParent; // ???
  HWND hwndThis; // ???
  HDC hdcWindow; // ???
  short UpdateParam; // ??? ???
  HACCEL haccel; // ???

  int GetStatusHeight() const;
  void SetStatusText(const char *psz);

  virtual LRESULT MessageHandler(HWND hwnd,
                                 UINT Message,
                                 WPARAM wParam,
                                 LPARAM lParam);

 public: //////////////////////////////////////////////////////////////////
  CSubWindow();
  virtual ~CSubWindow();

  //
  // CWindow overrides
  //
  virtual HWND GetHandle() const {return hwndThis;}
  virtual int GetVisibility() const {return GetVisible();}
  virtual void SetVisibility(int f) {SetVisible(f);}

  //
  // Other functions
  //
  void Initialize(HWND hwndParent,
                  LPCSTR lpszTitre,
                  HMENU hMenu,
                  UINT idItem,
                  DWORD dwStyle);
  void Update(short Param = 0);
  void Size(int x, int y, int cx, int cy);

  void SetVisible(BOOL f);
  void SetTitle(BOOL f);
  void SetStatus(BOOL f);
  void SetMenu(HMENU hmenuNew);

  int GetStatus() const {return fStatus;}
  int GetTitle() const {return fTitle;}
  int GetVisible() const {return fVisible;}
  int GetState() const {return fState;}
  void GetTitleString(char *psz, int Size) const;

  void MessageLoop();
};

// USER Messages
#define WM_CREATESSFEN WM_USER
#define WM_SUBWIN_USER (WM_USER + 1)

// Custom system commands
#define SC_DEACTIVATE 0xfff0
#define SC_HIDETITLE 0xffe0


#endif
