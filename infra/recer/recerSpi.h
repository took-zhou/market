#ifndef __MARKETRECERSPI__
#define __MARKETRECERSPI__
#include <vector>
#include "common/self/utils.h"

struct RecerSpi {
 public:
  virtual bool receMsg(utils::ItpMsg &msg) = 0;
};

#endif