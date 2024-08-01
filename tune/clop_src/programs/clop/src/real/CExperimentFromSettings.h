/////////////////////////////////////////////////////////////////////////////
//
// CExperimentFromSettings.h
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CExperimentFromSettings_Declared
#define CExperimentFromSettings_Declared

#include "CParameterCollection.h"
#include "CResults.h"
#include "CPFQuadratic.h"
#include "CPFIndependentQuadratic.h"
#include "CRegression.h"
#include "CMERegressionMAPMax.h"
#include "CSPWeight.h"
#include "CRealProblem.h"
#include "CRealExperiment.h"
#include "CExperimentLogFile.h"
#include "CEigenSystem.h"

class CExperimentSettings;

class CExperimentFromSettings
{
 public: ////////////////////////////////////////////////////////////////////
  CParameterCollection paramcol;
  CResults results;
  CPFQuadratic pfq;
  CPFIndependentQuadratic pfiq;
  CRegression reg;
  CMERegressionMAPMax me;
  CSPWeight sp;
  CRealProblem problem;
  CRealExperiment rexp;
  CExperimentLogFile elf;
  CEigenSystem eigen;

 public: ////////////////////////////////////////////////////////////////////
  CExperimentFromSettings(const CExperimentSettings &es);
  CRealExperiment &GetExperiment() {return rexp;}

  void QuickLoad();
};

#endif // CExperimentFromSettings_Declared
