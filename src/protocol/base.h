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

#include "../iobuf.h"
#include "../server_call.h"

namespace urpc {

class ClientTransport;

namespace protocol {

class BaseProtocol {
public:
    virtual ~BaseProtocol() = default;

    /// A constant bytes which always appears in the header of network message
    /// packets.
    virtual const char* Header() const = 0;

    /// Parse the protocol request. If the payload isn't enough, ERR_TOO_SMALL
    /// is returned. If the header is mismatched, ERR_MISMATCH is returned.
    virtual int ParseRequest(IOBuf* buf, ServerCall** server_call) = 0;

    /// Parse the protocol response. If the payload isn't enough, ERR_TOO_SMALL
    /// is returned. If the header is mismatched, ERR_MISMATCH is returned.
    virtual int ParseResponse(IOBuf* buf, ClientTransport* transport) = 0;
};

}  // namespace protocol
}  // namespace urpc