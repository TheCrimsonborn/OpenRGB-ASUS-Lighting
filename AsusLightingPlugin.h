// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <QObject>

#include "OpenRGBPluginInterface.h"

class QWidget;

class AsusLightingPlugin final : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID OpenRGBPluginInterface_IID FILE "plugin.json")
    Q_INTERFACES(OpenRGBPluginInterface)

public:
    OpenRGBPluginInfo GetPluginInfo() override;
    unsigned int GetPluginAPIVersion() override;
    void Load(OpenRGBPluginAPIInterface* pluginApi) override;
    QWidget* GetWidget() override;
    QMenu* GetTrayMenu() override;
    void Unload() override;
    void OnProfileAboutToLoad() override;
    void OnProfileLoad(nlohmann::json profileData) override;
    nlohmann::json OnProfileSave() override;
    unsigned char* OnSDKCommand(unsigned int packetId, unsigned char* packetData, unsigned int* packetSize) override;
    void ProfileManagerUpdated(unsigned int updateReason) override;
    void ResourceManagerUpdated(unsigned int updateReason) override;
    void SettingsManagerUpdated(unsigned int updateReason) override;

private:
    QWidget* widget_ = nullptr;
};
