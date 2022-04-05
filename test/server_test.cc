// Copyright 2022 The urpc Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <echo.pb.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <urpc/channel.h>
#include <urpc/controller.h>
#include <urpc/endpoint.h>
#include <urpc/server.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <ratio>
#include <thread>

#include "urpc/endpoint.h"
#include "urpc/io_context.h"

using namespace google::protobuf;

using namespace urpc;
using namespace test;

class EchoServiceImpl : public EchoService {
public:
    ~EchoServiceImpl() override {}

    void Echo(RpcController* controller, const EchoRequest* request,
              EchoResponse* response, Closure* done) override;
};

void EchoServiceImpl::Echo(RpcController* controller,
                           const EchoRequest* request, EchoResponse* response,
                           Closure* done) {
    LOG(FATAL) << "Not implemented";
}

void RunEchoService(Server* server) {
    auto service = std::make_unique<EchoServiceImpl>();
    server->AddService(service.release(),
                       ServiceOwnership::SERVER_OWNS_SERVICE);
}

void HandleEchoResponse(Controller* cntl, EchoResponse* response) {
    std::unique_ptr<Controller> cntl_guard(cntl);
    std::unique_ptr<EchoResponse> response_guard(response);

    if (cntl->Failed()) {
        LOG(WARNING) << "Fail to send EchoRequest, " << cntl->ErrorText();
        return;
    }
    LOG(INFO) << "Received response success";
}

TEST(EchoTest, Basic) {
    google::InstallFailureSignalHandler();

    std::atomic<bool> ready = false;
    std::thread server_handle([&]() {
        Server server;
        RunEchoService(&server);
        if (server.Start(EndPoint(IP_ANY, 8086)) != 0) {
            LOG(FATAL) << "Start server failed";
            return;
        }

        ready.store(true, std::memory_order_release);
        IOContext context;
    });

    while (!ready.load(std::memory_order_acquire))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    LOG(INFO) << "Service is bootstrapted";

    ChannelOptions options;
    Channel channel;
    if (channel.Init("0.0.0.0:8086", options) != 0) {
        LOG(FATAL) << "Fail to initialize channel";
    }

    EchoService_Stub stub(&channel);
    auto cntl = NewURPCController();
    auto resp = new EchoResponse();
    EchoRequest req;
    google::protobuf::Closure* done =
        NewCallback(&HandleEchoResponse, cntl, resp);
    stub.Echo(cntl, &req, resp, done);
    server_handle.join();
}
