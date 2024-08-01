///////////////////////////////////////////////////////////////////////////
//                                                                       
// window.cpp
//
// CWindow
//
// Rémi Coulom
//
// August, 1998
//
///////////////////////////////////////////////////////////////////////////
#define STRICT
#include <windows.h>

#include "window.h"

////////////////////////////////////////////////////////////////////////////
// Get window visibility
////////////////////////////////////////////////////////////////////////////
int CWindow::GetVisibility() const
{
 return IsWindowVisible(GetHandle());
}

////////////////////////////////////////////////////////////////////////////
// Set window visibility
////////////////////////////////////////////////////////////////////////////
void CWindow::SetVisibility(int f)
{
 if (f)
  ShowWindow(GetHandle(), SW_SHOWNA | SW_SHOWNORMAL);
 else
 {
  SetFocus(GetParent(GetHandle()));
  ShowWindow(GetHandle(), SW_HIDE);
 }
}

////////////////////////////////////////////////////////////////////////////
// Function to get the x position of this window
////////////////////////////////////////////////////////////////////////////
int CWindow::GetX() const
{
 HWND hwnd = GetHandle();
 if (hwnd)
 {
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(wndpl);   
  GetWindowPlacement(hwnd, &wndpl);
  return wndpl.rcNormalPosition.left;
 }
 else
  return -1;
}

////////////////////////////////////////////////////////////////////////////
// Function to get the y position of this window
////////////////////////////////////////////////////////////////////////////
int CWindow::GetY() const
{
 HWND hwnd = GetHandle();
 if (hwnd)
 {
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(wndpl);   
  GetWindowPlacement(hwnd, &wndpl);
  return wndpl.rcNormalPosition.top;
 }
 else
  return -1;
}

////////////////////////////////////////////////////////////////////////////
// Function to set the x position of this window
////////////////////////////////////////////////////////////////////////////
void CWindow::SetX(int i)
{
 HWND hwnd = GetHandle();
 if (hwnd)
 {
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(wndpl);   
  GetWindowPlacement(hwnd, &wndpl);
  wndpl.rcNormalPosition.right += i - wndpl.rcNormalPosition.left;
  wndpl.rcNormalPosition.left = i;
  SetWindowPlacement(hwnd, &wndpl);
 }
}

////////////////////////////////////////////////////////////////////////////
// Function to set the y position of this window
////////////////////////////////////////////////////////////////////////////
void CWindow::SetY(int i)
{
 HWND hwnd = GetHandle();
 if (hwnd)
 {
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(wndpl);   
  GetWindowPlacement(hwnd, &wndpl);
  wndpl.rcNormalPosition.bottom += i - wndpl.rcNormalPosition.top;
  wndpl.rcNormalPosition.top = i;
  SetWindowPlacement(hwnd, &wndpl);
 }
}
