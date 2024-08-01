/////////////////////////////////////////////////////////////////////////////
//
// CQLRIO.cpp
//
// Rémi Coulom
//
// May, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CQLRIO.h"
#include "CRegression.h"
#include "CLogistic.h"
#include "random.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
// Dump samples
/////////////////////////////////////////////////////////////////////////////
void CQLRIO::DumpSamples(const std::string &sFileName, const CRegression &reg)
{
 std::ofstream out(sFileName.c_str());

 for (int i = 0; i < reg.GetSamples(); i++)
 {
  const double *v = reg.GetSample(i);
  for (int j = 0; j < reg.GetPF().GetDimensions(); j++)
   out << std::setw(13) << v[j] << ' ';
  if (reg.GetOutcome(i) == COutcome::Win)
   out << 1;
  else
   out << 0;
  out << ' ' << reg.GetSampleWeight(i);
  out << '\n';
 }
}

/////////////////////////////////////////////////////////////////////////////
// Recursive plot
/////////////////////////////////////////////////////////////////////////////
void CQLRIO::RecursivePlot(std::ostream &out,
                           const CParametricFunction &pf,
                           const double *vParam,
                           std::vector<double> &v,
                           int Dimension)
{
 if (--Dimension < 0)
 {
  double Strength = pf.GetValue(vParam, &v[0]);
  for (int i = 0; i < pf.GetDimensions(); i++)
   out << v[i] << ' ';
  out << Strength << ' ';
  out << CLogistic::f(Strength) << '\n';
  return;
 }

 int Steps = 100;
 if (pf.GetDimensions() > 1)
  Steps = 20;

 const double Min = -1.0;
 const double Max = +1.0;

 for (int i = 0; i <= Steps; i++)
 {
  double x = Min + ((Max - Min) * i) / Steps;

  v[pf.GetDimensions() - 1 - Dimension] = x;
  RecursivePlot(out, pf, vParam, v, Dimension);
 }

 out << '\n';
}

/////////////////////////////////////////////////////////////////////////////
// Plot one regressor
/////////////////////////////////////////////////////////////////////////////
void CQLRIO::Plot(std::ostream &out,
                  const CParametricFunction &pf,
                  const double *vParam)
{
 std::vector<double> v(pf.GetDimensions());
 RecursivePlot(out, pf, vParam, v, pf.GetDimensions());
}

/////////////////////////////////////////////////////////////////////////////
// Write data files for plotting 1D distribution
/////////////////////////////////////////////////////////////////////////////
void CQLRIO::Distrib(CRegression &reg, const std::string &sPrefix)
{
 {
  std::ostringstream oss;
  oss << sPrefix << "MAP.dat";
  std::ofstream ofs(oss.str().c_str());
  CQLRIO::Plot(ofs, reg.GetPF(), reg.MAP());
 }

 {
  std::ostringstream ossRandom;
  ossRandom << sPrefix << "random.dat";
  std::ofstream ofs(ossRandom.str().c_str());

  std::ostringstream ossMax;
  ossMax << sPrefix << "max.dat";
  std::ofstream ofsMax(ossMax.str().c_str());

  CRandom<unsigned> rnd;

  const int PosteriorSamples = 50;

  for (int i = PosteriorSamples; --i >= 0;)
  {
   std::vector<double> vParam(reg.GetPF().GetParameters());
//   reg.GaussianSample(rnd, vParam);
   reg.MCMCSample(rnd, vParam, 20);

   if (reg.GetPF().GetDimensions() <= 1 || i < 5)
   {
    CQLRIO::Plot(ofs, reg.GetPF(), &vParam[0]);
    ofs << '\n';
   }

   std::vector<double> vMax(reg.GetPF().GetDimensions());
   if (reg.GetPF().GetMax(&vParam[0], &vMax[0]))
   {
    for (unsigned j = 0; j < vMax.size(); j++)
     ofsMax << std::setw(13) << vMax[j];
    ofsMax << std::setw(13) << reg.GetPF().GetValue(&vParam[0], &vMax[0]);
    ofsMax << '\n';
   }
  }
 }
}
