#ifndef HASHFS_FUSE_H
#define HASHFS_FUSE_H

#include "hashfs.h"
#include "cachefs.h"

#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 700
#define _FILE_OFFSET_BITS 64
#include <fuse.h>

int hfs_main(fuse_args args, HashFS *hashfs, CacheFS *cachedFS);

#endif //HASHFS_FUSE_H
