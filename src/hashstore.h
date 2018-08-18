#ifndef HASHFS_DATASTORE_H
#define HASHFS_DATASTORE_H

#include <string>
#include "remotefsconnection.h"

class HashStore {
private:
    std::string base_path;

    RemoteFSConnection * downloader;

    std::string hash_to_path(const std::string &hash);

public:
    explicit HashStore(std::string store_path, RemoteFSConnection *downloader);

    int open_hash(const std::string &hash, const std::string &real_path, int flags);
};


#endif //HASHFS_DATASTORE_H
