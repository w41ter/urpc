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

#include "base.h"
#include "poller.h"

namespace urpc {

Transport::~Transport() {
    CHECK(!fd_.valid()) << "Please reset transport before destruction";
}

void Transport::Reset(int code, std::string reason) {
    write_buf_.clear();
    read_buf_.clear();

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
    } else {
        LOG(INFO) << "Transport::StartRead already pollin";
    }

    return 0;
}

int Transport::StartWrite(Controller* cntl, IOBuf buf) {
    assert(!buf.empty());

    if (current_cntl_) {
        LOG(INFO) << "Transport::StartWrite insert into pending writes";
        pending_writes_.emplace_back(std::pair{cntl, std::move(buf)});
    } else {
        LOG(INFO) << "Transport::StartWrite invoke DoWrite()";
        current_cntl_ = cntl;
        write_buf_ = std::move(buf);
        DoWrite();
    }

    return 0;
}

int Transport::DoWrite() {
    LOG(INFO) << "Transport::DoWrite";
    if (current_cntl_)
        HandleWriteEvent();
    return 0;
}

int Transport::HandleReadEvent() {
    assert(fd_.valid());
    while (true) {
        int n = read_buf_.append_from_file_descriptor(fd_, 1024 * 1024);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            else if (errno != EAGAIN) {
                PLOG(ERROR)
                    << "append from file descriptor " << static_cast<int>(fd_);
                assert(false && "unknown error");
            } else {
                assert(poll_in());
                break;
            }
        } else if (n == 0) {
            // EOF
            assert(false && "TODO handle end of file");
        } else {
            LOG(INFO) << "Read " << n << " bytes from fd "
                      << static_cast<int>(fd_);
            // TODO(w41ter) handle result.
            int res = OnRead(&read_buf_);
            if (res != ERR_OK) {
                return res;
            }
        }
    }
    return 0;
}

int Transport::HandleWriteEvent() {
    while (!write_buf_.empty()) {
        int n = write_buf_.cut_into_file_descriptor(fd_);
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
        LOG(INFO) << "Write " << n << " bytes to fd " << static_cast<int>(fd_);

        if (write_buf_.empty()) {
            write_buf_.clear();
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
