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

#include <google/protobuf/service.h>
#include <urpc/endpoint.h>

namespace urpc {

enum ProtocolType {
    PROTOCOL_UNKNOWN = 0,
    PROTOCOL_BAIDU_STD = 1,
};

struct ChannelOptions {
    ChannelOptions();

    // Issue error when a connection is not established after so many
    // milliseconds. -1 means wait indefinitely.
    //
    // Default: 200 (milliseconds)
    // Maximum: 0x7fffffff (roughly 30 days)
    int32_t connect_timeout_ms;

    // Max duration of RPC over this Channel. -1 means wait indefinitely.
    // Overridable by Controller.set_timeout_ms().
    //
    // Default: 500 (milliseconds)
    // Maximum: 0x7fffffff (roughly 30 days)
    int32_t timeout_ms;

    ProtocolType protocol;
};

class ClientTransport;

class Channel : public google::protobuf::RpcChannel {
public:
    Channel() {}
    ~Channel() override = default;

    int Init(const char* url, const ChannelOptions& options);

protected:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) override;

private:
    EndPoint server_address_;
    ChannelOptions options_;
    ClientTransport* transport_;
};

}  // namespace urpc
