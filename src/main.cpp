#include <iostream>
#include <unistd.h>
#include "fs_fuse.h"


int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage:\n");
        printf("cloudrun-fs <filesystem meta file> <hash store path> <private store path> <remote address:remote port>\n");
    }

    auto *remoteFSConnection = new RemoteFSConnection(argv[4]);
    auto *store = new HashStore(argv[2], remoteFSConnection);
    auto *fs = new filesystem::Filesystem;

    int res = open(argv[1], O_RDONLY);
    if (res == -1) {
        printf("unable to open fs.meta\n");
        exit(1);
    }
    if (!fs->ParseFromFileDescriptor(res)) {
        printf("Unable to parse fs.meta\n");
        exit(1);
    }
    close(res);

    auto *hashfs = new HashFS(std::unique_ptr<filesystem::Filesystem>(fs), store);
    auto *cachedFileStore = new CachedFileStore(argv[3], remoteFSConnection);
    auto *cachefs = new CacheFS(remoteFSConnection, cachedFileStore);

    struct fuse_args args = FUSE_ARGS_INIT(argc - 4, argv + 4);
    fuse_opt_parse(&args, nullptr, nullptr, nullptr);
    fuse_opt_add_arg(&args, "-f");
    fuse_opt_add_arg(&args, "-oro,allow_other,default_permissions");

    return hfs_main(args, hashfs, cachefs);
}
