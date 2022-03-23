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

#include "../../coding.h"

namespace urpc {
namespace protocol {

void EchoClientCall::OnComplete() { LOG(FATAL) << "Not implemented"; }

void EchoServerCall::Run(Transport *trans) {
    IOBuf buf;
    buf.append(reinterpret_cast<const uint8_t *>("ECHO"), 4);

    uint8_t buf_32[4];
    EncodeFixed32(buf_32, buf_.size());
    buf.append(buf_32, 4);
    buf.append(buf_);

    trans->StartWrite(this, std::move(buf));
}

}  // namespace protocol
}  // namespace urpc
