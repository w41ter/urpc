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

#include <glog/logging.h>

namespace urpc {
namespace protocol {

ProtocolManager* ProtocolManager::singleton() {
    static ProtocolManager manager;
    return &manager;
}

ProtocolManager::ProtocolManager() {}

void ProtocolManager::Register(BaseProtocol* protocol) {
    protocols_.insert_or_assign(protocol->Header(), protocol);
}

BaseProtocol* ProtocolManager::ProbeProtocol(const IOBuf& buf) {
    LOG(FATAL) << "Not implemented";
    return nullptr;
}

}  // namespace protocol
}  // namespace urpc