// SPDX-License-Identifier: GPL-2.0-or-later
#include "AsusLightingPlugin.h"
#include "AsioSuperIo.h"
#include "NativeSetups.h"
#include <QVBoxLayout>
#include <QMetaObject>

OpenRGBPluginInfo AsusLightingPlugin::GetPluginInfo() {
    OpenRGBPluginInfo info{};info.Name="ASUS PRIME H610M-K D4 Lighting";
    info.Description="Native motherboard RGB device: modes, colors, speeds, and profiles";
    info.Version="0.5.0";info.Commit=PLUGIN_GIT_COMMIT;info.URL=PLUGIN_PROJECT_URL;
    info.Location=OPENRGB_PLUGIN_LOCATION_SETTINGS;info.Label="ASUS Lighting";return info;
}
unsigned int AsusLightingPlugin::GetPluginAPIVersion(){return OPENRGB_PLUGIN_API_VERSION;}
void AsusLightingPlugin::Load(OpenRGBPluginAPIInterface* api) {
    if(widget_)return;api_=api;closing_=false;widget_=new QWidget;auto* layout=new QVBoxLayout(widget_);
    auto* note=new QLabel("ASUS PRIME H610M-K D4 lighting is controlled from Devices. Its hardware details are listed under Information.",widget_);note->setWordWrap(true);layout->addWidget(note);
    status_=new QLabel("No lighting command sent.",widget_);status_->setWordWrap(true);layout->addWidget(status_);layout->addStretch();
    QString product;if(!AsioSuperIo::IsSupportedBoard(&product)){Status("Unsupported motherboard: "+product);return;}
    if(api_){auto setup=AsusNativeSetup(this,&AsusLightingPlugin::UpdateNative);if(!native_.Start(api_,setup))Status("Could not register the motherboard device.");}
}
QWidget* AsusLightingPlugin::GetWidget(){return widget_;}
QMenu* AsusLightingPlugin::GetTrayMenu(){return nullptr;}
void AsusLightingPlugin::Status(const QString& text){QPointer<QLabel> label=status_;if(label)QMetaObject::invokeMethod(label.data(),[label,text](){if(label)label->setText(text);},Qt::QueuedConnection);}
void AsusLightingPlugin::UpdateNative(void* self){auto* p=static_cast<AsusLightingPlugin*>(self);if(!p->closing_)p->ApplyNative();}
void AsusLightingPlugin::ApplyNative(){
    auto* d=native_.device;if(!d||closing_)return;const int active=d->GetActiveMode();if(active<0||active>4)return;
    LightingSettings s;s.mode=static_cast<LightingSettings::Mode>(active);s.speed=static_cast<int>(d->GetModeSpeed(active));
    if(s.mode!=LightingSettings::Off){s.colors.clear();const auto count=d->GetModeColorsCount(active);if(count<1||count>8){Status("Invalid color stage count.");return;}
        for(unsigned int i=0;i<count;++i){auto rgb=d->GetModeColor(active,i);s.colors.push_back(QColor(RGBGetRValue(rgb),RGBGetGValue(rgb),RGBGetBValue(rgb)));}}
    QString error;const bool ok=AsioSuperIo::Apply(s,&error);Status(ok?"Motherboard mode register confirmed. Check the LEDs.":error);
    if(api_)api_->LogEntry("AsusLightingPlugin.cpp",__LINE__,ok?3:1,"[ASUS Lighting] mode=%d stages=%d result=%s",active,int(s.colors.size()),ok?"register confirmed":error.toUtf8().constData());
}
void AsusLightingPlugin::Unload(){closing_=true;native_.Stop();delete widget_;widget_=nullptr;status_.clear();api_=nullptr;}
void AsusLightingPlugin::OnProfileAboutToLoad(){}
void AsusLightingPlugin::OnProfileLoad(nlohmann::json){}
nlohmann::json AsusLightingPlugin::OnProfileSave(){return nlohmann::json::object();}
unsigned char* AsusLightingPlugin::OnSDKCommand(unsigned int,unsigned char*,unsigned int* size){if(size)*size=0;return nullptr;}
void AsusLightingPlugin::ProfileManagerUpdated(unsigned int){}
void AsusLightingPlugin::ResourceManagerUpdated(unsigned int){}
void AsusLightingPlugin::SettingsManagerUpdated(unsigned int){}
