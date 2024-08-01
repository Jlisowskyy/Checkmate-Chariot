////////////////////////////////////////////////////////////////////////////
//
// justtext.h
//
// CJustifiedText class definition
// a really ugly class
//
// Remi Coulom
//
// August 1996
//
////////////////////////////////////////////////////////////////////////////
#ifndef JUSTTEXT_H
#define JUSTTEXT_H

#include <iosfwd>
#include <string>

class CJustifiedText;

CJustifiedText& operator<<(CJustifiedText &jt, const std::string &s);
CJustifiedText& operator<<(CJustifiedText &jt, const char *psz);
CJustifiedText& operator<<(CJustifiedText &jt, int i);

class CJustifiedText // jt
{
 friend
  CJustifiedText& operator<<(CJustifiedText &jt, const char *psz);

 private: //////////////////////////////////////////////////////////////////
  enum {MaxTextLine = 2048};

  const int LineLength;
  const int Indent;
  std::ostream *postr;
  char szBuffer[MaxTextLine];
  int Size;

 public: ///////////////////////////////////////////////////////////////////
  explicit CJustifiedText(std::ostream &ostr,
                          int LineLengthInit = MaxTextLine,
                          int IndentInit = 0);
  ~CJustifiedText() {Flush();};
  void Flush(void);
};

#endif
