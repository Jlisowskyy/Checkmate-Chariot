/////////////////////////////////////////////////////////////////////////////
//
// CProblemRunner.h
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CProblemRunner_Declared
#define CProblemRunner_Declared

#include <QObject>
#include <QProcess>
#include <string>

#include "COutcome.h"

class CRealProblem;

class CProblemRunner: public QObject
{
 Q_OBJECT

 private: ///////////////////////////////////////////////////////////////////
  const int id;
  const CRealProblem &problem;
  const std::string &sProcessor;
  std::string sError;

  int CurrentSeed;
  QProcess proc;
  bool fError;

 private slots: /////////////////////////////////////////////////////////////
  void OnError(QProcess::ProcessError);
  void OnFinished(int, QProcess::ExitStatus);

 public: ////////////////////////////////////////////////////////////////////
  CProblemRunner(int id,
                 const CRealProblem &problem,
                 const std::string &sProcessor);

  void Run(int Seed, const double v[]);
  void Wait() {proc.waitForFinished(-1);}
  const std::string &GetProcessorName() const {return sProcessor;}
  const std::string &GetError() const {return sError;}

 signals: ///////////////////////////////////////////////////////////////////
  void Outcome(int id, int Seed, COutcome outcome);
  void Error(int id, int Seed);

 public slots: //////////////////////////////////////////////////////////////
  void Kill() {proc.kill();}
};

#endif
