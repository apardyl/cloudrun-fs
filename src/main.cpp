#include <iostream>
#include <unistd.h>
#include "hashfs_fuse.h"


int main(int argc, char *argv[]) {
    FileRequester downloader(30001);

    HashStore store("/tmp/data", &downloader);
    filesystem::Filesystem fs;

    int res = open("fs.meta", O_RDONLY);
    if (res == -1) {
        printf("unable to open fs.meta\n");
        exit(1);
    }
    if (!fs.ParseFromFileDescriptor(res)) {
        printf("Unable to parse fs.meta\n");
        exit(1);
    }
    close(res);

    HashFS *hashfs = new HashFS(std::make_unique<filesystem::Filesystem>(std::move(fs)),
                                std::make_unique<HashStore>(std::move(store)));

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, nullptr, nullptr, nullptr);
    fuse_opt_add_arg(&args, "-oro,allow_other,default_permissions");

    return hfs_main(args, hashfs);
}