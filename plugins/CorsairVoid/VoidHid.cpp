// SPDX-License-Identifier: GPL-2.0-or-later
#include "VoidHid.h"
#include <QCoreApplication>

bool VoidHid::Resolve(QString* error) {
    if(enumerate_)return true;
    library_.setFileName(QCoreApplication::applicationDirPath()+"/hidapi-hotplug.dll");
    if(!library_.load()){*error=library_.errorString();return false;}
    auto enumerate=reinterpret_cast<Enumerate>(library_.resolve("hid_enumerate"));
    auto free=reinterpret_cast<Free>(library_.resolve("hid_free_enumeration"));
    auto open=reinterpret_cast<Open>(library_.resolve("hid_open_path"));
    auto write=reinterpret_cast<Write>(library_.resolve("hid_write"));
    auto close=reinterpret_cast<Close>(library_.resolve("hid_close"));
    auto err=reinterpret_cast<Error>(library_.resolve("hid_error"));
    if(!enumerate||!free||!open||!write||!close||!err){*error="Missing HID API functions.";return false;}
    enumerate_=enumerate;free_=free;open_=open;write_=write;close_=close;error_=err;return true;
}
bool VoidHid::Find(QByteArray* path,QString* error) {
    if(!Resolve(error))return false;
    hid_device_info* first=enumerate_(0x1B1C,0x0A56); int count=0;
    for(auto* d=first;d;d=d->next)if(IsVoidLighting(*d)&&d->path){*path=QByteArray(d->path);++count;}
    free_(first);
    if(count!=1){*error=QString("Expected one VOID ELITE USB lighting interface; found %1. No command sent.").arg(count);return false;}
    return true;
}
bool VoidHid::Detect(QString* error) {
    std::lock_guard<std::mutex> lock(mutex_);QByteArray path;return Find(&path,error);
}
bool VoidHid::SetLights(bool on,QString* error) {
    std::lock_guard<std::mutex> lock(mutex_);QByteArray path;
    if(!Find(&path,error))return false;
    hid_device* handle=open_(path.constData());
    if(!handle){const wchar_t* text=error_(nullptr);*error="Cannot open headset lighting interface: "+(text?QString::fromWCharArray(text):QString("unknown HID error"));return false;}
    const auto packet=VoidPacket(on);const int sent=write_(handle,packet.data(),packet.size());
    // Windows HIDAPI may pad the three-byte payload to the 20-byte report.
    if(sent<static_cast<int>(packet.size())){const wchar_t* text=error_(handle);*error=QString("Headset LED write failed (%1): %2").arg(sent).arg(text?QString::fromWCharArray(text):QString("unknown HID error"));}
    close_(handle);return sent>=static_cast<int>(packet.size());
}
