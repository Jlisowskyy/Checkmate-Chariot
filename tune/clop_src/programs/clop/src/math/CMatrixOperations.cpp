/////////////////////////////////////////////////////////////////////////////
//
// CMatrixOperations.cpp
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CMatrixOperations.h"

#include <cmath>
#include <algorithm>

static const double CholeskyThreshold = 1e-10;
static const double JacobiThreshold = 1e-10;
static const int MaxJacobiSweeps = 10;

/////////////////////////////////////////////////////////////////////////////
// Cholesky:
//  - Writes the lower triangle
//  - vMatrix and vCholesky may be the same matrix
//  - Return false if failure
//  - The results C is such that vMatrix = CC'
/////////////////////////////////////////////////////////////////////////////
bool CMatrixOperations::Cholesky(const double *vMatrix,
                                 double *vCholesky,
                                 int Size)
{
 for (int i = 0; i < Size; i++)
  for (int j = i; j < Size; j++)
  {
   double Sum = vMatrix[i * Size + j];

   for (int k = i; --k >= 0;)
    Sum -= vCholesky[i * Size + k] * vCholesky[j * Size + k];

   if (i == j)
    if (Sum < CholeskyThreshold)
     return false;
    else
     vCholesky[i * Size + i] = std::sqrt(Sum);
   else
    vCholesky[j * Size + i] = Sum / vCholesky[i * Size + i];
  }

 return true;
}

/////////////////////////////////////////////////////////////////////////////
// Compute the whole inverse of Cholesky
//  - Write upper triangle
//  - vMatrix and vInverse may be the same matrix
/////////////////////////////////////////////////////////////////////////////
void CMatrixOperations::Inverse(const double *vMatrix,
                                double *vInverse,
                                int Size)
{
 for (int i = 0; i < Size; i++)
 {
  vInverse[i * Size + i] = 1.0 / vMatrix[i * Size + i];
  for (int j = i + 1; j < Size; j++)
  {
   double Sum = 0.0;
   for (int k = i; k < j; k++)
    Sum -= vMatrix[j * Size + k] * vInverse[i * Size + k];
   vInverse[i * Size + j] = Sum / vMatrix[j * Size + j];
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Solve equation after Cholesky decomposition
/////////////////////////////////////////////////////////////////////////////
void CMatrixOperations::Solve(const double * restrict vMatrix,
                              double * restrict v,
                              int Size)
{
 for (int i = 0; i < Size; i++)
 {
  for (int j = i; --j >= 0;)
   v[i] -= v[j] * vMatrix[i * Size + j];
  v[i] /= vMatrix[i * Size + i];
 }
 for (int i = Size; --i >= 0;)
 {
  for (int j = i + 1; j < Size; j++)
   v[i] -= v[j] * vMatrix[j * Size + i];
  v[i] /= vMatrix[i * Size + i];
 }
}

/////////////////////////////////////////////////////////////////////////////
// Compute the square of an upper triangle
//  - Write upper triangle
//  - vMatrix and vSquare may not be the same matrix
/////////////////////////////////////////////////////////////////////////////
void CMatrixOperations::Square(const double * restrict vMatrix,
                               double * restrict vSquare,
                               int Size)
{
 for (int i = 0; i < Size; i++)
  for (int j = 0; j <= i; j++)
  {
   int n;
   if (i > j)
    n = j;
   else
    n = i;

   double Sum = 0.0;
   for (int k = n + 1; --k >= 0;)
    Sum += vMatrix[k * Size + i] * vMatrix[k * Size + j];

   vSquare[j * Size + i] = Sum;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Jacobi transformation of a symmetric matrix
// Input:
//  - vMatrix is a symmetric matrix (lower triangle).
// Output:
//  - the diagonal of vMatrix is made of eigenvalues
//  - vEigenVectors is an orthonormal matrix of eigenvectors
/////////////////////////////////////////////////////////////////////////////
void CMatrixOperations::Jacobi(double * restrict vMatrix,
                               double * restrict vEigenVectors,
                               int Size)
{
 //
 // Initialize vEigenVectors to the identity matrix
 //
 std::fill(vEigenVectors, vEigenVectors + Size * Size, 0.0);
 for (int i = Size; --i >= 0;)
  vEigenVectors[i * (Size + 1)] = 1.0;

 //
 // Loop over Jacobi sweeps
 //
 for (int Sweep = 0; Sweep < MaxJacobiSweeps; Sweep++)
 {
  int Rotations = 0;

  for (int p = Size; --p >= 0;)
   for (int q = Size; --q > p;)
   {
    //
    // Don't rotate if already small enough
    //
    const double a_qp = vMatrix[q * Size + p];
    if (a_qp * a_qp < JacobiThreshold)
     continue;
    else
     Rotations++;

    //
    // Determine rotation angle
    //
    const double a_pp = vMatrix[p * (Size + 1)];
    const double a_qq = vMatrix[q * (Size + 1)];

    double theta = 0.5 * (a_qq - a_pp) / a_qp;
    double t;
    if (theta < 0)
     t = -1.0 / (-theta + std::sqrt(theta * theta + 1.0));
    else
     t = 1.0 / (theta + std::sqrt(theta * theta + 1.0));
    double c = 1.0 / std::sqrt(1.0 + t * t);
    double s = t * c;

    //
    // Right-multiply vEigenVectors by the rotation matrix
    //
    for (int r = Size; --r >= 0;)
    {
     double e_rp = vEigenVectors[Size * r + p];
     double e_rq = vEigenVectors[Size * r + q];
     vEigenVectors[Size * r + p] = c * e_rp - s * e_rq;
     vEigenVectors[Size * r + q] = s * e_rp + c * e_rq;
    }

    //
    // Transform vMatrix accordingly
    //
    {
     //
     // First, deal with intersections of lines and columns
     //
     vMatrix[q * Size + p] = 0.0;
     vMatrix[p * (Size + 1)] -= t * a_qp;
     vMatrix[q * (Size + 1)] += t * a_qp;

     //
     // Then, three loops over the rest
     //
     for (int r = 0; r < p; r++)
     {
      double a_pr = vMatrix[p * Size + r];
      double a_qr = vMatrix[q * Size + r];
      vMatrix[p * Size + r] = c * a_pr - s * a_qr;
      vMatrix[q * Size + r] = s * a_pr + c * a_qr;
     }

     for (int r = p + 1; r < q; r++)
     {
      double a_rp = vMatrix[r * Size + p];
      double a_qr = vMatrix[q * Size + r];
      vMatrix[r * Size + p] = c * a_rp - s * a_qr;
      vMatrix[q * Size + r] = s * a_rp + c * a_qr;
     }

     for (int r = q + 1; r < Size; r++)
     {
      double a_rp = vMatrix[r * Size + p];
      double a_rq = vMatrix[r * Size + q];
      vMatrix[r * Size + p] = c * a_rp - s * a_rq;
      vMatrix[r * Size + q] = s * a_rp + c * a_rq;
     }
    }
   }

  if (Rotations == 0)
   break;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Class for sorting eigenvalues
/////////////////////////////////////////////////////////////////////////////
class CJacobiComparator
{
 private: ///////////////////////////////////////////////////////////////////
  const double *vMatrix;
  int Size;

 public: ////////////////////////////////////////////////////////////////////
  CJacobiComparator(const double *vMatrix, int Size):
   vMatrix(vMatrix),
   Size(Size)
  {
  }

  bool operator()(int i, int j) const
  {
   return vMatrix[i * (Size + 1)] < vMatrix[j * (Size + 1)];
  }
};

/////////////////////////////////////////////////////////////////////////////
// Used for ordering eigenvalues
/////////////////////////////////////////////////////////////////////////////
void CMatrixOperations::JacobiSort(const double *vMatrix, int *vOrder, int Size)
{
 for (int i = Size; --i >= 0;)
  vOrder[i] = i;

 std::sort(vOrder, vOrder + Size, CJacobiComparator(vMatrix, Size));
}
