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

#include "acceptor.h"

#include <errno.h>
#include <glog/logging.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "poller.h"
#include "server_transport.h"

namespace urpc {

Acceptor::Acceptor(int listen_fd) : listen_fd_(listen_fd) {
    Poller::singleton()->AddPollIn(this);
}

Acceptor::~Acceptor() {}

int Acceptor::HandleReadEvent() {
    while (true) {
        // REQUIRED: Linux 2.6.28, glibc 2.10
        int fd = accept4(listen_fd_, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else if (errno == EINTR)
                continue;
            else {
                PLOG(FATAL) << "accept4";
            }
        }

        auto server_cntl = new ServerTransport(fd);
        server_cntl->StartRead();
    }
    return 0;
}

int Acceptor::HandleWriteEvent() {
    LOG(FATAL) << "Not supported";
    return 0;
}

void Acceptor::Reset(int code, std::string reason) {
    LOG(FATAL) << "Not supported";
}

}  // namespace urpc