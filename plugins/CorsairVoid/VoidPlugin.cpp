// SPDX-License-Identifier: GPL-2.0-or-later
#include "VoidPlugin.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMetaObject>

OpenRGBPluginInfo VoidPlugin::GetPluginInfo() {
    OpenRGBPluginInfo info{};info.Name="Corsair VOID ELITE USB Lighting";
    info.Description="Headset LED on/off for Corsair VOID ELITE USB (1B1C:0A56)";
    info.Version="0.5.0";info.Location=OPENRGB_PLUGIN_LOCATION_DEVICES;info.Label="VOID Headset";
    return info;
}
void VoidPlugin::Load(OpenRGBPluginAPIInterface* api) {
    if(widget_)return;api_=api;closing_=false;
    widget_=new QWidget;auto* layout=new QVBoxLayout(widget_);
    layout->addWidget(new QLabel("Corsair VOID ELITE USB lighting",widget_));
    auto* note=new QLabel("LED on/off only. Turning LEDs on restores the headset's existing effect. The LED state is not queried automatically.",widget_);note->setWordWrap(true);layout->addWidget(note);
    auto* on=new QPushButton("Turn headset LEDs on",widget_);auto* off=new QPushButton("Turn headset LEDs off",widget_);
    on->setObjectName("headsetOn");off->setObjectName("headsetOff");layout->addWidget(on);layout->addWidget(off);
    status_=new QLabel("No command sent.",widget_);status_->setWordWrap(true);layout->addWidget(status_);layout->addStretch();
    connect(on,&QPushButton::clicked,this,[this](){Send(true);});
    connect(off,&QPushButton::clicked,this,[this](){Send(false);});
    QString error;const bool found=hid_.Detect(&error);Status(found?"Headset found. No command sent.":error);

}
void VoidPlugin::Status(const QString& text) {
    QPointer<QLabel> label=status_;if(!label)return;
    QMetaObject::invokeMethod(label.data(),[label,text](){if(label)label->setText(text);},Qt::QueuedConnection);
}
void VoidPlugin::Send(bool on) {
    if(closing_)return;QString error;const bool ok=hid_.SetLights(on,&error);
    Status(ok?(on?"LED ON command sent. Check the headset.":"LED OFF command sent. Check the headset."):error);
    if(api_)api_->LogEntry("VoidPlugin.cpp",__LINE__,ok?3:1,"[VOID headset] LED %s: %s",on?"ON":"OFF",ok?"HID write accepted":error.toUtf8().constData());
}
void VoidPlugin::Unload() {
    closing_=true;delete widget_;widget_=nullptr;status_.clear();api_=nullptr;
}
