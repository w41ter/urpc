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

#include <string>
#include <utility>

#include <protocol/urpc/urpc_meta.pb.h>

#include "urpc/client_call.h"
#include "urpc/server_call.h"

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

class URPCServerCall : public ServerCall, public google::protobuf::Closure {
public:
    URPCServerCall(uint64_t request_id, std::string service_name,
                   std::string method_name, IOBuf buf)
        : request_id_(request_id),
          buf_(std::move(buf)),
          service_name_(std::move(service_name)),
          method_name_(std::move(method_name)) {}
    ~URPCServerCall() override = default;

    int Serve(Transport* trans) override;
    void Run() override;

private:
    const uint64_t request_id_;
    const std::string service_name_;
    const std::string method_name_;
    Transport* transport_;
    google::protobuf::Message* response_;
    IOBuf buf_;
};

}  // namespace urpc
}  // namespace protocol
}  // namespace urpc
