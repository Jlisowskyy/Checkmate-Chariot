////////////////////////////////////////////////////////////////////////////
//
// CLUTestCUI.cpp
//
// Remi Coulom
//
// December, 2000
//
////////////////////////////////////////////////////////////////////////////
#include "CLUTestCUI.h"
#include "CLUDecomposition.h"

#include <sstream>

////////////////////////////////////////////////////////////////////////////
// Command strings
////////////////////////////////////////////////////////////////////////////
const char * const CLUTestCUI::tszCommands[] =
{
 "?",
 "size",
 "read",
 "write",
 "decompose",
 "solve",
 "solvet",
 0
};

//
// Command identifiers
//
enum
{
 IDC_Help,
 IDC_Size,
 IDC_Read,
 IDC_Write,
 IDC_Decompose,
 IDC_Solve,
 IDC_SolveT
};

////////////////////////////////////////////////////////////////////////////
// Local prompt
////////////////////////////////////////////////////////////////////////////
void CLUTestCUI::PrintLocalPrompt(std::ostream &out)
{
 out << "LUTest";
}

////////////////////////////////////////////////////////////////////////////
// Set matrix size
////////////////////////////////////////////////////////////////////////////
void CLUTestCUI::SetSize(int NewN)
{
 if (NewN != n)
 {
  if (n)
  {
   delete[] pdMatrix;
   delete[] pIndex;
   delete[] pdVector;
   delete[] pdResult;
  }
  n = NewN;
  if (n)
  {
   pdMatrix = new double[n * n];
   pIndex = new int[n];
   pdVector = new double[n];
   pdResult = new double[n];
  }
 }
}

////////////////////////////////////////////////////////////////////////////
// Function to process commands
////////////////////////////////////////////////////////////////////////////
int CLUTestCUI::ProcessCommand(const char *pszCommand,
                               const char *pszParameters,
                               std::istream &in,
                               std::ostream &out)
{
 switch (ArrayLookup(pszCommand, tszCommands))
 {
  case IDC_Help: ///////////////////////////////////////////////////////////
   CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
   out << "LU Test commands\n";
   out << "~~~~~~~~~~~~~~~~\n";
   out << "size [n] ........ get[Set] matrix size\n";
   out << "read ............ read in matrix coefficients\n";
   out << "write ........... write matrix\n";
   out << "decompose ....... decompose matrix\n";
   out << "solve ........... read in vector and solve\n";
   out << "solvet .......... read in vector and solve transposed equations\n";
   out << '\n';
  break;

  case IDC_Size: ///////////////////////////////////////////////////////////
  {
   int NewN = n;
   std::istringstream(pszParameters) >> NewN;
   SetSize(NewN);
   out << n << '\n';
  }
  break;

  case IDC_Read: ///////////////////////////////////////////////////////////
   for (int i = 0; i < n * n; i++)
    in >> pdMatrix[i];
  break;

  case IDC_Write: //////////////////////////////////////////////////////////
   for (int i = 0; i < n; i++)
   {
    for (int j = 0; j < n; j++)
     out << pdMatrix[i * n + j] << ' ';
    out << '\n';
   }
   out << '\n';
   for (int i = 0; i < n; i++)
    out << pIndex[i] << ' ';
   out << '\n';
  break;

  case IDC_Decompose: //////////////////////////////////////////////////////
  {
   CLUDecomposition lud(n);
   lud.Decompose(pdMatrix, pIndex);
  }
  break;

  case IDC_Solve: //////////////////////////////////////////////////////////
  {
   for (int i = 0; i < n; i++)
    in >> pdVector[i];
   CLUDecomposition lud(n);
   lud.Solve(pdMatrix, pIndex, pdVector, pdResult);
   for  (int i = 0; i < n; i++)
    out << pdResult[i] << ' ';
   out << '\n';
  }
  break;

  case IDC_SolveT: /////////////////////////////////////////////////////////
  {
   for (int i = 0; i < n; i++)
    in >> pdVector[i];
   CLUDecomposition lud(n);
   lud.SolveTranspose(pdMatrix, pIndex, pdVector, pdResult);
   for  (int i = 0; i < n; i++)
    out << pdResult[i] << ' ';
   out << '\n';
  }
  break;

  default: /////////////////////////////////////////////////////////////////
   return CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
 }

 return PC_Continue;
}
