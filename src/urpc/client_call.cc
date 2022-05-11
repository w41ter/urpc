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

#include "client_call.h"

#include <glog/logging.h>

#include "client_transport.h"

using namespace google::protobuf;

namespace urpc {

void ClientCall::OnComplete() { LOG(FATAL) << "Not implemented"; }

void ClientCall::IssueRPC(ClientTransport* transport,
                          const MethodDescriptor* method,
                          const Message* request, Message* response,
                          Closure* done) {
    LOG(FATAL) << "Not implemented";
}

}  // namespace urpc