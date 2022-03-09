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

#include "transport.h"

#include <assert.h>
#include <errno.h>
#include <glog/logging.h>

#include "poller.h"

namespace urpc {

Transport::~Transport() {
    CHECK(!fd_.valid()) << "Please reset transport before destruction";
}

void Transport::Reset(int code, std::string reason) {
    write_buf_.reset();
    read_buf_.reset();

    if (current_cntl_) {
        // TODO
        current_cntl_->SetFailed(code, reason);
        current_cntl_ = nullptr;
    }

    for (auto&& [cntl, buf] : pending_writes_) {
        cntl->SetFailed(code, reason);
    }
    pending_writes_.clear();

    if (poll_in() || poll_out())
        Poller::singleton()->RemoveConsumer(this);

    fd_.reset();
}

int Transport::StartRead() {
    if (!poll_in()) {
        Poller::singleton()->AddPollIn(this);
    }

    return 0;
}

int Transport::StartWrite(Controller* cntl, IOBuf buf) {
    assert(!buf.empty());

    if (current_cntl_) {
        pending_writes_.emplace_back(std::pair{cntl, std::move(buf)});
    } else {
        current_cntl_ = cntl;
        write_buf_ = std::move(buf);
        DoWrite();
    }

    return 0;
}

int Transport::DoWrite() {
    if (current_cntl_)
        HandleWriteEvent();
    return 0;
}

int Transport::HandleReadEvent() {
    while (true) {
        int n = read_buf_.read(fd_);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            else if (errno != EAGAIN) {
                assert(false && "unknown error");
            } else {
                assert(poll_in());
                break;
            }
        } else if (n == 0) {
            // EOF
            assert(false && "TODO handle end of file");
        } else {
            // TODO(w41ter) handle result.
            OnRead(&read_buf_);
        }
    }
    return 0;
}

int Transport::HandleWriteEvent() {
    while (!write_buf_.empty()) {
        int n = write_buf_.write(fd_);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            else if (errno != EAGAIN)
                assert(false && "unknown error");
            else {
                if (!poll_out())
                    Poller::singleton()->AddPollOut(this);
                break;
            }
        }

        if (write_buf_.empty()) {
            write_buf_.reset();
            OnWriteDone(current_cntl_);
            current_cntl_ = nullptr;

            if (!pending_writes_.empty()) {
                current_cntl_ = pending_writes_.front().first;
                write_buf_ = std::move(pending_writes_.front().second);
                pending_writes_.pop_front();
            }
        }
    }
    return 0;
}

}  // namespace urpc
