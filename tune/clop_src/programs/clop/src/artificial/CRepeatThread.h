/////////////////////////////////////////////////////////////////////////////
//
// CRepeatThread.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRepeatThread_Declared
#define CRepeatThread_Declared

#include <vector>

class CRepeatThreads;
class CArtificialExperiment;

class CRepeatThread // rt
{
 friend class CRepeatThreads;

 private: ///////////////////////////////////////////////////////////////////
  CRepeatThreads &rts;
  CArtificialExperiment &artexp;
  int Seed;

  std::vector<double> vMax;
  std::vector<double> vOptimal;

 public: ////////////////////////////////////////////////////////////////////
  CRepeatThread(CRepeatThreads &rts, CArtificialExperiment &artexp);

  void operator()();
};

#endif
