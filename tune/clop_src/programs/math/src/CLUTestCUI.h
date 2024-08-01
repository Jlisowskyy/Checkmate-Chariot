////////////////////////////////////////////////////////////////////////////
//
// CLUTestCUI.h
//
// Remi Coulom
//
// December, 2000
//
////////////////////////////////////////////////////////////////////////////
#ifndef Math_CLUTestCUI_Declared
#define Math_CLUTestCUI_Declared

#include "consolui.h"   // CConsoleUI

class CLUTestCUI : public CConsoleUI // lutestcui
{
 private: //////////////////////////////////////////////////////////////////
  static const char * const tszCommands[];

  int n;
  double *pdMatrix;
  int *pIndex;
  double *pdVector;
  double *pdResult;

  void SetSize(int NewN);
 
 protected: ////////////////////////////////////////////////////////////////
  virtual int ProcessCommand(const char *pszCommand,
                             const char *pszParameters,
                             std::istream &in,
                             std::ostream &out);

  virtual void PrintLocalPrompt(std::ostream &out);

 public: ///////////////////////////////////////////////////////////////////
  CLUTestCUI(CConsoleUI *pcui = 0, int openmodeInit = OpenModal) :
   CConsoleUI(pcui, openmodeInit),
   n(0)
  {}

  ~CLUTestCUI() {SetSize(0);}
};

#endif
