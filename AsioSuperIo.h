// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include <array>

struct LightingSettings
{
    enum Mode { Static, Blinking, Fading, Strobing, Off };
    Mode mode = Static;
    int speed = 0;
    QVector<QColor> colors = {Qt::red};
};

struct LightingEncoding
{
    int mode = 0;
    unsigned char speed = 0x10;
    std::array<unsigned char, 4> timing = {6,5,6,5};
    std::array<unsigned char, 12> palette{};
    unsigned char single = 1;
};

bool EncodeLighting(const LightingSettings& settings, LightingEncoding* encoded, QString* error);

class AsioSuperIo
{
public:
    static bool IsSupportedBoard(QString* product = nullptr);
    static bool Apply(const LightingSettings& settings, QString* error = nullptr);
};
