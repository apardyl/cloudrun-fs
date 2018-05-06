#include "datastore.h"

#include <utility>
#include <fcntl.h>

int DataStore::open_hash(const std::string &hash, const std::string &real_path, int flags) {
    if (hash.size() < 8) {
        printf("Invalid hash %s\n", hash.c_str());
        errno = EINVAL;
        return -1;
    }
    std::string path = hash_to_path(hash);
    int res = open(path.c_str(), flags);
    if (res != -1 || errno != ENOENT) {
        return res;
    }
    if (hash_not_found(hash, real_path) == 0) {
        res = open(path.c_str(), flags);
        if (res == -1) {
            printf("ERROR: Unable to open file %s of hash %s after successful fetch\n", real_path.c_str(), hash.c_str());
            exit(1);
        }
        return res;
    } else {
        printf("No file for hash %s\n", hash.c_str());
        errno = ENOENT;
        return -1;
    }
}

DataStore::DataStore(std::string base_path) : base_path(std::move(base_path)) {
    if (base_path[base_path.size() - 1] == '/') {
        this->base_path.pop_back();
    }
}

int DataStore::hash_not_found(const std::string &hash, const std::string &real_path) {
    printf("File fetching not implemented, missing hash %s\n", hash.c_str());
    return -1;
}

inline std::string DataStore::hash_to_path(const std::string &hash) {
    return base_path + '/' + hash[0] + hash[1] + '/' + hash[2] + hash[3] + '/' + hash[4] + hash[5] + '/' + hash[6] +
           hash[7] + '/' + hash;
}
