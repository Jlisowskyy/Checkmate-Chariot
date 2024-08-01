/////////////////////////////////////////////////////////////////////////////
//
// CEigenSystem
//
// Rémi Coulom
//
// June, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CEigenSystem_Declared
#define CEigenSystem_Declared

#include <vector>

#include "CObserver.h"

class CResults;
class CRegression;
class CPFQuadratic;

class CEigenSystem: private CObserver // eigen
{
 private: ///////////////////////////////////////////////////////////////////
  CRegression &reg;
  const CPFQuadratic &pfq;

  std::vector<double> vEigenValues;
  std::vector<double> vEigenVectors;
  std::vector<int> vEigenOrder;

  bool fError;
  bool fValid;

 public: ////////////////////////////////////////////////////////////////////
  CEigenSystem(CResults &results,
               CRegression &reg,
               const CPFQuadratic &pfq);

  //
  // Overrides of CObserver
  //
  void OnReset() {fValid = false;}
  void OnOutcome(int i) {fValid = false;}
  void OnRefresh() {fValid = false;}

  //
  // Cached computation
  //
  void Update();

  //
  // Access to vectors. Update() should have been called before.
  //
  const double *GetValues() {return &vEigenValues[0];}
  const double *GetVectors() {return &vEigenVectors[0];}
  const int *GetOrder() {return &vEigenOrder[0];}
};

#endif
