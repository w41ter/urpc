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

#include <string>
#include <unordered_map>

#include "base.h"

namespace urpc {
namespace protocol {

class ProtocolManager {
public:
    static ProtocolManager* singleton();

    /// Register or update a protocol by the corresponding protocol header.
    bool Register(BaseProtocol* protocol);

    /// Find the corresponding protocol by the specified header. `nullptr` is
    /// returned if no such protocol exists.
    BaseProtocol* FindProtocol(const char* header);

    /// Probe the corresponding protocol of the buf, `nullptr` is returned if no
    /// such protocol exists.
    BaseProtocol* ProbeProtocol(const IOBuf& buf);

private:
    ProtocolManager();

    std::unordered_map<std::string, BaseProtocol*> protocols_;
};

}  // namespace protocol
}  // namespace urpc
