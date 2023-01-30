#ifndef _DARK_NTT_H_
#define _DARK_NTT_H_

#include "number.h"

namespace dark {

/**
 * @brief Base data for big integer type int2048.
 * 
 */
class int_base {
  protected:
    int_base() = default;
    static constexpr int Mult_Min = 10;

};

}

#endif
