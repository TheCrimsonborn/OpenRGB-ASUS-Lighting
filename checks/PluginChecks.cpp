// SPDX-License-Identifier: GPL-2.0-or-later
#include "AsioSuperIo.h"
#include "OpenRGBPluginInterface.h"
#include <QApplication>
#include <QPluginLoader>
#include <QComboBox>
#include <QPushButton>
#include <cstdio>
#define CHECK(x) do { if(!(x)){std::fprintf(stderr,"Check failed at line %d: %s\n",__LINE__,#x);return 1;} }while(0)
int main(int argc,char** argv)
{
    QApplication app(argc,argv);
    LightingSettings s; LightingEncoding e; QString error;
    s.colors={QColor(0x12,0x34,0x56),QColor(0xAB,0xCD,0xEF)};
    CHECK(EncodeLighting(s,&e,&error)); CHECK(e.mode==0); CHECK(e.palette[0]==0xA1); CHECK(e.palette[4]==0xC3);CHECK(e.palette[8]==0xE5);CHECK(e.palette[3]==0xA1);CHECK(e.single==0);
    s.colors={Qt::white}; CHECK(EncodeLighting(s,&e,&error));CHECK(e.single==1);for(auto b:e.palette) CHECK(b==255);
    const int modes[]={0,2,3,3,1};
    for(int m=0;m<5;++m){s.mode=static_cast<LightingSettings::Mode>(m);CHECK(EncodeLighting(s,&e,&error));CHECK(e.mode==modes[m]);}
    s.mode=LightingSettings::Blinking;const int blink[]={16,8,1};
    for(int i=0;i<3;++i){s.speed=i;CHECK(EncodeLighting(s,&e,&error));CHECK(e.speed==blink[i]);CHECK(e.timing[0]==6);}
    s.mode=LightingSettings::Strobing;const int strobe[]={16,10,7};
    for(int i=0;i<3;++i){s.speed=i;CHECK(EncodeLighting(s,&e,&error));CHECK(e.speed==strobe[i]);CHECK(e.timing[0]==2&&e.timing[1]==15&&e.timing[2]==255&&e.timing[3]==255);}
    s.mode=LightingSettings::Fading;CHECK(EncodeLighting(s,&e,&error));CHECK(e.speed==16&&e.timing[0]==3&&e.timing[1]==3);
    s.colors={Qt::red,Qt::green,Qt::blue};CHECK(!EncodeLighting(s,&e,&error));s.colors={Qt::red};s.speed=3;CHECK(!EncodeLighting(s,&e,&error));
    CHECK(argc>1); QPluginLoader loader(QString::fromLocal8Bit(argv[1]));auto* instance=loader.instance();
    if(!instance) std::fprintf(stderr,"%s\n",loader.errorString().toUtf8().constData());
    CHECK(instance);auto* plugin=qobject_cast<OpenRGBPluginInterface*>(instance);CHECK(plugin);CHECK(plugin->GetPluginAPIVersion()==5);plugin->Load(nullptr);
    QWidget* w=plugin->GetWidget();CHECK(w);
    auto* mode=w->findChild<QComboBox*>("lightingMode");auto* speed=w->findChild<QComboBox*>("lightingSpeed");auto* stages=w->findChild<QComboBox*>("lightingStages");
    CHECK(mode&&speed&&stages);CHECK(mode->count()==5);CHECK(stages->count()==4);
    for(int m=0;m<5;++m){mode->setCurrentIndex(m);CHECK(speed->isEnabled()==(m==1||m==3));CHECK(stages->isEnabled()==(m!=4));}
    mode->setCurrentIndex(2);stages->setCurrentIndex(3);speed->setCurrentIndex(2);
    for(int i=1;i<=8;++i){auto* button=w->findChild<QPushButton*>(QString("stageColor%1").arg(i));CHECK(button&&button->isEnabled());}
    const auto saved=plugin->OnProfileSave();CHECK(saved.at("mode")==2&&saved.at("stages")==8&&saved.at("speed")==2);
    mode->setCurrentIndex(0);stages->setCurrentIndex(0);plugin->OnProfileLoad(saved);CHECK(plugin->OnProfileSave()==saved);
    auto invalid=saved;invalid["stages"]=3;plugin->OnProfileLoad(invalid);CHECK(plugin->OnProfileSave()==saved);
    plugin->Unload();CHECK(loader.unload());std::puts("Encoding, mode controls, stage controls, profile round-trip and invalid-profile checks passed. No hardware commands sent.");return 0;
}
