#include "hashstore.h"

#include <utility>
#include <fcntl.h>

int HashStore::open_hash(const std::string &hash, const std::string &real_path, int flags) {
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
    if (downloader->fetch_file(real_path, path) == 0) {
        res = open(path.c_str(), flags);
        if (res == -1) {
            printf("ERROR: Unable to open file %s of hash %s after successful fetch\n", real_path.c_str(),
                   hash.c_str());
            exit(1);
        }
        return res;
    } else {
        printf("No file for hash %s\n", hash.c_str());
        errno = ENOENT;
        return -1;
    }
}

HashStore::HashStore(std::string base_path, FileRequester *downloader)
        : base_path(std::move(base_path)), downloader(downloader) {
    if (base_path[base_path.size() - 1] == '/') {
        this->base_path.pop_back();
    }
}

inline std::string HashStore::hash_to_path(const std::string &hash) {
    return base_path + '/' + hash[0] + hash[1] + '/' + hash[2] + hash[3] + '/' + hash[4] + hash[5] + '/' + hash[6] +
           hash[7] + '/' + hash;
}
