#ifndef HASHFS_DATASTORE_H
#define HASHFS_DATASTORE_H

#include <string>
#include "filerequester.h"

class HashStore {
private:
    std::string base_path;

    FileRequester * downloader;

    std::string hash_to_path(const std::string &hash);

public:
    explicit HashStore(std::string store_path, FileRequester *downloader);

    int open_hash(const std::string &hash, const std::string &real_path, int flags);
};


#endif //HASHFS_DATASTORE_H
