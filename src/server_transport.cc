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

#include "server_transport.h"

#include <glog/logging.h>

namespace urpc {

ServerTransport::~ServerTransport() {}

int ServerTransport::OnWriteDone(Controller* cntl) {
    LOG(FATAL) << "TODO";
    return 0;
}

int ServerTransport::OnRead(IOBuf* buf) {
    LOG(FATAL) << "TODO";
    return 0;
}

}  // namespace urpc
