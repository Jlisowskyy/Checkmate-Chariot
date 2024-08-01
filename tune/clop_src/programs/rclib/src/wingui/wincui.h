////////////////////////////////////////////////////////////////////////////
//
// wincui.h
//
// CWindowCUI
//
// Remi Coulom
//
// August, 1998
//
////////////////////////////////////////////////////////////////////////////
#ifndef WINCUI_H
#define WINCUI_H

#include "consolui.h"   // CConsoleUI

class CWindow;

class CWindowCUI : public CConsoleUI // swcui
{
 private: //////////////////////////////////////////////////////////////////
  static const char * const tszCommands[];
  CWindow &win;

 protected: ////////////////////////////////////////////////////////////////
  virtual int ProcessCommand(const char *pszCommand,
                             const char *pszParameters,
                             std::istream &in,
                             std::ostream &out);

  virtual void PrintLocalPrompt(std::ostream &out);

 public: ///////////////////////////////////////////////////////////////////
  CWindowCUI(CWindow &winInit,
             CConsoleUI *pcui = 0,
             int openmode = OpenModal) :
   CConsoleUI(pcui, openmode),
   win(winInit)
  {}
};

#endif
