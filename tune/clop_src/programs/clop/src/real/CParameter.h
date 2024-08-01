/////////////////////////////////////////////////////////////////////////////
//
// CParameter.h
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CParameter_Declared
#define CParameter_Declared

#include <string>

class CParameter // param
{
 private: //////////////////////////////////////////////////////////////////
  const std::string sName;

 public: ///////////////////////////////////////////////////////////////////
  explicit CParameter(const std::string &s): sName(s) {}

  const std::string &GetName() const {return sName;}

  virtual double TransformToQLR(double x) const = 0;
  virtual double TransformFromQLR(double x) const = 0;

  virtual ~CParameter() {}
};

#endif
