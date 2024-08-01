/////////////////////////////////////////////////////////////////////////////
//
// CMatrixOperations.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CMatrixOperations_Declared
#define CMatrixOperations_Declared

class CMatrixOperations
{
 public: ////////////////////////////////////////////////////////////////////
  static bool Cholesky(const double *vMatrix, double *vCholesky, int Size);
  static void Inverse(const double *vMatrix, double *vInverse, int Size);
  static void Solve(const double *vMatrix, double *v, int Size);
  static void Square(const double *vMatrix, double *vSquare, int Size);
  static void Jacobi(double *vMatrix, double *vEigenVectors, int Size);
  static void JacobiSort(const double *vMatrix, int *vOrder, int Size);
};

#endif
