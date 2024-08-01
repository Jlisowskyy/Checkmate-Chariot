/////////////////////////////////////////////////////////////////////////////
//
// CResults.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CResults_Declared
#define CResults_Declared

#include "COutcome.h"

#include <vector>
#include <list>

class CObserver;

class CResults // results
{
 friend class CObserver;

 private: ///////////////////////////////////////////////////////////////////
  int Dimensions;
  std::vector<double> vSample;
  std::vector<COutcome> vOutcome;
  std::list<CObserver *> lObs;
  unsigned Samples;

  void AddObserver(CObserver &obs) {lObs.push_back(&obs);}
  void RemoveObserver(CObserver &obs) {lObs.remove(&obs);}

 public: ////////////////////////////////////////////////////////////////////
  CResults(int Dimensions): Dimensions(Dimensions), Samples(0) {}

  int AddSample(const double *v);
  void AddSample(const double *v, COutcome outcome);
  void AddOutcome(int i, COutcome outcome);
  void Reset();
  void Refresh();
  void Reserve(unsigned n);

  int GetDimensions() const {return Dimensions;}
  unsigned GetSamples() const {return Samples;}
  const double *GetSample(int i) const {return &vSample[i * Dimensions];}
  COutcome GetOutcome(int i) const {return vOutcome[i];}

  int CountOutcomes(COutcome outcome, int MinIndex = 0) const;

  ~CResults();
};

#endif
