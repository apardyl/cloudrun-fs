#ifndef HASHFS_FUSE_H
#define HASHFS_FUSE_H

#include "hashfs.h"
#include "cachefs.h"

int hfs_main(fuse_args args, HashFS *hashfs, CacheFS *cachedFS);

#endif //HASHFS_FUSE_H
