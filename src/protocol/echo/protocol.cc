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

#include <glog/logging.h>
#include <string.h>

#include "../../coding.h"
#include "../base.h"
#include "../manager.h"
#include "call.h"

namespace urpc {
namespace protocol {

class EchoProtocol final : public BaseProtocol {
public:
    EchoProtocol(ProtocolManager* mgr) { mgr->Register(this); }
    ~EchoProtocol() override = default;

    /// A constant bytes which always appears in the header of network message
    /// packets.
    const char* Header() const override { return "ECHO"; }

    /// Parse the protocol request. If the payload isn't enough, ERR_TOO_SMALL
    /// is returned. If the header is mismatched, ERR_MISMATCH is returned.
    int ParseRequest(const IOBuf& buf, ServerCall** server_call) override;

    /// Parse the protocol response. If the payload isn't enough, ERR_TOO_SMALL
    /// is returned. If the header is mismatched, ERR_MISMATCH is returned.
    int ParseResponse(const IOBuf& buf) override {
        LOG(FATAL) << "Not implemented";
        return 0;
    }
};

int EchoProtocol::ParseRequest(const IOBuf& buf, ServerCall** server_call) {
    std::vector<uint8_t> header;
    if (buf.peek(&header, 4) < 4) {
        return ERR_TOO_SMALL;
    }

    if (strcmp(reinterpret_cast<const char*>(header.data()), Header())) {
        return ERR_MISMATCH;
    }

    IOBuf new_buf = buf.slice(4);
    std::vector<uint8_t> len_buf;
    if (new_buf.peek(&len_buf, 4) < 4) {
        return ERR_TOO_SMALL;
    }

    uint32_t length = DecodeFixed32(len_buf.data());
    if (new_buf.size() < 4 + length) {
        return ERR_TOO_SMALL;
    }

    IOBuf data = buf.slice(8, 8 + length);
    *server_call = new EchoServerCall(std::move(data));
    return ERR_OK;
}

static EchoProtocol echo_protocol(ProtocolManager::singleton());

}  // namespace protocol
}  // namespace urpc
