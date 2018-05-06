#include <iostream>
#include "hashfs_fuse.h"

int main(int argc, char *argv[]) {
    DataStore store("/tmp/data");
    filesystem::Filesystem fs;
    fs.set_allocated_root(new filesystem::Node());
    auto *root = fs.mutable_root();
    root->set_mode(0x41ed);
    root->set_size(4096);
    root->set_blockcnt(8);
    root->set_uid(0);
    root->set_gid(0);
    root->set_ctime_ns(1508368470000000000);
    root->set_mtime_ns(1508368470000000000);

    auto *f = new filesystem::Node();
    f->set_mode(0x41ed);
    f->set_size(4096);
    f->set_blockcnt(8);
    f->set_uid(0);
    f->set_gid(0);
    f->set_ctime_ns(1508368460000000000);
    f->set_mtime_ns(1508368460000000000);

    (*root->mutable_childern())["dupa"] = *f;

    HashFS *hashfs = new HashFS(std::make_unique<filesystem::Filesystem>(std::move(fs)),
                                std::make_unique<DataStore>(std::move(store)));

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    fuse_opt_parse(&args, nullptr, nullptr, nullptr);
    fuse_opt_add_arg(&args, "-oro,allow_other,default_permissions");

    return hfs_main(args, hashfs);
}