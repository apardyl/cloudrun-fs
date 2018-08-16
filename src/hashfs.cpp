#include "hashfs.h"

using namespace filesystem;

HashFS::HashFS(std::unique_ptr<Filesystem> fs, std::unique_ptr<HashStore> ds) {
    this->fs = std::move(fs);
    this->store = std::move(ds);
}

const Node *HashFS::getNode(const char *path) {
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
            return nullptr;
        } else {
            node = &n->second;
        }
    }
    return node;
}

void HashFS::lstat(const Node *n, struct stat *st) {
    st->st_mode = n->mode();
    st->st_nlink = S_ISDIR(n->mode()) ? 2 : 1;
    st->st_uid = n->uid();
    st->st_gid = n->gid();
    st->st_size = n->size();
    st->st_blocks = n->blockcnt();
    st->st_atim.tv_nsec = n->mtime_ns() % 1000000000; // use mtime as atime
    st->st_atim.tv_sec = n->mtime_ns() / 1000000000;
    st->st_mtim.tv_nsec = n->mtime_ns() % 1000000000;
    st->st_mtim.tv_sec = n->mtime_ns() / 1000000000;
    st->st_ctim.tv_nsec = n->ctime_ns() % 1000000000;
    st->st_ctim.tv_sec = n->ctime_ns() / 1000000000;
}


int HashFS::lstat(const char *path, struct stat *st) {
    const Node *n = getNode(path);
    if (n != nullptr) {
        lstat(n, st);
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

ssize_t HashFS::readlink(const char *path, char *buf, size_t bufsiz) {
    const Node *n = getNode(path);
    if (n != nullptr) {
        if (!S_ISLNK(n->mode())) {
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

int HashFS::getdir(const char *path, void *buf, fuse_fill_dir_t filler) {
    const Node *n = getNode(path);
    if (n != nullptr) {
        if (!S_ISDIR(n->mode())) {
            errno = ENOTDIR;
            return -1;
        } else {
            struct stat st{};
            lstat(n, &st);
            filler(buf, ".", &st, 0);
            filler(buf, "..", nullptr, 0);
            for (const auto &d : n->childern()) {
                lstat(&d.second, &st);
                if (filler(buf, d.first.c_str(), &st, 0)) {
                    break;
                }
            }
            return 0;
        }

    } else {
        errno = ENOENT;
        return -1;
    }
}

int HashFS::open(const char *pathname, int flags) {
    if ((flags & (O_WRONLY | O_RDWR)) != 0) {
        errno = EACCES;
        return -1;
    }
    const Node *n = getNode(pathname);
    if (n != nullptr) {
        if (!S_ISREG(n->mode())) {
            errno = EINVAL;
            return -1;
        } else {
            return store->open_hash(n->target(), std::string(pathname), flags);
        }
    } else {
        errno = ENOENT;
        return -1;
    }
}

