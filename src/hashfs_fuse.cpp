#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 700

#include <fuse.h>
#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>

#include "hashfs_fuse.h"
#include "hashfs.h"
#include "config.h"

static HashFS *fs;

static int hfs_getattr(const char *path, struct stat *stbuf) {
    int res;
    debug_print("getattr %s\n", path);

    res = fs->lstat(path, stbuf);
    if (res == -1) {
        return -errno;
    }

    return 0;
}

static int hfs_readlink(const char *path, char *buf, size_t size) {
    ssize_t res;
    debug_print("readlink %s\n", path);

    res = fs->readlink(path, buf, size - 1);
    if (res == -1) {
        return -errno;
    }

    buf[res] = '\0';
    return 0;
}

static int hfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    debug_print("readdir %s\n", path);

    (void) offset;
    (void) fi;

    int res = fs->getdir(path, buf, filler);
    if (res == -1) {
        return -errno;
    }

    return 0;
}

static int hfs_open(const char *path, struct fuse_file_info *fi) {
    debug_print("open %s\n", path);

    int fd;

    fd = fs->open(path, fi->flags);
    if (fd == -1) {
        return -errno;
    }

    fi->fh = static_cast<uint64_t>(fd);
    return 0;
}

static int hfs_release(const char *path, struct fuse_file_info *fi) {
    (void) path;
    close(static_cast<int>(fi->fh));
    return 0;
}

static int hfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    debug_print("read %s\n", path);

    int res;

    (void) path;
    res = static_cast<int>(pread(static_cast<int>(fi->fh), buf, size, offset));
    if (res == -1)
        res = -errno;

    return res;
}

int hfs_main(fuse_args args, HashFS *hashfs) {
    fs = hashfs;

    umask(0);
    fuse_operations hfs_operations{};
    hfs_operations.getattr = hfs_getattr;
    hfs_operations.readlink = hfs_readlink;
    hfs_operations.readdir = hfs_readdir;
    hfs_operations.open = hfs_open;
    hfs_operations.read = hfs_read;
    hfs_operations.release = hfs_release;
    return fuse_main(args.argc, args.argv, &hfs_operations, nullptr);
}