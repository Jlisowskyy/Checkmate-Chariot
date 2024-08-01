/////////////////////////////////////////////////////////////////////////////
//
// CValueWithGradient.h
//
// June, 2005
//
/////////////////////////////////////////////////////////////////////////////
#ifndef MATH_CValueWithGradient_Declared
#define MATH_CValueWithGradient_Declared

#include <vector>
#include <iosfwd>

class CValueWithGradient
{
 public: ///////////////////////////////////////////////////////////////////
  double Value;
  std::vector<double> vGradient;

 public: ///////////////////////////////////////////////////////////////////
  CValueWithGradient(int Size): vGradient(Size) {}

  operator double() const {return Value;}
};

const CValueWithGradient &operator+=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR);
const CValueWithGradient &operator-=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR);
const CValueWithGradient &operator*=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR);
const CValueWithGradient &operator/=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR);

CValueWithGradient operator+(const CValueWithGradient &vgA,
                             const CValueWithGradient &vgB);
CValueWithGradient operator-(const CValueWithGradient &vgA,
                             const CValueWithGradient &vgB);
CValueWithGradient operator*(const CValueWithGradient &vgA,
                             const CValueWithGradient &vgB);
CValueWithGradient operator/(const CValueWithGradient &vgA,
                             const CValueWithGradient &vgB);

std::ostream &operator<<(std::ostream &out, const CValueWithGradient &vg);
#endif
