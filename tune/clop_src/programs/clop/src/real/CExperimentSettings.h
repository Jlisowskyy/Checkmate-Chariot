/////////////////////////////////////////////////////////////////////////////
//
// CExperimentSettings.h
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CExperimentSettings_Declared
#define CExperimentSettings_Declared

#include <iosfwd>
#include <list>
#include <string>

class CParameter;

class CExperimentSettings // es
{
 public: ////////////////////////////////////////////////////////////////////
  std::list<CParameter *> lparam;
  std::string sScript;
  std::list<std::string> lsScriptParam;
  std::string sName;
  std::list<std::string> lsProcessors;
  int Replications;
  double DrawRating;
  bool fStopOnError;
  double H;
  bool fIndependent;

 public: ////////////////////////////////////////////////////////////////////
  CExperimentSettings(std::istream &is);
  ~CExperimentSettings();
};

#endif
