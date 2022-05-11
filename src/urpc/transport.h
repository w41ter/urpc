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

#include <urpc/controller.h>

#include <deque>
#include <string>
#include <utility>

#include "urpc/base.h"
#include "urpc/iobuf.h"
#include "urpc/owned_fd.h"

namespace urpc {

class Transport : public IOHandle {
public:
    Transport() {}
    explicit Transport(int fd) : fd_(fd) {}
    Transport(const Transport&) = delete;
    Transport& operator=(const Transport&) = delete;

    ~Transport() override;

    int StartRead();
    int StartWrite(Controller* cntl, IOBuf buf);

    int fd() const override { return fd_; }
    void Reset(int code, std::string reason) override;

protected:
    virtual int DoWrite();
    virtual int OnWriteDone(Controller* cntl) = 0;
    virtual int OnRead(IOBuf* buf) = 0;

    int HandleReadEvent() override;
    int HandleWriteEvent() override;

    OwnedFD fd_;
    IOPortal read_buf_;
    IOBuf write_buf_;
    Controller* current_cntl_{nullptr};
    std::deque<std::pair<Controller*, IOBuf>> pending_writes_;
};

}  // namespace urpc
