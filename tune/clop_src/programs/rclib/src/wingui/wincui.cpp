////////////////////////////////////////////////////////////////////////////
//
// wincui.cpp
//
// CWindowCUI
//
// Remi Coulom
//
// August, 1998
//
////////////////////////////////////////////////////////////////////////////
#define STRICT
#include <windows.h>

#include "wincui.h"
#include "window.h" // CWindow

using namespace std;

//
// Array of command strings
//
const char * const CWindowCUI::tszCommands[] =
{
 "?",
 "win",
 "xpos",
 "ypos",
 0
};

//
// Command identifiers
//
enum
{
 IDC_Help,
 IDC_Win,
 IDC_XPos,
 IDC_YPos
};

////////////////////////////////////////////////////////////////////////////
// Local prompt
////////////////////////////////////////////////////////////////////////////
void CWindowCUI::PrintLocalPrompt(ostream &out)
{
 out << "Window";
}

////////////////////////////////////////////////////////////////////////////
// Function to process commands
////////////////////////////////////////////////////////////////////////////
int CWindowCUI::ProcessCommand(const char *pszCommand,
                               const char *pszParameters,
                               istream &in,
                               ostream &out)
{
 switch (ArrayLookup(pszCommand, tszCommands))
 {
  case IDC_Help: ///////////////////////////////////////////////////////////
   CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
   out << "Window commands\n";
   out << "~~~~~~~~~~~~~~~\n";
   out << "win [on|off] ...... get[set] visibility\n";
   out << "xpos [n] .......... get[set] x position\n";
   out << "ypos [n] .......... get[set] y position\n";
   out << '\n';
  break;

  case IDC_Win: ////////////////////////////////////////////////////////////
  {
   int fVisible = win.GetVisibility();
   out << SetFlag(fVisible, pszParameters);
   win.SetVisibility(fVisible);
  }
  break;

  case IDC_XPos: ///////////////////////////////////////////////////////////
  {
   int xpos = win.GetX();
   if (*pszParameters)
   {
    xpos = (int)strtoul(pszParameters, 0, 10);
    win.SetX(xpos);
   }
   out << win.GetX() << '\n';
  }
  break;

  case IDC_YPos: ///////////////////////////////////////////////////////////
  {
   int ypos = win.GetY();
   if (*pszParameters)
   {
    ypos = (int)strtoul(pszParameters, 0, 10);
    win.SetY(ypos);
   }
   out << win.GetY() << '\n';
  }
  break;

  default: /////////////////////////////////////////////////////////////////
   return CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
 }

 return PC_Continue;
}
