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

#include <stdint.h>
#include <unistd.h>

#include <algorithm>
#include <deque>

namespace urpc {

class IOBuf {
public:
    IOBuf() {}

    int read(int fd) {
        constexpr size_t cap = 1024;
        char buf[cap];
        int n = ::read(fd, buf, cap);
        if (n > 0) {
            for (int i = 0; i < n; ++i) payloads_.push_back(buf[i]);
        }
        return n;
    }

    int write(int fd) {
        constexpr size_t cap = 1024;
        char buf[cap];
        size_t total_write = 0;
        while (!payloads_.empty()) {
            size_t size = std::min(payloads_.size(), cap);
            for (size_t i = 0; i < size; ++i) {
                buf[i] = payloads_[i];
            }
            int n = ::write(fd, buf, size);
            if (n > 0) {
                for (size_t i = 0; i < n; ++i) payloads_.pop_front();
                total_write += n;
            }
        }
        return total_write;
    }

    bool empty() const { return payloads_.empty(); }

    void reset() { payloads_.clear(); }

private:
    // FIXME(w41ter) more efficient implementation.
    std::deque<uint8_t> payloads_;
};

}  // namespace urpc
