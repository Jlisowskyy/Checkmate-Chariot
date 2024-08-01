////////////////////////////////////////////////////////////////////////////
//
// subwin.cpp
//
// CSubWindow class definition
//
// Rémi Coulom
//
// April, 1995
//
////////////////////////////////////////////////////////////////////////////
#define STRICT
#include <windows.h>
#include <commctrl.h>

#include "debug.h"
#include "subwin.h"
#include "instance.h"   // GetInstance()

//
// Name of the class for subwindows
//
static const char szClassName[] = "RCSubWindow";

////////////////////////////////////////////////////////////////////////////
// Access to the title of the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::GetTitleString(char *psz, int Size) const
{
 GetWindowText(hwndThis, psz, Size);
}

////////////////////////////////////////////////////////////////////////////
// Create Window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::Create(LPCSTR lpszTitre)
{
 //
 // Destroy old status bar if necessary
 //
 if (hwndStatus)
 {
  DestroyWindow(hwndStatus);
  hwndStatus = 0;
 }

 //
 // Create new window
 //
 hwndThis = CreateWindow(szClassName,
                         lpszTitre,
                         fTitle ? dwStyleWithTitle :
                                  dwStyleWithoutTitle,
                         0, 0, 100, 100,
                         hwndParent,
                         0,
                         GetInstance(),
                         0);
 FATAL(!hwndThis);

 //
 // Menus
 //
 {
  HMENU hmenuSystem;

  if (!!(hmenuSystem = GetSystemMenu(hwndThis, FALSE)))
  {
   AppendMenu(hmenuSystem, MF_SEPARATOR, 0, 0);
   AppendMenu(hmenuSystem, MF_ENABLED, SC_DEACTIVATE, "&Deactivate\tEsc");
   AppendMenu(hmenuSystem, MF_ENABLED, SC_HIDETITLE, "&Hide title\tT");
  }
 }
 if (fTitle)
  ::SetMenu(hwndThis, hmenu);

 //
 // 'this' is written in the window data so that the correct MessageHandler
 // can be called
 //
 SetWindowLong(hwndThis, 0, (long)this);

 //
 // Device Context
 //
 hdcWindow = GetDC(hwndThis);
 FATAL(!hdcWindow);

 //
 // Status bar
 //
 SetStatus(fStatus);
}

////////////////////////////////////////////////////////////////////////////
// Function to update the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::Update(short Param)
{
 UpdateParam = Param;
 RedrawWindow(hwndThis, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW); 
}

////////////////////////////////////////////////////////////////////////////
// Function to resize the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::Size(int x, int y, int cx, int cy)
{
 SetWindowPos(hwndThis, 0, x, y, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
}

////////////////////////////////////////////////////////////////////////////
// Function to set the title of the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::SetTitle(BOOL f)
{
 if (f != fTitle)
 {
  if (hwndThis)
  {
   WINDOWPLACEMENT wndpl;
   wndpl.length = sizeof(wndpl);   
   GetWindowPlacement(hwndThis, &wndpl);
//   int fActive = (GetActiveWindow() == hwndThis);

   char szTitle[100];
   GetWindowText(hwndThis, szTitle, sizeof(szTitle));

   HWND hwndOld = hwndThis;
   HDC hdcOld = hdcWindow;
   fTitle = f;
   Create(szTitle);
   FATAL(!hwndThis);

   SendMessage(hwndThis, WM_CREATESSFEN, 0, 0);
   wndpl.flags |= WPF_SETMINPOSITION;
//   if (!fVisible)
//    wndpl.showCmd = SW_HIDE;
//   else
//    wndpl.showCmd = fActive ? SW_SHOW : SW_SHOWNA;
   SetWindowPlacement(hwndThis, &wndpl);

   ReleaseDC(hwndOld, hdcOld);
   ::SetMenu(hwndOld, 0); // prevent menu destruction
   DestroyWindow(hwndOld);
  }
  else
   fTitle = f;
 }
}       

////////////////////////////////////////////////////////////////////////////
// Function to set the status bar of the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::SetStatus(BOOL f)
{
 fStatus = f;

 if (fStatus && !hwndStatus)
  hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "", hwndThis, 1);
 else if (!fStatus && hwndStatus)
 {
  DestroyWindow(hwndStatus);
  hwndStatus = 0;
 }
}

////////////////////////////////////////////////////////////////////////////
// Set status text
////////////////////////////////////////////////////////////////////////////
void CSubWindow::SetStatusText(const char *psz)
{
 SetWindowText(hwndStatus, psz);
}

////////////////////////////////////////////////////////////////////////////
// Get the height of the status bar
////////////////////////////////////////////////////////////////////////////
int CSubWindow::GetStatusHeight() const
{
 if (hwndStatus)
 {
  RECT rc;
  GetWindowRect(hwndStatus, &rc);
  return rc.bottom - rc.top;
 }
 else
  return 0;
}

////////////////////////////////////////////////////////////////////////////
// Function to set the visibility of the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::SetVisible(BOOL f)
{
 if (!!(fVisible = f))
 {
  if (hwndStatus)
   ShowWindow(hwndStatus, SW_SHOW);
  ShowWindow(hwndThis, SW_SHOWNA);
 }
 else
 {
  if (hwndParent)
   SetFocus(hwndParent);
  ShowWindow(hwndThis, SW_HIDE);
 }
}

////////////////////////////////////////////////////////////////////////////
// Message handler
////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK SubWindowMessage(HWND hwnd,
                                  UINT Message,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
 long lThis;

 if (!(lThis = GetWindowLong(hwnd, 0)))
  return DefWindowProc(hwnd, Message, wParam, lParam);
 else
  return((CSubWindow *)lThis)->MessageHandler(hwnd, Message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////
// Virtual function to handle messages
////////////////////////////////////////////////////////////////////////////
LRESULT CSubWindow::MessageHandler(HWND hwnd,
                                   UINT Message,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
 switch (Message)
 {
  case WM_SIZE: ////////////////////////////////////////////////////////////
   if (hwndStatus)
    SendMessage(hwndStatus, WM_SIZE, wParam, lParam);
  goto lblDefault;

  case WM_CLOSE: ///////////////////////////////////////////////////////////
   SetVisible(FALSE);
  break;

  case WM_SHOWWINDOW: //////////////////////////////////////////////////////
   if (hMenu)
    CheckMenuItem(hMenu, idItem, wParam ? MF_CHECKED : MF_UNCHECKED);
   if (wParam)
    ShowOwnedPopups(hwnd, TRUE);
  goto lblDefault;

  case WM_LBUTTONDOWN: /////////////////////////////////////////////////////
   if (!fTitle)
   {
    POINT p;

    p.x = (short)LOWORD(lParam);
    p.y = (short)HIWORD(lParam);

    ClientToScreen(hwnd, &p);

    SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELONG(p.x, p.y));
   }
   else goto lblDefault;
  break;

  case WM_PAINT: ///////////////////////////////////////////////////////////
  {
   RECT rc;

   if (GetUpdateRect(hwnd, &rc, FALSE))
   {
    PAINTSTRUCT ps;

    HDC hdc = BeginPaint(hwnd, &ps);
    HBRUSH hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

    FillRect(hdc, &rc, hbr);

    DeleteObject(hbr);
    EndPaint(hwnd, &ps);
   }
  }
  break;

  case WM_SYSCOMMAND: //////////////////////////////////////////////////////
   switch (wParam & 0xfff0)
   {
    case SC_DEACTIVATE: lblDeactivate:
     if (hwndParent)
      SetFocus(hwndParent);
    break;

    case SC_HIDETITLE: lblHideTitle:
     SetTitle(!fTitle);
    break;

    default: goto lblDefault;
   }
  break;

  case WM_KEYDOWN: /////////////////////////////////////////////////////////
   switch (wParam)
   {
    case VK_ESCAPE: goto lblDeactivate;
    case 'T': goto lblHideTitle;
    default: goto lblDefault;
   }

  case WM_LBUTTONDBLCLK: case WM_RBUTTONDBLCLK: ////////////////////////////
   goto lblHideTitle;

  default: lblDefault: /////////////////////////////////////////////////////
   return DefWindowProc(hwnd, Message, wParam, lParam);
 }

 return 0;
}

////////////////////////////////////////////////////////////////////////////
// Function to initialize the window
////////////////////////////////////////////////////////////////////////////
void CSubWindow::Initialize(HWND hwndParent,
                            LPCSTR lpszTitre,
                            HMENU hMenu,
                            UINT idItem,
                            DWORD dwStyle)
{
 //
 // Do nothing if the window has already been initialized
 //
 if (fState)
  return;

 CSubWindow::hwndParent = hwndParent;
 CSubWindow::hMenu = hMenu;
 CSubWindow::idItem = idItem;

 //
 // The class is registered if it has not been done before
 //
 WNDCLASS wc;

 if (!GetClassInfo(GetInstance(), szClassName, &wc))
 {
  wc.style = CS_BYTEALIGNCLIENT |
             CS_BYTEALIGNWINDOW |
             CS_PARENTDC |
             CS_DBLCLKS |
             CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = SubWindowMessage;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 4;
  wc.hInstance = GetInstance();
  wc.hIcon = LoadIcon(0, IDI_APPLICATION);
  wc.hCursor = LoadCursor (0, IDC_ARROW);
  wc.hbrBackground = 0;
  wc.lpszMenuName = 0;
  wc.lpszClassName = szClassName;

  if (!RegisterClass(&wc))
   return;
 }

 //
 // Creation of this window
 //
 dwStyleWithoutTitle = dwStyle | WS_POPUP | WS_BORDER | WS_THICKFRAME;
 dwStyleWithTitle = dwStyleWithoutTitle |
                    WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

 Create(lpszTitre);
 SendMessage(hwndThis, WM_CREATESSFEN, 0, 0);

 //
 // Flag indicating that the window was successfuly built
 //
 fState = 1;
}

////////////////////////////////////////////////////////////////////////////
// Default constructor
////////////////////////////////////////////////////////////////////////////
CSubWindow::CSubWindow() :
 hmenu(0),
 hwndStatus(0),
 fStatus(0),
 fState(0),
 fVisible(0),
 fTitle(1),
 hwndThis(0),
 hdcWindow(0),
 haccel(0)
{
}

////////////////////////////////////////////////////////////////////////////
// Message loop
////////////////////////////////////////////////////////////////////////////
void CSubWindow::MessageLoop()
{
 MSG msg;
 while (fVisible && GetMessage(&msg, 0, 0, 0) == TRUE)
 {
  if (haccel && TranslateAccelerator(hwndThis, haccel, &msg))
   continue;
  TranslateMessage(&msg);
  DispatchMessage(&msg);
 }
}

////////////////////////////////////////////////////////////////////////////
// Set menu
////////////////////////////////////////////////////////////////////////////
void CSubWindow::SetMenu(HMENU hmenuNew)
{
 hmenu = hmenuNew;
 ::SetMenu(hwndThis, hmenu);
}

////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
CSubWindow::~CSubWindow()
{
 if (hwndStatus)
  DestroyWindow(hwndStatus);
 if (hdcWindow)
  ReleaseDC(hwndThis, hdcWindow);
 if (hwndThis)
  DestroyWindow(hwndThis);
 if (haccel)
  DestroyAcceleratorTable(haccel);
}
