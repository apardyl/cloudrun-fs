#include <iostream>
#include <unistd.h>
#include "fs_fuse.h"


int main(int argc, char *argv[]) {
    RemoteFSConnection remoteFSConnection("127.0.0.1:30000");

    HashStore store("/tmp/data", &remoteFSConnection);
    filesystem::Filesystem fs;

    int res = open("filesystem.meta", O_RDONLY);
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
    CachedFileStore cachedFileStore("/tmp/private", &remoteFSConnection);
    auto *cachefs = new CacheFS(&remoteFSConnection, &cachedFileStore);

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, nullptr, nullptr, nullptr);
    fuse_opt_add_arg(&args, "-oro,allow_other,default_permissions");

    return hfs_main(args, hashfs, cachefs);
}
