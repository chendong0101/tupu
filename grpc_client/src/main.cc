/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>
#include <thread>
#include <chrono>
#include "socket_test.h"

#include "message/entity.pb.h"
#include "message/entity.grpc.pb.h"

using namespace std::chrono;
using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using proto::SearchRequest;
using proto::FatSearchResult;
using proto::TupuSearchService;

class GreeterClient {
  public:
    explicit GreeterClient(std::shared_ptr<Channel> channel)
            : stub_(proto::TupuSearchService::NewStub(channel)) {}

    // Assembles the client's payload and sends it to the server.
    void SayHello(const std::string& user) {
        // Data we are sending to the server.
        SearchRequest request;
        request.set_sql(user);

        // Call object to store rpc data
        AsyncClientCall* call = new AsyncClientCall;

        // stub_->PrepareAsyncSayHello() creates an RPC object, returning
        // an instance to store in "call" but does not actually start the RPC
        // Because we are using the asynchronous API, we need to hold on to
        // the "call" instance in order to get updates on the ongoing RPC.
        call->response_reader =
            stub_->PrepareAsyncsearch(&call->context, request, &cq_);

        // StartCall initiates the RPC call
        call->response_reader->StartCall();

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the memory address of the call object.
        call->response_reader->Finish(&call->reply, &call->status, (void*)call);

    }

    // Loop while listening for completed responses.
    // Prints out the response from the server.
    void AsyncCompleteRpc() {
        void* got_tag;
        bool ok = false;

        // Block until the next result is available in the completion queue "cq".
        while (cq_.Next(&got_tag, &ok)) {
            // The tag in this example is the memory location of the call object
            AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

            // Verify that the request was completed successfully. Note that "ok"
            // corresponds solely to the request for updates introduced by Finish().
            GPR_ASSERT(ok);

            if (call->status.ok())
            {
                FatSearchResult res = call->reply;
                for (const auto &entity : call->reply.entities()) {
                    std::cout << entity.kgc_id() << ":" << std::endl;
                    for (const auto &t : entity.triads()) {
                        std::cout << t.predicate() << ":\t" << t.value() << std::endl;
                    }
                }

            } else
                std::cout << "RPC failed" << std::endl;

            // Once we're complete, deallocate the call object.
            delete call;
        }
    }

  private:

    // struct for keeping state and data information
    struct AsyncClientCall {
        // Container for the data we expect from the server.
        FatSearchResult reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // Storage for the status of the RPC upon completion.
        Status status;


        std::unique_ptr<ClientAsyncResponseReader<FatSearchResult>> response_reader;
    };

    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<proto::TupuSearchService::Stub> stub_;

    // The producer-consumer queue we use to communicate asynchronously with the
    // gRPC runtime.
    CompletionQueue cq_;
};

int main(int argc, char** argv) {

    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint (in this case,
    // localhost at port 50051). We indicate that the channel isn't authenticated
    // (use of InsecureChannelCredentials()).
     GreeterClient greeter(grpc::CreateChannel(
             "localhost:50051", grpc::InsecureChannelCredentials()));

    // // Spawn reader thread that loops indefinitely
     std::thread thread_ = std::thread(&GreeterClient::AsyncCompleteRpc, &greeter);

     for (int i = 0; i < 1; i++) {
         std::string user("SELECT 配偶 WHERE NAME = 陈毅");
         greeter.SayHello(user);  // The actual RPC call!
     }

     std::cout << "Press control-c to quit" << std::endl << std::endl;
     thread_.join();  //blocks forever
//    std::vector<std::thread *> ts;
//    setTest();
//
//    milliseconds start_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
//    for (int i = 0; i < 200; i++) {
//        std::thread *t = new std::thread(&clientTest);
//        ts.push_back(t);
//    }
//    for (auto t : ts) {
//        t->join();
//    }
//    milliseconds end_time = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
//    printf("total time: %ld\n", (end_time - start_time));
    
    return 0;
}
