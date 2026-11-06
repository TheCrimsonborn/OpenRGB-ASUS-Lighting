// SPDX-License-Identifier: GPL-2.0-or-later
#include "VoidHid.h"
#include "../../NativeSetups.h"
#include <QJsonObject>
#include "OpenRGBPluginInterface.h"
#include <QApplication>
#include <QPluginLoader>
#include <QPushButton>
#include <cstdio>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"Check failed: %s at %d\n",#x,__LINE__);return 1;}}while(0)
int main(int argc,char** argv){
    QApplication app(argc,argv);
    hid_device_info d{};d.vendor_id=0x1b1c;d.product_id=0x0a56;d.interface_number=3;d.usage_page=0xffc5;d.usage=1;
    CHECK(IsVoidLighting(d));d.usage_page=0xC;CHECK(!IsVoidLighting(d));d.usage_page=0xffc5;d.product_id=0x0a51;CHECK(!IsVoidLighting(d));d.product_id=0x0a56;d.interface_number=0;CHECK(!IsVoidLighting(d));
    const auto off=VoidPacket(false),on=VoidPacket(true);CHECK(off[0]==0xC8&&off[1]==1&&off[2]==0);CHECK(on[0]==0xC8&&on[1]==0&&on[2]==0);
    auto setup=VoidNativeSetup(nullptr,nullptr);CHECK(setup.type==DEVICE_TYPE_HEADSET&&setup.modes.size()==2&&setup.modes[0].name=="Off"&&setup.modes[1].name=="On");
    CHECK(argc>1);QPluginLoader loader(QString::fromLocal8Bit(argv[1]));auto* instance=loader.instance();if(!instance)std::fprintf(stderr,"%s\n",loader.errorString().toUtf8().constData());CHECK(instance);
    auto* plugin=qobject_cast<OpenRGBPluginInterface*>(instance);CHECK(plugin);CHECK(plugin->GetPluginAPIVersion()==5);CHECK(plugin->GetPluginInfo().Location==OPENRGB_PLUGIN_LOCATION_SETTINGS);
    CHECK(!plugin->GetPluginInfo().Commit.empty()&&!plugin->GetPluginInfo().URL.empty());plugin->Load(nullptr);auto* w=plugin->GetWidget();CHECK(w);CHECK(w->findChild<QPushButton*>("headsetOn"));CHECK(w->findChild<QPushButton*>("headsetOff"));plugin->Unload();CHECK(loader.unload());
    std::puts("Exact HID targeting, LED packets, Native headset setup/metadata, UI creation and unload passed. No HID writes sent.");return 0;
}
