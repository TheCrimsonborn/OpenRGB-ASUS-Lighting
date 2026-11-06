// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "OpenRGBPluginInterface.h"
#include <thread>

class NativeController {
public:
    RGBControllerInterface* device=nullptr;
    bool Start(OpenRGBPluginAPIInterface* api, RGBController_Setup& setup) {
        if(!api)return false; api_=api; device=api_->CreateVirtualRGBController(&setup);
        if(!device)return false;
        worker_=std::thread([this](){api_->RegisterVirtualRGBController(device);registered_=true;});
        return true;
    }
    void Stop() {
        if(worker_.joinable())worker_.join();
        if(device&&api_){if(registered_)api_->UnregisterVirtualRGBController(device);api_->DeleteVirtualRGBController(device);}
        device=nullptr;registered_=false;api_=nullptr;
    }
private:
    OpenRGBPluginAPIInterface* api_=nullptr;
    std::thread worker_;
    bool registered_=false;
};
