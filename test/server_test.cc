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

#include "google/protobuf/stubs/callback.h"
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
    response->set_message(request->message());
    response->set_message_count(1);
    LOG(INFO) << "EchoService::Echo message=" << request->message();
    done->Run();
}

void RunEchoService(Server* server) {
    auto service = std::make_unique<EchoServiceImpl>();
    server->AddService(service.release(),
                       ServiceOwnership::SERVER_OWNS_SERVICE);
}

class HandleResponseClosure : public Closure {
public:
    HandleResponseClosure(Controller* cntl, EchoResponse* response,
                          std::atomic<bool>* exit_flag)
        : cntl_(cntl), resp_(response), exit_flag_(exit_flag) {}
    ~HandleResponseClosure() override {
        LOG(INFO) << "Set exit flag to true";
        exit_flag_->store(true, std::memory_order_release);
    }

    void Run() override {
        if (cntl_->Failed()) {
            LOG(WARNING) << "Fail to send EchoRequest, " << cntl_->ErrorText();
        } else {
            LOG(INFO) << "Receive response success";
        }
        delete this;
    }

private:
    std::unique_ptr<Controller> cntl_;
    std::unique_ptr<EchoResponse> resp_;
    std::atomic<bool>* exit_flag_;
};

TEST(EchoTest, Basic) {
    google::InstallFailureSignalHandler();

    std::atomic<bool> ready = false;
    std::atomic<bool> exit = false;
    std::thread server_handle([&]() {
        Server server;
        RunEchoService(&server);
        if (server.Start(EndPoint(IP_ANY, 8086)) != 0) {
            LOG(FATAL) << "Start server failed";
            return;
        }

        ready.store(true, std::memory_order_release);
        while (!exit.load(std::memory_order_acquire)) {
            IOContext context(LOOP_ONCE);
        }
        LOG(INFO) << "Server thread exit";
    });

    while (!ready.load(std::memory_order_acquire))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    LOG(INFO) << "Service is bootstrapted";

    std::thread client_handle([&]() {
        ChannelOptions options;
        Channel channel;
        if (channel.Init("0.0.0.0:8086", options) != 0) {
            LOG(FATAL) << "Fail to initialize channel";
        }

        EchoService_Stub stub(&channel);
        auto cntl = NewURPCController();
        auto resp = new EchoResponse();
        EchoRequest req;
        req.set_message("hello world");
        google::protobuf::Closure* done =
            NewCallback(new HandleResponseClosure(cntl, resp, &exit),
                        &HandleResponseClosure::Run);
        stub.Echo(cntl, &req, resp, done);
        while (!exit.load(std::memory_order_acquire)) {
            LOG(INFO) << "Exit flag is "
                      << exit.load(std::memory_order_relaxed);
            IOContext context(LOOP_ONCE);
        }
        LOG(INFO) << "Client thread exit";
    });
    server_handle.join();
    client_handle.join();
}
