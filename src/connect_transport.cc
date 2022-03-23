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

#include "connect_transport.h"

#include <asm-generic/errno.h>
#include <glog/logging.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <utility>

#include "endpoint.h"
#include "owned_fd.h"
#include "poller.h"

namespace urpc {

ConnectTransport::~ConnectTransport() {}

int ConnectTransport::DoWrite() {
    if (ConnectIfNot() == 0) {
        HandleWriteEvent();
    }

    return 0;
}

int ConnectTransport::HandleWriteEvent() {
    if (connecting_) {
        OnConnect();
    }

    return Transport::HandleWriteEvent();
}

int ConnectTransport::ConnectIfNot() {
    if (connected_) {
        return 0;
    }

    if (connecting_) {
        return -1;
    }

    // REQUIRED: Linux >= 2.6.27
    urpc::OwnedFD sockfd(
        socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0));
    if (sockfd < 0) {
        return -1;
    }

    struct sockaddr_in serv_addr;
    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = endpoint_.ip;
    serv_addr.sin_port = htons(endpoint_.port);
    int rc = ::connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (rc < 0 && errno != EINPROGRESS) {
        return -1;
    }

    fd_ = std::move(sockfd);
    if (rc < 0 && errno == EINPROGRESS) {
        connecting_ = true;
        Poller::singleton()->AddPollOut(this);
    } else {
        connected_ = true;
    }

    return rc;
}

int ConnectTransport::OnConnect() {
    connected_ = true;
    connecting_ = false;
    return 0;
}

void ConnectTransport::Reset(int code, std::string reason) {
    connecting_ = false;
    connected_ = false;

    Transport::Reset(code, std::move(reason));
}

}  // namespace urpc
