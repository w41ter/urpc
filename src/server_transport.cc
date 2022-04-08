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

#include "server_transport.h"

#include <glog/logging.h>

#include <memory>

#include "base.h"
#include "protocol/manager.h"

using urpc::protocol::ProtocolManager;

namespace urpc {

ServerTransport::~ServerTransport() {}

int ServerTransport::OnWriteDone(Controller* cntl) {
    Reset(ERR_OK, "");
    delete this;
    return 0;
}

int ServerTransport::OnRead(IOBuf* buf) {
    ServerCall* raw_call = nullptr;

    int code = ERR_MISMATCH;
    if (protocol_)
        code = protocol_->ParseRequest(buf, &raw_call);
    if (code == ERR_MISMATCH) {
        protocol_ = ProtocolManager::singleton()->ProbeProtocol(*buf);
        if (!protocol_) {
            LOG(INFO) << "NOT supported protocol";
            Reset(ERR_NOT_SUPPORTED, "unknown protocol");
            return -1;
        }
    }

    code = protocol_->ParseRequest(buf, &raw_call);
    if (code != ERR_OK) {
        if (code == ERR_TOO_SMALL)
            return 0;
        LOG(INFO) << "parse request code " << code;
        Reset(code, "parse request");
        return -1;
    }

    return raw_call->Serve(this);
}

}  // namespace urpc
