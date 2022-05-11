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

#include <unistd.h>

#include <utility>

namespace urpc {

class OwnedFD {
public:
    OwnedFD() : fd_(-1) {}
    explicit OwnedFD(int fd) : fd_(fd) {}
    OwnedFD(const OwnedFD&) = delete;
    OwnedFD(OwnedFD&& rhs) : fd_(rhs.fd_) { rhs.fd_ = -1; }
    ~OwnedFD() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }
    OwnedFD& operator=(const OwnedFD&) = delete;
    OwnedFD& operator=(OwnedFD&& rhs) {
        if (&rhs != this) {
            std::swap(rhs.fd_, fd_);
        }
        return *this;
    }

    bool valid() const noexcept { return fd_ >= 0; }

    void reset() {
        if (fd_ >= 0) {
            close(fd_);
        }
        fd_ = -1;
    }

    int release() {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

    operator int() const noexcept { return fd_; }

private:
    int fd_;
};

}  // namespace urpc
