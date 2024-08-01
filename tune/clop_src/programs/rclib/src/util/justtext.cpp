////////////////////////////////////////////////////////////////////////////
//
// justtext.cpp
//
// CJustifiedText
//
// Remi Coulom
//
// August 1996
//
////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <cstring>

#include "debug.h"
#include "justtext.h"

////////////////////////////////////////////////////////////////////////////
// CJustifiedText constructor
////////////////////////////////////////////////////////////////////////////
CJustifiedText::CJustifiedText(std::ostream &ostr,
                               int LineLengthInit,
                               int IndentInit) :
 LineLength(LineLengthInit),
 Indent(IndentInit)
{
 ASSERT(LineLengthInit > 0 && LineLengthInit < MaxTextLine);
 Size = 0;
 postr = &ostr;
 szBuffer[0] = 0;
}

////////////////////////////////////////////////////////////////////////////
// CJustifiedText Flush member function
////////////////////////////////////////////////////////////////////////////
void CJustifiedText::Flush(void)
{
 *postr << szBuffer << '\n';
 postr->flush();
 Size = 0;
 szBuffer[0] = 0;
}

////////////////////////////////////////////////////////////////////////////
// CJustifiedText output operator for strings
////////////////////////////////////////////////////////////////////////////
CJustifiedText& operator<<(CJustifiedText &jt, const std::string &s)
{
 return jt << s.c_str();
}

CJustifiedText& operator<<(CJustifiedText &jt, const char *psz)
{
 //
 // No white spaces are allowed at the beginning of the Buffer
 //
 if (jt.Size == 0)
  while (*psz == ' ')
   psz++;

 //
 // The string is added to the buffer
 //
 strcpy(jt.szBuffer + jt.Size, psz);
 jt.Size += int(strlen(psz));

 //
 // If it is longer than LineLength, then the line is cut at the first space
 //
 while (jt.Size > jt.LineLength)
 {
  int l = jt.LineLength;
  while (jt.szBuffer[l] != ' ' && l > 0) l--;
  while (jt.szBuffer[l] == ' ' && l > 0) l--; ++l;

  //
  // A space has been found and the line can be cut
  //
  if (jt.szBuffer[l] == ' ')
  {
   jt.szBuffer[l] = 0;
   *jt.postr << jt.szBuffer << '\n';
   for (int i = jt.Indent; --i >= 0;)
    *jt.postr << ' ';
   while (jt.szBuffer[++l] == ' ') {}
   jt.Size -= l;
   {
    for (int i = 0; i <= jt.Size; i++)
     jt.szBuffer[i] = jt.szBuffer[i + l];
   }
  }

  //
  // Otherwise, the full line is printed
  //
  else
   jt.Flush();
 }

 return jt;
}

////////////////////////////////////////////////////////////////////////////
// CJustifiedText output operator for integers
////////////////////////////////////////////////////////////////////////////
CJustifiedText& operator<<(CJustifiedText &jt, int i)
{
 std::ostringstream oss;
 oss << i << '\0';
 jt << oss.str();

 return jt;
}
