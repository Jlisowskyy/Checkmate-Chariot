/////////////////////////////////////////////////////////////////////////////
//
// CCPLConsole.cpp
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CCPLConsole.h"
#include "CCheckPointData.h"

#include <iostream>
#include <iomanip>
#include <cmath>

using namespace boost::posix_time;

/////////////////////////////////////////////////////////////////////////////
// OnStart: write headers for progress information
/////////////////////////////////////////////////////////////////////////////
void CCPLConsole::OnStart(const CCheckPointData &cpd)
{
 std::cout << std::setw(8) << "TimeLeft";
 std::cout << std::setw(9) << "Remain";
 std::cout << std::setw(9) << "MaxSeed";
 std::cout << std::setw(13) << "MaxError";
 std::cout << std::setw(13) << "MeanError";
 std::cout << std::setw(13) << "Median";
 std::cout << std::setw(13) << "Deviation";
 std::cout << '\n';
 StartTime = microsec_clock::universal_time();
 PreviousTimeLeft = 0;
 flag.Reset();
 flag.Activate();
}

/////////////////////////////////////////////////////////////////////////////
// Write progress information
/////////////////////////////////////////////////////////////////////////////
void CCPLConsole::OnCollect(const CCheckPointData &cpd)
{
 int RemainingRepeats = cpd.GetRemainingRepeats();

 if (RemainingRepeats % Every == 0)
 {
  ptime CurrentTime = microsec_clock::universal_time();

  long TimeLeft = milliseconds(
   (RemainingRepeats * (CurrentTime - StartTime).total_milliseconds()) /
   cpd.GetRepeats()).total_seconds();

  if (RemainingRepeats > 0)
   TimeLeft += 1;

  if (PreviousTimeLeft > 0 && TimeLeft > PreviousTimeLeft)
   TimeLeft = PreviousTimeLeft;
  else
   PreviousTimeLeft = TimeLeft;

  std::cout << std::setw( 8) << to_simple_string(seconds(TimeLeft));
  std::cout << std::setw( 9) << RemainingRepeats;
  std::cout << std::setw( 9) << cpd.GetMaxErrorSeed();
  std::cout << std::setw(13) << cpd.GetMaxError();
  std::cout << std::setw(13) << cpd.GetMeanError();
  std::cout << std::setw(13) << cpd.GetPercentile(0.5);
  std::cout << std::setw(13) << std::sqrt(cpd.GetVariance());
  std::cout << '\r';
  std::cout.flush();
 }
}

/////////////////////////////////////////////////////////////////////////////
// OnStop: print worst case and other data
/////////////////////////////////////////////////////////////////////////////
void CCPLConsole::OnStop(const CCheckPointData &cpd)
{
 std::cout << '\n';
 if (flag.IsSet())
  std::cout << "Interrupted!\n";
 std::cout << "Duration = " <<
  microsec_clock::universal_time() - StartTime << '\n';
}
