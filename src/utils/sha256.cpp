#include <cstdio>
#include <string>
#include <stdexcept>
#include <openssl/sha.h>
#include "config.h"

#include "sha256.h"

const int buffsize = 32768;

int sha256_file(const std::string &path, std::string *checksum) {
    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        debug_print("Unable to open file: %s, errno: %d\n", path.c_str(), errno);
        return errno;
    }
    SHA256_CTX sha256_ctx{};
    SHA256_Init(&sha256_ctx);
    unsigned char buffer[buffsize];
    size_t bytesread = 0;
    while ((bytesread = fread(buffer, 1, buffsize, file))) {
        SHA256_Update(&sha256_ctx, buffer, bytesread);
    }
    fclose(file);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256_ctx);
    char hex[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex + (i * 2), "%02x", hash[i]);
    }
    hex[64] = 0;
    debug_print("Generated hash for %s : %s\n", path.c_str(), hex);
    *checksum = hex;
    return 0;
}
