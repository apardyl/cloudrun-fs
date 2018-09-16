#ifndef CLOUDRUN_HASHFS_CACHEFS_H
#define CLOUDRUN_HASHFS_CACHEFS_H

#include "remotefsconnection.h"
#include "cachedfilestore.h"

class CacheFS {
private:
    std::unique_ptr<filesystem::Filesystem> fs;
    std::mutex fs_mutex;

    RemoteFSConnection *downloader;
    CachedFileStore *store;

    const filesystem::Node *getNode(const char *path);

    void fetch_node(const std::string &path, filesystem::Node *node);

public:
    CacheFS(RemoteFSConnection *downloader, CachedFileStore *store);

    int lstat(const char *path, struct stat *st);

    ssize_t readlink(const char *path, char *buf, size_t bufsiz);

    int getdir(const char *path, std::map<std::string, struct stat> *dirs);

    int open(const char *pathname, int flags);
};


#endif //CLOUDRUN_HASHFS_CACHEFS_H
