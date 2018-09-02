#include <utils/fsutils.h>
#include "cachefs.h"
#include "config.h"

using namespace filesystem;

const filesystem::Node *CacheFS::getNode(const char *path) {
    std::lock_guard<std::mutex> guard(fs_mutex);

    if (*path != '/') {
        throw std::runtime_error("invalid path: " + std::string(path));
    }

    Node *node = fs->mutable_root();
    if (fs->root().stat().mode() == 0) {
        debug_print("Creating new cache root %s\n", "/");
        int res = downloader->get_stat("/", fs->mutable_root()->mutable_stat());
        if (res != 0) {
            return nullptr;
        }
        fetch_node("/", fs->mutable_root());
    }

    path++;
    std::string name;
    std::string current_path = "/";
    for (; *path != '\0'; path++) {
        current_path += *path;
        if (*path == '/') {
            auto n = node->mutable_childern()->find(name);
            name.clear();
            if (n == node->childern().end()) {
                return nullptr;
            } else {
                node = &n->second;
                if (node->partial()) {
                    fetch_node(current_path, node);
                }
            }
        } else {
            name.push_back(*path);
        }
    }
    if (!name.empty()) {
        auto n = node->mutable_childern()->find(name);
        if (n == node->mutable_childern()->end()) {
            return nullptr;
        } else {
            node = &n->second;
        }
    }
    if (node->partial() && S_ISDIR(node->stat().mode())) {
        fetch_node(current_path, node);
    }
    return node;
}

void CacheFS::fetch_node(const std::string &path, filesystem::Node *node) {
    int res = downloader->get_dir(path, node);
    if (res != 0) {
        debug_print("Unable to fetch node %s, errno: %d\n", path.c_str(), res);
    } else {
        debug_print("Fetched node %s\n", path.c_str());
    }
    // Set partial to false regardless of the result (also cache errors)
    node->set_partial(false);
}

CacheFS::CacheFS(RemoteFSConnection *downloader, CachedFileStore *store) : downloader(downloader), store(store) {
    fs = std::make_unique<Filesystem>(Filesystem());
    fs->mutable_root()->set_partial(true);
}

int CacheFS::lstat(const char *path, struct stat *st) {
    const Node *n = getNode(path);
    if (n != nullptr) {
        protoToStat(n->stat(), st);
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

ssize_t CacheFS::readlink(const char *path, char *buf, size_t bufsiz) {
    const Node *n = getNode(path);
    if (n != nullptr) {
        if (!S_ISLNK(n->stat().mode())) {
            errno = EINVAL;
            return -1;
        } else if (n->target().size() > bufsiz) {
            errno = ENAMETOOLONG;
            return -1;
        } else {
            strncpy(buf, n->target().data(), bufsiz);
            return n->target().size();
        }

    } else {
        errno = ENOENT;
        return -1;
    }
}

int CacheFS::getdir(const char *path, std::map<std::string, struct stat> *dirs) {
    const Node *n = getNode(path);
    if (n != nullptr) {
        if (!S_ISDIR(n->stat().mode())) {
            errno = ENOTDIR;
            return -1;
        }
        struct stat st{};
        protoToStat(n->stat(), &st);
        for (const auto &d : n->childern()) {
            protoToStat(d.second.stat(), &(*dirs)[d.first]);
        }
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

int CacheFS::open(const char *pathname, int flags) {
    const Node *n = getNode(pathname);
    if (n != nullptr) {
        if (!S_ISREG(n->stat().mode())) {
            errno = EINVAL;
            return -1;
        } else {
            return store->open_file(std::string(pathname), flags);
        }
    } else {
        errno = ENOENT;
        return -1;
    }
}
