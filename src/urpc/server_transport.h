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

#include "protocol/base.h"
#include "transport.h"

namespace urpc {

class ServerTransport : public Transport {
public:
    explicit ServerTransport(int fd) : Transport(fd) {}
    ~ServerTransport() override;

protected:
    int OnWriteDone(Controller* cntl) override;
    int OnRead(IOBuf* buf) override;

private:
    /// The last successfully parsed protocol, used to optimize protocol
    /// lookuping.
    protocol::BaseProtocol* protocol_{nullptr};
};

}  // namespace urpc
