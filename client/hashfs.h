#ifndef HASHFS_HASHFS_H
#define HASHFS_HASHFS_H

#include <sys/stat.h>
#include "common/proto/filesystem.pb.h"
#include "hashstore.h"

class HashFS {
private:
    std::unique_ptr<filesystem::Filesystem> fs;
    HashStore *store;

    const filesystem::Node *getNode(const char *path, bool *  noent_authoritative);

public:
    HashFS(std::unique_ptr<filesystem::Filesystem> fs, HashStore *ds);

    int lstat(const char *path, struct stat *st);

    ssize_t readlink(const char *path, char *buf, size_t bufsiz);

    int getdir(const char *path, std::map<std::string, struct stat> *dirs);

    int open(const char *pathname, int flags);
};


#endif //HASHFS_HASHFS_H
