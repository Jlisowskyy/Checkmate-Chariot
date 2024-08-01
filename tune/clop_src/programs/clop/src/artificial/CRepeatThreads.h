/////////////////////////////////////////////////////////////////////////////
//
// CRepeatThreads.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRepeatThreads_Declared
#define CRepeatThreads_Declared

#include "CRepeatThread.h"
#include "CCheckPointData.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <vector>

class CCPListener;
class CArtificialExperiment;

class CRepeatThreads // rts
{
 friend class CRepeatThread;

 private: ///////////////////////////////////////////////////////////////////
  const int TotalRepeats;
  const int Samples;

  std::vector<CArtificialExperiment *> vpartexp;
  boost::thread_group tg;

  boost::mutex mutDispatch;
  int CurrentSeed;
  bool Dispatch(CRepeatThread &rt);

  std::vector<CCheckPointData*> vpcpd;
  CCPListener *pcpl;

 public: ////////////////////////////////////////////////////////////////////
  CRepeatThreads(int Repeats, int Samples, CCPListener *pcpl = 0);

  void AddThread(CArtificialExperiment &artexp) {vpartexp.push_back(&artexp);}

  // Check points must be added in decreasing order of Samples
  // The constructor already creates a checkpoint at Samples = SamplesInit
  // More checkpoints must be added manually
  void AddCheckPoint(int Samples)
  {
   vpcpd.push_back(new CCheckPointData(TotalRepeats, Samples, 0));
  }
  int GetCheckPoints() {return int(vpcpd.size());}
  const CCheckPointData &GetCheckPointData(int i) {return *vpcpd[i];}

  void Start();
  void WaitForTermination();

  ~CRepeatThreads();
};

#endif
