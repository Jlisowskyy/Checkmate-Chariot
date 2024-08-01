/////////////////////////////////////////////////////////////////////////////
//
// CObjectSelector.h
//
// Rémi Coulom
//
// February, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CObjectSelector_Declared
#define CObjectSelector_Declared

#include <map>
#include <string>
#include <iostream>

template<class T>
class  CObjectSelector // os
{
 private: ///////////////////////////////////////////////////////////////////
  typedef std::map<std::string, T*> CMap;
  typedef typename CMap::const_iterator CMapi;

  CMap m;
  CMapi mi;

 public: ////////////////////////////////////////////////////////////////////

  void Add(const std::string &s, T &t) {m[s] = &t; mi = m.find(s);}

  T &operator*() {return *mi->second;}

  const std::string &GetSelectedName() const {return mi->first;}

  bool Select(const std::string &s) {return Select(s.c_str());}

  bool Select(const char *pszParameters)
  {
   CMapi i = m.find(pszParameters);
   if (i != m.end())
   {
    mi = i;
    return true;
   }
   return false;
  }

  void ProcessCommand(const char *pszParameters, std::ostream &out)
  {
   if (Select(pszParameters))
    out << mi->first << '\n';
   else
    for (CMapi i = m.begin(); i != m.end(); i++)
    {
     out << i->first;
     if (i == mi)
      out << '*';
     out << '\n';
    }
  }
};

#endif
