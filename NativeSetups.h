// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "OpenRGBPluginInterface.h"

inline void AddLightingZone(RGBController_Setup& s,const char* name) {
    zone z;z.name=name;z.type=ZONE_TYPE_SINGLE;z.leds_count=z.leds_min=z.leds_max=1;s.zones={z};
    led l;l.name=name;s.leds={l};
}
inline RGBController_Setup AsusNativeSetup(void* self,void(*update)(void*)) {
    RGBController_Setup s{};s.name="ASUS PRIME H610M-K D4";s.vendor="ASUS";s.version="0.5.0";
    s.description="Motherboard RGB header controlled through PawnIO";s.location="Super I/O 0x2E, LDN 0x13 (PawnIO)";
    s.type=DEVICE_TYPE_MOTHERBOARD;s.flags=CONTROLLER_FLAG_VIRTUAL;s.active_mode=4;
    s.object_ptr=self;s.DeviceUpdateMode=update;s.DeviceUpdateLEDs=update;
    const char* names[]={"Static","Blinking","Fading","Strobing","Off"};
    for(int i=0;i<5;++i){mode m;m.name=names[i];m.value=i;m.flags=MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
        if(i!=4){m.flags|=MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;m.color_mode=MODE_COLORS_MODE_SPECIFIC;m.colors_min=1;m.colors_max=8;m.colors={ToRGBColor(255,255,255)};}
        if(i==1||i==3){m.flags|=MODE_FLAG_HAS_SPEED;m.speed_min=0;m.speed_max=2;}
        s.modes.push_back(m);
    }
    AddLightingZone(s,"RGB header");return s;
}
inline RGBController_Setup VoidNativeSetup(void* self,void(*update)(void*)) {
    RGBController_Setup s{};s.name="Corsair VOID ELITE USB";s.vendor="Corsair";s.version="0.5.0";
    s.description="Headset LED On/Off control";s.location="USB 1B1C:0A56 interface 3, HID FFC5:0001";
    s.type=DEVICE_TYPE_HEADSET;s.flags=CONTROLLER_FLAG_VIRTUAL;s.active_mode=0;s.object_ptr=self;s.DeviceUpdateMode=update;
    for(int i=0;i<2;++i){mode m;m.name=i?"On":"Off";m.value=i;m.flags=MODE_FLAG_REQUIRES_ENTIRE_DEVICE;s.modes.push_back(m);}
    AddLightingZone(s,"Headset LEDs");return s;
}
