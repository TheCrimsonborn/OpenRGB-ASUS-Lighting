// SPDX-License-Identifier: GPL-2.0-or-later
#include "AsioSuperIo.h"
#include "NativeSetups.h"
#include "OpenRGBPluginInterface.h"
#include <QApplication>
#include <QJsonObject>
#include <QPluginLoader>
#include <cstdio>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"Check failed at line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static void Dummy(void*){}
int main(int argc,char** argv){
    QApplication app(argc,argv);LightingSettings s;LightingEncoding e;QString error;
    s.colors={QColor(0x12,0x34,0x56),QColor(0xAB,0xCD,0xEF)};CHECK(EncodeLighting(s,&e,&error));CHECK(e.palette[0]==0xA1&&e.palette[4]==0xC3&&e.palette[8]==0xE5&&e.palette[3]==0xA1);
    s.colors={Qt::white};CHECK(EncodeLighting(s,&e,&error));CHECK(e.single==1);for(auto b:e.palette)CHECK(b==255);
    const int modes[]={0,2,3,3,1};for(int m=0;m<5;++m){s.mode=static_cast<LightingSettings::Mode>(m);CHECK(EncodeLighting(s,&e,&error));CHECK(e.mode==modes[m]);}
    s.mode=LightingSettings::Blinking;const int blink[]={16,8,1};for(int i=0;i<3;++i){s.speed=i;CHECK(EncodeLighting(s,&e,&error));CHECK(e.speed==blink[i]);}
    s.mode=LightingSettings::Strobing;const int strobe[]={16,10,7};for(int i=0;i<3;++i){s.speed=i;CHECK(EncodeLighting(s,&e,&error));CHECK(e.speed==strobe[i]&&e.timing[0]==2&&e.timing[1]==15&&e.timing[2]==255&&e.timing[3]==255);}
    s.mode=LightingSettings::Fading;CHECK(EncodeLighting(s,&e,&error));CHECK(e.speed==16&&e.timing[0]==3&&e.timing[1]==3);
    for(int n=1;n<=8;++n){s.colors=QVector<QColor>(n,Qt::red);CHECK(EncodeLighting(s,&e,&error));}s.colors.clear();CHECK(!EncodeLighting(s,&e,&error));s.colors=QVector<QColor>(9,Qt::red);CHECK(!EncodeLighting(s,&e,&error));s.colors={Qt::red};s.speed=3;CHECK(!EncodeLighting(s,&e,&error));
    const auto setup=AsusNativeSetup(nullptr,&Dummy);CHECK(setup.type==DEVICE_TYPE_MOTHERBOARD&&setup.modes.size()==5&&setup.zones.size()==1&&setup.active_mode==4);CHECK(setup.DeviceUpdateMode==&Dummy&&setup.DeviceUpdateLEDs==&Dummy);
    CHECK(setup.modes[0].colors_min==1&&setup.modes[0].colors_max==8);CHECK(setup.modes[1].flags&MODE_FLAG_HAS_SPEED);CHECK(!(setup.modes[2].flags&MODE_FLAG_HAS_SPEED));CHECK(setup.modes[4].color_mode==MODE_COLORS_NONE);
    CHECK(argc>1);QPluginLoader loader(QString::fromLocal8Bit(argv[1]));auto* instance=loader.instance();if(!instance)std::fprintf(stderr,"%s\n",loader.errorString().toUtf8().constData());CHECK(instance);auto* plugin=qobject_cast<OpenRGBPluginInterface*>(instance);CHECK(plugin);CHECK(plugin->GetPluginAPIVersion()==5);auto info=plugin->GetPluginInfo();CHECK(info.Location==OPENRGB_PLUGIN_LOCATION_SETTINGS);CHECK(!info.Commit.empty()&&info.URL.find("https://github.com/")==0);CHECK(loader.metaData().value("MetaData").toObject().value("Commit").toString().toStdString()==info.Commit);
    plugin->Load(nullptr);CHECK(plugin->GetWidget());plugin->Unload();CHECK(loader.unload());std::puts("ASUS encoding, native motherboard/modes/colors, metadata and lifecycle checks passed. No hardware commands sent.");return 0;
}
