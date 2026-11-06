// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <QString>
#include <QByteArray>
#include <QLibrary>
#include <array>
#include <mutex>
#include "hidapi.h"

inline bool IsVoidLighting(const hid_device_info& d) {
    return d.vendor_id==0x1B1C && d.product_id==0x0A56 && d.interface_number==3 && d.usage_page==0xFFC5 && d.usage==1;
}
inline std::array<unsigned char,3> VoidPacket(bool on) { return {0xC8,static_cast<unsigned char>(on?0:1),0}; }
class VoidHid {
public:
    bool Detect(QString* error);
    bool SetLights(bool on, QString* error);
private:
    bool Resolve(QString* error);
    bool Find(QByteArray* path,QString* error);
    using Enumerate = hid_device_info* (*)(unsigned short,unsigned short);
    using Free = void (*)(hid_device_info*);
    using Open = hid_device* (*)(const char*);
    using Write = int (*)(hid_device*,const unsigned char*,size_t);
    using Close = void (*)(hid_device*);
    using Error = const wchar_t* (*)(hid_device*);
    QLibrary library_;
    Enumerate enumerate_=nullptr; Free free_=nullptr; Open open_=nullptr;
    Write write_=nullptr; Close close_=nullptr; Error error_=nullptr;
    std::mutex mutex_;
};
