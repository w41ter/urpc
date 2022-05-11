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

#include <assert.h>
#include <errno.h>
#include <glog/logging.h>
#include <sys/epoll.h>

#include <unordered_set>

#include "base.h"
#include "owned_fd.h"
#include "poller.h"

namespace urpc {

class EPoller : public Poller {
public:
    EPoller();
    ~EPoller() override = default;
    EPoller(const EPoller&) = delete;
    EPoller& operator=(const EPoller&) = delete;

    int PollOnce() override;
    int AddPollIn(IOHandle*) override;
    int AddPollOut(IOHandle*) override;
    int RemoveConsumer(IOHandle*) override;

private:
    void DestoryDelayedIOHandles();

    OwnedFD pollfd_;
    std::unordered_set<IOHandle*> handles_;
    std::vector<IOHandle*> delayed_destories_;
};

EPoller::EPoller() {
    int fd = epoll_create1(EPOLL_CLOEXEC);
    if (fd < 0) {
        PLOG(FATAL) << "epoll_create1";
    }
    LOG(INFO) << "epoll_create1 " << fd;
    pollfd_ = fd;
}

int EPoller::PollOnce() {
    constexpr int MAX_EVENTS = 32;
    struct epoll_event events[MAX_EVENTS];
    ssize_t n = epoll_wait(pollfd_, events, MAX_EVENTS, 0);
    LOG(INFO) << "epoll_wait fd " << static_cast<int>(pollfd_) << " found " << n
              << " active events";
    if (n < 0) {
        return n;
    }

    for (ssize_t i = 0; i < n; ++i) {
        struct epoll_event* event = &events[i];
        auto handle = reinterpret_cast<IOHandle*>(event->data.ptr);
        if (event->events & EPOLLOUT) {
            if (handle->HandleWriteEvent() != ERR_OK) {
                delayed_destories_.push_back(handle);
                continue;
            }
        }
        if (event->events & EPOLLIN) {
            if (handle->HandleReadEvent() != ERR_OK) {
                delayed_destories_.push_back(handle);
                continue;
            }
        }
    }

    return n;
}

int EPoller::AddPollIn(IOHandle* handle) {
    if (handle->poll_in())
        return 0;

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = handle;
    int op = EPOLL_CTL_ADD;
    if (handle->poll_out()) {
        op = EPOLL_CTL_MOD;
        ev.events |= EPOLLOUT;
    } else {
        handle->AddRef();
        handles_.insert(handle);
    }
    if (epoll_ctl(pollfd_, op, handle->fd(), &ev) < 0) {
        PLOG(FATAL) << "epoll_ctl " << pollfd_ << " new fd " << handle->fd();
    }
    LOG(INFO) << "AddPollIn " << static_cast<int>(pollfd_) << " fd is " << handle->fd();

    IOHandleAccessor(handle).SetPollIn();

    return 0;
}

int EPoller::AddPollOut(IOHandle* handle) {
    if (handle->poll_out())
        return 0;

    struct epoll_event ev;
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.ptr = handle;
    int op = EPOLL_CTL_ADD;
    if (handle->poll_in()) {
        op = EPOLL_CTL_MOD;
        ev.events |= EPOLLIN;
    } else {
        handle->AddRef();
        handles_.insert(handle);
    }
    if (epoll_ctl(pollfd_, op, handle->fd(), &ev) < 0) {
        PLOG(FATAL) << "epoll_ctl";
    }
    LOG(INFO) << "AddPollOut fd is " << handle->fd();

    IOHandleAccessor(handle).SetPollOut();

    return 0;
}

int EPoller::RemoveConsumer(IOHandle* handle) {
    // REQUIRED: Linux >= 2.6.9
    int res = epoll_ctl(pollfd_, EPOLL_CTL_DEL, handle->fd(), NULL);
    if (res < 0 && errno != ENOENT) {
        PLOG(FATAL) << "epoll_ctl";
    } else {
        delayed_destories_.push_back(handle);
    }

    IOHandleAccessor accessor(handle);
    accessor.ClearPollIn();
    accessor.ClearPollOut();

    return 0;
}

void EPoller::DestoryDelayedIOHandles() {
    for (auto handle : delayed_destories_) {
        handle->RelRef();
        handles_.erase(handle);
    }
    delayed_destories_.clear();
}

Poller* poller() {
    static thread_local EPoller epoller;
    return &epoller;
}

}  // namespace urpc
