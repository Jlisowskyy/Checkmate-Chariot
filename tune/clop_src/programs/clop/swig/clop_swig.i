/////////////////////////////////////////////////////////////////////////////
//
// clop_swig.i
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
%module clop_swig

%{
#include <fstream>
#include <boost/thread/thread.hpp>
#include <boost/version.hpp>
unsigned hardware_concurrency()
{
#if (BOOST_VERSION >= 103500)
 return boost::thread::hardware_concurrency();
#else
 return 0;
#endif
}

#include "CResults.h"

#include "CPQuadratic1D.h"
#include "CPQuadratic2D.h"
#include "CPQuadraticND.h"
#include "CPLog1D.h"
#include "CPDiscontinuous.h"
#include "CPPositive.h"
#include "CPComplicated1D.h"
#include "CPNonQuadratic2D.h"
#include "CPNonQuadraticND.h"
#include "CPZeroND.h"
#include "CPAbsND.h"
#include "CPFish1D.h"
#include "CPDonut.h"
#include "CPParametric.h"
#include "CPRosenbrock.h"
#include "CPVolcano.h"
#include "COlivier.h"
#include "CPSinExp.h"
#include "CPFlat.h"
#include "CPIllIndependent.h"
#include "CPIllCorrelated.h"
#include "CPPower1D.h"
#include "CPMultiple.h"

#include "CBAST.h"
#include "CCrossEntropy.h"
#include "CSPSA.h"
#include "CRSPSA.h"

#include "CRegression.h"
#include "CPFIndependentQuadratic.h"
#include "CPFQuadratic.h"
#include "CPFCubic.h"
#include "CPFGaussian.h"
#include "CPFConcave.h"

#include "CDFVariance.h"
#include "CDFVarianceAlpha.h"
#include "CDFVarianceDelta.h"

#include "CSPCOptimal.h"
#include "CSPCOptimalMAP.h"
#include "CSPComposite.h"
#include "CSPQLRMax.h"
#include "CSPDyadic.h"
#include "CSPUCB.h"
#include "CSPUniform.h"
#include "CSPVOptimal.h"
#include "CSPWeight.h"

#include "CMERegressionMAPMax.h"
#include "CMEQLRLCB.h"
#include "CMESampleMean.h"

#include "CArtificialExperiment.h"
#include "CCheckPointData.h"
#include "CRepeatThreads.h"
#include "CRepeatThread.h"
#include "CCPLConsole.h"

#include "CQLRIO.h"
#include "CTikZ.h"
#include "CTikZOneD.h"
#include "CTikZTwoD.h"

#include "CMatrixOperations.h"
%}

/////////////////////////////////////////////////////////////////////////////
// Append code to methods that keep a reference (mainly constructors)
/////////////////////////////////////////////////////////////////////////////

%pythonappend
CSPVOptimal::CSPVOptimal(const CRegression &reg, CDFVariance &dfvar)
%{
        #check-pythonappend CSPVOptimal::CSPVOptimal
        self.reg_reference = args[0]
        self.dfvar_reference = args[1]
%}

%pythonappend
CSPQLRMax::CSPQLRMax(CRegression &reg)
%{
        #check-pythonappend CSPQLRMax::CSPQLRMax
        self.reg_reference = args[0]
%}

%pythonappend
CSPComposite::CSPComposite(const CRegression &reg,
                           CSamplingPolicy &sp1,
                           CSamplingPolicy &sp2, 
                           int MinSamples)
%{
        #check-pythonappend CSPComposite::CSPComposite
        self.reg_reference = args[0]
        self.sp1_reference = args[1]
        self.sp2_reference = args[2]
%}

%pythonappend
CDFVarianceAlpha::CDFVarianceAlpha(CRegression &reg)
%{
        #check-pythonappend CDFVarianceAlpha::CDFVarianceAlpha
        self.reg_reference = args[0]
%}

%pythonappend
CDFVarianceDelta::CDFVarianceDelta(CRegression &reg)
%{
        #check-pythonappend CDFVarianceDelta::CDFVarianceDelta
        self.reg_reference = args[0]
%}

%pythonappend
CMERegressionMAPMax::CMERegressionMAPMax(CRegression &reg)
%{
        #check-pythonappend CMERegressionMAPMax::CMERegressionMAPMax
        self.reg_reference = args[0]
%}

%pythonappend
CMEQLRLCB::CMEQLRLCB(CRegression &reg)
%{
        #check-pythonappend CMEQLRLCB::CMEQLRLCB
        self.reg_reference = args[0]
%}

%pythonappend
CRepeatThreads::CRepeatThreads(int Repeats, int Samples)
%{
        #check-pythonappend CRepeatThreads::CRepeatThreads
        self.prtl_reference = args[2]
%}

%pythonappend
CArtificialExperiment::CArtificialExperiment(CArtificialProblem &problem,
                                             CSamplingPolicy &sp,
                                             CMaxEstimator &me,
                                             CResults &results)
%{
        #check-pythonappend CArtificialExperiment::CArtificialExperiment
        self.problem_reference = args[0]
        self.sp_reference = args[1]
        self.me_reference = args[2]
        self.results_reference = args[3]
%}

%pythonappend
CRegression::CRegression(CResults &results,
                         const CParametricFunction &pf)
%{
        #check-pythonappend CRegression::CRegression
        self.results_reference = args[0]
        self.pf_reference = args[1]
%}

%pythonappend
CPParametric::CPParametric(const CParametricFunction &pf,
                           const double *vParam)
%{
        #check-pythonappend CPParametric::CPParametric
        self.pf_reference = args[0]
%}

%pythonappend
CBAST::CBAST(CResults &results)
%{
        #check-pythonappend CBAST::CBAST
        self.results_reference = args[0]
%}

%pythonappend
CCrossEntropy::CCrossEntropy(CResults &results)
%{
        #check-pythonappend CCrossEntropy::CCrossEntropy
        self.results_reference = args[0]
%}

%pythonappend
CSPSA::CSPSA(CResults &results)
%{
        #check-pythonappend CSPSA::CSPSA
        self.results_reference = args[0]
%}

%pythonappend
CRSPSA::CRSPSA(CResults &results)
%{
        #check-pythonappend CRSPSA::CRSPSA
        self.results_reference = args[0]
%}

%pythonappend
CPMultiple::CPMultiple(int Multiplier, const CArtificialProblem &problem)
%{
        #check-pythonappend CPMultiple::CPMultiple
        self.problem_reference = args[1]
%}

/////////////////////////////////////////////////////////////////////////////
// SWIG interfaces
/////////////////////////////////////////////////////////////////////////////

unsigned hardware_concurrency();

%include "random.h"
%template(Random) CRandom<unsigned>;

%include "carrays.i"
%array_class(double, doubleArray);
%array_class(int, intArray);

%include "std_string.i"
%include "std_vector.i"
%include "std_list.i"

namespace std {
 %template(vectord) vector<double>;
 %template(slist) list<std::string>;
}

%include "CResults.h"
%include "COutcome.h"

%include "CArtificialProblem.h"

%include "CPQuadratic1D.h"
%include "CPQuadratic2D.h"
%include "CPQuadraticND.h"
%include "CPLog1D.h"
%include "CPDiscontinuous.h"
%include "CPPositive.h"
%include "CPComplicated1D.h"
%include "CPNonQuadratic2D.h"
%include "CPNonQuadraticND.h"
%include "CPZeroND.h"
%include "CPAbsND.h"
%include "CPFish1D.h"
%include "CPDonut.h"
%include "CPParametric.h"
%include "CPRosenbrock.h"
%include "CPVolcano.h"
%include "COlivier.h"
%include "CPSinExp.h"
%include "CPFlat.h"
%include "CPIllIndependent.h"
%include "CPIllCorrelated.h"
%include "CPPower1D.h"
%include "CPMultiple.h"

%include "CObserver.h"
%include "CRegression.h"
%include "CParametricFunction.h"
%include "CPFIndependentQuadratic.h"
%include "CPFQuadratic.h"
%include "CPFCubic.h"
%include "CPFGaussian.h"
%include "CPFConcave.h"

%include "CSamplingPolicy.h"
%include "CSPUniform.h"
%include "CSPDyadic.h"
%include "CSPUCB.h"
%include "CSPVOptimal.h"
%include "CSPQLRMax.h"
%include "CSPComposite.h"
%include "CSPCOptimal.h"
%include "CSPCOptimalMAP.h"
%include "CSPWeight.h"

%include "CDiffFunction.h"
%include "CDFVariance.h"
%include "CDFConfidence.h"
%include "CDFVarianceAlpha.h"
%include "CDFVarianceDelta.h"

%include "CMaxEstimator.h"
%include "CMERegressionMAPMax.h"
%include "CMEQLRLCB.h"
%include "CMESampleMean.h"

%include "CBAST.h"
%include "CCrossEntropy.h"
%include "CSPSA.h"
%include "CRSPSA.h"

%include "CRepeatThreads.h"
%include "CArtificialExperiment.h"
%include "CCheckPointData.h"
%include "CCPListener.h"
%include "CCPLConsole.h"

%include "CQLRIO.h"
%include "CTikZ.h"
%include "CTikZOneD.h"
%include "CTikZTwoD.h"

%include "CMatrixOperations.h"
