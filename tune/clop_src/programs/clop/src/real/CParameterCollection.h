/////////////////////////////////////////////////////////////////////////////
//
// CParameterCollection.h
//
// Rémi Coulom
//
// August, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CParameterCollection_Declared
#define CParameterCollection_Declared

#include <vector>

class CParameter;

class CParameterCollection
{
 private: ///////////////////////////////////////////////////////////////////
  std::vector<const CParameter *> vpparam;
  mutable std::vector<double> vTranslation;

 public: ////////////////////////////////////////////////////////////////////
  void Add(const CParameter &param);
  const double *Translate(const double *v) const;
  int GetSize() const {return vpparam.size();}
  const CParameter &GetParam(int i) const {return *vpparam[i];}
};

#endif
