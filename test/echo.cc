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

#include "echo.h"

#include <glog/logging.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

EchoService EchoService::Bootstrap() {
    int fd = socket(AF_INET, SOCK_CLOEXEC | SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        PLOG(FATAL) << "socket";
    }

    struct sockaddr_in serv_addr;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(0);
    if (bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        PLOG(FATAL) << "bind";
    }

    if (listen(fd, 20480) < 0) {
        PLOG(FATAL) << "listen";
    }

    return EchoService(fd);
}

EchoService::~EchoService() {
    if (fd_ >= 0) {
        close(fd_);
    }
}

bool EchoService::TryRun() {
    int fd = accept4(fd_, NULL, NULL, SOCK_CLOEXEC | SOCK_NONBLOCK);
    if (fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
            PLOG(FATAL) << "accept4";
        return false;
    }

    return true;
}
