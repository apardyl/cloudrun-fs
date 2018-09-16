#include <cstring>
#include <vector>
#include <string>

#include "rpc/RemoteFSServer.h"
#include "scanfs/scanfs.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc >= 4) {
        if (strcmp(argv[1], "scan") == 0) {
            vector<string> paths_to_scan;
            for (int i = 3; i < argc; i++) {
                paths_to_scan.emplace_back(argv[i]);
            }
            return scan_filesystem(paths_to_scan, string(argv[2]));
        }
    }
    if (argc == 3 && strcmp(argv[1], "serve") == 0) {
        RemoteFSServer().run_server(std::string(argv[2]));
        return 1;
    }
    printf("Usage:\n");
    printf("Run server: cloudrun-client-daemon serve <IP:PORT OR unix://<path>>\n");
    printf("Scan filesystem: cloudrun-client-daemon scan <save file name> <list of  directories to scan>\n");
    return 1;
}
