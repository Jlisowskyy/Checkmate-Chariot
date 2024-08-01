/////////////////////////////////////////////////////////////////////////////
//
// COutcome.h
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef COutcome_Declared
#define COutcome_Declared

class COutcome // outcome
{
 public: ////////////////////////////////////////////////////////////////////
  enum Value {Loss = 0,
              Win = 1,
              Draw = 2,
              Outcomes = 3,
              Unknown = 3,
              InProgress = 4,
              AllOutcomes = 5};

  COutcome() {}
  COutcome(Value v): v(v) {}

  bool operator==(Value vx) const {return v == vx;}
  bool operator!=(Value vx) const {return v != vx;}
#ifndef SWIG
  operator int() const {return v;}
#endif

 private: ///////////////////////////////////////////////////////////////////
  Value v;
};

#endif
