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

#include "../base.h"

namespace urpc {
namespace protocol {
namespace urpc {

class URPCProtocol final : public BaseProtocol {
public:
    ~URPCProtocol() override = default;

    /// A constant bytes which always appears in the header of network message
    /// packets.
    const char* Header() const override { return "URPC"; }

    /// Parse the protocol request. If the payload isn't enough, ERR_TOO_SMALL
    /// is returned. If the header is mismatched, ERR_MISMATCH is returned.
    int ParseRequest(IOBuf* buf, ServerCall** server_call) override;

    /// Parse the protocol response. If the payload isn't enough, ERR_TOO_SMALL
    /// is returned. If the header is mismatched, ERR_MISMATCH is returned.
    int ParseResponse(IOBuf* buf, ClientTransport* transport) override;

    static bool registered;
};

}  // namespace urpc
}  // namespace protocol
}  // namespace urpc
