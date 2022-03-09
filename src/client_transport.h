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

#include "connect_transport.h"
#include "endpoint.h"
#include "iobuf.h"
#include "protocol/base.h"

namespace urpc {

class ClientTransport : public ConnectTransport {
public:
    ClientTransport(butil::EndPoint endpoint) : ConnectTransport(endpoint) {}
    ~ClientTransport() override;

protected:
    int OnWriteDone(Controller* cntl) override;
    int OnRead(IOBuf* buf) override;

private:
};

}  // namespace urpc
