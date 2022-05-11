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

#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include <urpc/server.h>  // ServiceOwnership

namespace urpc {

class ServiceHolder final {
    using Service = google::protobuf::Service;
    using MethodDescriptor = google::protobuf::MethodDescriptor;

public:
    static ServiceHolder* singleton();

    ~ServiceHolder();

    int AddService(Service* service, ServiceOwnership ownership);

    /// Find the corresponding method descriptor, return [`nullptr`] if no such
    /// method are found.
    const MethodDescriptor* FindMethod(const std::string& service_name,
                                       const std::string& method_name);

    /// Find the corresponding service, return [`nullptr`] if no such service
    /// are found.
    Service* FindService(const std::string& service_name);

private:
    ServiceHolder();

    std::vector<Service*> owned_services_;
    std::unordered_map<std::string, Service*> services_;
    std::unordered_map<std::string, const MethodDescriptor*> descriptor_;
};

}  // namespace urpc
