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
#pragma once

#include <protocol/urpc/urpc_meta.pb.h>

#include "../../client_call.h"
#include "../../server_call.h"

namespace urpc {
namespace protocol {
namespace urpc {

class URPCClientCall : public ClientCall {
public:
    ~URPCClientCall() override = default;

    void IssueRPC(ClientTransport* transport,
                  const google::protobuf::MethodDescriptor* method,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done) override;

protected:
    void OnComplete() override;
    int ProcessResponse(const IOBuf& response) override;

private:
    ClientTransport* transport_ = nullptr;
    google::protobuf::Message* response_ = nullptr;
    google::protobuf::Closure* done_ = nullptr;
};

class URPCServerCall : public ServerCall {
public:
    URPCServerCall(IOBuf buf) : buf_(buf) {}
    ~URPCServerCall() override = default;

    void Run(Transport* trans) override;

private:
    IOBuf buf_;
};

}  // namespace urpc
}  // namespace protocol
}  // namespace urpc