#include "hash_store.h"
#include "common/config.h"
#include "common/random_utils.h"
#include "common/sha256.h"

#include <utility>
#include <fcntl.h>
#include <unistd.h>

int HashStore::open_hash(const std::string &hash, const std::string &real_path, int flags) {
    if (hash.size() < 8) {
        debug_print("Invalid hash %s\n", hash.c_str());
        errno = EACCES;
        return -1;
    }
    std::string path = hash_to_path(hash);
    int res = open(path.c_str(), flags);
    if (res != -1 || errno != ENOENT) {
        debug_print("Found file for hash %s\n", hash.c_str());
        return res;
    }

    {
        std::lock_guard<std::mutex> guard(hash_missmaches_mutex);
        if (hash_missmaches.count(real_path) > 0) {
            debug_print("Previous hash mismatch for %s, skipping\n", real_path.c_str());
            return -2;
        }
    }

    debug_print("Fetching file for hash %s: %s\n", hash.c_str(), real_path.c_str());

    std::string tmp_path = path + '.' + random_string(10);

    if (downloader->fetch_file(real_path, tmp_path) == 0) {
        std::string new_hash;
        if (sha256_file(tmp_path, &new_hash)) {
            debug_print("Unable to calculate hash for: %s\n", tmp_path.c_str(), errno);
            if (unlink(tmp_path.c_str())) {
                debug_print("Unable to remove: %s\n", tmp_path.c_str(), errno);
            }
            errno = EACCES;
            return -1;
        }

        if (new_hash != hash) {
            debug_print("Hash mismatch for %s - is: %s, should be: %s\n", tmp_path.c_str(), new_hash.c_str(),
                        hash.c_str());
            if (unlink(tmp_path.c_str())) {
                debug_print("Unable to remove: %s\n", tmp_path.c_str(), errno);
            }
            {
                std::lock_guard<std::mutex> guard(hash_missmaches_mutex);
                hash_missmaches.insert(real_path);
            }
            errno = EACCES;
            // TODO: copy/move file to private store.
            return -2;
        }

        if (rename(tmp_path.c_str(), path.c_str())) {
            debug_print("Error moving file %s to %s errno: %d\n", tmp_path.c_str(), path.c_str(), errno);
            errno = EACCES;
            return -1;
        }

        debug_print("Fetch ok: %s\n", path.c_str());

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

HashStore::HashStore(std::string shared_cache_path, RemoteFSConnection *downloader)
        : shared_cache_path(std::move(shared_cache_path)), downloader(downloader) {
    if (shared_cache_path[shared_cache_path.size() - 1] == '/') {
        this->shared_cache_path.pop_back();
    }
}

inline std::string HashStore::hash_to_path(const std::string &hash) {
    return shared_cache_path + '/' + hash[0] + hash[1] + '/' + hash[2] + hash[3] + '/' + hash[4] + hash[5] + '/' +
           hash[6] + hash[7] + '/' + hash;
}
