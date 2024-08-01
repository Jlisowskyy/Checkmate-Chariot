/////////////////////////////////////////////////////////////////////////////
//
// CArtificialExperiment.h
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CArtificialExperiment_Declared
#define CArtificialExperiment_Declared

class CArtificialProblem;
class CSamplingPolicy;
class CMaxEstimator;
class CResults;

class CArtificialExperiment // artexp
{
 public: ////////////////////////////////////////////////////////////////////
  CArtificialProblem &problem;
  CSamplingPolicy &sp;
  CMaxEstimator &me;
  CResults &results;

 public: ////////////////////////////////////////////////////////////////////
  CArtificialExperiment(CArtificialProblem &problem,
                        CSamplingPolicy &sp,
                        CMaxEstimator &me,
                        CResults &results):
   problem(problem),
   sp(sp),
   me(me),
   results(results)
  {}

  void Reserve(unsigned n);
};

#endif
