#ifndef CLOUDRUN_CLIENT_REMOTEFSSERVER_H
#define CLOUDRUN_CLIENT_REMOTEFSSERVER_H

#include <string>
#include <grpc++/server.h>


class RemoteFSServer {
public:
    void run_server(const std::string &listen_on);
};


#endif //CLOUDRUN_CLIENT_REMOTEFSSERVER_H
