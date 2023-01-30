#ifndef _DARK_NUMBER_CPP_
#define _DARK_NUMBER_CPP_

#include "number.h"
#if NUMBER_MODE == FFT_MODE
#include "FFT.cc"
#else
#include "NTT.cc"
#endif


namespace dark {





}

#endif
