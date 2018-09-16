#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "common/config.h"
#include "scanfs.h"
#include "common/proto/filesystem.pb.h"
#include "common/sha256.h"

#define ISSUPPORTED(mode) (S_ISLNK(mode) || S_ISREG(mode) || S_ISDIR(mode))

using namespace filesystem;

static void check_paths(std::vector<std::string> &included_dirs) {
    for (std::string &path_to_scan : included_dirs) {
        assert(!path_to_scan.empty());
        if (path_to_scan[0] != '/') {
            printf("ERROR: %s not an absolute path! Unable to scan.\n", path_to_scan.c_str());
            exit(1);
        }
        while (path_to_scan.size() > 1 && path_to_scan[path_to_scan.size() - 1] == '/') {
            path_to_scan.pop_back();
        }
    }
}

static Filesystem load_or_create_fs(const std::string &save_file) {
    Filesystem fs;
    int fd = open(save_file.c_str(), O_RDONLY);
    if (fd == -1) {
        verbose_print("%s\n", "Starting a new scan");
        return fs;
    }
    if (fs.ParseFromFileDescriptor(fd)) {
        close(fd);
        verbose_print("Found old scan results %s, updating...\n", save_file.c_str());
        return fs;
    } else {
        close(fd);
        verbose_print("Unable to read old scan results %s, starting a new scan...\n", save_file.c_str());
        return Filesystem();
    }
}

bool stat_to_proto_changed(struct stat *st, Stat *proto) {
    proto->set_mode(st->st_mode);
    proto->set_uid(st->st_uid);
    proto->set_gid(st->st_gid);
    proto->set_size(static_cast<google::protobuf::uint64>(st->st_size));
    proto->set_blockcnt(static_cast<google::protobuf::uint64>(st->st_blocks));
    proto->set_ctime_ns(static_cast<google::protobuf::uint64>(st->st_ctim.tv_nsec + st->st_ctim.tv_sec * 1000000000));
    if (proto->mtime_ns() != st->st_mtim.tv_nsec + st->st_mtim.tv_sec * 1000000000) {
        proto->set_mtime_ns(
                static_cast<google::protobuf::uint64>(st->st_mtim.tv_nsec + st->st_mtim.tv_sec * 1000000000));
        return true;
    }
    return false;
}

static bool set_stat_and_check_if_modified(Node *v, const std::string &path) {
    struct stat st{};
    if (lstat(path.c_str(), &st) == -1) {
        debug_print("Unable to stat %s\n", path.c_str());
        return false;
    } else {
        return stat_to_proto_changed(&st, v->mutable_stat());
    }
}

std::string read_link(const std::string &path) {
    char buff[PATH_MAX + 1];
    ssize_t res = readlink(path.c_str(), buff, PATH_MAX);
    if (res == -1) {
        debug_print("Unable to read link %s", path.c_str());
        return "";
    }
    buff[res] = '\0';
    debug_print("Read link %s : %s\n", path.c_str(), buff);
    return std::string(buff);
}

static Node *find_or_create_base(Node *root, const std::string &base_path) {
    set_stat_and_check_if_modified(root, "/");

    size_t pos = 0;
    while (pos < std::string::npos) {
        size_t next = base_path.find('/', pos + 1);
        if (pos + 1 < next) {
            std::string path = base_path.substr(0, next);
            int res = open(path.c_str(), O_RDONLY | O_DIRECTORY);
            if (res != -1) {
                close(res);
                std::string dir = base_path.substr(pos + 1, next - pos - 1);
                Node *u = &(*root->mutable_children())[dir];
                set_stat_and_check_if_modified(u, path);
                if (S_ISLNK(u->stat().mode())) {
                    u->set_target(read_link(path));
                    verbose_print("WARNING: %s is a symlink, omitting subtree\n", path.c_str());
                    return nullptr;
                } else if (S_ISREG(u->stat().mode())) {
                    sha256_file(path, u->mutable_target());
                    verbose_print("WARNING: %s is a file, omitting subtree\n", path.c_str());
                    return nullptr;
                }
                root = u;
                root->set_partial(true);
                debug_print("Scanned %s\n", path.c_str());
            } else {
                debug_print("Unable to create/update directory %s\n", path.c_str());
                return nullptr;
            }
        }
        pos = next;
    }

    return root;
}

static void scan_dfs(Node *v, const std::string &path) {
    debug_print("Scanning %s\n", path.c_str());
    bool changed = set_stat_and_check_if_modified(v, path);

    if (changed) {
        if (S_ISLNK(v->stat().mode())) {
            v->set_target(read_link(path));
            verbose_print("Updated link %s\n", path.c_str());
        } else if (S_ISREG(v->stat().mode())) {
            sha256_file(path, v->mutable_target());
            verbose_print("Updated file %s\n", path.c_str());
        }
    }

    if (S_ISDIR(v->stat().mode())) {
        // remove not existing children.
        auto &m = *v->mutable_children();
        for (auto it = m.begin(); it != m.end();) {
            std::string p = path + '/' + it->first;
            struct stat st{};
            int res = lstat(p.c_str(), &st);
            if (res == -1 || !ISSUPPORTED(st.st_mode)) {
                verbose_print("Removing non-existing entry: %s\n", p.c_str());
                it = m.erase(it);
            } else {
                it++;
                debug_print("Keeping entry: %s\n", p.c_str());
            }
        }

        // add/update children.
        DIR *dir = opendir(path.c_str());
        if (dir == nullptr) {
            debug_print("Unable to scan dir %s\n", path.c_str());
        } else {
            dirent *d;
            while ((d = readdir(dir))) {
                if ((strcmp(d->d_name, ".") == 0) || (strcmp(d->d_name, "..") == 0)) {
                    continue;
                }
                std::string p = path + '/' + d->d_name;
                struct stat st{};
                int res = lstat(p.c_str(), &st);
                if (res != -1 && ISSUPPORTED(st.st_mode)) {
                    std::string q(d->d_name);
                    debug_print("Creating/updating entry of %s - %s\n", path.c_str(), q.c_str());
                    scan_dfs(&(*v->mutable_children())[q], p);
                }
            }
        }
        closedir(dir);
    }
}

int scan_filesystem(std::vector<std::string> included_dirs, const std::string &save_file) {
    check_paths(included_dirs);
    Filesystem fs = load_or_create_fs(save_file);

    fs.set_version(1);

    if (!fs.has_root()) {
        fs.set_allocated_root(new Node());
    }
    fs.mutable_root()->set_partial(true);

    for (auto &path : included_dirs) {
        verbose_print("Scanning %s\n", path.c_str());
        Node *v = find_or_create_base(fs.mutable_root(), path);
        if (v != nullptr) {
            debug_print("Got node for path %s\n", path.c_str());
            scan_dfs(v, path);
        }
    }
    verbose_print("Scanning finished, saving results %s...\n", save_file.c_str());
    int fd = open(save_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        printf("ERROR: Unable to save file %s\n", save_file.c_str());
        exit(1);
    }
    if (!fs.SerializeToFileDescriptor(fd)) {
        printf("ERROR: Unable to serialize scan results to %s\n", save_file.c_str());
        exit(1);
    }
    close(fd);
    return 0;
}
