/////////////////////////////////////////////////////////////////////////////
//
// clop.cpp
//
// Rémi Coulom
//
// May, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <iomanip>

#include "CRegression.h"
#include "CPLog1D.h"
#include "CTikZOneD.h"
#include "CResults.h"
#include "CSPUniform.h"
#include "CDFRatingLCB.h"
#include "CDFVarianceAlpha.h"
#include "CDFVarianceDelta.h"
#include "CMERegressionMAPMax.h"
#include "CArtificialExperiment.h"
#include "CRepeatThreads.h"
#include "CCPLConsole.h"
#include "CCrossEntropy.h"
#include "CSPVOptimal.h"
#include "CPFQuadratic.h"
#include "CSPSA.h"
#include "CRSPSA.h"

/////////////////////////////////////////////////////////////////////////////
// Test functions
/////////////////////////////////////////////////////////////////////////////
static void DFTest()
{
 CPLog1D problem;
 CResults results(problem.GetDimensions());

 CPFQuadratic pf(problem.GetDimensions());
 CRegression reg(results, pf);

 CSPUniform sp(problem.GetDimensions(), -1, 1);

// sp.Seed(0);
// problem.Seed(1);
 for (int i = 100; --i >= 0;)
 {
  const double *v = sp.NextSample(results.GetSamples());
  COutcome r = problem.GetOutcome(v);
  results.AddSample(v, r);
 }
 results.Refresh();

// CDFRatingLCB df(reg, 1.96);
 CDFVarianceAlpha df(reg);
// CDFVarianceDelta df(reg);

 const int Points = 20;
 double v[1];
 for (int i = 0; i <= Points; i++)
 {
  double x = -1.0 + 2.0 * i / Points;
  v[0] = x;
  double y = df.GetOutput(v);
  df.ComputeGradient();
  double g = df.GetGradient()[0];
  df.CDFVariance::ComputeGradient();
  double h = df.GetGradient()[0];
  std::cout << std::setw(13) << x <<
               std::setw(13) << y <<
               std::setw(13) << g <<
               std::setw(13) << h;
  std::cout << '\n';
 }
}

/////////////////////////////////////////////////////////////////////////////
// test threads
/////////////////////////////////////////////////////////////////////////////
static void ThreadsTest()
{
 CPLog1D problem;
 CResults results(problem.GetDimensions());

// CSPUniform sp(problem.GetDimensions());

#if 0
 CPFQuadratic pf(problem.GetDimensions());
 CRegression reg(results, pf);
 reg.SetRefreshRate(0.1);
 reg.SetLocalizationHeight(2.0);
 CMERegressionMAPMax me(reg);

 CDFVarianceSamples var(reg, 1);
 var.SetMinSamples(problem.GetDimensions());
 CSPVOptimal sp(reg, var, 0);
 CArtificialExperiment artexp(problem, sp, me, results);
#else
 //CCrossEntropy alt(results, 0.9);
 //CSPSA alt(results);
 CRSPSA alt(results);
 CArtificialExperiment artexp(problem, alt, alt, results);
#endif

 CCPLConsole cplc;
 CRepeatThreads rts(79, 100000, &cplc);
 rts.AddThread(artexp);

 CPLog1D problemBis;
 CResults resultsBis(problemBis.GetDimensions());
 CRSPSA altBis(resultsBis);
 CArtificialExperiment artexpBis(problemBis, altBis, altBis, resultsBis);
 rts.AddThread(artexpBis);

 rts.Start();
 rts.WaitForTermination();
}

/////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////
int main()
{
 DFTest();
 ThreadsTest();
 return 0;
}
