/////////////////////////////////////////////////////////////////////////////
//
// CRegression.h
//
// Rémi Coulom
//
// February, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRegression_Declared
#define CRegression_Declared

#include <vector>

#include "CObserver.h"
#include "CParametricFunction.h"
#include "random.h"
#include "CResults.h"
#include "CSampleData.h"

class CRegression: private CObserver // reg
{
 friend class CDFLogP;
 friend class CDFVariance;

 private: ///////////////////////////////////////////////////////////////////
  //
  // Class parameters
  //
  const CParametricFunction &pf;

  //
  // Sample data
  //
  int Samples;
  mutable std::vector<CSampleData> vsd;
  int tCount[COutcome::Outcomes];

  //
  // Draw rating
  //
  double DrawRating;

  //
  // Regression data
  //
  std::vector<double> vParamMAP;
  std::vector<double> vMonomial;

  double L;
  std::vector<double> vGradient;
  std::vector<double> vHessian;         // upper triangle
  std::vector<double> vCholesky;        // lower triangle
  std::vector<double> vCholeskyInverse; // upper triangle
  bool fCholesky;

  //
  // State management
  //
  bool fAutoLocalize;
  bool fDirty;
  int State;
  enum {S_LogP            = 0x01,
        S_Gradient        = 0x02,
        S_Hessian         = 0x04,
        S_Cholesky        = 0x08,
        S_CholeskyInverse = 0x10,
        S_MAP             = 0x20,
        S_All             = 0x3f};

  void EnsureState(int Flags);

  //
  // Some functions to compute cached data
  //
  void UpdateLogP();
  void ComputeGradient();
  void ComputeHessian();
  void Newton();

  //
  // Control of cache refresh
  //
  double RefreshRate;
  int RefreshCounter;
  int NextRefresh;

  //
  // LogP functions
  //
  double GetProba(int k, const double *vParam) const;
  double GetLogP(const double *vParam) const;

  //
  // Weighting
  //
  double LocalizationHeight;
  double LocalizationPower;
  int MaxWeightIterations;

  struct CWeighting
  {
   std::vector<double> vParam;
   double Radius;
   double Mean;
  };
  std::vector<CWeighting> vWeighting;

  double TotalWeight;
  std::vector<double> vTotalWeightedSample;

  void ResetWeightedSample();
  void AddWeightedSample(int k, int Count);

  void UpdateWeights();

  //
  // Override of CObserver
  //
  void OnReset();
  void OnSample();
  void OnOutcome(int i);
  void OnRefresh()
  {
   if (fDirty)
   {
    State = 0;
    fDirty = false;
   }
   ComputeLocalWeights();
  }

 public: ////////////////////////////////////////////////////////////////////
  CRegression(CResults &results, const CParametricFunction &pf);

  const CParametricFunction &GetPF() const {return pf;}

  //
  // Outcome model
  //
  void SetDrawRating(double r);
  double ResultProbability(double Rating, COutcome outcome) const;
  double WinRate(double Rating) const;
  double Rating(double WinRate) const;

  //
  // Refresh rate
  //
  void SetRefreshRate(double x) {RefreshRate = x;}
  void Refresh() {OnRefresh();}

  //
  // Obtain posterior samples
  //
  const double *MAP() {EnsureState(S_MAP); return &vParamMAP[0];}
  void GaussianSample(CRandom<unsigned> &rnd, std::vector<double> &vParam);
  void MCMCSample(CRandom<unsigned> &rnd, std::vector<double> &vParam, int n);
  double GetParamPositivity(int i);
  void GetPosteriorInfo(const double *vLocation,
                        double &Rating,
                        double &Variance);

  //
  // Access to sample data
  //
  void Reserve(unsigned n) const {vsd.resize(n);}
  int GetSamples() const {return Samples;}
  const double *GetSample(int i) const {return results.GetSample(i);}
  int GetReplications(int i) const {return vsd[i].Replications;}
  const CSampleData &GetSampleData(int i) const {return vsd[i];}
  COutcome GetOutcome(int i) const {return results.GetOutcome(i);}
  int GetCount(COutcome o) const {return tCount[o];}

  //
  // Weighting
  //
  double GetLocalizationHeight() const {return LocalizationHeight;}
  double GetLocalizationPower() const {return LocalizationPower;}
  int GetMaxWeightIterations() const {return MaxWeightIterations;}
  void SetLocalizationHeight(double h) {LocalizationHeight = h;}
  void SetLocalizationPower(double p) {LocalizationPower = p;}
  void SetMaxWeightIterations(int M) {MaxWeightIterations = M;}

  void SetAutoLocalize(bool f) {fAutoLocalize = f;}
  void SetUniformWeights();
  void LocalizationIteration();
  void ComputeLocalWeights();

  void GetMeanAndDeviation(double &Mean, double &Deviation);

  double GetSampleWeight(int i) const {return vsd[vsd[i].Index].Weight;}
  double GetLogWeight(const double *vx) const;
  double GetWeight(const double *vx) const;
  double GetTotalWeight() const {return TotalWeight;}
  const double *GetTotalWeightedSample() const
  {
   return &vTotalWeightedSample[0];
  }
  void GetAverageSample(double *vx) const
  {
   for (int i = pf.GetDimensions(); --i >= 0;)
    vx[i] = vTotalWeightedSample[i] / TotalWeight;
  }
};

#endif
