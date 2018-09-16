#include "RemoteFSServer.h"
#include "RemoteFSService.h"

#include <grpc/grpc.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include "common/config.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

void RemoteFSServer::run_server(const std::string &listen_on) {
    RemoteFSService service;
    ServerBuilder builder;
    builder.AddListeningPort(listen_on, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    verbose_print("Server listening on: %s\n", listen_on.c_str());
    server->Wait();
}
