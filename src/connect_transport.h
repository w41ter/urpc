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

#include "endpoint.h"
#include "transport.h"

namespace urpc {

class ConnectTransport : public Transport {
public:
    ConnectTransport(butil::EndPoint endpoint) : endpoint_(endpoint){};
    ~ConnectTransport() override;

protected:
    void Reset() override;
    int DoWrite() override;
    int HandleWriteEvent() override;

private:
    int ConnectIfNot();
    int OnConnect();

    bool connected_;
    bool connecting_;
    butil::EndPoint endpoint_;
};

}  // namespace urpc
