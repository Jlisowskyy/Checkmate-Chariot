#include <streambuf>

class CDoubleStreamBuffer : public streambuf
{
 private: ///////////////////////////////////////////////////////////////////
  streambuf &buf1;
  streambuf &buf2;

 public: ////////////////////////////////////////////////////////////////////
  CDoubleStreamBuffer(streambuf &bufInit1, streambuf &bufInit2) :
   buf1(bufInit1),
   buf2(bufInit2)
  {
  }

  virtual int sync()
  {
   buf2.sync();
   return buf1.sync();
  }

  virtual int overflow(int ch)
  {
   buf2.overflow(ch);
   return buf1.overflow(ch);
  }
};
