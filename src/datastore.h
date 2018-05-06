#ifndef HASHFS_DATASTORE_H
#define HASHFS_DATASTORE_H

#include <string>

class DataStore {
private:
    std::string base_path;
    int hash_not_found(const std::string &hash, const std::string &real_path);
    std::string hash_to_path(const std::string& hash);
public:
    explicit DataStore(std::string base_path);

    int open_hash(const std::string &hash, const std::string& real_path, int flags);
};


#endif //HASHFS_DATASTORE_H
