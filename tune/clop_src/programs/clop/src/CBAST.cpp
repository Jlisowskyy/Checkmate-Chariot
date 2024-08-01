/////////////////////////////////////////////////////////////////////////////
//
// CBAST.cpp
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CBAST.h"
#include "CResults.h"
#include "debug.h"

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CBAST::CBAST(CResults &results, double Exploration):
 CObserver(results),
 area(results.GetDimensions()),
 Dimensions(results.GetDimensions()),
 Exploration(Exploration),
 pnodeRoot(0)
{
 OnReset();
}

/////////////////////////////////////////////////////////////////////////////
// NextSample
/////////////////////////////////////////////////////////////////////////////
const double *CBAST::NextSample(int i)
{
 CNode *pnode = pnodeRoot;
 area.Reset();

 while (true)
 {
  pnode->Games++;

  if (pnode->tpnodeChild[0])
  {
   const double LogT = std::log(double(pnode->Games)) * 0.5;

   const double v0 = pnode->tpnodeChild[0]->Victories + 1;
   const double n0 = pnode->tpnodeChild[0]->Games + 2;
   const double v1 = pnode->tpnodeChild[1]->Victories + 1;
   const double n1 = pnode->tpnodeChild[1]->Games + 2;

   if (n1 * (v0 + Exploration * std::sqrt(LogT * n0)) >
       n0 * (v1 + Exploration * std::sqrt(LogT * n1)))
    pnode = area.Follow(*pnode, 0);
   else
    pnode = area.Follow(*pnode, 1);
  }
  else
  {
   if (pnode->outcome == COutcome::Unknown)
   {
    pnode->outcome = COutcome::InProgress;
    qmInProgress.add(i, *pnode);
    return &area.vCenter[0];
   }

   CreateChildren(*pnode);
   pnode = area.Follow(*pnode, 1);
   continue;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// MaxParameter
/////////////////////////////////////////////////////////////////////////////
bool CBAST::MaxParameter(double vMax[]) const
{
 CNode *pnode = pnodeRoot;
 area.Reset();

 while (pnode->Victories &&
        pnode->tpnodeChild[0] &&
        pnode->tpnodeChild[0]->Games &&
        pnode->tpnodeChild[1]->Games &&
        (pnode->tpnodeChild[0]->Victories + pnode->tpnodeChild[1]->Victories))
 {
  double v0 = pnode->tpnodeChild[0]->Victories;
  double n0 = pnode->tpnodeChild[0]->Games;
  double v1 = pnode->tpnodeChild[1]->Victories;
  double n1 = pnode->tpnodeChild[1]->Games;

  if (v0 * n1 > v1 * n0)
   pnode = area.Follow(*pnode, 0);
  else
   pnode = area.Follow(*pnode, 1);
 }

 std::copy(area.vCenter.begin(), area.vCenter.end(), vMax);

 return true;
}

/////////////////////////////////////////////////////////////////////////////
// OnOutcome
/////////////////////////////////////////////////////////////////////////////
void CBAST::OnOutcome(int i)
{
 CNode *pnode = qmInProgress.remove(i);
 FATAL(!pnode);
 FATAL(pnode->outcome != COutcome::InProgress);

 COutcome outcome = results.GetOutcome(i);
 pnode->outcome = outcome;

 if (outcome == COutcome::Win)
  do
  {
   pnode->Victories++;
   pnode = pnode->pnodeParent;
  }
  while (pnode);
}

/////////////////////////////////////////////////////////////////////////////
// OnReset()
/////////////////////////////////////////////////////////////////////////////
void CBAST::OnReset()
{
 nodepool.Reset();
 pnodeRoot = &nodepool.Allocate();
 pnodeRoot->Depth = 0;
 pnodeRoot->Dim = 0;
 pnodeRoot->pnodeParent = 0;
 pnodeRoot->outcome = COutcome::InProgress;

 qmInProgress.clear();
}

/////////////////////////////////////////////////////////////////////////////
// CNode constructor
/////////////////////////////////////////////////////////////////////////////
void CBAST::CNode::Reset()
{
 outcome = COutcome::Unknown;
 Games = 0;
 Victories = 0;
 tpnodeChild[0] = 0;
 tpnodeChild[1] = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Create node children
/////////////////////////////////////////////////////////////////////////////
void CBAST::CreateChildren(CNode &node)
{
 for (int i = 2; --i >= 0;)
 {
  CNode &nodeNew = nodepool.Allocate();
  node.tpnodeChild[i] = &nodeNew;
  nodeNew.pnodeParent = &node;
  nodeNew.Depth = node.Depth + 1;
  nodeNew.Dim = nodeNew.Depth % Dimensions;
 }
}
