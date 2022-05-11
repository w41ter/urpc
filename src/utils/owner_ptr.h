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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <utility>

namespace urpc {
namespace utils {

class RefCount {
public:
    RefCount() : ref_count_(1) {}
    ~RefCount() { assert(ref_count_ <= 1); }

    void AddRef() { ref_count_ += 1; }
    void RelRef() {
        if (--ref_count_ == 0) {
            delete this;
        }
    }

private:
    size_t ref_count_;
};

template <typename T>
class owner_ptr {
public:
    owner_ptr() : ptr_(nullptr) {}
    explicit owner_ptr(T* ptr) : ptr_(ptr) {}

    ~owner_ptr() {
        if (ptr_)
            ptr_->RelRef();
    }
    owner_ptr(owner_ptr<T>&& rhs) : owner_ptr() { std::swap(rhs.ptr_, ptr_); }
    owner_ptr& operator=(owner_ptr<T>&& rhs) {
        std::swap(rhs.ptr_, ptr_);
        return *this;
    }
    owner_ptr(const owner_ptr<T>&) = delete;
    owner_ptr& operator=(const owner_ptr<T>&) = delete;

    T* get() { return ptr_; }
    const T* get() const { return ptr_; }

    T& operator*() { return ptr_; }
    const T& operator*() const { return ptr_; }

    T* operator->() { return ptr_; }
    const T* operator->() const { return ptr_; }

    explicit operator bool() const { return get() == nullptr; }

    T* release() { return ptr_; }

    void reset(T* ptr = nullptr) {
        if (ptr_ && ptr != ptr_)
            ptr_->RelRef();
        ptr_ = ptr;
    }

private:
    T* ptr_;
};

}  // namespace utils
}  // namespace urpc
