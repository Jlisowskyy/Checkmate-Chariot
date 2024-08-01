/////////////////////////////////////////////////////////////////////////////
//
// DummyExperiment.cpp
//
// Rémi Coulom
//
// May, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CParameterCollection.h"
#include "CLinearParameter.h"
#include "CRealProblem.h"
#include "CResults.h"
#include "CRegression.h"
#include "CPFQuadratic.h"
#include "CBAST.h"
#include "CRealExperiment.h"
#include "CRealObserver.h"

#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////////////////////////
int main()
{
 CParameterCollection paramcol;

 CLinearParameter param1("p1", 0, 1.0);

 paramcol.Add(param1);

 CRealProblem problem("c:\\Python26\\python.exe DummyScript.py", paramcol);

 CResults results(paramcol.GetSize());

// CPFQuadratic pfq(problem.GetDimensions());
// CRegression reg(results, pfq);

 CBAST sp(results);
 CMaxEstimator &me = sp;

 CRealExperiment rexp(problem, sp, results, "DummyExperiment");
 CRealObserver ro(results, paramcol, me);

 for (int i = 10; --i >= 0;)
 {
  std::ostringstream oss;
  oss << "processor-" << i;
  rexp.AddProcessor(oss.str());
 }

 rexp.Run(100);

 return 0;
}
