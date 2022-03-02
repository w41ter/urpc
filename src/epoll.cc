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
    OwnedFD pollfd_;
    std::unordered_set<uintptr_t> handles;
};

EPoller::EPoller() {
    int fd = epoll_create1(EPOLL_CLOEXEC);
    if (fd < 0) {
        PLOG(FATAL) << "epoll_create1";
    }
    pollfd_ = fd;
}

int EPoller::PollOnce() {
    constexpr int MAX_EVENTS = 32;
    struct epoll_event events[MAX_EVENTS];
    ssize_t n = epoll_wait(pollfd_, events, MAX_EVENTS, 0);
    if (n < 0) {
        return n;
    }

    for (ssize_t i = 0; i < n; ++i) {
        struct epoll_event* event = &events[i];
        auto handle = reinterpret_cast<IOHandle*>(event->data.ptr);
        if (event->events & EPOLLIN) {
            handle->HandleReadEvent();
        } else if (event->events & EPOLLOUT) {
            handle->HandleWriteEvent();
        } else {
            assert(false && "unknown event");
        }
    }

    return 0;
}

int EPoller::AddPollIn(IOHandle* handle) {
    if (handle->poll_in()) return 0;

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = handle;
    int op = EPOLL_CTL_ADD;
    if (handle->poll_out()) {
        op = EPOLL_CTL_MOD;
        ev.events |= EPOLLOUT;
    }
    if (epoll_ctl(pollfd_, op, handle->fd(), &ev) < 0) {
        PLOG(FATAL) << "epoll_ctl";
    }

    IOHandleAccessor(handle).SetPollIn();

    return 0;
}

int EPoller::AddPollOut(IOHandle* handle) {
    if (handle->poll_out()) return 0;

    struct epoll_event ev;
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.ptr = handle;
    int op = EPOLL_CTL_ADD;
    if (handle->poll_in()) {
        op = EPOLL_CTL_MOD;
        ev.events |= EPOLLIN;
    }
    if (epoll_ctl(pollfd_, op, handle->fd(), &ev) < 0) {
        PLOG(FATAL) << "epoll_ctl";
    }

    IOHandleAccessor(handle).SetPollOut();

    return 0;
}

int EPoller::RemoveConsumer(IOHandle* handle) {
    // REQUIRED: Linux >= 2.6.9
    int res = epoll_ctl(pollfd_, EPOLL_CTL_DEL, handle->fd(), NULL);
    if (res < 0 && errno != ENOENT) {
        PLOG(FATAL) << "epoll_ctl";
    }

    IOHandleAccessor accessor(handle);
    accessor.ClearPollIn();
    accessor.ClearPollOut();

    return 0;
}

Poller* poller() {
    static EPoller epoller;
    return &epoller;
}

}  // namespace urpc
