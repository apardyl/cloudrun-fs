#define FUSE_USE_VERSION 26
#define _XOPEN_SOURCE 700

#include <fuse.h>
#include <cstdio>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>

#include "fs_fuse.h"
#include "hashfs.h"
#include "config.h"
#include "cachefs.h"

static HashFS *hfs;
static CacheFS *cfs;

static int hfs_getattr(const char *path, struct stat *stbuf) {
    int res;
    debug_print("getattr %s\n", path);

    res = hfs->lstat(path, stbuf);
    if (res == -2) {
        res = cfs->lstat(path, stbuf);
    }
    if (res == -1) {
        return -errno;
    }
    return 0;
}

static int hfs_readlink(const char *path, char *buf, size_t size) {
    ssize_t res;
    debug_print("readlink %s\n", path);

    res = hfs->readlink(path, buf, size - 1);
    if (res == -2) {
        res = cfs->readlink(path, buf, size - 1);
    }
    if (res == -1) {
        return -errno;
    }
    buf[res] = '\0';
    return 0;
}

static int hfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
    int res;
    debug_print("readdir %s\n", path);

    (void) offset;
    (void) fi;

    std::map<std::string, struct stat> dirs;
    res = hfs->getdir(path, &dirs);
    if (res == -2) {
        res = cfs->getdir(path, &dirs);
    }
    if (res == -1) {
        return -errno;
    }
    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);
    for (auto &d : dirs) {
        filler(buf, d.first.c_str(), &d.second, 0);
    }

    return 0;
}

static int hfs_open(const char *path, struct fuse_file_info *fi) {
    debug_print("open %s\n", path);

    if ((fi->flags & (O_WRONLY | O_RDWR)) != 0) {
        errno = EACCES;
        return -1;
    }

    int fd;

    fd = hfs->open(path, fi->flags);
    if (fd == -2) {
        fd = cfs->open(path, fi->flags);
    }
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

int hfs_main(fuse_args args, HashFS *hashfs, CacheFS *cachedFS) {
    hfs = hashfs;
    cfs = cachedFS;

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
