/////////////////////////////////////////////////////////////////////////////
//
// CProblemRunner.cpp
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CProblemRunner.h"
#include "CParameterCollection.h"
#include "CParameter.h"
#include "CRealProblem.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CProblemRunner::CProblemRunner(int id,
                               const CRealProblem &problem,
                               const std::string &sProcessor) :
 id(id),
 problem(problem),
 sProcessor(sProcessor)
{
 connect(&proc,
         SIGNAL(error(QProcess::ProcessError)),
         SLOT(OnError(QProcess::ProcessError)));
 connect(&proc,
         SIGNAL(finished(int, QProcess::ExitStatus)),
         SLOT(OnFinished(int, QProcess::ExitStatus)));
}

/////////////////////////////////////////////////////////////////////////////
// Run script to get one outcome
/////////////////////////////////////////////////////////////////////////////
void CProblemRunner::Run(int Seed, const double v[])

{
 //
 // Build list of arguments
 //
 QStringList arguments;

 for (std::list<std::string>::const_iterator i = problem.lsScriptParam.begin();
      i != problem.lsScriptParam.end();
      ++i)
  arguments << QString::fromStdString(*i);

 arguments << QString::fromStdString(sProcessor);
 arguments << QString::number(Seed);

 for (int i = 0; i < problem.paramcol.GetSize(); i++)
 {
  const CParameter &param = problem.paramcol.GetParam(i);
  arguments << QString::fromStdString(param.GetName());

  //
  // No floating-point formatting for integers
  //
  double x = param.TransformFromQLR(v[i]);
  if (double(int(x)) == x)
   arguments << QString::number(int(x));
  else
   arguments << QString::number(x);
 }

 //
 // Create and run process
 //
 CurrentSeed = Seed;
 fError = false;
 proc.start(QString::fromStdString(problem.sScriptName), arguments);
}

/////////////////////////////////////////////////////////////////////////////
// Error with this process
/////////////////////////////////////////////////////////////////////////////
void CProblemRunner::OnError(QProcess::ProcessError error)
{
 fError = true;
 sError = proc.errorString().toStdString();
 emit Error(id, CurrentSeed);
}

/////////////////////////////////////////////////////////////////////////////
// Parse output to find game result
/////////////////////////////////////////////////////////////////////////////
void CProblemRunner::OnFinished(int exitCode, QProcess::ExitStatus)
{
 if (!fError)
 {
  char result;
  if (proc.getChar(&result))
   switch (result)
   {
    case 'W': emit Outcome(id, CurrentSeed, COutcome::Win); return;
    case 'L': emit Outcome(id, CurrentSeed, COutcome::Loss); return;
    case 'D': emit Outcome(id, CurrentSeed, COutcome::Draw); return;
    default: proc.ungetChar(result);
   }

  sError = "No Outcome.";
  sError += "\nStandardOutput:\n";
  sError += std::string(proc.readAllStandardOutput().constData());
  sError += "\nStandardError:\n";
  sError += std::string(proc.readAllStandardError().constData());
  emit Error(id, CurrentSeed);
 }
}
