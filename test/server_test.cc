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
#include <urpc/server.h>

#include <memory>

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

void RunEchoService() {
    auto service = std::make_unique<EchoServiceImpl>();
    Server server;
    server.AddService(service.release(), ServiceOwnership::SERVER_OWNS_SERVICE);
}

void HandleEchoResponse(Controller* cntl, EchoResponse* response) {
    std::unique_ptr<Controller> cntl_guard(cntl);
    std::unique_ptr<EchoResponse> response_guard(response);

    if (cntl->Failed()) {
        LOG(WARNING) << "Fail to send EchoRequest, " << cntl->ErrorText();
        return;
    }
    // LOG(INFO) << "Received response from " << cntl->remote_side() << ": "
    //           << response->message()
    //           << " (attached=" << cntl->response_attachment() << ")"
    //           << " latency=" << cntl->latency_us() << "us";
}

TEST(EchoTest, Basic) {
    google::InstallFailureSignalHandler();
    RunEchoService();

    ChannelOptions options;
    Channel channel;
    if (channel.Init("", options) != 0) {
        LOG(FATAL) << "Fail to initialize channel";
    }

    EchoService_Stub stub(&channel);
    auto cntl = new Controller();
    auto resp = new EchoResponse();
    EchoRequest req;
    google::protobuf::Closure* done =
        NewCallback(&HandleEchoResponse, cntl, resp);
    stub.Echo(cntl, &req, resp, done);
}
