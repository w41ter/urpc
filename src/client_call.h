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

#include <google/protobuf/service.h>

#include <urpc/controller.h>
#include "iobuf.h"

namespace urpc {

class ClientTransport;
class ClientCall : public Controller {
public:
    ~ClientCall() override = default;

    void OnComplete() override;

    virtual void IssueRPC(ClientTransport* transport,
                          const google::protobuf::MethodDescriptor* method,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);

    // Invoke if OK. Otherwise Failed is setted.
    virtual int ProcessResponse(const IOBuf& response) = 0;
};

}  // namespace urpc
