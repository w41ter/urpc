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
#include <urpc/channel.h>

#include "client_call.h"

using namespace google::protobuf;

namespace urpc {

int Channel::Init(const char* url, const ChannelOptions& options) {
    LOG(FATAL) << "Not implemented";
    return 0;
}

void Channel::CallMethod(const MethodDescriptor* method, RpcController* cntl,
                         const Message* request, Message* response,
                         Closure* done) {
    reinterpret_cast<ClientCall*>(cntl)->IssueRPC(method, cntl, request,
                                                  response, done);
}

}  // namespace urpc
