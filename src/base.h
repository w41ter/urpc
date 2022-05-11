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

#include <string>

#include "utils/owner_ptr.h"

namespace urpc {

enum ErrCode : int {
    ERR_OK = 0,
    /// The payloads isn't enough to parse.
    ERR_TOO_SMALL = 1001,
    /// The header of payloads is different with the protocol header.
    ERR_MISMATCH = 1002,
    /// This protocol doesn't supported.
    ERR_NOT_SUPPORTED = 1003,
};

class IOHandle : public utils::RefCount {
    friend class IOHandleAccessor;

public:
    virtual ~IOHandle() {}

    virtual int fd() const = 0;

    virtual int HandleReadEvent() = 0;
    virtual int HandleWriteEvent() = 0;

    virtual void Reset(int code, std::string reason) = 0;

    bool poll_in() const noexcept { return flags_ & kPollIn; }
    bool poll_out() const noexcept { return flags_ & kPollOut; }

private:
    enum : unsigned { kPollIn = 1 << 0, kPollOut = 1 << 1 };

    unsigned flags_{0};
};

class IOHandleAccessor {
public:
    IOHandleAccessor(IOHandle* handle) : handle_(handle) {}

    void SetPollIn() { handle_->flags_ |= IOHandle::kPollIn; }
    void SetPollOut() { handle_->flags_ |= IOHandle::kPollOut; }
    void ClearPollIn() { handle_->flags_ &= ~IOHandle::kPollIn; }
    void ClearPollOut() { handle_->flags_ &= ~IOHandle::kPollOut; }

private:
    IOHandle* handle_;
};

}  // namespace urpc
