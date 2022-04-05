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

using namespace google::protobuf;

namespace urpc {

class ServerImpl {
public:
    ~ServerImpl();

    int AddService(Service* service, ServiceOwnership ownership);
    int Start(EndPoint endpoint);

private:
    struct ServiceHolder {
        Service* service;
        ServiceOwnership ownership;
    };

    std::unique_ptr<Acceptor> acceptor_;
    std::unordered_map<std::string, ServiceHolder> services_;
    std::unordered_map<std::string, const MethodDescriptor*> descriptor_;
};

int ServerImpl::AddService(Service* service, ServiceOwnership ownership) {
    const ServiceDescriptor* descriptor = service->GetDescriptor();
    LOG(INFO) << "Add service " << descriptor->name();
    LOG(INFO) << "Add service (full name) " << descriptor->full_name();
    int method_count = descriptor->method_count();
    for (int i = 0; i < method_count; ++i) {
        const MethodDescriptor* method = descriptor->method(i);
        LOG(INFO) << "Add method " << method->name();
        LOG(INFO) << "Add method (full name) " << method->full_name();
        descriptor_.insert({method->full_name(), method});
    }

    services_.insert(
        {descriptor->full_name(), ServiceHolder{service, ownership}});
    return 0;
}

int ServerImpl::Start(EndPoint endpoint) {
    int fd = tcp_listen(endpoint);
    if (fd < 0)
        return fd;

    acceptor_ = std::make_unique<Acceptor>(fd);
    return 0;
}

ServerImpl::~ServerImpl() {
    descriptor_.clear();
    for (auto&& [_, holder] : services_) {
        if (holder.ownership == ServiceOwnership::SERVER_OWNS_SERVICE)
            delete holder.service;
    }
}

Server::Server() : impl_(new ServerImpl) {}

Server::~Server() {}

int Server::AddService(Service* service, ServiceOwnership ownership) {
    return impl_->AddService(service, ownership);
}

int Server::Start(EndPoint endpoint) { return impl_->Start(endpoint); }

}  // namespace urpc
