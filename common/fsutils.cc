#include "fsutils.h"

void protoToStat(const filesystem::Stat& s, struct stat *st) {
    st->st_mode = s.mode();
    st->st_nlink = S_ISDIR(s.mode()) ? 2 : 1;
    st->st_uid = s.uid();
    st->st_gid = s.gid();
    st->st_size = s.size();
    st->st_blocks = s.blockcnt();
    st->st_atim.tv_nsec = s.mtime_ns() % 1000000000; // use mtime as atime
    st->st_atim.tv_sec = s.mtime_ns() / 1000000000;
    st->st_mtim.tv_nsec = s.mtime_ns() % 1000000000;
    st->st_mtim.tv_sec = s.mtime_ns() / 1000000000;
    st->st_ctim.tv_nsec = s.ctime_ns() % 1000000000;
    st->st_ctim.tv_sec = s.ctime_ns() / 1000000000;
}
