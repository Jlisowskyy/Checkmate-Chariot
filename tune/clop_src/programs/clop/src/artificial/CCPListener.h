/////////////////////////////////////////////////////////////////////////////
//
// CCPListener.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CCPListener_Declared
#define CCPListener_Declared

class CCheckPointData;

class CCPListener // cpl
{
 public: ////////////////////////////////////////////////////////////////////
  virtual void OnStart(const CCheckPointData &cpd) {}
  virtual void OnCollect(const CCheckPointData &cpd) {}
  virtual void OnStop(const CCheckPointData &cpd) {}
  virtual bool Continue() const {return true;}
  virtual ~CCPListener() {}
};

#endif
