////////////////////////////////////////////////////////////////////////////
//
// lsort.h
//
// CListSort class template definition
// no very nice, but works
//
// Remi Coulom
//
// July 1996
//
////////////////////////////////////////////////////////////////////////////
#include "list.h"

template<class T>
class CListSort // lst
{
 private: //////////////////////////////////////////////////////////////////
  int (*Compare)(const T &t1, const T &t2);

  CListCell<T> *ListMerge(CListCell<T> *pcell1, CListCell<T> *pcell2);
  CListCell<T> *ListSortBis(CListCell<T> *pcell1, int Size);

 public: ///////////////////////////////////////////////////////////////////
  CListSort(CList<T> &lt, int (*Comp)(const T &t1, const T &t2));
};

////////////////////////////////////////////////////////////////////////////
// Function to merge to sublists
////////////////////////////////////////////////////////////////////////////
template<class T>
CListCell<T> *CListSort<T>::ListMerge(CListCell<T> *pcell1,
                                      CListCell<T> *pcell2)
{
 if (!pcell1)
  return pcell2;

 if (!pcell2)
  return pcell1;

 if (Compare(pcell1->Value(), pcell2->Value()) < 0)
 {
  pcell1->Link(ListMerge(pcell1->Next(), pcell2));
  return pcell1;
 }
 else
 {
  pcell2->Link(ListMerge(pcell1, pcell2->Next()));
  return pcell2;
 }
}

////////////////////////////////////////////////////////////////////////////
// Secondary function to sort a list
////////////////////////////////////////////////////////////////////////////
template<class T>
CListCell<T> *CListSort<T>::ListSortBis(CListCell<T> *pcell1, int Size)
{
 if (Size < 2)
  return pcell1;

 //
 // Division of the list in two halves of the same size
 //
 int s1 = Size / 2;
 int s2 = Size - s1;

 CListCell<T> *pcell2;
 {
  CListCell<T> *pcell = pcell1;

  for (int i = s1; --i > 0;)
   pcell = pcell->Next();
  pcell2 = pcell->Next();
  pcell->Link(0);
 }

 //
 // Two recursive calls to sort the two sublists
 //
 pcell1 = ListSortBis(pcell1, s1);
 pcell2 = ListSortBis(pcell2, s2);

 //
 // The resulting list are merged
 //
 return ListMerge(pcell1, pcell2);
}

////////////////////////////////////////////////////////////////////////////
// Main function to sort a list
////////////////////////////////////////////////////////////////////////////
template<class T>
CListSort<T>::CListSort(CList<T> &lt, int (*Comp)(const T &t1, const T &t2))
{
 Compare = Comp;
 int Size = lt.GetSize();
 lt.pcellFirst = ListSortBis(lt.pcellFirst, Size);
}
