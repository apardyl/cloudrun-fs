#include <sys/socket.h>
#include <netinet/in.h>
#include "filerequester.h"

int FileRequester::fetch_file(std::string filename, std::string save_as) {
    printf("File fetching not implemented, missing file %s\n", save_as.c_str());
    return -1;
    // TODO: implement
}

FileRequester::FileRequester(int server_port) {
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Unable to open socket");
        exit(1);
    }
    int opt = 1;
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Unable to set socket opts");
        exit(1);
    }
    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(server_port));
    if (bind(server_socket_fd, reinterpret_cast<struct sockaddr *>( &address), sizeof(address))) {
        perror("Unable to bind to port");
        exit(1);
    }
    if (listen(server_socket_fd, 2)) {
        perror("Unable to listen on socket");
        exit(1);
    }
    server_thread = std::thread(&FileRequester::server_run, this);
}

void FileRequester::server_run() {
    while (true) {
        client_socket_fd = accept(server_socket_fd, nullptr, nullptr);
        if (client_socket_fd < 0) {
            if (errno != ENETDOWN && errno != EPROTO && errno != ENOPROTOOPT && errno != EHOSTDOWN &&
                errno != ENONET && errno != EHOSTUNREACH && errno != EOPNOTSUPP && errno != ENETUNREACH) {
                perror("Error while accepting connection");
                exit(1);
            }
            continue;
        }
        // TODO: handle connection.
    }
}
