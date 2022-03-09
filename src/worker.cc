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
#include <error.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include "poll.h"

namespace urpc {

struct Env {
    size_t context_depth = 0;
    std::unique_ptr<Poller> poller;
};

thread_local std::unique_ptr<Env> env;

Poller* CurrentPoller() { return env->poller.get(); }

class Controller : public IOHandler {
public:
    Controller(int fd) : fd_(fd) {}
    ~Controller() override = default;

    virtual int HandleRead(IOBuf* buf) = 0;
    virtual int HandleWriteDone() = 0;

    int OnRead() override;
    int OnWrite() override;

    int FD() const override { return fd_; }

    int DoWrite(std::unique_ptr<IOBuf> buf);

protected:
    OwnedFD fd_;
    bool pollin_ = false;
    bool pollout_ = false;
    std::unique_ptr<IOBuf> read_buf_;
    std::unique_ptr<IOBuf> write_buf_;
    std::deque<std::unique_ptr<IOBuf>> pending_writes_;
};

int Controller::OnRead() {
    ssize_t nread = 0;
    while (true) {
        int n = read_buf_->read(fd_);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            else if (errno != EAGAIN) {
                assert(false && "unknown error");
            }
        } else {
            nread += n;
        }

        if (nread > 0)
            HandleRead(read_buf_.get());
        else
            break;
    }
    return 0;
}

int Controller::OnWrite() {
    while (write_buf_) {
        int n = write_buf_->write(fd_);
        if (n >= 0) {
            if (write_buf_->empty()) {
                HandleWriteDone();
                if (!pending_writes_.empty()) {
                    write_buf_ = std::move(pending_writes_.front());
                    pending_writes_.pop_front();
                }
            }
        } else {
            if (errno == EINTR)
                continue;
            else if (errno != EAGAIN)
                assert(false && "unknown error");
            else {
                if (!pollout_) CurrentPoller()->AddPollOut(this);
                break;
            }
        }
    }
    return 0;
}

int Controller::DoWrite(std::unique_ptr<IOBuf> buf) {
    if (write_buf_) {
        // There already exists a write request.
        pending_writes_.push_back(std::move(buf));
        return 0;
    }

    write_buf_ = std::move(buf);
    return OnWrite();
}

class ServerController : public Controller {
public:
    ServerController(int fd) : Controller(fd) {}
    ~ServerController() override {}

    int HandleRead(IOBuf* buf) override;
    int HandleWriteDone() override;

    int StartRead();
    int StartWrite(Controller* cntl, std::unique_ptr<IOBuf> buf);

private:
    std::deque<Controller*> cntl_list_;
};

int ServerController::StartRead() {
    if (pollin_) {
        CurrentPoller()->AddPollIn(this);
        pollin_ = true;
    }

    return 0;
}

int ServerController::HandleRead(IOBuf* buf) {
    // Read and execute service

    return 0;
}

int ServerController::HandleWriteDone() {
    Controller* cntl = cntl_list_.front();
    cntl_list_.pop_front();
    if (cntl) {
        cntl->HandleWriteDone();
    }
}

int ServerController::StartWrite(Controller* cntl, std::unique_ptr<IOBuf> buf) {
    cntl_list_.push_back(cntl);
    return DoWrite(std::move(buf));
}

class Acceptor : public IOHandler {
public:
    Acceptor(int fd) : listenfd_(fd) { CurrentPoller()->AddPollIn(this); }
    ~Acceptor() override = default;

    int OnRead() override;
    int OnWrite() override;

private:
    OwnedFD listenfd_;
};

int Acceptor::OnRead() {
    int childfd;
    while (true) {
        int childfd = accept(listenfd_, NULL, 0);
        if (childfd == -1) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                break;
            } else {
                assert(false && "unreachable");
            }
        }

        ServerController* cntl = new ServerController(childfd);
        cntl->OnRead();
    }
    return 0;
}

int Acceptor::OnWrite() {
    assert("unreachable");
    return 0;
}

class Endpoint {
public:
    Endpoint Parse(const char*);
};

class Call {
public:
    virtual ~Call() = 0;

    virtual int OnWriteDone() = 0;
};

class ClientCall : public Call {
public:
    ~ClientCall() override;

    int OnWriteDone() override;
    int OnComplete();
};

class MsgHandler {
private:
    std::unordered_map<size_t, Call*> pending_calls_;
};

class ClientMsgHandler : public MsgHandler {
public:
    int OnRead();
};

class ServerMsgHandler : public MsgHandler {
public:
    int OnWrite();
};

class Transport : public IOHandler {
public:
    Transport(int fd) : fd_(fd) {}
    Transport(Endpoint endpoint) {}
    ~Transport() override {}

    virtual int StartWrite(std::unique_ptr<IOBuf> buf);
    virtual int StartRead();

protected:
    virtual int DoWrite();
    virtual int HandleWrite();

    virtual int HandleRead(std::unique_ptr<IOBuf> buf) = 0;

    virtual int OnWriteDone(Call*) = 0;

    virtual int OnRead() override;
    virtual int OnWrite() override;

protected:
    OwnedFD fd_;
    bool pollout_;
};

class ConnectTransport : public Transport {
public:
    int StartWrite(std::unique_ptr<IOBuf> buf) override;

private:
    int DoWrite() override;
    int HandleWrite() override;
    int ConnectIfNot();
    int OnConnect();

    bool connected_;
    bool connecting_;
};

int ConnectTransport::StartWrite(std::unique_ptr<IOBuf> buf) {
    if (!ConnectIfNot()) {
        return 0;
    }
}

int ConnectTransport::ConnectIfNot() {
    if (connected_) return 0;

    if (pollout_)
        // Already register poll out
        return -1;

    // TODO connecting

    if (pollout_) {
        CurrentPoller()->AddPollOut(this);
    }
}

struct EventOwner {
    enum : uint8_t {
        kRead = 0x1,
        kWrite = 0x2,
    };
    uint8_t flags;
    IOHandler* handler;
};

class Epoller : public Poller {
public:
    Epoller() {
        epollfd_ = epoll_create(0);
        assert(epollfd_ >= 0);
    }

    ~Epoller() override = default;

    int PollOnce() override;

    int AddPollIn(IOHandler*) override;
    int AddPollOut(IOHandler*) override;

private:
    OwnedFD epollfd_;
    std::unordered_map<int, EventOwner> owned_events_;
};

int Epoller::AddPollIn(IOHandler* handler) {
    int fd = handler->FD();
    auto it = owned_events_.find(fd);
    assert(it == owned_events_.end());
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    int res = epoll_ctl(epollfd_, EPOLL_CTL_ADD, handler->FD(), &event);
    assert(res == 0);

    EventOwner owner;
    owner.flags = EventOwner::kRead;
    owner.handler = handler;
    owned_events_.insert(std::make_pair(fd, owner));
    return 0;
}

int Epoller::AddPollOut(IOHandler* handler) {
    int fd = handler->FD();
    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLET;
    event.data.fd = fd;
    auto it = owned_events_.find(fd);
    if (it != owned_events_.end()) {
        assert(it->second.flags & EventOwner::kRead);
        it->second.flags |= EventOwner::kWrite;
        event.events |= EPOLLIN;
        int res = epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event);
        assert(res == 0);
    } else {
        int res = epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event);
        assert(res == 0);
        EventOwner owner;
        owner.flags = EventOwner::kRead;
        owner.handler = handler;
        owned_events_.insert(std::make_pair(fd, owner));
    }
    return 0;
}

int Epoller::PollOnce() {
    struct epoll_event events[32];
    ssize_t n = epoll_wait(epollfd_, events, 32, 0);
    if (n < 0) {
        return n;
    }

    for (ssize_t i = 0; i < n; ++i) {
        struct epoll_event* event = &events[i];
        int fd = event->data.fd;
        if (event->events & EPOLLIN) {
            owned_events_[fd].handler->OnRead();
        } else if (event->events & EPOLLOUT) {
            owned_events_[fd].handler->OnWrite();
        } else {
            assert(false && "unknown event");
        }
    }

    return -1;
}

enum Behaviour {
    LoopOnce = 0,
    LoopForever = 1,
};

class Context {
public:
    Context Build() {
        if (!env) {
            auto e = std::make_unique<Env>();
            e->poller = std::make_unique<Epoller>();
        }
        return Context();
    }

    ~Context();

private:
};

Context::~Context() {
    if (--(env->context_depth) == 0) {
        while (true) {
            env->poller->PollOnce();
        }
    }
}

}  // namespace urpc
