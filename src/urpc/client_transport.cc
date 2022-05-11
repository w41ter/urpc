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

#include "client_transport.h"

#include <glog/logging.h>

#include "protocol/manager.h"

using urpc::protocol::ProtocolManager;

namespace urpc {

ClientTransport::~ClientTransport() {}

int ClientTransport::OnWriteDone(Controller* cntl) { return 0; }

int ClientTransport::OnRead(IOBuf* buf) {
    int code = ERR_MISMATCH;
    if (protocol_)
        code = protocol_->ParseResponse(buf, this);
    if (code == ERR_MISMATCH) {
        protocol_ = ProtocolManager::singleton()->ProbeProtocol(*buf);
        if (!protocol_) {
            Reset(ERR_NOT_SUPPORTED, "unknown protocol");
            return -1;
        }
    }

    code = protocol_->ParseResponse(buf, this);
    if (code != ERR_OK) {
        if (code == ERR_TOO_SMALL)
            return 0;
        Reset(code, "parse request");
        return -1;
    }

    return 0;
}

ClientCall* ClientTransport::TakeClientCall(uint64_t request_id) {
    auto it = pending_calls_.find(request_id);
    if (it == pending_calls_.end()) {
        return nullptr;
    }

    ClientCall* client_call = it->second;
    pending_calls_.erase(it);
    return client_call;
}

void ClientTransport::InstallClientCall(uint64_t request_id, ClientCall* call) {
    pending_calls_.insert({request_id, call});
}

}  // namespace urpc
