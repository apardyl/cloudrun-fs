#ifndef CLOUDRUN_HASHFS_FILESTORE_H
#define CLOUDRUN_HASHFS_FILESTORE_H

#include "remotefsconnection.h"

class CachedFileStore {
    std::string private_cache_path;

    std::unordered_set<std::string> known_paths;
    std::mutex set_mutex;

    RemoteFSConnection * downloader;
public:
    CachedFileStore(std::string private_cache_path, RemoteFSConnection *downloader);

    int open_file(const std::string &path, int flags);

    void clear_cache();
};


#endif //CLOUDRUN_HASHFS_FILESTORE_H
