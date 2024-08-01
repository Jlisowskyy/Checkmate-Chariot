/////////////////////////////////////////////////////////////////////////////
//
// CSPUCB.cpp
//
// Rémi Coulom
//
// May, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CSPUCB.h"
#include "CDFRatingLCB.h"

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Find the maximum of the Upper Confidence Bound
/////////////////////////////////////////////////////////////////////////////
const double *CSPUCB::NextSample(int i)
{
 //
 // UCB function
 //
 double x = std::sqrt(std::log(double(reg.GetSamples())));
 CDFRatingLCB dfrlcb(reg, alpha * x);

 //
 // Conjugate gradient from a random start
 //
 for (int j = int(vSample.size()); --j >= 0;)
  vSample[j] = -1.0 + 2.0 * rnd.NextDouble();
 dfrlcb.CG(&vSample[0]);

 return &vSample[0];
}
