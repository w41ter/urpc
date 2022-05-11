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

#include "manager.h"

#include <mutex>

#include <glog/logging.h>

#include "urpc/protocol.h"

namespace urpc {
namespace protocol {
namespace internal {

static urpc::URPCProtocol urpc_protocol;
static std::once_flag flag;

static void RegisterAllProtocols(ProtocolManager* manager) {
    manager->Register(&urpc_protocol);
}

}  // namespace internal

ProtocolManager* ProtocolManager::singleton() {
    static ProtocolManager manager;
    std::call_once(internal::flag, internal::RegisterAllProtocols, &manager);
    return &manager;
}

ProtocolManager::ProtocolManager() {}

bool ProtocolManager::Register(BaseProtocol* protocol) {
    assert(strlen(protocol->Header()) == 4);
    LOG(INFO) << "Register new protocol " << protocol->Header();
    return protocols_.insert_or_assign(protocol->Header(), protocol).second;
}

BaseProtocol* ProtocolManager::ProbeProtocol(const IOBuf& buf) {
    if (buf.size() < 4) {
        return nullptr;
    }

    std::string header;
    buf.append_to(&header, 4);
    LOG(INFO) << "ProbeProtocol header is " << header;

    auto it = protocols_.find(header);
    if (it == protocols_.end()) {
        return nullptr;
    }

    return it->second;
}

}  // namespace protocol
}  // namespace urpc
