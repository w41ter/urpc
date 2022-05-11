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

#include "service_holder.h"

#include <glog/logging.h>

#include "urpc/server.h"

using namespace google::protobuf;

namespace urpc {

ServiceHolder* ServiceHolder::singleton() {
    static thread_local ServiceHolder holder;
    return &holder;
}

ServiceHolder::ServiceHolder() = default;

ServiceHolder::~ServiceHolder() {
    for (auto&& service : owned_services_) {
        delete service;
    }
}

int ServiceHolder::AddService(Service* service, ServiceOwnership ownership) {
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

    services_.insert({descriptor->full_name(), service});
    if (ownership == ServiceOwnership::SERVER_OWNS_SERVICE) {
        owned_services_.push_back(service);
    }

    return 0;
}

const MethodDescriptor* ServiceHolder::FindMethod(
    const std::string& service_name, const std::string& method_name) {
    std::string full_name = service_name + "." + method_name;
    auto it = descriptor_.find(full_name);
    if (it == descriptor_.end()) {
        return nullptr;
    }
    return it->second;
}

Service* ServiceHolder::FindService(const std::string& service_name) {
    auto it = services_.find(service_name);
    if (it == services_.end()) {
        return nullptr;
    }
    return it->second;
}

}  // namespace urpc
