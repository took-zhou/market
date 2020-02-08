#ifndef STD_TYPES_H
#define STD_TYPES_H


// 标准C库文件
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include "log.h"

using namespace std;

/**********************************************************************************************************************
 *  GLOBAL FUNCTION MACROS
 *********************************************************************************************************************/
#define TESTBIT( operand, bit_mask )        (((operand) &  (bit_mask)) != ((bit_mask) - (bit_mask)))
#define SETBIT( operand, bit_mask )         ((operand) |= (bit_mask))
#define CLEARBIT( operand, bit_mask )       ((operand) &= (~(bit_mask)))
#define TOGGLEBIT( operand, bit_mask )      ((operand) ^= (bit_mask))

                          
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned long   uint32;
      
typedef signed char   int8;
typedef signed short  int16;
typedef signed long   int32;

typedef float                 float32;
typedef double                float64;
      
#endif
