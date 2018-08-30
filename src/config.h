#ifndef CLOUDRUN_HASHFS_CONFIG_H
#define CLOUDRUN_HASHFS_CONFIG_H

#define DEBUG 0

#include <cstdio>
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#endif //CLOUDRUN_HASHFS_CONFIG_H
