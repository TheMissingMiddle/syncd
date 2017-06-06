# syncd
Data sync daemon between MongoDB and Redis

## Notice
Still in early development. Should be able to sync user data in the next commit or so.

## Requirements
It is presumed that `syncd` is deployed on a Debian-based distribution. The requirements are listed below, and all available through installation via `apt-get`.

**Requirements**:

**UPDATE**: The `mongoc` driver version installed through `apt-get` is too old. Consider [compiling from source](http://mongoc.org) if necessary.
```
 libmongoc0.10
 libmongoc-dev
 libbson0.10
 libbson-dev
 libhiredis0.13
 libhiredis-dev
 pkg-config
 cmake
```

`mongoc` and `bson` are required for MongoDB connectivity. A raw tarball can be found at [mongoc website](http://mongoc.org). `hiredis` is required for redis connectivity. `hiredis` can be found at [hiredis GitHub repository](https://github.com/redis/hiredis).

## How syncd works

 Syncd treats Redis as a volatile memory space for caching Mongo contents. There are two operations: `push` and `pull`.
### Pull
 Any Mongo document is pulled as a Mongo BSON document via the Mongo C Driver, parsed into JSON, and a field is selected from the BSON document as the key in Redis. Finally, syncd caches the document in Redis via the key format `_syncd_pulled_<Mongo Database Name>_<Mongo Collection Name>_<User-specified value in field>`
 
## Push
 A redis list, key/value pair, or hashset. `syncd` utilizes `KEYS *` to scan all key-value pairs in Redis's memory and decides which one is specified to be pushed to Mongo. A specification is done through using the prefix `_syncd_push_` in a Redis key. The key usage here applies to the The Missing Middle message platform. All messages are stored in Redis lists, and the message themselves represent a BSON-parsable JSON document. The entire list is seen as a message document by syncd. Syncd fetches the entire list, assigns it an OID, and stores it as a BSON document in Mongo.


## Compatibility notice
The source code is compliant to the ANSI C11 standard.
