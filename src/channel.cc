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

#include <glog/logging.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <urpc/channel.h>
#include <urpc/endpoint.h>

#include <string>
#include <unordered_map>

#include "client_call.h"
#include "client_transport.h"

using namespace google::protobuf;

namespace urpc {

class SocketMap {
public:
    ~SocketMap() {}

    static SocketMap* singleton() {
        thread_local SocketMap socket_map;
        return &socket_map;
    }

    ClientTransport* GetOrCreateTransport(const char* url,
                                          const ChannelOptions& options) {
        auto it = connection_map_.find(std::string(url));
        if (it == connection_map_.end()) {
            EndPoint endpoint;
            if (str2endpoint(url, &endpoint) == -1) {
                LOG(WARNING) << "Invalid endpoint " << url;
                return nullptr;
            }
            auto client_transport = new ClientTransport(endpoint);
            it = connection_map_.insert({std::string(url), client_transport})
                     .first;
        }
        return it->second;
    }

private:
    SocketMap() {}

    std::unordered_map<std::string, ClientTransport*> connection_map_;
};

ChannelOptions::ChannelOptions()
    : connect_timeout_ms(200), timeout_ms(500), protocol(PROTOCOL_UNKNOWN) {}

int Channel::Init(const char* url, const ChannelOptions& options) {
    transport_ = SocketMap::singleton()->GetOrCreateTransport(url, options);
    return 0;
}

void Channel::CallMethod(const MethodDescriptor* method, RpcController* cntl,
                         const Message* request, Message* response,
                         Closure* done) {
    LOG_IF(FATAL, transport_ == nullptr)
        << "Please invoke Channel::Init() first";
    LOG(INFO) << method->full_name() << request->SerializeAsString();
    reinterpret_cast<ClientCall*>(cntl)->IssueRPC(transport_, method, request,
                                                  response, done);
}

}  // namespace urpc
