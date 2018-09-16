#include "cachedfilestore.h"
#include "config.h"

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common/randomutils.h"

CachedFileStore::CachedFileStore(std::string private_cache_path, RemoteFSConnection *downloader)
        : private_cache_path(std::move(private_cache_path)), downloader(downloader) {
}

int CachedFileStore::open_file(const std::string &path, int flags) {
    std::string cache_path = private_cache_path + path;

    int fd = open(cache_path.c_str(), flags);
    if (fd == -1 && errno != ENOENT) {
        return fd;
    }
    if (fd != -1) {
        {
            std::lock_guard<std::mutex> guard(set_mutex);
            if (known_paths.count(path) > 0) {
                debug_print("Found file in cache %s\n", cache_path.c_str());
                return fd;
            }
        }
        struct stat st{};
        if (fstat(fd, &st) != 0) {
            debug_print("Unable to stat file %s,, errno: %d\n", cache_path.c_str(), errno);
            return fd;
        }
        struct stat nst{};
        if (downloader->get_stat(path, &nst) != 0) {
            debug_print("Unable to get origin info for file %s, it might be out of date\n", cache_path.c_str());
            return fd;
        }
        if (st.st_mtim.tv_nsec > nst.st_mtim.tv_nsec) {
            {
                std::lock_guard<std::mutex> guard(set_mutex);
                known_paths.insert(path);
            }
            debug_print("File in cache up to date %s\n", cache_path.c_str());
            return fd;
        }
        debug_print("File in cache out of date %s\n", cache_path.c_str());
        if (close(fd) != 0) {
            debug_print("ERROR: Unable to close file descriptor for %s\n", cache_path.c_str());
        }
    }
    debug_print("Fetching file %s\n", path.c_str());

    std::string tmp_path = cache_path + '.' +random_string(10);

    if (downloader->fetch_file(path, tmp_path) == 0) {
        if (rename(tmp_path.c_str(), cache_path.c_str())) {
            debug_print("Error moving file %s to %s errno: %d\n", tmp_path.c_str(), cache_path.c_str(), errno);
            errno = EACCES;
            return -1;
        }
        fd = open(cache_path.c_str(), flags);
        if (fd == -1) {
            debug_print("ERROR: Unable to open file %s from cache %s after successful fetch\n",
                        path.c_str(), cache_path.c_str());
            exit(1);
        }
        {
            std::lock_guard<std::mutex> guard(set_mutex);
            known_paths.insert(path);
        }
        return fd;
    } else {
        debug_print("File %s unavailable\n", path.c_str());
        errno = EACCES;
        return -1;
    }
}

void CachedFileStore::clear_cache() {
    std::lock_guard<std::mutex> guard(set_mutex);
    known_paths.clear();
}
