/////////////////////////////////////////////////////////////////////////////
//
// CBAST.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CBAST_Declared
#define CBAST_Declared

#include "CSamplingPolicy.h"
#include "CMaxEstimator.h"
#include "CObserver.h"
#include "COutcome.h"
#include "CQuickMap.h"

#include <deque>
#include <vector>
#include <algorithm>

class CBAST: // bast
 public CSamplingPolicy,
 public CMaxEstimator,
 private CObserver
{
 private: ///////////////////////////////////////////////////////////////////
  struct CNode // node
  {
   CNode *tpnodeChild[2];
   CNode *pnodeParent;

   int Depth;
   int Dim;

   COutcome outcome;

   int Games;
   int Victories;

   void Reset();
  };

  class CNodePool
  {
   private:
    std::deque<CNode> dqnode;
    std::deque<CNode>::iterator i;

   public:
    CNodePool() {Reset();}
    void Reset() {i = dqnode.begin();}
    void Reserve(unsigned n) {dqnode.resize(2 * n);}

    CNode &Allocate()
    {
     if (i == dqnode.end())
      i = dqnode.insert(i, CNode());
     i->Reset();
     return *i++;
    }

  } nodepool;

  void CreateChildren(CNode &node);

 private: ///////////////////////////////////////////////////////////////////
  mutable struct CArea
  {
   std::vector<double> vCenter;
   std::vector<double> vRadius;

   explicit CArea(int n): vCenter(n), vRadius(n) {Reset();}

   void Reset()
   {
    std::fill(vCenter.begin(), vCenter.end(), 0.0);
    std::fill(vRadius.begin(), vRadius.end(), 1.0);
   }

   CNode *Follow(const CNode &node, int i)
   {
    const int Dim = node.Dim;
    vRadius[Dim] *= 0.5;
    vCenter[Dim] += vRadius[Dim] * (2 * i - 1);
    return node.tpnodeChild[i];
   }
  } area;

 private: ///////////////////////////////////////////////////////////////////
  const int Dimensions;
  const double Exploration;

  CNode *pnodeRoot;
  CQuickMap<CNode> qmInProgress;

  //
  // CObserver
  //
  void OnOutcome(int i);
  void OnReset();

 public: ////////////////////////////////////////////////////////////////////
  explicit CBAST(CResults &results, double Exploration = 1.0);

  //
  // CSamplingPolicy
  //
  const double *NextSample(int i);
  bool IsFlexible() const {return false;}

  //
  // CMaxEstimator
  //
  bool MaxParameter(double vMax[]) const;
  void Reserve(unsigned n) {nodepool.Reserve(n);}
};

#endif
