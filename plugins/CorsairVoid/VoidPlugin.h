// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <QObject>
#include <QPointer>
#include <QLabel>
#include <atomic>
#include "../../NativeController.h"
#include "OpenRGBPluginInterface.h"
#include "VoidHid.h"
class VoidPlugin final:public QObject,public OpenRGBPluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID FILE "VoidPluginMetadata.json")
    Q_INTERFACES(OpenRGBPluginInterface)
public:
    OpenRGBPluginInfo GetPluginInfo() override;
    unsigned int GetPluginAPIVersion() override {return OPENRGB_PLUGIN_API_VERSION;}
    void Load(OpenRGBPluginAPIInterface* api) override;
    QWidget* GetWidget() override {return widget_;}
    QMenu* GetTrayMenu() override {return nullptr;}
    void Unload() override;
    void OnProfileAboutToLoad() override {}
    void OnProfileLoad(nlohmann::json) override {}
    nlohmann::json OnProfileSave() override {return nlohmann::json::object();}
    unsigned char* OnSDKCommand(unsigned int,unsigned char*,unsigned int* size) override {if(size)*size=0;return nullptr;}
    void ProfileManagerUpdated(unsigned int) override {}
    void ResourceManagerUpdated(unsigned int) override {}
    void SettingsManagerUpdated(unsigned int) override {}
private:
    void Send(bool on);
    void Status(const QString& text);
    OpenRGBPluginAPIInterface* api_=nullptr;
    QWidget* widget_=nullptr;
    QPointer<QLabel> status_;
    VoidHid hid_;
    std::atomic<bool> closing_{false};
    NativeController native_;
    static void UpdateNative(void* self);
};
