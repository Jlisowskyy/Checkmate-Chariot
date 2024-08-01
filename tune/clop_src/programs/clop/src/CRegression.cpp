/////////////////////////////////////////////////////////////////////////////
//
// CRegression.cpp
//
// Rémi Coulom
//
// February, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CRegression.h"
#include "CLogistic.h"
#include "CDFLogP.h"
#include "CDFConfidence.h"
#include "CMatrixOperations.h"

/////////////////////////////////////////////////////////////////////////////
// Outcome model (missing multiplier for draws, should do 1 - w - l?)
/////////////////////////////////////////////////////////////////////////////
double CRegression::ResultProbability(double Rating, COutcome outcome) const
{
 switch(outcome)
 {
  case COutcome::Win:
   return CLogistic::f(Rating - DrawRating);
  case COutcome::Loss:
   return CLogistic::f(-Rating - DrawRating);
  case COutcome::Draw:
   return CLogistic::f(Rating - DrawRating) *
          CLogistic::f(-Rating - DrawRating);
 }
 return 1.0;
}

/////////////////////////////////////////////////////////////////////////////
// Compute win rate for a rating
/////////////////////////////////////////////////////////////////////////////
double CRegression::WinRate(double Rating) const
{
 double W = ResultProbability(Rating, COutcome::Win);
 double L = ResultProbability(Rating, COutcome::Loss);
 return W + 0.5 * (1.0 - W - L);
}

/////////////////////////////////////////////////////////////////////////////
double CRegression::Rating(double Rate) const
/////////////////////////////////////////////////////////////////////////////
{
 const int Iter = 30;
 double Max = 10.0;
 double Min = -10.0;
 double MaxRate = WinRate(Max);
 double MinRate = WinRate(Min);

 for (int i = Iter; --i >= 0;)
 {
  double Middle = (Max + Min) * 0.5;
  double MiddleRate = WinRate(Middle);
  if (Rate > MiddleRate)
  {
   Min = Middle;
   MinRate = MiddleRate;
  }
  else
  {
   Max = Middle;
   MaxRate = MiddleRate;
  }
 }

 return (Max + Min) * 0.5;
}

/////////////////////////////////////////////////////////////////////////////
// Compute probability of one sample
/////////////////////////////////////////////////////////////////////////////
double CRegression::GetProba(int k, const double *vParam) const
{
 return ResultProbability(pf.GetValue(vParam, GetSample(k)),
                          results.GetOutcome(k));
}

/////////////////////////////////////////////////////////////////////////////
// Compute LogP
/////////////////////////////////////////////////////////////////////////////
double CRegression::GetLogP(const double *vParam) const
{
 double Total = pf.GetPrior(vParam);

 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;
  Total += std::log(GetProba(k, vParam)) * vsd[k].Weight * vsd[k].GetCount();
 }

 return Total;
}

/////////////////////////////////////////////////////////////////////////////
// Compute all probabilities
/////////////////////////////////////////////////////////////////////////////
void CRegression::UpdateLogP()
{
 //
 // Prior
 //
 L = pf.GetPrior(&vParamMAP[0]);

 //
 // Probability of each sample game
 //
 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;

  const double r = pf.GetValue(&vParamMAP[0], GetSample(k));
  vsd[k].tProba[0] = CLogistic::f(-r - DrawRating);
  vsd[k].tProba[1] = CLogistic::f(+r - DrawRating);

  double tLogP[3];
  tLogP[COutcome::Loss] = std::log(vsd[k].tProba[0]);
  tLogP[COutcome::Win] = std::log(vsd[k].tProba[1]);
  tLogP[COutcome::Draw] = tLogP[COutcome::Loss] + tLogP[COutcome::Win];

  L += vsd[k].Weight * (tLogP[0] * vsd[k].tCount[0] +
                        tLogP[1] * vsd[k].tCount[1] +
                        tLogP[2] * vsd[k].tCount[2]);
 }

 State |= S_LogP;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Gradient
/////////////////////////////////////////////////////////////////////////////
void CRegression::ComputeGradient()
{
 EnsureState(S_LogP);

 pf.GetPriorGradient(&vParamMAP[0], &vGradient[0]);

 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;
  const double TotalMul = vsd[k].GetGradient();

  pf.GetMonomials(GetSample(k), &vMonomial[0]);
  for (int p = pf.GetParameters(); --p >= 0;)
   vGradient[p] += TotalMul * vMonomial[p];
 }

 State |= S_Gradient;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Hessian (opposite of)
/////////////////////////////////////////////////////////////////////////////
void CRegression::ComputeHessian()
{
 EnsureState(S_LogP);

 //
 // Prior
 //
 pf.GetPriorHessian(&vParamMAP[0], &vHessian[0]);

 //
 // Loop over samples
 //
 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;
  const double TotalMul = vsd[k].GetHessian();

  pf.GetMonomials(GetSample(k), &vMonomial[0]);
  for (int p1 = pf.GetParameters(); --p1 >= 0;)
   for (int p2 = pf.GetParameters(); --p2 >= p1;)
    vHessian[p1 * pf.GetParameters() + p2] += TotalMul *
                                              vMonomial[p1] *
                                              vMonomial[p2];
 }

 State |= S_Hessian;
}

/////////////////////////////////////////////////////////////////////////////
// Compute MAP with Newton's method
/////////////////////////////////////////////////////////////////////////////
void CRegression::Newton()
{
 CDFLogP func(*this);
 std::vector<double> v = vParamMAP;

 func.Newton(v);
 vParamMAP = v;
 func.GetOutput(&v[0]);

 State = S_MAP | S_LogP;
}

/////////////////////////////////////////////////////////////////////////////
// Compute or keep cache
/////////////////////////////////////////////////////////////////////////////
void CRegression::EnsureState(int Flags)
{
 while (Flags & ~State)
 {
  if (fDirty)
  {
   fDirty = false;
   State = 0;
  }

  if ((Flags & ~State) & S_MAP)
   Newton();

  if ((Flags & ~State) & S_LogP)
   UpdateLogP();

  if ((Flags & ~State) & S_Gradient)
   ComputeGradient();

  if ((Flags & ~State) & S_Hessian)
   ComputeHessian();

  if ((Flags & ~State) & S_Cholesky)
  {
   EnsureState(S_Hessian);
   fCholesky = CMatrixOperations::Cholesky(&vHessian[0],
                                           &vCholesky[0],
                                           pf.GetParameters());
   State |= S_Cholesky;
  }

  if ((Flags & ~State) & S_CholeskyInverse)
  {
   EnsureState(S_Cholesky);
   CMatrixOperations::Inverse(&vCholesky[0],
                              &vCholeskyInverse[0],
                              pf.GetParameters());
   State |= S_CholeskyInverse;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CRegression::CRegression(CResults &results,
                         const CParametricFunction &pf):
 CObserver(results),
 pf(pf),
 DrawRating(0),
 vParamMAP(pf.GetParameters()),
 vMonomial(pf.GetParameters()),
 vGradient(pf.GetParameters()),
 vHessian(pf.GetParameters() * pf.GetParameters()),
 vCholesky(pf.GetParameters() * pf.GetParameters()),
 vCholeskyInverse(pf.GetParameters() * pf.GetParameters()),
 RefreshRate(0.0),
 LocalizationHeight(3.0),
 LocalizationPower(0.0),
 MaxWeightIterations(0),
 vTotalWeightedSample(pf.GetDimensions())
{
 OnReset();
}

/////////////////////////////////////////////////////////////////////////////
// Outcome model
/////////////////////////////////////////////////////////////////////////////
void CRegression::SetDrawRating(double r)
{
 DrawRating = r;
 State = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Reset
/////////////////////////////////////////////////////////////////////////////
void CRegression::OnReset()
{
 Samples = results.GetSamples();
 if (int(vsd.size()) < Samples)
  vsd.resize(Samples);

 std::fill(tCount, tCount + COutcome::Outcomes, 0);
 for (int i = Samples; --i >= 0;)
  tCount[results.GetOutcome(i)]++;

 RefreshCounter = 0;
 NextRefresh = 0;

 SetUniformWeights();
 pf.GetPriorParam(&vParamMAP[0]);

 State = 0;
 fDirty = false;
 fAutoLocalize = true;
}

/////////////////////////////////////////////////////////////////////////////
// OnSample
/////////////////////////////////////////////////////////////////////////////
void CRegression::OnSample()
{
 Samples = results.GetSamples();
 if (int(vsd.size()) < Samples)
  vsd.resize(Samples);

 const int k = Samples - 1;

 if (k > 0 &&
     std::equal(GetSample(k - 1),
                GetSample(k - 1) + pf.GetDimensions(),
                GetSample(k)))
 {
  vsd[k] = vsd[k - 1];
  vsd[k].Replications++;
 }
 else
 {
  vsd[k].Weight = GetWeight(GetSample(k));
  vsd[k].Index = k;
  vsd[k].Replications = 1;
  vsd[k].tCount[0] = 0;
  vsd[k].tCount[1] = 0;
  vsd[k].tCount[2] = 0;
 }

 AddWeightedSample(k, 1);
}

/////////////////////////////////////////////////////////////////////////////
// OnOutcome
/////////////////////////////////////////////////////////////////////////////
void CRegression::OnOutcome(int i)
{
 //
 // Store this new outcome
 //
 fDirty = true;
 const COutcome outcome = results.GetOutcome(i);
 tCount[outcome]++;
 if (outcome < 3)
  vsd[vsd[i].Index].tCount[outcome]++;

 //
 // Handle refreshment
 //
 if (RefreshCounter == NextRefresh)
 {
  State = 0;
  fDirty = false;
  NextRefresh += 1 + int(NextRefresh * RefreshRate);

  if (fAutoLocalize)
   ComputeLocalWeights();
 }

 RefreshCounter++;
}

/////////////////////////////////////////////////////////////////////////////
// Sample a model of the posterior at random (Gaussian approximation)
/////////////////////////////////////////////////////////////////////////////
void CRegression::GaussianSample(CRandom<unsigned> &rnd,
                                 std::vector<double> &vParam)
{
 EnsureState(S_MAP | S_CholeskyInverse);

 //
 // Vector of N(0,1) iid values
 //
 std::vector<double> vIID(pf.GetParameters());
 for (int i = pf.GetParameters(); --i >= 0;)
  vIID[i] = rnd.NextGaussian();

 //
 // Multiply by inverse of Cholesky of opposite of Hessian
 //
 vParam = vParamMAP;
 for (int i = pf.GetParameters(); --i >= 0;)
  for (int j = i; j < pf.GetParameters(); j++)
   vParam[i] += vIID[j] * vCholeskyInverse[i * pf.GetParameters() + j];
}

/////////////////////////////////////////////////////////////////////////////
// Sample a model of the posterior at random (MCMC)
/////////////////////////////////////////////////////////////////////////////
void CRegression::MCMCSample(CRandom<unsigned> &rnd,
                             std::vector<double> &vParam,
                             int n)
{
 OnRefresh();
 EnsureState(S_MAP | S_LogP | S_CholeskyInverse);

 vParam = vParamMAP;
 double LogCurrent = L;

 for (int k = 0; k < n; k++)
 {
  //
  // Vector of N(0,1) iid values
  //
  std::vector<double> vIID(pf.GetParameters());
  for (int i = pf.GetParameters(); --i >= 0;)
   vIID[i] = rnd.NextGaussian();

  //
  // Multiply by inverse of Cholesky of opposite of Hessian
  //
  std::vector<double> vParamNew = vParamMAP;
  for (int i = pf.GetParameters(); --i >= 0;)
   for (int j = i; j < pf.GetParameters(); j++)
    vParamNew[i] += vIID[j] * vCholeskyInverse[i * pf.GetParameters() + j];

  //
  // Metropolis-Hastings accept / reject
  //
  double LogNew = GetLogP(&vParamNew[0]);
  if (k == 0 || rnd.NextDouble() < std::exp(LogNew - LogCurrent))
  {
   vParam = vParamNew;
   LogCurrent = LogNew;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Log weight for CSPWeight
/////////////////////////////////////////////////////////////////////////////
double CRegression::GetLogWeight(const double *vx) const
{
 double lw = 0.0;

 for (int i = int(vWeighting.size()); --i >= 0;)
 {
  const double v_i = pf.GetValue(&vWeighting[i].vParam[0], vx);
  const double lw_i = (v_i - vWeighting[i].Mean) / vWeighting[i].Radius;
  if (lw_i < lw)
   lw = lw_i; 
 }

 return lw;
}

/////////////////////////////////////////////////////////////////////////////
// Compute weight at one location
/////////////////////////////////////////////////////////////////////////////
double CRegression::GetWeight(const double *vx) const
{
 return std::exp(GetLogWeight(vx));
}

/////////////////////////////////////////////////////////////////////////////
// Reset TotalWeight and vTotalWeightedSample
/////////////////////////////////////////////////////////////////////////////
void CRegression::ResetWeightedSample()
{
 std::fill(vTotalWeightedSample.begin(), vTotalWeightedSample.end(), 0.0);
 TotalWeight = 0.0;
}

/////////////////////////////////////////////////////////////////////////////
// Add one sample to TotalWeight and vTotalWeightedSample
/////////////////////////////////////////////////////////////////////////////
void CRegression::AddWeightedSample(int k, int Count)
{
 double Weight = vsd[k].Weight * Count;
 TotalWeight += Weight;
 for (int i = pf.GetDimensions(); --i >= 0;)
  vTotalWeightedSample[i] += Weight * GetSample(k)[i];
}

/////////////////////////////////////////////////////////////////////////////
// Update all weights after a change of the weighting function
/////////////////////////////////////////////////////////////////////////////
void CRegression::UpdateWeights()
{
 ResetWeightedSample();
 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;
  vsd[k].Weight = GetWeight(GetSample(k));
  AddWeightedSample(k, vsd[k].GetCount());
 }
}

/////////////////////////////////////////////////////////////////////////////
// One iteration of localization
/////////////////////////////////////////////////////////////////////////////
void CRegression::GetMeanAndDeviation(double &Mean, double &Deviation)
{
 //
 // Initialize weighted counts
 //
 double tWC[3];
 std::fill(tWC, tWC + 3, 0.0);
 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;
  tWC[0] += vsd[k].Weight * vsd[k].tCount[0];
  tWC[1] += vsd[k].Weight * vsd[k].tCount[1];
  tWC[2] += vsd[k].Weight * vsd[k].tCount[2];
 }

 //
 // MAP of a constant value
 //
 const double GammaD = std::exp(-DrawRating);
 const double Delta =
  (tWC[1] - tWC[0]) * (tWC[1] - tWC[0]) +
  4 * GammaD * GammaD * (tWC[0] + tWC[2]) * (tWC[1] + tWC[2]);
 const double Gamma = (tWC[1] - tWC[0] + std::sqrt(Delta)) /
  (2 * GammaD * (tWC[0] + tWC[2]));
 Mean = std::log(Gamma);
 
 //
 // Deviation of this estimate
 //
 const double PWin = CLogistic::f(Mean - DrawRating);
 const double PLoss = CLogistic::f(-Mean - DrawRating);
 const double x = (tWC[1] + tWC[2]) * PWin * (1.0 - PWin) +
                  (tWC[0] + tWC[2]) * PLoss * (1.0 - PLoss);
 Deviation = std::sqrt(1.0 / x);
}

/////////////////////////////////////////////////////////////////////////////
// One iteration of localization
/////////////////////////////////////////////////////////////////////////////
void CRegression::LocalizationIteration()
{
 if (LocalizationHeight > 0 &&
     tCount[COutcome::Loss] > 1 &&
     tCount[COutcome::Win] > 1)
 {
  //
  // Find Mean and Deviation
  //
  double Mean;
  double Deviation;
  GetMeanAndDeviation(Mean, Deviation);

  //
  // Update weighting function
  //
  EnsureState(S_MAP);
  CWeighting weighting;
  weighting.vParam = vParamMAP;
  weighting.Radius = Deviation *
                     LocalizationHeight *
                     std::pow(double(Samples), LocalizationPower);
  weighting.Mean = Mean;
  vWeighting.push_back(weighting);

  UpdateWeights();
  State = 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Set uniform weights
/////////////////////////////////////////////////////////////////////////////
void CRegression::SetUniformWeights()
{
 vWeighting.resize(0);

 ResetWeightedSample();
 for (int k = Samples; --k >= 0;)
 {
  k = vsd[k].Index;
  vsd[k].Weight = 1.0;
  AddWeightedSample(k, vsd[k].GetCount());
 }

 std::fill(vParamMAP.begin(), vParamMAP.end(), 0.0);

 State = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Local Weights
/////////////////////////////////////////////////////////////////////////////
void CRegression::ComputeLocalWeights()
{
 SetUniformWeights();
 double PreviousTotalWeight;

 int Iterations = 0;
 do
 {
  PreviousTotalWeight = TotalWeight;
  LocalizationIteration();
  Iterations++;
  if (MaxWeightIterations && Iterations >= MaxWeightIterations)
   break;
 }
 while (TotalWeight < PreviousTotalWeight * 0.99);

 if (MaxWeightIterations == 0 && vWeighting.size() > 0)
 {
  vWeighting.pop_back();
  UpdateWeights();
 }
}

/////////////////////////////////////////////////////////////////////////////
// Estimate posterior distribution at any point
/////////////////////////////////////////////////////////////////////////////
void CRegression::GetPosteriorInfo(const double *vLocation,
                                   double &Rating,
                                   double &Variance)
{
 Rating = pf.GetValue(&vParamMAP[0], vLocation);
 CDFConfidence dfconf(*this);
 dfconf.ComputeVariance(vLocation);
 Variance = dfconf.GetVariance();
}

/////////////////////////////////////////////////////////////////////////////
// Parameter value divided by deviation
// Probability of being positive: 0.5 * (1.0 + erf(std::sqrt(h * 0.5) * x))
/////////////////////////////////////////////////////////////////////////////
double CRegression::GetParamPositivity(int i)
{
 EnsureState(S_MAP | S_Hessian);
 double x = vParamMAP[i];
 double h = vHessian[i * (pf.GetParameters() + 1)];
 return std::sqrt(h) * x;
}
