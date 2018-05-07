#ifndef HASHFS_HASHFS_H
#define HASHFS_HASHFS_H

#include <sys/stat.h>
#include <fuse.h>
#include "proto/hashfs/filesystem.pb.h"
#include "datastore.h"

class HashFS {
private:
    std::unique_ptr<filesystem::Filesystem> fs;
    std::unique_ptr<DataStore> store;

    const filesystem::Node *getNode(const char *path);

    void lstat(const filesystem::Node *n, struct stat *st);

public:
    HashFS(std::unique_ptr<filesystem::Filesystem> fs, std::unique_ptr<DataStore> ds);

    int lstat(const char *path, struct stat *st);

    ssize_t readlink(const char *path, char *buf, size_t bufsiz);

    int getdir(const char *path, void *buf, fuse_fill_dir_t filler);

    int open(const char *pathname, int flags);
};


#endif //HASHFS_HASHFS_H
