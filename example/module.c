#include "../redismodule.h"
#include "../rmutil/util.h"
#include "../rmutil/strings.h"
#include "../rmutil/test_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char meta_access_id[] = "$.meta.accessId";
static const char root_id[] = "$";

/* Return true if the users accessId list contains the obj id */
int isObjectAcessible(RedisModuleCtx *ctx, RedisModuleCallReply *accessIds,
                      RedisModuleCallReply *object_aid) {

  if (RedisModule_CallReplyType(accessIds) != REDISMODULE_REPLY_ARRAY) {
    return RedisModule_ReplyWithError(ctx, "Requested auth key doesnt exist");
  }

  const char *buff = RedisModule_CallReplyStringPtr(object_aid, NULL);
  char obj_aid[20];
  memcpy(obj_aid, &buff[2], 19);
  obj_aid[19] = '\0';
  // printf("Checking if user can access key: %s \n", obj_aid);

  RedisModuleString *mystr;
  int items = RedisModule_CallReplyLength(accessIds);

  for (int j = 0; j < items; j++) {
    RedisModuleCallReply *ele = RedisModule_CallReplyArrayElement(accessIds, j);
    mystr = RedisModule_CreateStringFromCallReply(ele);
    const char *val = RedisModule_StringPtrLen(mystr, NULL);

    // printf("Access ID : %s vs %s \n", val, obj_aid);
    char *pch = strstr(val, obj_aid);
    if (pch) {
      return 1;
    }
  }
  // printf("No match... \n");
  return 0;
}

// Command that pre-authorises object access for JSON.GET
// MVP.AUTH <AUTH_KEY> <COMMAND> <PAYLOAD>
int MVPAuth_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

  // Test number of arguments
  if (argc < 4) {
    return RedisModule_WrongArity(ctx);
  }

  // https://redis.io/docs/reference/modules/modules-api-ref/#RedisModule_AutoMemory
  RedisModule_AutoMemory(ctx);

  // Build the key for to obtain the requestsed accessIds
  size_t l1;
  const char *k = RedisModule_StringPtrLen(argv[1], &l1);
  char authKey[32] = "mvp:auth:id:";
  strcat(authKey, k);

  // Get all access Keys
  printf("Using AUTH Key: %s \n", authKey);
  RedisModuleCallReply *accessIds = RedisModule_Call(ctx, "LRANGE", "cll", authKey, 0, -1);
  if (RedisModule_CallReplyType(accessIds) != REDISMODULE_REPLY_ARRAY) {
    return RedisModule_ReplyWithError(ctx, "Requested auth key doesnt exist");
  }
 
  // Check key exists
  RedisModuleKey *key = RedisModule_OpenKey(ctx, argv[2], REDISMODULE_READ | REDISMODULE_WRITE);
  if (RedisModule_KeyType(key) != REDISMODULE_KEYTYPE_EMPTY) {
    return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
  }

  // get the current value of the auth
  const char *c = RedisModule_StringPtrLen(argv[2], &l1);
  const char *a = RedisModule_StringPtrLen(argv[3], NULL);
  printf("Using command %s & args: %s \n", c, a);
  RedisModuleCallReply *objAccessId = RedisModule_Call(ctx, c, "cc", a, meta_access_id);
  
  // Check the accessId exists
  if (RedisModule_CallReplyType(objAccessId) == REDISMODULE_REPLY_ERROR || 
      RedisModule_CallReplyType(objAccessId) == REDISMODULE_REPLY_NULL) {
      return RedisModule_ReplyWithError(ctx, "Requested object does not have an accessId");
  }

  // Check access to object is eligible
  if (!isObjectAcessible(ctx, accessIds, objAccessId)) {
    return RedisModule_ReplyWithError(ctx, "No access to requested Object");
  }

  // return response
  RedisModuleCallReply *object = RedisModule_Call(ctx, c, "cc", a, root_id);
  RedisModule_ReplyWithCallReply(ctx, object);
  return REDISMODULE_OK;
}

// MVP.RAND - Simple command that returns a random long
int MVPRand_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  RedisModule_ReplyWithLongLong(ctx, rand());
  return REDISMODULE_OK;
}

// MVP.ECHO <GREETING> - Simple command that prints to console
int MVPGreeting_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {

  // Test number of arguments
  if (argc != 2) {
    return RedisModule_WrongArity(ctx);
  }

  size_t l1;
  RedisModule_ReplyWithSimpleString(ctx, RedisModule_StringPtrLen(argv[1], &l1));
  return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (RedisModule_Init(ctx, "mvp", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  if (RedisModule_CreateCommand(ctx, "mvp.rand", MVPRand_RedisCommand, "fast random", 0, 0, 0) ==
      REDISMODULE_ERR)
    return REDISMODULE_ERR;

  if (RedisModule_CreateCommand(ctx, "mvp.echo", MVPGreeting_RedisCommand, "fast readonly", 0, 0,
                                0) == REDISMODULE_ERR)
    return REDISMODULE_ERR;

  if (RedisModule_CreateCommand(ctx, "mvp.auth", MVPAuth_RedisCommand, "fast readonly", 0, 0, 0) ==
      REDISMODULE_ERR)
    return REDISMODULE_ERR;

  return REDISMODULE_OK;
}
