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

#include "call.h"

#include <glog/logging.h>

#include "../../client_transport.h"
#include "../../coding.h"
#include "../../iobuf.h"
#include "urpc_meta.pb.h"

namespace urpc {
namespace protocol {
namespace urpc {

void URPCClientCall::OnComplete() { LOG(FATAL) << "Not implemented"; }

void URPCClientCall::IssueRPC(ClientTransport* transport,
                              const google::protobuf::MethodDescriptor* method,
                              const google::protobuf::Message* request,
                              google::protobuf::Message* response,
                              google::protobuf::Closure* done) {
    transport_ = transport;
    response_ = response;
    done_ = done;

    RPCMeta rpc_meta;
    auto* req = rpc_meta.mutable_request();
    req->set_service_name(method->service()->full_name());
    req->set_method_name(method->name());
    req->set_log_id(0);
    rpc_meta.set_attachment_size(0);
    rpc_meta.set_correlation_id(0);

    const size_t payload_size =
        rpc_meta.ByteSizeLong() + request->ByteSizeLong();
    IOBuf buf;
    buf.append("URPC");

    uint8_t dst[4];
    EncodeFixed32(dst, payload_size);
    buf.append((const char*)dst);
    IOBufAsZeroCopyOutputStream out(&buf);
    rpc_meta.SerializeToZeroCopyStream(&out);
    request->SerializeToZeroCopyStream(&out);

    transport->StartWrite(this, std::move(buf));
}

int URPCClientCall::ProcessResponse(const IOBuf& response) {
    IOBufAsZeroCopyInputStream in(response);
    if (!response_->ParseFromZeroCopyStream(&in)) {
        // TODO(walter) report error.
    }

    // success
    done_->Run();
    return 0;
}

void URPCServerCall::Run(Transport* trans) { LOG(FATAL) << "Not supported"; }

}  // namespace urpc
}  // namespace protocol
}  // namespace urpc
