#ifndef _block
#define _block

#include <iostream>

#include "global.h"

using namespace std;

struct Block {
  BYTE_T	*data;
  SIZE_T 	length;
  double        lastaccessed;  // for use in buffercache only
  bool          dirty;         // for use in buffercahce only

  Block();
  Block(const SIZE_T size);
  Block(const Block &rhs);
  Block(const char *data);
  virtual ~Block();
  Block & operator=(const Block &rhs);

  // returns one of ERROR_NOERROR (zero)
  // ERROR_NOMEM or other nonzero error code.
  ERROR_T Resize(const SIZE_T newlength, const bool copy=true);

  bool operator<(const Block &rhs) const;
  bool operator==(const Block &rhs) const;

  ostream & Print(ostream &os) const;
};

inline ostream & operator<<(ostream &os, const Block &b) { return b.Print(os);}


#endif
