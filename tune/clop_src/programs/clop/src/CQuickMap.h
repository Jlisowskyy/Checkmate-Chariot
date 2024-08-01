/////////////////////////////////////////////////////////////////////////////
//
// CQuickMap
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CQuickMap_Declared
#define CQuickMap_Declared

#include <map>

template<class C>
class CQuickMap // qm
{
 private: ////////////////////////////////////////////////////////////////////
  int Index;
  C *p;
  std::map<int, C*> m;

 public: /////////////////////////////////////////////////////////////////////
  CQuickMap(): p(0) {}

  bool empty() const
  {
   return m.empty();
  }

  void clear()
  {
   p = 0;
   m.clear();
  }

  void add(int i, C& c)
  {
   if (p)
    m[i] = &c;
   else
   {
    p = &c;
    Index = i;
   }
  }

  C *remove(int i)
  {
   if (p && i == Index)
   {
    C *pCopy = p;
    p = 0;
    return pCopy;
   }
   else
   {
    typename std::map<int, C*>::iterator mi = m.find(i);
    if (mi == m.end())
     return 0;
    else
    {
     return mi->second;
     m.erase(mi);
    }
   }
  }
};

#endif
