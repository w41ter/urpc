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

#include "base.h"
#include "owned_fd.h"

namespace urpc {

class Acceptor : public IOHandle {
public:
    Acceptor(int listen_fd) : listen_fd_(listen_fd) {}
    ~Acceptor() override;

    int fd() const override { return listen_fd_; }

    int HandleReadEvent() override;
    int HandleWriteEvent() override;

    void Reset() override;

private:
    OwnedFD listen_fd_;
};

}  // namespace urpc
