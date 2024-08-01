/////////////////////////////////////////////////////////////////////////////
//
// CRealExperiment.h
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRealExperiment_Declared
#define CRealExperiment_Declared

#include "CResults.h"
#include "CObserver.h"

#include <QObject>

#include <vector>
#include <string>
#include <fstream>
#include <set>

class CRealProblem;
class CProblemRunner;
class CSamplingPolicy;
class CResults;

class CRealExperiment: // rexp
 public QObject,
 private CObserver
{
 Q_OBJECT

 private: ///////////////////////////////////////////////////////////////////
  const CRealProblem &problem;
  CSamplingPolicy &sp;
  CResults &results;
  const std::string sName;
  const int Replications;
  const bool fStopOnError;

  bool fAbortLoading;

  int TargetSamples;
  bool fStop;

  std::vector<CProblemRunner *> vpr;
  int RunningProcessors;

  bool Done() const;

  std::fstream fsDat;
  std::set<int> setInProgress;

  void OnSample();
  void OnOutcome(int i);
  void ReplayBackup(std::ifstream &ifs);
  void StartGame(int id);
  void AfterRun(int id);

 private slots: /////////////////////////////////////////////////////////////
  void OnPROutcome(int id, int Seed, COutcome outcome);
  void OnPRError(int id, int Seed);

 public: ////////////////////////////////////////////////////////////////////
  CRealExperiment(const CRealProblem &problem,
                  CSamplingPolicy &sp,
                  CResults &results,
                  const std::string &sName,
                  int Replications,
                  bool fStopOnError);

  void StartLogs();
  void AddProcessor(const std::string &s);
  void Run();
  void SoftStop();
  void HardStop();

  ~CRealExperiment();

 public slots: //////////////////////////////////////////////////////////////
  void AbortLoading() {fAbortLoading = true;}

 signals: ///////////////////////////////////////////////////////////////////
  void StartLoading(int size);
  void LoadingProgress(int pos);
  void Started();
  void Result();
  void Sample();
  void Finished();
  void Message(std::string sMessage);
};

#endif
