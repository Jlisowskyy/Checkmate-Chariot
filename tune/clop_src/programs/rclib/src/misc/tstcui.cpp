////////////////////////////////////////////////////////////////////////////
//
// tstcui.cpp
//
// Test of the CConsoleUI class
//
// Remi Coulom
//
// june 1996
//
////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "consolui.h"

////////////////////////////////////////////////////////////////////////////
// a simple console class definition
// (should be in a separate file, but I am too lazy)
////////////////////////////////////////////////////////////////////////////
class CTestCUI : public CConsoleUI // testcui
{
 private: //////////////////////////////////////////////////////////////////
  static const char * const tszCommands[];

 protected: ////////////////////////////////////////////////////////////////
  virtual int ProcessCommand(const char *pszCommand,
                             const char *pszParameters,
                             istream &in,
                             ostream &out);

  virtual void PrintLocalPrompt(ostream &out) {out << "test";};

 public: ///////////////////////////////////////////////////////////////////
  CTestCUI(CConsoleUI *pcuiParent = 0, int openmode = OpenModal) :
   CConsoleUI(pcuiParent, openmode)
  {
   cerr << "test CUI created !\n";
  };
  
  ~CTestCUI(void)
  {
   cerr << "test CUI deleted !\n";
  }
};

////////////////////////////////////////////////////////////////////////////
// List of commands
////////////////////////////////////////////////////////////////////////////
const char * const      CTestCUI::tszCommands[]=
{
 "?",
 "hello",
 "addchild",
 0
};

enum
{
 IDC_Help,
 IDC_Hello,
 IDC_AddChild
};

////////////////////////////////////////////////////////////////////////////
// Function to process commands
////////////////////////////////////////////////////////////////////////////
int CTestCUI::ProcessCommand(const char *pszCommand,
                             const char *pszParameters,
                             istream &in,
                             ostream &out)
{
 switch(ArrayLookup(pszCommand, tszCommands))
 {
  case IDC_Help: ///////////////////////////////////////////////////////////
   CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
   out << "Test Commands summary\n";
   out << "~~~~~~~~~~~~~~~~~~~~~\n";
   out << "hello .... Say hello\n";
   out << "addchild . Add a child\n";
   out << '\n';
  break;

  case IDC_Hello: //////////////////////////////////////////////////////////
   out << "Hello ! How are you today ?\n";
  break;

  case IDC_AddChild: ///////////////////////////////////////////////////////
   AddChild(new CTestCUI(this, OpenModeless), in, out);
  break;

  default: /////////////////////////////////////////////////////////////////
   return CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
 }

 return PC_Continue;
};


////////////////////////////////////////////////////////////////////////////
// main function
////////////////////////////////////////////////////////////////////////////
int main(void)
{
 CTestCUI test;

 test.MainLoop(cin, cout);

 return 0;
}
