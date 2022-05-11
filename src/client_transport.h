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

#include <urpc/endpoint.h>

#include "client_call.h"
#include "connect_transport.h"
#include "iobuf.h"
#include "protocol/base.h"

namespace urpc {

class ClientTransport : public ConnectTransport {
public:
    ClientTransport(EndPoint endpoint) : ConnectTransport(endpoint) {}
    ~ClientTransport() override;

    ClientCall* TakeClientCall(uint64_t request_id);
    void InstallClientCall(uint64_t request_id, ClientCall* call);
    uint64_t NextRequestId() { return next_request_id_++; }

protected:
    int OnWriteDone(Controller* cntl) override;
    int OnRead(IOBuf* buf) override;

private:
    uint64_t next_request_id_{1};

    /// The last successfully parsed protocol, used to optimize protocol
    /// lookuping.
    protocol::BaseProtocol* protocol_{nullptr};

    std::unordered_map<uint64_t, ClientCall*> pending_calls_;
};

}  // namespace urpc
