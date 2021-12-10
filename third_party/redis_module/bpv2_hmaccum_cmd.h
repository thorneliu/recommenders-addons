#ifndef BPV2_HMACCUM_CMD_H
#define BPV2_HMACCUM_CMD_H

#include "redismodule.h"

#define OP_SUCCESS 1
#define OP_FAILURE 0

#define BPV2_ERRORMSG_VALUETYPENOTSUPPORTED "Not supported valueType"

// COMMAND TYPE
#define BPV2_HMACCUM_CMD "HMACCUM"

// MODULE INFO
#define MODULE_NAME "BPV2"
#define MODULE_VERSION 1

enum valueType {
  DT_DOUBLE, /* double */
  DT_FLOAT,  /* float */
  DT_INT32,  /* int32 */
  DT_INT64,  /* int64 */
  DT_INT8,   /* int8  */
  DT_INVALID
};

#define ACCUMULATEVALE(value_dtype)                                  \
  {                                                                  \
    for (size_t i = 0; i < oldvalLen; i = i + sizeof(value_dtype)) { \
      value_dtype *pOld = (value_dtype *)(oldData + i);              \
      value_dtype *pDelta = (value_dtype *)(deltaData + i);          \
      (*pOld) = (*pOld) + (*pDelta);                                 \
    }                                                                \
    break;                                                           \
  }

#endif
