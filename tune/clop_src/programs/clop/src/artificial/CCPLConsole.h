/////////////////////////////////////////////////////////////////////////////
//
// CCPLConsole.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CCPLConsole_Declared
#define CCPLConsole_Declared

#include "CCPListener.h"
#include "userflag.h"
#include <boost/date_time/posix_time/posix_time.hpp>

class CCheckPointData;

class CCPLConsole: public CCPListener
{
 private: ///////////////////////////////////////////////////////////////////
  boost::posix_time::ptime StartTime;
  const int Every;
  long PreviousTimeLeft;

  CUserFlag flag;

 public: ////////////////////////////////////////////////////////////////////
  explicit CCPLConsole(int Every = 1): Every(Every) {}
  void OnStart(const CCheckPointData &cpd);
  void OnCollect(const CCheckPointData &cpd);
  void OnStop(const CCheckPointData &cpd);
  bool Continue() const {return !flag.IsSet();}
};

#endif
