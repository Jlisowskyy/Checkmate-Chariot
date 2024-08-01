/////////////////////////////////////////////////////////////////////////////
//
// CSPSA.cpp
//
// Rémi Coulom
//
// Based on that description:
// http://www.jhuapl.edu/spsa/Pages/MATLAB.htm
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#include "CSPSA.h"
#include "CResults.h"
#include "COutcome.h"
#include "debug.h"

#include <cmath>

const double CSPSA::tOutcomeValue[COutcome::Outcomes] = {0.0, 1.0, 0.5};

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CSPSA::CSPSA(CResults &results,
             double a,
             double A,
             double c,
             double alpha,
             double gamma):
 CObserver(results),
 Dimensions(results.GetDimensions()),
 a(a),
 A(A),
 c(c),
 alpha(alpha),
 gamma(gamma),
 vTheta(Dimensions),
 vDelta(Dimensions),
 vThetaPlusMinus(Dimensions)
{
 OnReset();
}

/////////////////////////////////////////////////////////////////////////////
// OnOutcome
/////////////////////////////////////////////////////////////////////////////
void CSPSA::OnOutcome(int i)
{
 FATAL(i != k);

 if ((k & 1) == 0)
  yPlus = tOutcomeValue[results.GetOutcome(i)];
 else
 {
  const double yMinus = tOutcomeValue[results.GetOutcome(i)];
  const double a_k = std::pow(a / (k + 1 + A), alpha);
  for (int i = Dimensions; --i >= 0;)
  {
   vTheta[i] += a_k * (yPlus - yMinus) / (2.0 * c_k * vDelta[i]);
   Bound(vTheta[i], 1.0 - c_k);
  }
 }

 k = k + 1;
}

/////////////////////////////////////////////////////////////////////////////
// OnReset
/////////////////////////////////////////////////////////////////////////////
void CSPSA::OnReset()
{
 k = 0;
 std::fill(vTheta.begin(), vTheta.end(), 0.0);
}

/////////////////////////////////////////////////////////////////////////////
// NextSample
/////////////////////////////////////////////////////////////////////////////
const double *CSPSA::NextSample(int i)
{
 //
 // Even k: generate random Delta, return ThetaPlus
 //
 if ((k & 1) == 0)
 {
  c_k = std::pow(c / (k + 1.0), gamma);
  for (int i = Dimensions; --i >= 0;)
  {
   vDelta[i] = (rnd.NewValue() & 1) ? 1.0 : -1.0;
   vThetaPlusMinus[i] = vTheta[i] + c_k * vDelta[i];
  }
 }

 //
 // Odd k: ThetaMinus
 //
 else
 {
  for (int i = Dimensions; --i >= 0;)
   vThetaPlusMinus[i] = vTheta[i] - c_k * vDelta[i];
 }

 //
 // Boundaries
 //
 for (int i = Dimensions; --i >= 0;)
  Bound(vThetaPlusMinus[i], 1.0);

 return &vThetaPlusMinus[0];
}

/////////////////////////////////////////////////////////////////////////////
// MaxParameter
/////////////////////////////////////////////////////////////////////////////
bool CSPSA::MaxParameter(double vMax[]) const
{
 std::copy(vTheta.begin(), vTheta.end(), vMax);
 return true;
}
