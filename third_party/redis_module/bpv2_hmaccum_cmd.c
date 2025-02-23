#include <string.h>
#include "bpv2_hmaccum_cmd.h"

enum valueType getValueType(RedisModuleString *typeStr) {
  size_t len = 0;
  const char *type = RedisModule_StringPtrLen(typeStr, &len);
  if (0 == strcmp(type, "float")) return DT_FLOAT;
  if (0 == strcmp(type, "double")) return DT_DOUBLE;
  if (0 == strcmp(type, "int32")) return DT_INT32;
  if (0 == strcmp(type, "int64")) return DT_INT64;
  if (0 == strcmp(type, "int8")) return DT_INT8;

  return DT_INVALID;
}

int TensorValueAccump(RedisModuleCtx *ctx, RedisModuleString *old,
                      RedisModuleString *delta, enum valueType type) {
  size_t oldvalLen = 0, deltaLen = 0;
  const char *oldData = RedisModule_StringPtrLen(old, &oldvalLen);
  const char *deltaData = RedisModule_StringPtrLen(delta, &deltaLen);

  if (oldvalLen != deltaLen) {
    RedisModule_Log(ctx, "warning",
                    "mismatched tensor shape oldvalLen = %ld, delta = %ld",
                    oldvalLen, deltaLen);
    return OP_FAILURE;
  }

  switch (type) {
    case DT_FLOAT:
      ACCUMULATEVALE(float)
    case DT_DOUBLE:
      ACCUMULATEVALE(double)
    case DT_INT32:
      ACCUMULATEVALE(int32_t)
    case DT_INT64:
      ACCUMULATEVALE(int64_t)
    case DT_INT8:
      ACCUMULATEVALE(char)
    default: {
      RedisModule_Log(ctx, "warning", "not supported value type");
    }
  }

  return OP_SUCCESS;
}

int CustomHmaccumCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                         int argc) {
  /* Use automatic memory management */
  RedisModule_AutoMemory(ctx);

  /* we need 2 * n + 3 argument, where n is the number of <K,V> pairs */
  if (argc < 3 || argc % 2 == 0) return RedisModule_WrongArity(ctx);

  RedisModuleKey *key =
      RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
  int keyType = RedisModule_KeyType(key);
  if (keyType != REDISMODULE_KEYTYPE_HASH &&
      keyType != REDISMODULE_KEYTYPE_EMPTY) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  size_t n = (argc - 3) / 2, i = 0;
  enum valueType value_dtype = getValueType(argv[2]);
  if (DT_INVALID == value_dtype) {
    RedisModule_Log(ctx, "warning", "not supported valueType");
    return RedisModule_ReplyWithError(ctx, BPV2_ERRORMSG_VALUETYPENOTSUPPORTED);
  }

  RedisModuleString *field, *value_or_delta, *oldval;
  for (; i < n; i++) {
    field = argv[2 * i + 3];
    value_or_delta = argv[2 * i + 4];
    RedisModule_HashGet(key, REDISMODULE_HASH_NONE, field, &oldval, NULL);
    if (oldval) {
      int rc = TensorValueAccump(ctx, oldval, value_or_delta, value_dtype);
      if (rc)
        RedisModule_HashSet(key, REDISMODULE_HASH_NONE, field, oldval, NULL);
    } else {
      RedisModule_HashSet(key, REDISMODULE_HASH_NONE, field, value_or_delta,
                          NULL);
    }
  }

  RedisModule_ReplyWithLongLong(ctx, i);
  return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv,
                       int argc) {
  REDISMODULE_NOT_USED(argv);
  REDISMODULE_NOT_USED(argc);

  if (RedisModule_Init(ctx, MODULE_NAME, MODULE_VERSION,
                       REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }

  return RedisModule_CreateCommand(ctx, BPV2_HMACCUM_CMD, CustomHmaccumCommand,
                                   "readonly", 1, 1, 1);
}