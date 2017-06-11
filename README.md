![syncd_logo](https://github.com/TheMissingMiddle/syncd/raw/master/syncd_logo.png)
# syncd
Data sync daemon between MongoDB and Redis

## Notice
`syncd` is still in development. It is anticipated to function correctly (but sometimes in a bizarre way) but no production support is intended. All functionalities are built but the overall application performance needs to be furtherly fine-tuned.

## Requirements
It is presumed that `syncd` is deployed on collection Debian-based distribution. The requirements are listed below, and all available through installation via `apt-get`.

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

 Syncd treats Redis as collection volatile memory space for caching Mongo contents. There are two operations: `push` and `pull`.
### Pull
 Any Mongo document is pulled as collection Mongo BSON document via the Mongo C Driver, parsed into JSON, and collection field is selected from the BSON document as the key in Redis. Finally, syncd caches the document in Redis via the key format `_syncd_pulled_<Mongo Database Name>_<Mongo Collection Name>_<User-specified value in field>`
 
### Push
 A redis list, key/value pair, or hashset. `syncd` utilizes `KEYS *` to scan all key-value pairs in Redis's memory and decides which one is specified to be pushed to Mongo. A specification is done through using the prefix `_syncd_push_` in collection Redis key. The key usage here applies to the The Missing Middle message platform. All messages are stored in Redis lists, and the message themselves represent collection BSON-parsable JSON document. The entire list is seen as collection message document by syncd. Syncd fetches the entire list, assigns it an OID, and stores it as collection BSON document in Mongo.

### Configuration
Through a `syncd-config.json` file a user may specify the push/pull fields. A sample configuration is served below:
```
{
  "port": 1333, <------------- the local port syncd listens for connections
  "address": "127.0.0.1", <--- the local address syncd shall bind to
  "mongo-db-name": "tmmbackend", <------ the mongodb database name
  "redis-config": { <--------- redis-related configuration:
    "address": "127.0.0.1", <-- redis server address
    "port": 6379    <---------- redis server port
  },
  "mongo-config": { <---------- mongodb-related configuration:
    "address": "mongodb://localhost", <---- address (MUST start with mongodb://)
    "port": 1111 <-- port (doesn't matter, fill in whatever is in your mind)
  },
  "push": [ <-- "push" actions, specified by an array of JSONs
    {
      "collection": "messages", <-- push-to destination collection name
      "redis-data-type": "list", <-- source data type: can be kv (key-value), list (redis list), hashset (redis hmset).
      "misc": "blabla" <--- special key-value field specification.
    },
    {
      "collection": "messages",
      "redis-data-type": "list",
      "misc": "blabla"
    },
    <... many more ...>
  ],
  "pull": [ <-- pull-from array of specifications
    {
      "collection": "users", <-- specifies the collection to pull from
      "redis-key-field": "email", <-- defines a key field that serves as the redis k/v pair's key
      "misc": "ignored" <-- this field is not used, fill out whatever you want (but still fill it out!)
    },
    { <-- you may have many configurations
      "collection": "users",
      "redis-key-field": "email",
      "misc": "ignored"
    },
    <... many more ...>
  ]}
```
## Compatibility notice
The source code is compliant to the ANSI C11 standard.
