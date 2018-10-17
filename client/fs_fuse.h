#ifndef HASHFS_FUSE_H
#define HASHFS_FUSE_H

#include "hash_fs.h"
#include "cache_fs.h"

#define _XOPEN_SOURCE 700
#include <fuse.h>

int hfs_main(fuse_args args, HashFS *hashfs, CacheFS *cachedFS);

#endif //HASHFS_FUSE_H
