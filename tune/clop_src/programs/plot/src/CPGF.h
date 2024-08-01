/////////////////////////////////////////////////////////////////////////////
//
// CPGF.h
//
// Rémi Coulom
//
// February, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CPGF_Declared
#define Plot_CPGF_Declared

#include <iosfwd>
#include <string>

class CPGF
{
 public:
  static void Point(std::ostream &out,
                    double x,
                    double y,
                    int Precision = 1);

  static void Circle(std::ostream &out,
                     int n,
                     double x,
                     double y,
                     double r);

  static void Cross(std::ostream &out,
                    double x,
                    double y,
                    double r);

  static int GetPrecision(double DMax);

  static const double cmtopt;
};

#endif
