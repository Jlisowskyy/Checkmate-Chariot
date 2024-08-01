/////////////////////////////////////////////////////////////////////////////
//
// CValueWithGradient.cpp
//
// June, 2005
//
/////////////////////////////////////////////////////////////////////////////
#include "CValueWithGradient.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
const CValueWithGradient &operator+=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR)
{
 for (int i = int(vgL.vGradient.size()); --i >= 0;)
  vgL.vGradient[i] += vgR.vGradient[i];
 vgL.Value += vgR.Value;
 return vgL;
}

/////////////////////////////////////////////////////////////////////////////
const CValueWithGradient &operator-=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR)
{
 for (int i = int(vgL.vGradient.size()); --i >= 0;)
  vgL.vGradient[i] -= vgR.vGradient[i];
 vgL.Value -= vgR.Value;
 return vgL;
}

/////////////////////////////////////////////////////////////////////////////
const CValueWithGradient &operator*=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR)
{
 for (int i = int(vgL.vGradient.size()); --i >= 0;)
  vgL.vGradient[i] = vgL.Value * vgR.vGradient[i] +
                     vgL.vGradient[i] * vgR.Value;
 vgL.Value *= vgR.Value;
 return vgL;
}

/////////////////////////////////////////////////////////////////////////////
const CValueWithGradient &operator/=(CValueWithGradient &vgL,
                                     const CValueWithGradient &vgR)
{
 for (int i = int(vgL.vGradient.size()); --i >= 0;)
  vgL.vGradient[i] = (vgL.vGradient[i] * vgR.Value -
                      vgL.Value * vgR.vGradient[i]) / (vgR.Value * vgR.Value);
 vgL.Value /= vgR.Value;
 return vgL;
}

/////////////////////////////////////////////////////////////////////////////
CValueWithGradient operator+(const CValueWithGradient &vgL,
                             const CValueWithGradient &vgR)
{
 CValueWithGradient vgResult = vgL;
 vgResult += vgR;
 return vgResult;
}

/////////////////////////////////////////////////////////////////////////////
CValueWithGradient operator-(const CValueWithGradient &vgL,
                             const CValueWithGradient &vgR)
{
 CValueWithGradient vgResult = vgL;
 vgResult -= vgR;
 return vgResult;
}

/////////////////////////////////////////////////////////////////////////////
CValueWithGradient operator*(const CValueWithGradient &vgL,
                             const CValueWithGradient &vgR)
{
 CValueWithGradient vgResult = vgL;
 vgResult *= vgR;
 return vgResult;
}

/////////////////////////////////////////////////////////////////////////////
CValueWithGradient operator/(const CValueWithGradient &vgL,
                             const CValueWithGradient &vgR)
{
 CValueWithGradient vgResult = vgL;
 vgResult /= vgR;
 return vgResult;
}

/////////////////////////////////////////////////////////////////////////////
std::ostream &operator<<(std::ostream &out, const CValueWithGradient &vg)
{
 out << vg.Value << " (";
 for (unsigned i = 0; i < vg.vGradient.size(); i++)
 {
  out << vg.vGradient[i];
  if (i + 1 < vg.vGradient.size())
   out << ' ';
 }
 out << ')';
 return out;
}
