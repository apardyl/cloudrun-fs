#include <iostream>
#include <unistd.h>
#include "fs_fuse.h"


int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage:\n");
        printf("cloudrun-fs <filesystem meta file> <hash store path> <private store path> <remote address:remote port>\n");
    }

    RemoteFSConnection remoteFSConnection(argv[4]);

    HashStore store(argv[2], &remoteFSConnection);
    filesystem::Filesystem fs;

    int res = open(argv[1], O_RDONLY);
    if (res == -1) {
        printf("unable to open fs.meta\n");
        exit(1);
    }
    if (!fs.ParseFromFileDescriptor(res)) {
        printf("Unable to parse fs.meta\n");
        exit(1);
    }
    close(res);

    auto *hashfs = new HashFS(std::make_unique<filesystem::Filesystem>(std::move(fs)), &store);
    CachedFileStore cachedFileStore(argv[3], &remoteFSConnection);
    auto *cachefs = new CacheFS(&remoteFSConnection, &cachedFileStore);

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, nullptr, nullptr, nullptr);
    fuse_opt_add_arg(&args, "-oro,allow_other,default_permissions");

    return hfs_main(args, hashfs, cachefs);
}
