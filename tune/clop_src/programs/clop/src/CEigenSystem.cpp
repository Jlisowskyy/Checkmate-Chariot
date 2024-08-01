/////////////////////////////////////////////////////////////////////////////
//
// CEigenSystem
//
// Rémi Coulom
//
// June, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CEigenSystem.h"
#include "CPFQuadratic.h"
#include "CRegression.h"
#include "CMatrixOperations.h"
#include "debug.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CEigenSystem::CEigenSystem(CResults &results,
                           CRegression &reg,
                           const CPFQuadratic &pfq):
 CObserver(results),
 reg(reg),
 pfq(pfq),
 vEigenValues(pfq.GetDimensions() * pfq.GetDimensions()),
 vEigenVectors(pfq.GetDimensions() * pfq.GetDimensions()),
 vEigenOrder(pfq.GetDimensions()),
 fError(&pfq != &reg.GetPF()),
 fValid(false)
{
 //FATAL(&pfq != &reg.GetPF());
}

/////////////////////////////////////////////////////////////////////////////
// Keep in sync with reg
/////////////////////////////////////////////////////////////////////////////
void CEigenSystem::Update()
{
 if (!fError && !fValid)
 {
  pfq.GetHessian(reg.MAP(), &vEigenValues[0]);

  CMatrixOperations::Jacobi(&vEigenValues[0],
                            &vEigenVectors[0],
                            pfq.GetDimensions());

  CMatrixOperations::JacobiSort(&vEigenValues[0],
                                &vEigenOrder[0],
                                pfq.GetDimensions());

  fValid = true;
 }
}
