# Redis Custom Module

This is a fork of the RedisModulesSDK : https://github.com/RedisLabsModules/RedisModulesSDK

## What it includes:

### 1. redismodule.h

The only file you really need to start writing Redis modules. Either put this path into your module's include path, or copy it. 

Notice: This is an up-to-date copy of it from the Redis repo.

### 2. LibRMUtil 

A small library of utility functions and macros for module developers, including:

* Easier argument parsing for your commands.
* Testing utilities that allow you to wrap your module's tests as a redis command.
* `RedisModuleString` utility functions (formatting, comparison, etc)
* The entire `sds` string library, lifted from Redis itself.
* A generic scalable Vector library. Not redis specific but we found it useful.
* A few other helpful macros and functions.
* `alloc.h`, an include file that allows modules implementing data types to implicitly replace the `malloc()` function family with the Redis special allocation wrappers.

It can be found under the `rmutil` folder, and compiles into a static library you link your module against.    

### 3. An example Module

A minimal module implementing a few commands and demonstarting both the Redis Module API, and use of rmutils.

You can treat it as a template for your module, and extend its code and makefile.

**It includes 3 commands:**

* `EXAMPLE.PARSE` - demonstrating rmutil's argument helpers.
* `EXAMPLE.HGETSET` - an atomic HGET/HSET command, demonstrating the higher level Redis module API.
* `EXAMPLE.TEST` - a unit test of the above commands, demonstrating use of the testing utilities of rmutils.  
  
### 4. Documentation Files:

1. [API.md](API.md) - The official manual for writing Redis modules, copied from the Redis repo. 
Read this before starting, as it's more than an API reference.

2. [FUNCTIONS.md](FUNCTIONS.md) - Generated API reference documentation for both the Redis module API, and LibRMUtil.

3. [TYPES.md](TYPES.md) - Describes the API for creating new data structures inside Redis modules, 
copied from the Redis repo.

4. [BLOCK.md](BLOCK.md) - Describes the API for blocking a client while performing asynchronous tasks on a separate thread.


# Quick Start Guide

This module was built using a Mac, therefore a few additions have been made: 

```bash
example/MakeFile > $(LD) -o $@ module.o $(SHOBJ_LDFLAGS) $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -lc
↳ $(LD) -o $@ module.o $(SHOBJ_LDFLAGS) $(LIBS) -L$(RMUTIL_LIBDIR) -lrmutil -L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lc
```

If building on Linux please remove `-L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lc` and replace with `-lc`

- To Build please run: `make`

The MakeFile can automatically deploy the module onto redis-server, but this function assumes: 
-  `redis-server` is available on your cli
- [RedisJSON](https://github.com/RedisJSON/RedisJSON) module is compiled on your machine 

Please update the MakeFile (in root folder) to point out your RedisJSON module: 
 e.g. `redis-server --loadmodule ./module.so --loadmodule /usr/local/etc/redis-modules/librejson.dylib`

> **_NOTE:_**  use `librejson.dylib` for MacOS & `librejson.so` for linux

The MakeFile has a function called 'dev' which will automatically build and deploy the code to redis.

run `make dev`

```bash
bash-3.2$ make dev
/Library/Developer/CommandLineTools/usr/bin/make -C ./example
/Library/Developer/CommandLineTools/usr/bin/make -C ../rmutil
gcc -g -fPIC -O3 -std=gnu99 -Wall -Wno-unused-function -I../   -c -o util.o util.c
gcc -g -fPIC -O3 -std=gnu99 -Wall -Wno-unused-function -I../   -c -o strings.o strings.c
gcc -g -fPIC -O3 -std=gnu99 -Wall -Wno-unused-function -I../   -c -o sds.o sds.c
gcc -g -fPIC -O3 -std=gnu99 -Wall -Wno-unused-function -I../   -c -o vector.o vector.c
gcc -g -fPIC -O3 -std=gnu99 -Wall -Wno-unused-function -I../   -c -o alloc.o alloc.c
gcc -g -fPIC -O3 -std=gnu99 -Wall -Wno-unused-function -I../   -c -o periodic.o periodic.c
ar rcs librmutil.a util.o strings.o sds.o vector.o alloc.o periodic.o
gcc -I../ -Wall -g -fPIC -lc -lm -std=gnu99     -c -o module.o module.c
clang: warning: -lc: 'linker' input unused [-Wunused-command-line-argument]
clang: warning: -lm: 'linker' input unused [-Wunused-command-line-argument]
ld -o module.so module.o -bundle -undefined dynamic_lookup  -L../rmutil -lrmutil -L /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lc
cp ./example/module.so .
redis-server --loadmodule ./module.so --loadmodule /usr/local/etc/redis-modules/librejson.dylib
34222:C 09 Aug 2022 10:14:36.486 # oO0OoO0OoO0Oo Redis is starting oO0OoO0OoO0Oo
34222:C 09 Aug 2022 10:14:36.486 # Redis version=6.2.7, bits=64, commit=00000000, modified=0, pid=34222, just started
34222:C 09 Aug 2022 10:14:36.486 # Configuration loaded
34222:M 09 Aug 2022 10:14:36.487 * monotonic clock: POSIX clock_gettime
34222:M 09 Aug 2022 10:14:36.489 # A key '__redis__compare_helper' was added to Lua globals which is not on the globals allow list nor listed on the deny list.
                _._                                                  
           _.-``__ ''-._                                             
      _.-``    `.  `_.  ''-._           Redis 6.2.7 (00000000/0) 64 bit
  .-`` .-```.  ```\/    _.,_ ''-._                                  
 (    '      ,       .-`  | `,    )     Running in standalone mode
 |`-._`-...-` __...-.``-._|'` _.-'|     Port: 6379
 |    `-._   `._    /     _.-'    |     PID: 34222
  `-._    `-._  `-./  _.-'    _.-'                                   
 |`-._`-._    `-.__.-'    _.-'_.-'|                                  
 |    `-._`-._        _.-'_.-'    |           https://redis.io       
  `-._    `-._`-.__.-'_.-'    _.-'                                   
 |`-._`-._    `-.__.-'    _.-'_.-'|                                  
 |    `-._`-._        _.-'_.-'    |                                  
  `-._    `-._`-.__.-'_.-'    _.-'                                   
      `-._    `-.__.-'    _.-'                                       
          `-._        _.-'                                           
              `-.__.-'                                               

34222:M 09 Aug 2022 10:14:36.490 # Server initialized
34222:M 09 Aug 2022 10:14:36.706 * Module 'mvp' loaded from ./module.so
34222:M 09 Aug 2022 10:14:36.712 * <ReJSON> version: 999999 git sha: f2cbf6d branch: master
34222:M 09 Aug 2022 10:14:36.712 * <ReJSON> Exported RedisJSON_V1 API
34222:M 09 Aug 2022 10:14:36.712 * <ReJSON> Exported RedisJSON_V2 API
34222:M 09 Aug 2022 10:14:36.712 * <ReJSON> Enabled diskless replication
34222:M 09 Aug 2022 10:14:36.712 * <ReJSON> Created new data type 'ReJSON-RL'
34222:M 09 Aug 2022 10:14:36.712 * Module 'ReJSON' loaded from /usr/local/etc/redis-modules/librejson.dylib
34222:M 09 Aug 2022 10:14:36.713 * Loading RDB produced by version 6.2.7
34222:M 09 Aug 2022 10:14:36.713 * RDB age 20 seconds
34222:M 09 Aug 2022 10:14:36.713 * RDB memory usage when created 1.00 Mb
34222:M 09 Aug 2022 10:14:36.715 # Done loading RDB, keys loaded: 4, keys expired: 0.
34222:M 09 Aug 2022 10:14:36.715 * DB loaded from disk: 0.003 seconds
34222:M 09 Aug 2022 10:14:36.715 * Ready to accept connections
```

Now in a new terminal run `redis-cli`

```
127.0.0.1:6379> MVP.RAND
(integer) 989443365

127.0.0.1:6379> MVP.ECHO 'Hello World'
Hello World


# Add some test data
127.0.0.1:6379> LPUSH mvp:auth:id:abc123 'xxxx-yyyy-zzzz-1111'
(integer) 1
127.0.0.1:6379> LPUSH mvp:auth:id:abc123 'xxxx-yyyy-zzzz-2222'
(integer) 2
127.0.0.1:6379> LPUSH mvp:auth:id:abc123 'xxxx-yyyy-zzzz-3333'
(integer) 3

127.0.0.1:6379> JSON.SET 'mvp:trades:id:001' $ '{"meta": { "objectId":"/trades/trade-001", "accessId":"xxxx-yyyy-zzzz-1111" }, "content": { "fieldA":"hello-world", "fieldB":"goodbye-world" }}'
OK
127.0.0.1:6379> JSON.SET mvp:trades:id:002 $ '{"meta": { "objectId":"/trades/trade-002", "accessId":"xxxx-yyyy-zzzz-3333" }, "content": { "fieldA":"testing", "fieldB":"testing" }}'
OK
127.0.0.1:6379> JSON.SET mvp:trades:id:003 $ '{"meta": { "objectId":"/trades/trade-003", "accessId":"xxxx-yyyy-zzzz-XXXX" }, "content": { "fieldA":"unknown", "fieldB":"redacted" }}'
OK

# Test MVP.AUTH
127.0.0.1:6379> mvp.auth abc123 JSON.GET mvp:trades:id:001
"[{\"meta\":{\"objectId\":\"/trades/trade-001\",\"accessId\":\"xxxx-yyyy-zzzz-1111\"},\"content\":{\"fieldA\":\"hello-world\",\"fieldB\":\"goodbye-world\"}}]"

127.0.0.1:6379> mvp.auth abc123 JSON.GET mvp:trades:id:002
"[{\"meta\":{\"objectId\":\"/trades/trade-002\",\"accessId\":\"xxxx-yyyy-zzzz-3333\"},\"content\":{\"fieldA\":\"testing\",\"fieldB\":\"testing\"}}]"

127.0.0.1:6379> mvp.auth abc123 JSON.GET mvp:trades:id:003
(error) No access to requested Object
```

Enjoy!
    
