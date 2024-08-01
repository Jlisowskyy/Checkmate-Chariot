/////////////////////////////////////////////////////////////////////////////
//
// CSampleData
//
// Rémi Coulom
//
// February, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSampleData_Declared
#define CSampleData_Declared

struct CSampleData // sd
{
 double Weight;
 double tProba[2];
 int Replications;
 int Index;
 int tCount[3];

 double GetGradient() const
 {
  return Weight *
   (tCount[0] * (tProba[0] - 1.0) +
    tCount[1] * (1.0 - tProba[1]) +
    tCount[2] * (tProba[0] - tProba[1]));
 }

 double GetHessian() const
 {
  return Weight *
   ((tCount[0] + tCount[2]) * tProba[0] * (1.0 - tProba[0]) +
    (tCount[1] + tCount[2]) * tProba[1] * (1.0 - tProba[1]));
 }

 int GetCount() const
 {
  return tCount[0] + tCount[1] + tCount[2];
 }
};

#endif
