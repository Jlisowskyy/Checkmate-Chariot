/////////////////////////////////////////////////////////////////////////////
//
// CRealObserver.cpp
//
// Rémi Coulom
//
// November, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CRealObserver.h"
#include "CResults.h"
#include "CParameterCollection.h"
#include "CMaxEstimator.h"
#include "CParameter.h"

#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
// Display stats about current results
/////////////////////////////////////////////////////////////////////////////
void CRealObserver::OnOutcome(int i)
{
 //results.Refresh();

 int Losses = results.CountOutcomes(COutcome::Loss);
 int Wins = results.CountOutcomes(COutcome::Win);

 std::cout << std::setw(8) << results.GetSamples();
 std::cout << std::setw(8) << Wins;
 std::cout << std::setw(8) << Losses;
 std::cout << std::setw(13) << float(Wins) / (Wins + Losses);

 std::vector<double> v(paramcol.GetSize());
 bool fMax = me.MaxParameter(&v[0]);
 if (fMax)
 {
  for (int j = 0; j < paramcol.GetSize(); j++)
  {
   const CParameter &param = paramcol.GetParam(j);
   std::cout << std::setw(13) << param.TransformFromQLR(v[j]);
  }
 }

 std::cout << '\n';
 std::cout.flush();
}
