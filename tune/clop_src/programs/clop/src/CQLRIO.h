/////////////////////////////////////////////////////////////////////////////
//
// CQLRIO.h
//
// Rémi Coulom
//
// May, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CQLRIO_Declared
#define CQLRIO_Declared

#include <iosfwd>
#include <string>
#include <vector>

class CParametricFunction;
class CRegression;

class CQLRIO
{
 private: ///////////////////////////////////////////////////////////////////
  static void RecursivePlot(std::ostream &out,
                            const CParametricFunction &pf,
                            const double *vParam,
                            std::vector<double> &v,
                            int Dimension);

 public: ////////////////////////////////////////////////////////////////////
  static void DumpSamples(const std::string &sFileName, const CRegression &reg);

  static void Plot(std::ostream &out,
                   const CParametricFunction &pf,
                   const double *vParam);

  static void Distrib(CRegression &reg, const std::string &sPrefix = "");
};

#endif
