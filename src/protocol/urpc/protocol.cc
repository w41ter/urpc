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

#include "protocol.h"

#include <glog/logging.h>
#include <string.h>

#include "../../client_transport.h"
#include "../../coding.h"
#include "../base.h"
#include "../manager.h"
#include "call.h"
#include "urpc_meta.pb.h"

namespace urpc {
namespace protocol {
namespace urpc {

int URPCProtocol::ParseRequest(IOBuf* buf, ServerCall** server_call) {
    std::string header;
    if (buf->append_to(&header, 4) < 4) {
        return ERR_TOO_SMALL;
    }

    LOG(INFO) << "ParseRequest header is " << header;
    if (header != Header()) {
        return ERR_MISMATCH;
    }

    std::vector<uint8_t> len_buf(4, 0);
    if (buf->copy_to(len_buf.data(), 4, 4) < 4) {
        return ERR_TOO_SMALL;
    }

    uint32_t length = DecodeFixed32(len_buf.data());
    LOG(INFO) << "payload length is " << length;
    LOG(INFO) << "buf length is " << buf->size();
    if (buf->size() < 8 + length) {
        return ERR_TOO_SMALL;
    }

    IOBuf data;
    buf->pop_front(8);
    buf->cutn(&data, length);

    IOBufAsZeroCopyInputStream in(data);
    RPCMeta rpc_meta;
    rpc_meta.ParseFromZeroCopyStream(&in);
    data.pop_front(in.ByteCount());
    auto request_id = rpc_meta.correlation_id();
    *server_call =
        new URPCServerCall(request_id, rpc_meta.request().service_name(),
                           rpc_meta.request().method_name(), std::move(data));
    return ERR_OK;
}

int URPCProtocol::ParseResponse(IOBuf* buf, ClientTransport* transport) {
    std::string header;
    if (buf->append_to(&header, 4) < 4) {
        return ERR_TOO_SMALL;
    }

    LOG(INFO) << "ParseRequest header is " << header;
    if (header != Header()) {
        return ERR_MISMATCH;
    }

    std::vector<uint8_t> len_buf(4, 0);
    if (buf->copy_to(len_buf.data(), 4, 4) < 4) {
        return ERR_TOO_SMALL;
    }

    uint32_t length = DecodeFixed32(len_buf.data());
    if (buf->size() < 8 + length) {
        return ERR_TOO_SMALL;
    }

    IOBuf data;
    buf->pop_front(8);
    buf->cutn(&data, length);

    IOBufAsZeroCopyInputStream in(data);
    RPCMeta rpc_meta;
    rpc_meta.ParseFromZeroCopyStream(&in);
    auto request_id = rpc_meta.correlation_id();
    auto cntl = transport->TakeClientCall(rpc_meta.correlation_id());
    if (!cntl) {
        // TODO(walter) return error and close transport.
        return -1;
    }

    data.pop_front(in.ByteCount());
    return cntl->ProcessResponse(data);
}

}  // namespace urpc
}  // namespace protocol
}  // namespace urpc
