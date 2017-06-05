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


## Compatibility notice
The source code is compliant to the ANSI C11 standard.
