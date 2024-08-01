/////////////////////////////////////////////////////////////////////////////
//
// CSamplingPolicy.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSamplingPolicy_Declared
#define CSamplingPolicy_Declared

class CSamplingPolicy // sp
{
 public: ////////////////////////////////////////////////////////////////////
  virtual void Seed(unsigned n) {}
  virtual void Reserve(unsigned n) {}
  virtual const double *NextSample(int i) = 0;
  virtual bool IsFlexible() const {return true;}
  virtual ~CSamplingPolicy() {}
};

#endif
