/////////////////////////////////////////////////////////////////////////////
//
// CRealObserver.h
//
// Rémi Coulom
//
// November, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRealObserver_Declared
#define CRealObserver_Declared

#include "CObserver.h"
class CParameterCollection;
class CMaxEstimator;

class CRealObserver: public CObserver // robs
{
 private: ///////////////////////////////////////////////////////////////////
  const CParameterCollection &paramcol;
  const CMaxEstimator &me;

 public: ////////////////////////////////////////////////////////////////////
  CRealObserver(CResults &results,
                const CParameterCollection &paramcol,
                const CMaxEstimator &me):
   CObserver(results),
   paramcol(paramcol),
   me(me)
  {}

  void OnOutcome(int i);
};

#endif
