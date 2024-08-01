/////////////////////////////////////////////////////////////////////////////
//
// C2SPSA.cpp
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#include "C2SPSA.h"
#include "CResults.h"
#include "COutcome.h"
#include "debug.h"

#include <cmath>

const double C2SPSA::tOutcomeValue[COutcome::Outcomes] = {0.0, 1.0, 0.5};

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
C2SPSA::C2SPSA(CResults &results,
               double a,
               double A,
               double c,
               double alpha,
               double gamma,
               double d,
               double w):
 CObserver(results),
 Dimensions(results.GetDimensions()),
 a(a),
 A(A),
 c(c),
 alpha(alpha),
 gamma(gamma),
 d(d),
 w(w),
 vTheta(Dimensions),
 vDelta(Dimensions),
 vDeltaTilda(Dimensions),
 vSample(Dimensions),
 vGHat(Dimensions),
 vDeltaGHat(Dimensions),
 vHHat(Dimensions * Dimensions),
 vHBar(Dimensions * Dimensions),
 vHBarBar(Dimensions * Dimensions)
{
 OnReset();
}

/////////////////////////////////////////////////////////////////////////////
// OnOutcome
/////////////////////////////////////////////////////////////////////////////
void C2SPSA::OnOutcome(int i)
{
 FATAL(i != ExpectedIndex);

 ty[i & 3] = tOutcomeValue[results.GetOutcome(i)]; 

 if ((i & 3) == 3)
 {
  for (int i = Dimensions; --i >= 0;)
  {
   vGHat[i] = (ty[yplus] - ty[yminus]) / (2.0 * c_k * vDelta[i]);
   vDeltaGHat[i] = ((ty[yplustilda]-ty[yplus])-(ty[yminustilda]-ty[yminus])) /
                   (ctilda_k * vDeltaTilda[i]);
  }

  for (int i = Dimensions; --i >= 0;)
   for (int j = Dimensions; --j >= 0;)
    vHHat[i * Dimensions + j] = 
     0.5 * (vDeltaGHat[i] / (2.0 * c_k * vDelta[j]) +
            vDeltaGHat[j] / (2.0 * c_k * vDelta[i]));

  const double w_k = std::pow(w / (k + 1), d);
  for (int i = Dimensions * Dimensions; --i >= 0;)
   vHBar[i] = ((1.0 - w_k) * vHBar[i] + w_k * vHHat[i]);

  const double a_k = std::pow(a / (k + A), alpha);

  for (int j = Dimensions; --j >= 0;)
   Bound(vTheta[j], 1.0 - c_k - ctilda_k);

  k = k + 1.0;
 }
}

/////////////////////////////////////////////////////////////////////////////
// OnReset
/////////////////////////////////////////////////////////////////////////////
void C2SPSA::OnReset()
{
 k = 0.0;
 std::fill(vTheta.begin(), vTheta.end(), 0.0);
 std::fill(vHBar.begin(), vHBar.end(), 0.0);
}

/////////////////////////////////////////////////////////////////////////////
// NextSample
/////////////////////////////////////////////////////////////////////////////
const double *C2SPSA::NextSample(int i)
{
 ExpectedIndex = i;

 //
 // Every 4: generate random Delta, DeltaTilda
 //
 if ((i & 3) == 0)
 {
  c_k = std::pow(c / k, gamma);
  ctilda_k = c_k;

  for (int j = Dimensions; --j >= 0;)
  {
   vDelta[j] = (rnd.NewValue() & 1) ? 1.0 : -1.0;
   vDeltaTilda[j] = (rnd.NewValue() & 1) ? 1.0 : -1.0;
  }
 }

 //
 // 4 samples
 //
 switch(i & 3)
 {
  case yplus:
   for (int j = Dimensions; --j >= 0;)
    vSample[j] = vTheta[j] + c_k * vDelta[j];

  case yminus:
   for (int j = Dimensions; --j >= 0;)
    vSample[j] = vTheta[j] - c_k * vDelta[j];

  case yplustilda:
   for (int j = Dimensions; --j >= 0;)
    vSample[j] = vTheta[j] + c_k * vDelta[j] + ctilda_k * vDeltaTilda[j];

  case yminustilda:
   for (int j = Dimensions; --j >= 0;)
    vSample[j] = vTheta[j] - c_k * vDelta[j] + ctilda_k * vDeltaTilda[j];
 }

 return &vSample[0];
}

/////////////////////////////////////////////////////////////////////////////
// MaxParameter
/////////////////////////////////////////////////////////////////////////////
bool C2SPSA::MaxParameter(double vMax[]) const
{
 std::copy(vTheta.begin(), vTheta.end(), vMax);
 return true;
}
