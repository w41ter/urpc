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
#include <urpc/controller.h>

namespace urpc {

Controller::~Controller() {}

void Controller::Reset() {
    error_code_ = 0;
    error_text_.clear();
    completed_ = false;
}

void Controller::StartCancel() { LOG(FATAL) << "Not Supported"; }

void Controller::SetFailed(const std::string& reason) {
    LOG(FATAL) << "Not Supported";
}

void Controller::NotifyOnCancel(Closure* callback) {
    LOG(FATAL) << "Not Supported";
}

void Controller::SetFailed(int err_code, std::string reason) {
    error_code_ = err_code;
    error_text_ = std::move(reason);
}

void Controller::OnComplete() { LOG(FATAL) << "Not implemented"; }

}  // namespace urpc
