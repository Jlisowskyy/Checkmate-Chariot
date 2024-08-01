/////////////////////////////////////////////////////////////////////////////
//
// CCheckPointData
//
// Rémi Coulom
//
// November, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CCheckPointData_Declared
#define CCheckPointData_Declared

#include <boost/thread/mutex.hpp>
#include <vector>

class CCPListener;

class CCheckPointData // cpd
{
 private: //////////////////////////////////////////////////////////////////
  boost::mutex mut;

  int Samples;
  int Repeats;
  std::vector<double> vError;
  std::vector<bool> vDone;
  int MaxSeed;
  double MaxError;
  double TotalError;
  int TotalMisses;

  CCPListener *pcpl;

 public: ///////////////////////////////////////////////////////////////////

  CCheckPointData(int TotalRepeats, int Samples, CCPListener *pcpl);

  void Collect(int Seed, double Error, int Misses);

  int GetRepeats() const {return Repeats;}
  int GetSamples() const {return Samples;}
  int GetRemainingRepeats() const {return int(vError.size()) - Repeats;}
  int GetMaxErrorSeed() const {return MaxSeed;}
  int GetMisses() const {return TotalMisses;}
  double GetMaxError() const {return MaxError;}
  double GetMeanError() const {return TotalError / Repeats;}
  double GetPercentile(double x) const;
  double GetVariance() const;
};

#endif
