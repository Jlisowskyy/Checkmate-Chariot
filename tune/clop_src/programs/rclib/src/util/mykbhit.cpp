////////////////////////////////////////////////////////////////////////////
//
// mykbhit.cpp
//
// Remi Coulom
//
// May 1997
//
////////////////////////////////////////////////////////////////////////////
#include <iostream>

using namespace std;

#include "mykbhit.h"

////////////////////////////////////////////////////////////////////////////
// Function to test if input buffers are not empty
////////////////////////////////////////////////////////////////////////////
int TestInputBuffer()
{
 streambuf *pin = cin.rdbuf();
 return (pin && pin->in_avail() > 0);
}

#if defined(KBHIT_CONIO) ///////////////////////////////////////////////////

#include <conio.h>

int mykbhit()
{
 return TestInputBuffer() || kbhit();
}

#elif defined(KBHIT_BIOS) //////////////////////////////////////////////////

#include <bios.h>

int mykbhit()
{
 return TestInputBuffer() || bioskey(1);
}

#elif defined(KBHIT_WIN32) /////////////////////////////////////////////////

#include <windows.h>

int mykbhit()
{
 if (TestInputBuffer())
  return 1;

 static int init = 0;
 static int pipe = 0;
 static HANDLE inh;

 if (!init)
 {
  init = 1;
  inh = GetStdHandle(STD_INPUT_HANDLE);
  DWORD dw;
  pipe = !GetConsoleMode(inh, &dw);
  if (!pipe)
  {
   SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
   FlushConsoleInputBuffer(inh);
  }
 }

 if (pipe)
 {
  DWORD dw;
  if (!PeekNamedPipe(inh, 0, 0, 0, &dw, 0))
   return 1;
  return dw;
 }
 else while(1)
 {
  DWORD dw;
  GetNumberOfConsoleInputEvents(inh, &dw);
  if (dw > 0)
  {
   INPUT_RECORD ir;
   DWORD dwRead;
   PeekConsoleInput(inh, &ir, 1, &dwRead);
   if (dwRead && ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown)
    return 1;
   else if (dwRead)
    ReadConsoleInput(inh, &ir, 1, &dwRead);
  }
  else
   return 0;
 }

 return 0;
}

#elif defined(KBHIT_UNIX) //////////////////////////////////////////////////

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int mykbhit()
{
 if (TestInputBuffer())
  return 1;

 fd_set readfds;
 FD_ZERO(&readfds);
 FD_SET(STDIN_FILENO, &readfds);

 struct timeval tv;
 tv.tv_sec=0;
 tv.tv_usec=0;

 select(16, &readfds, 0, 0, &tv);

 return !!FD_ISSET(STDIN_FILENO, &readfds);
}

#else //////////////////////////////////////////////////////////////////////

int mykbhit()
{
 return TestInputBuffer();
}

#endif
