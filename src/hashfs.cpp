#include <utils/fsutils.h>
#include "hashfs.h"

using namespace filesystem;

HashFS::HashFS(std::unique_ptr<Filesystem> fs, HashStore *ds) {
    this->store = ds;
    this->fs = std::move(fs);
}

const Node *HashFS::getNode(const char *path, bool *noent_authoritative) {
    if (*path != '/') {
        throw std::runtime_error("invalid path: " + std::string(path));
    }

    const Node *node = &this->fs->root();
    path++;

    std::string name;
    for (; *path != '\0'; path++) {
        if (*path == '/') {
            auto n = node->childern().find(name);
            name.clear();
            if (n == node->childern().end()) {
                *noent_authoritative = !node->partial();
                return nullptr;
            } else {
                node = &n->second;
            }
        } else {
            name.push_back(*path);
        }
    }
    if (!name.empty()) {
        auto n = node->childern().find(name);
        if (n == node->childern().end()) {
            *noent_authoritative = !node->partial();
            return nullptr;
        } else {
            node = &n->second;
        }
    }
    return node;
}

int HashFS::lstat(const char *path, struct stat *st) {
    bool na;
    const Node *n = getNode(path, &na);
    if (n != nullptr) {
        protoToStat(n->stat(), st);
        return 0;
    } else {
        errno = ENOENT;
        return na ? -1 : -2;
    }
}

ssize_t HashFS::readlink(const char *path, char *buf, size_t bufsiz) {
    bool na;
    const Node *n = getNode(path, &na);
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
        return na ? -1 : -2;
    }
}

int HashFS::getdir(const char *path, std::map<std::string, struct stat> *dirs) {
    bool na;
    const Node *n = getNode(path, &na);
    if (n != nullptr) {
        if (!S_ISDIR(n->stat().mode())) {
            errno = ENOTDIR;
            return -1;
        } else {
            struct stat st{};
            protoToStat(n->stat(), &st);
            for (const auto &d : n->childern()) {
                protoToStat(d.second.stat(), &(*dirs)[d.first]);
            }
            return n->partial() ? -2 : 0;
        }

    } else {
        errno = ENOENT;
        return na ? -1 : -2;
    }
}

int HashFS::open(const char *pathname, int flags) {
    bool na;
    const Node *n = getNode(pathname, &na);
    if (n != nullptr) {
        if (!S_ISREG(n->stat().mode())) {
            errno = EINVAL;
            return -1;
        } else {
            return store->open_hash(n->target(), std::string(pathname), flags);
        }
    } else {
        errno = ENOENT;
        return na ? -1 : -2;
    }
}

