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

#include <glog/logging.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>
#include <urpc/endpoint.h>
#include <urpc/server.h>

#include <memory>

#include "acceptor.h"
#include "service_holder.h"
#include "utils/owner_ptr.h"

using namespace google::protobuf;

namespace urpc {

class ServerImpl {
public:
    ~ServerImpl();

    int Start(EndPoint endpoint);

private:
    utils::owner_ptr<Acceptor> acceptor_;
};

int ServerImpl::Start(EndPoint endpoint) {
    int fd = tcp_listen(endpoint);
    if (fd < 0)
        return fd;

    LOG(INFO) << "Server make Acceptor " << fd;
    acceptor_.reset(new Acceptor(fd));
    return 0;
}

ServerImpl::~ServerImpl() {}

Server::Server() : impl_(new ServerImpl) {}

Server::~Server() {}

int Server::AddService(Service* service, ServiceOwnership ownership) {
    return ServiceHolder::singleton()->AddService(service, ownership);
}

int Server::Start(EndPoint endpoint) { return impl_->Start(endpoint); }

}  // namespace urpc
