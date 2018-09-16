#ifndef CLOUDRUN_CLIENT_CONFIG_H
#define CLOUDRUN_CLIENT_CONFIG_H

#define DATA_CHUNK_SIZE (2*1024*1024)

#define DEBUG 1
#define VERBOSE 0
extern bool verbose;

#include <cstdio>
#define debug_print(fmt, ...) \
            do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

#define verbose_print(fmt, ...) \
            do { if (verbose) fprintf(stderr, fmt, __VA_ARGS__); } while(0)

#endif //CLOUDRUN_CLIENT_CONFIG_H
