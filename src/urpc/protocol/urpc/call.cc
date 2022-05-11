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
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include "urpc/client_transport.h"
#include "urpc/coding.h"
#include "urpc/iobuf.h"
#include "urpc/service_holder.h"
#include "urpc_meta.pb.h"

using namespace google::protobuf;

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

    uint64_t request_id = transport_->NextRequestId();

    RPCMeta rpc_meta;
    auto* req = rpc_meta.mutable_request();
    req->set_service_name(method->service()->full_name());
    req->set_method_name(method->name());
    req->set_log_id(0);
    rpc_meta.set_attachment_size(0);
    rpc_meta.set_correlation_id(request_id);

    IOBuf buf;
    buf.append("URPC");

    uint8_t dst[4];
    const size_t meta_size = rpc_meta.ByteSizeLong();
    EncodeFixed32(dst, meta_size);
    buf.append(dst, 4);

    const size_t body_size = request->ByteSizeLong();
    EncodeFixed32(dst, body_size);
    buf.append(dst, 4);

    IOBufAsZeroCopyOutputStream out(&buf);
    rpc_meta.SerializeToZeroCopyStream(&out);
    LOG(INFO) << "RPC Meta len " << out.ByteCount();
    request->SerializeToZeroCopyStream(&out);
    LOG(INFO) << "RPC Meta + Request len " << out.ByteCount();

    LOG(INFO) << "URPCClientCall::IssueRPC buf len is " << buf.size();

    transport_->InstallClientCall(request_id, this);
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

int URPCServerCall::Serve(Transport* trans) {
    transport_ = trans;
    Service* service = ServiceHolder::singleton()->FindService(service_name_);
    const MethodDescriptor* method =
        service->GetDescriptor()->FindMethodByName(method_name_);
    Message* request = service->GetRequestPrototype(method).New();
    response_ = service->GetResponsePrototype(method).New();
    IOBufAsZeroCopyInputStream in(buf_);
    request->ParseFromZeroCopyStream(&in);
    service->CallMethod(method, this, request, response_, this);
    return 0;
}

void URPCServerCall::Run() {
    RPCMeta rpc_meta;
    auto* resp = rpc_meta.mutable_response();
    resp->set_error_code(0);
    rpc_meta.set_attachment_size(0);
    rpc_meta.set_correlation_id(request_id_);

    const size_t payload_size =
        rpc_meta.ByteSizeLong() + response_->ByteSizeLong();
    IOBuf buf;
    buf.append("URPC");

    uint8_t dst[4];
    EncodeFixed32(dst, payload_size);
    buf.append(dst, 4);
    IOBufAsZeroCopyOutputStream out(&buf);
    rpc_meta.SerializeToZeroCopyStream(&out);
    response_->SerializeToZeroCopyStream(&out);

    LOG(INFO) << "URPCServerCall::Run buf len is " << buf.size();

    transport_->StartWrite(this, std::move(buf));
}

}  // namespace urpc
}  // namespace protocol

Controller* NewURPCController() { return new protocol::urpc::URPCClientCall(); }

}  // namespace urpc
