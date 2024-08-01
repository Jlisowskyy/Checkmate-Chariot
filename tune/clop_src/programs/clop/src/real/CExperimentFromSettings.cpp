/////////////////////////////////////////////////////////////////////////////
//
// CExperimentFromSettings.cpp
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CExperimentFromSettings.h"
#include "CExperimentSettings.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CExperimentFromSettings::CExperimentFromSettings(
 const CExperimentSettings &es
):
 results(es.lparam.size()),
 pfq(es.lparam.size()),
 pfiq(es.lparam.size()),
 reg(results, es.fIndependent ? static_cast<const CParametricFunction &>(pfiq) :
                                static_cast<const CParametricFunction &>(pfq)),
 me(reg),
 sp(reg, 0, 1000),
 problem(es.sScript, es.lsScriptParam, paramcol),
 rexp(problem, sp, results, es.sName, es.Replications, es.fStopOnError),
 elf((es.sName + ".log").c_str()),
 eigen(results, reg, pfq)
{
 for (std::list<CParameter *>::const_iterator i = es.lparam.begin();
      i != es.lparam.end();
      ++i)
  paramcol.Add(**i);

 reg.SetRefreshRate(0.1);
 reg.SetDrawRating(es.DrawRating);
 reg.SetLocalizationHeight(es.H);

 reg.SetMaxWeightIterations(7);
 pfq.SetPriorStrength(1e-2);

 for (std::list<std::string>::const_iterator i = es.lsProcessors.begin();
      i != es.lsProcessors.end();
      ++i)
  rexp.AddProcessor(*i);

 elf.connect(&rexp,
             SIGNAL(Message(std::string)),
             SLOT(OnMessage(std::string)));
}

/////////////////////////////////////////////////////////////////////////////
// Quick load (no auto-localize)
/////////////////////////////////////////////////////////////////////////////
void CExperimentFromSettings::QuickLoad()
{
 reg.SetAutoLocalize(false);
 rexp.StartLogs();
 reg.SetAutoLocalize(true);
 reg.ComputeLocalWeights();
}
