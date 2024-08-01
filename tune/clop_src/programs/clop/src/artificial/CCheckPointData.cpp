/////////////////////////////////////////////////////////////////////////////
//
// CCheckPointData
//
// Rémi Coulom
//
// November, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CCheckPointData.h"
#include "CCPListener.h"

#include <algorithm>
#include <cmath>
#include <boost/thread/thread.hpp>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CCheckPointData::CCheckPointData(int TotalRepeats,
                                 int Samples,
                                 CCPListener *pcpl):
 Samples(Samples),
 Repeats(0),
 vError(TotalRepeats),
 vDone(TotalRepeats, false),
 MaxSeed(-1),
 MaxError(0.0),
 TotalError(0.0),
 TotalMisses(0),
 pcpl(pcpl)
{
}

/////////////////////////////////////////////////////////////////////////////
// Collect the outcome of one seed
/////////////////////////////////////////////////////////////////////////////
void CCheckPointData::Collect(int Seed, double Error, int Misses)
{
// boost::lock_guard<boost::mutex> lg(mut);
 boost::mutex::scoped_lock sl(mut);

 Repeats++;
 vError[Seed] = Error;
 vDone[Seed] = true;
 TotalError += Error;
 TotalMisses += Misses;

 if (Error > MaxError || Error != Error /* std::isnan(Error) */)
 {
  MaxError = Error;
  MaxSeed = Seed;
 }

 if (pcpl)
  pcpl->OnCollect(*this);
}

/////////////////////////////////////////////////////////////////////////////
// Percentile. x should be between 0 and 1
/////////////////////////////////////////////////////////////////////////////
double CCheckPointData::GetPercentile(double x) const
{
 std::vector<double> v(Repeats);
 for (int i = Repeats, j = int(vDone.size()); --i >= 0;)
 {
  while (!vDone[--j]) {}
  v[i] = vError[j];
 }

 std::sort(v.begin(), v.end());

 double xIndex = x * (Repeats - 1);
 int Index = int(xIndex);

 if (Index >= Repeats - 1)
  Index = Repeats - 2;
 if (Index < 0)
  Index = 0;

 if (Index < Repeats - 1)
 {
  double Remainder = xIndex - Index;
  return v[Index] * (1 - Remainder) + v[Index + 1] * Remainder;
 }
 else
  return v[Index];
}

/////////////////////////////////////////////////////////////////////////////
// Variance
/////////////////////////////////////////////////////////////////////////////
double CCheckPointData::GetVariance() const
{
 if (Repeats < 2)
  return 0.0;

 const double Mean = GetMeanError();
 double TotalVariance = 0.0;

 for (int i = int(vDone.size()); --i >= 0;)
  if (vDone[i])
  {
   double Delta = vError[i] - Mean;
   TotalVariance += Delta * Delta;
  }

 return TotalVariance / (Repeats - 1);
}
