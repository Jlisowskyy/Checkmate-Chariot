/////////////////////////////////////////////////////////////////////////////
//
// CRealProblem.h
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRealProblem_Declared
#define CRealProblem_Declared

#include <string>
#include <list>

class CProblemRunner;
class CRealExperiment;
class CParameterCollection;

class CRealProblem // rproblem
{
 friend class CProblemRunner;
 friend class CRealExperiment;

 private: ///////////////////////////////////////////////////////////////////
  const std::string &sScriptName;
  const std::list<std::string> &lsScriptParam;
  const CParameterCollection &paramcol;

 public: ////////////////////////////////////////////////////////////////////
  CRealProblem(const std::string &sScriptName,
               const std::list<std::string> &lsScriptParam,
               const CParameterCollection &paramcol):
   sScriptName(sScriptName),
   lsScriptParam(lsScriptParam),
   paramcol(paramcol)
  {}
};

#endif
