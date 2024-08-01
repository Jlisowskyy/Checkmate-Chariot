/////////////////////////////////////////////////////////////////////////////
//
// real.cpp
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CExperimentSettings.h"
#include "CLinearParameter.h"
#include "CIntegerParameter.h"
#include "CGammaParameter.h"
#include "CIntegerGammaParameter.h"

#include <iostream>
#include <sstream>
#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Constructor: parse parameters from input stream
/////////////////////////////////////////////////////////////////////////////
CExperimentSettings::CExperimentSettings(std::istream &is):
 Replications(1),
 DrawRating(0.0),
 fStopOnError(true),
 H(3.0),
 fIndependent(false)
{
 std::string sLine;
 while (std::getline(is, sLine))
 {
  std::istringstream ss(sLine);
  std::string sFirst;
  ss >> sFirst;

  if (sFirst == "LinearParameter")
  {
   std::string s;
   double Min = 0.0;
   double Max = 0.0;
   ss >> s >> Min >> Max;
   lparam.push_back(new CLinearParameter(s, Min, Max));
  }
  else if (sFirst == "IntegerParameter")
  {
   std::string s;
   int Min = 0;
   int Max = 0;
   ss >> s >> Min >> Max;
   lparam.push_back(new CIntegerParameter(s, Min, Max));
  }
  else if (sFirst == "GammaParameter")
  {
   std::string s;
   double Min = 0;
   double Max = 0;
   ss >> s >> Min >> Max;
   lparam.push_back(new CGammaParameter(s, Min, Max));
  }
  else if (sFirst == "IntegerGammaParameter")
  {
   std::string s;
   int Min = 0;
   int Max = 0;
   ss >> s >> Min >> Max;
   lparam.push_back(new CIntegerGammaParameter(s, Min, Max));
  }
  else if (sFirst == "Name")
   ss >> sName;
  else if (sFirst == "Processor")
  {
   std::string s;
   ss >> s;
   lsProcessors.push_back(s);
  }
  else if (sFirst == "Script")
  {
   ss >> sScript;
   std::string s;
   while (ss >> s)
    lsScriptParam.push_back(s);
  }
  else if (sFirst == "Replications")
   ss >> Replications;
  else if (sFirst == "DrawElo")
  {
   double DrawElo;
   ss >> DrawElo;
   DrawRating = DrawElo * std::log(10.0) / 400.0;
  }
  else if (sFirst == "StopOnError")
  {
   std::string s;
   ss >> s;
   if (s == "true")
    fStopOnError = true;
   else if (s == "false")
    fStopOnError = false;
  }
  else if (sFirst == "H")
   ss >> H;
  else if (sFirst == "Correlations")
  {
   std::string s;
   ss >> s;
   if (s == "all")
    fIndependent = false;
   else if (s == "none")
    fIndependent = true;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
CExperimentSettings::~CExperimentSettings()
{
 for (std::list<CParameter *>::const_iterator i = lparam.begin();
      i != lparam.end();
      ++i)
  delete *i;
}
