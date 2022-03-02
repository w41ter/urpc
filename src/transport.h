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

#include <deque>

#include "base.h"
#include "controller.h"
#include "iobuf.h"
#include "owned_fd.h"

namespace urpc {

class Transport : public IOHandler {
public:
    Transport() {}
    Transport(int fd) : fd_(fd) {}
    Transport(const Transport&) = delete;
    Transport& operator=(const Transport&) = delete;

    ~Transport() override;

    int StartRead();
    int StartWrite(Controller* cntl, IOBuf buf);

    int fd() const override { return fd_; }
    void Reset() override;

protected:
    virtual int DoWrite();
    virtual int OnWriteDone(Controller* cntl) = 0;
    virtual int OnRead(IOBuf* buf) = 0;

    int HandleReadEvent() override;
    int HandleWriteEvent() override;

    OwnedFD fd_;
    IOBuf read_buf_;
    IOBuf write_buf_;
    Controller* current_cntl_;
    std::deque<std::pair<Controller*, IOBuf>> pending_writes_;
};

}  // namespace urpc
