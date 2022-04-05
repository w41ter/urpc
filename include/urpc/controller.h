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

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

#include <string>


namespace urpc {

using google::protobuf::Closure;

class Controller : public google::protobuf::RpcController {
public:
    ~Controller() override;

    void Reset() override;

    int ErrorCode() const { return error_code_; }
    std::string ErrorText() const override { return error_text_; }
    bool Failed() const override { return error_code_ != 0; }

    virtual void SetFailed(int err_code, std::string reason);

protected:
    virtual void OnComplete();

private:
    bool IsCanceled() const override { return false; }
    void StartCancel() override;
    void SetFailed(const std::string& reason) override;
    void NotifyOnCancel(Closure* callback) override;

    bool completed_;
    int error_code_{0};
    std::string error_text_;
};

Controller* NewURPCController();

}  // namespace urpc
