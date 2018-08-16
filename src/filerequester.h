#ifndef CLOUDRUN_HASHFS_FILEREQUESTER_H
#define CLOUDRUN_HASHFS_FILEREQUESTER_H

#include <string>
#include <thread>

class FileRequester {
    int server_socket_fd;
    int client_socket_fd;
    std::thread server_thread;

    void server_run();
public:
    explicit FileRequester(int server_port);

    int fetch_file(std::string filename, std::string save_as);
};


#endif //CLOUDRUN_HASHFS_FILEREQUESTER_H
