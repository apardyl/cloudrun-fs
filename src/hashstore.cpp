#include "hashstore.h"
#include "config.h"

#include <utility>
#include <fcntl.h>

int HashStore::open_hash(const std::string &hash, const std::string &real_path, int flags) {
    if (hash.size() < 8) {
        debug_print("Invalid hash %s\n", hash.c_str());
        errno = EINVAL;
        return -1;
    }
    std::string path = hash_to_path(hash);
    int res = open(path.c_str(), flags);
    if (res != -1 || errno != ENOENT) {
        debug_print("Found file for hash %s\n", hash.c_str());
        return res;
    }
    debug_print("Fetching file for hash %s\n", hash.c_str());
    if (downloader->fetch_file(real_path, path) == 0) {
        res = open(path.c_str(), flags);
        if (res == -1) {
            debug_print("ERROR: Unable to open file %s of hash %s after successful fetch\n", real_path.c_str(),
                   hash.c_str());
            exit(1);
        }
        return res;
    } else {
        debug_print("No file for hash %s\n", hash.c_str());
        errno = EACCES;
        return -1;
    }
}

HashStore::HashStore(std::string base_path, RemoteFSConnection *downloader)
        : base_path(std::move(base_path)), downloader(downloader) {
    if (base_path[base_path.size() - 1] == '/') {
        this->base_path.pop_back();
    }
}

inline std::string HashStore::hash_to_path(const std::string &hash) {
    return base_path + '/' + hash[0] + hash[1] + '/' + hash[2] + hash[3] + '/' + hash[4] + hash[5] + '/' + hash[6] +
           hash[7] + '/' + hash;
}
