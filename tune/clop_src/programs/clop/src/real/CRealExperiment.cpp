/////////////////////////////////////////////////////////////////////////////
//
// CRealExperiment.cpp
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CRealExperiment.h"
#include "CRealProblem.h"
#include "CSamplingPolicy.h"
#include "CParameter.h"
#include "CParameterCollection.h"
#include "CProblemRunner.h"
#include "debug.h"

#include <QTime>
#include <QFile>

#include <sstream>
#include <iomanip>
#include <limits>

#define LOG(x) do\
{\
 std::ostringstream oss;\
 oss << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString() << ' ';\
 oss << x;\
 emit Message(oss.str());\
} while(false)

#define LOG_ID(x) LOG("id = " << id << " (" << vpr[id]->GetProcessorName() << ")" << "; Seed = " << Seed << "; " << x)


/////////////////////////////////////////////////////////////////////////////
// Log new sample
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::OnSample()
{
 int Index = results.GetSamples() - 1;
 const double *v = results.GetSample(Index);

 fsDat << "S " << std::setw(8) << Index << ' ';

 const CParameterCollection &paramcol = problem.paramcol;
 for (int i = 0; i < paramcol.GetSize(); i++)
 {
  double x = paramcol.GetParam(i).TransformFromQLR(v[i]);
  fsDat << std::setw(fsDat.precision() + 4) << x << ' ';
 }

 fsDat << '\n';
 fsDat.flush();
}

/////////////////////////////////////////////////////////////////////////////
// Log new outcome
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::OnOutcome(int i)
{
 fsDat << "R ";
 fsDat << std::setw(8) << i << ' ';
 fsDat << results.GetOutcome(i) << '\n';
 fsDat.flush(); 
}

/////////////////////////////////////////////////////////////////////////////
// Replay back-up log in case of interruption
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::ReplayBackup(std::ifstream &ifs)
{
 //
 // Tell file size
 //
 ifs.seekg (0, std::ios::end);
 emit StartLoading(ifs.tellg());
 ifs.seekg (0, std::ios::beg);

 const CParameterCollection &paramcol = problem.paramcol;
 int ParamIndex = 0;
 int Results = 0;
 int Samples = 0;
 int Mismatches = 0;

 fAbortLoading = false;
 while(!fAbortLoading)
 {
  if ((Samples & 0xff) == 0xff)
   emit LoadingProgress(ifs.tellg());

  char c;
  ifs >> c;

  if (!ifs)
   break;

  switch(c)
  {
   case 'N': ////////////////////////////////////////////////////////////////
   {
    int N = -1;
    ifs >> N;
    if (N != paramcol.GetSize())
    {
     LOG("Wrong N (got " << N << ", expected " << paramcol.GetSize() << ')');
     return;
    }
   }
   break;

   case 'P': ////////////////////////////////////////////////////////////////
   {
    std::string s;
    ifs >> s;
    ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
 
    if (ParamIndex < paramcol.GetSize())
    {
     std::string sExpected = paramcol.GetParam(ParamIndex).GetName();
     if (s != sExpected)
     {
      LOG("Wrong parameter (got " << s << ", expected " << sExpected << ')'); 
      return;
     }
     ParamIndex++;
    }
   }
   break;

   case 'S': ////////////////////////////////////////////////////////////////
   {
    //
    // Read seed and parameter values from file
    //
    int Seed;
    ifs >> Seed;
    std::vector<double> vFile(paramcol.GetSize());
    for (unsigned i = 0; i < vFile.size(); i++)
    {
     double x;
     ifs >> x;
     vFile[i] = paramcol.GetParam(i).TransformToQLR(x);
    }
    
    //
    // Try to collect a sample according to sp, and check if it matches
    //
    if (!sp.IsFlexible())
    {
     const double *vSP = sp.NextSample(results.GetSamples());
     double N2 = 0.0;

     for (int i = 0; i < paramcol.GetSize(); i++)
     {
      double Delta = vFile[i] - vSP[i];
      N2 += Delta * Delta;
     }

     bool fMatch = (N2 <= 1e-9);

     if (!fMatch)
     {
      if (Mismatches == 0)
      {
       LOG("Sample mismatch, N2 = " << N2 << ", Seed = " << Seed);
       if (!sp.IsFlexible())
       {
        LOG("Sampling policy is not flexible. Stopping.");
        return;
       }
       else
        LOG("Sampling policy is flexible. Continuing.");
      }
      Mismatches++;
     }

     if (Mismatches == 0)
      results.AddSample(vSP);
     else
      results.AddSample(&vFile[0]);
    }
    else
     results.AddSample(&vFile[0]);

    //
    // Add this sample
    //
    setInProgress.insert(Seed);
    Samples++;
   }
   break;

   case 'R': ////////////////////////////////////////////////////////////////
   {
    int Index;
    int outcome;
    ifs >> Index >> outcome;

    Results++;
    setInProgress.erase(Index);
    results.AddOutcome(Index, COutcome(COutcome::Value(outcome)));
   }
   break;

   default: /////////////////////////////////////////////////////////////////
    LOG("Error parsing backup");
   return; 
  }
 }

 LOG("Backup done.");
 LOG("Samples = " << Samples <<
     "; Results = " << Results <<
     "; Mismatches = " << Mismatches << '.');
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CRealExperiment::CRealExperiment(const CRealProblem &problem,
                                 CSamplingPolicy &sp,
                                 CResults &results,
                                 const std::string &sName,
                                 int Replications,
                                 bool fStopOnError):
 CObserver(results),
 problem(problem),
 sp(sp),
 results(results),
 sName(sName),
 Replications(Replications),
 fStopOnError(fStopOnError),
 fStop(false),
 RunningProcessors(0)
{
 FATAL(Replications > 1 && !sp.IsFlexible());
}

/////////////////////////////////////////////////////////////////////////////
// Start logs
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::StartLogs()
{
 LOG ("===== Starting: " << sName << " =====");
 QString qName(sName.c_str());
 QString datName(qName + ".dat");
 QString backupName(qName + "-old.dat");

 //
 // Create a backup of previous log file
 //
 {
  QFile File(datName);
  if (File.exists())
  {
   LOG("Found existing " << File.fileName().toStdString());
   QFile Backup(backupName);
   if (Backup.exists())
   {
    LOG("Removing " << backupName.toStdString());
    Backup.remove();
   }
   LOG("Renaming " << datName.toStdString() <<
       " to " << backupName.toStdString());
   File.rename(backupName);
  }
 }

 //
 // Open data file for this experiment and write parameter names
 //
 fsDat.open(datName.toStdString().c_str(), std::ios_base::out);
 fsDat.precision(std::numeric_limits<double>::digits10 + 1);
 const CParameterCollection &paramcol = problem.paramcol;
 int N = paramcol.GetSize();
 fsDat << "N " << N << '\n';
 for (int i = 0; i < N; i++)
  fsDat << "P " << paramcol.GetParam(i).GetName() << '\n';
 fsDat.flush();

 //
 // Replay backup log if possible
 //
 if (QFile(backupName).exists())
 {
  LOG("Reading " << backupName.toStdString() << "...");
  std::ifstream ifs(backupName.toStdString().c_str(), std::ios::binary);
  ReplayBackup(ifs);
 }
}

/////////////////////////////////////////////////////////////////////////////
// Start a new game
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::StartGame(int id)
{
 if (fStop)
  return;

 RunningProcessors++;

 int Seed;

 if (setInProgress.empty())
 {
  Seed = results.GetSamples();
  LOG_ID("Starting new sample");
  if (Seed % Replications == 0)
   results.AddSample(problem.paramcol.Translate((sp.NextSample(Seed))));
  else
  {
   results.Reserve(Seed + 1);
   results.AddSample(results.GetSample(Seed - 1));
  }
  emit Sample();
 }
 else
 {
  Seed = *setInProgress.begin();
  LOG_ID("Starting from setInProgress");
  setInProgress.erase(Seed);
 }

 vpr[id]->Run(Seed, results.GetSample(Seed));
}

/////////////////////////////////////////////////////////////////////////////
// Outcome
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::OnPROutcome(int id, int Seed, COutcome outcome)
{
 LOG_ID("Done (" << outcome << ")");
 results.AddOutcome(Seed, outcome);
 emit Result();

 AfterRun(id);
}

/////////////////////////////////////////////////////////////////////////////
// Error
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::OnPRError(int id, int Seed)
{
 LOG_ID("Error: " << vpr[id]->GetError());

 if (fStopOnError)
 {
  fStop = true;
  setInProgress.insert(Seed);
 }

 AfterRun(id);
}

/////////////////////////////////////////////////////////////////////////////
// Stop or continue problem runner
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::AfterRun(int id)
{
 RunningProcessors--;
 if (!fStop)
  StartGame(id);
 else
 {
  LOG("id = " << id << " stopped. RunningProcessors = " << RunningProcessors);
  if (RunningProcessors == 0)
  {
   emit Finished();
   LOG("Stop");
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Run
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::Run()
{
 emit Started();

 LOG("Run");

 fStop = false;

 for (int id = vpr.size(); --id >= 0;)
  StartGame(id);
}

/////////////////////////////////////////////////////////////////////////////
// Soft stop (wait for running games to finish)
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::SoftStop()
{
 fStop = true;
}

/////////////////////////////////////////////////////////////////////////////
// Hard Stop (kill all running games)
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::HardStop()
{
 SoftStop();
 for (int id = vpr.size(); --id >= 0;)
  vpr[id]->Kill();
}

/////////////////////////////////////////////////////////////////////////////
// Add a processor
/////////////////////////////////////////////////////////////////////////////
void CRealExperiment::AddProcessor(const std::string &s)
{
 CProblemRunner *ppr = new CProblemRunner(vpr.size(), problem, s);
 vpr.push_back(ppr);
 connect(ppr,
         SIGNAL(Outcome(int, int, COutcome)),
         SLOT(OnPROutcome(int, int, COutcome)));
 connect(ppr,
         SIGNAL(Error(int, int)),
         SLOT(OnPRError(int, int)));
}

/////////////////////////////////////////////////////////////////////////////
// Destructor
/////////////////////////////////////////////////////////////////////////////
CRealExperiment::~CRealExperiment()
{
 if (RunningProcessors)
  HardStop();

 for (int i = vpr.size(); --i >= 0;)
 {
  vpr[i]->Wait();
  delete vpr[i];
 }
}
