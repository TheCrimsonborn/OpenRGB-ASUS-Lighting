// SPDX-License-Identifier: GPL-2.0-or-later
#include "AsioSuperIo.h"

bool EncodeLighting(const LightingSettings& s, LightingEncoding* e, QString* error)
{
    if (s.mode < LightingSettings::Static || s.mode > LightingSettings::Off || s.speed < 0 || s.speed > 2 ||
        (s.colors.size() < 1 || s.colors.size() > 8))
    { if(error) *error = "Invalid mode, speed, or stage count (use 1 to 8)."; return false; }
    for(const QColor& c : s.colors) if(!c.isValid())
    { if(error) *error = "Invalid stage color."; return false; }
    *e = LightingEncoding{};
    const int modes[] = {0,2,3,3,1};
    e->mode = modes[s.mode];
    if(s.mode == LightingSettings::Blinking)
    { const unsigned char speeds[] = {0x10,8,1}; e->speed = speeds[s.speed]; }
    if(s.mode == LightingSettings::Fading) e->timing = {3,3,6,5};
    if(s.mode == LightingSettings::Strobing)
    { const unsigned char speeds[] = {0x10,0x0A,7}; e->speed = speeds[s.speed]; e->timing = {2,15,255,255}; }
    e->single = s.colors.size() == 1 ? 1 : 0;
    // SIORGB 0x421134 packs odd color into low nibble and even into high.
    // F0..F3 = red pairs, F4..F7 = green, F8..FB = blue.
    for(int pair=0; pair<4; ++pair)
    {
        const QColor a=s.colors[(pair*2)%s.colors.size()], b=s.colors[(pair*2+1)%s.colors.size()];
        e->palette[pair] = static_cast<unsigned char>((b.red() & 0xF0) | (a.red() >> 4));
        e->palette[4+pair] = static_cast<unsigned char>((b.green() & 0xF0) | (a.green() >> 4));
        e->palette[8+pair] = static_cast<unsigned char>((b.blue() & 0xF0) | (a.blue() >> 4));
    }
    return true;
}
