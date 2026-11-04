// SPDX-License-Identifier: GPL-2.0-or-later
#include "AsioSuperIo.h"

#include <windows.h>

#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QLibrary>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace
{
constexpr DWORD SMBIOS_PROVIDER = 0x52534D42; // 'RSMB'
constexpr BYTE SIO_LDN_REGISTER = 0x07;
constexpr BYTE ASUS_LED_LDN = 0x13;
constexpr BYTE ASUS_LED_MODE_REGISTER = 0xFF;

QString ReadBaseboardProduct()
{
    const UINT required = GetSystemFirmwareTable(SMBIOS_PROVIDER, 0, nullptr, 0);
    if (required < 8)
    {
        return {};
    }

    std::vector<BYTE> table(required);
    const UINT received = GetSystemFirmwareTable(SMBIOS_PROVIDER, 0, table.data(), required);
    if (received < 8)
    {
        return {};
    }

    DWORD tableLength = 0;
    std::memcpy(&tableLength, table.data() + 4, sizeof(tableLength));
    const size_t end = std::min<size_t>(received, static_cast<size_t>(tableLength) + 8);
    size_t offset = 8;

    while (offset + 4 <= end)
    {
        const BYTE type = table[offset];
        const BYTE length = table[offset + 1];
        if (length < 4 || offset + length > end)
        {
            break;
        }

        if (type == 127)
        {
            break;
        }

        if (type == 2 && length >= 6)
        {
            const BYTE productIndex = table[offset + 5];
            size_t stringOffset = offset + length;
            BYTE currentIndex = 1;

            while (stringOffset < end && table[stringOffset] != 0)
            {
                const size_t start = stringOffset;
                while (stringOffset < end && table[stringOffset] != 0)
                {
                    ++stringOffset;
                }

                if (currentIndex == productIndex)
                {
                    return QString::fromLocal8Bit(
                        reinterpret_cast<const char*>(table.data() + start),
                        static_cast<int>(stringOffset - start));
                }

                ++currentIndex;
                if (stringOffset < end)
                {
                    ++stringOffset;
                }
            }
        }

        size_t next = offset + length;
        while (next + 1 < end && !(table[next] == 0 && table[next + 1] == 0))
        {
            ++next;
        }
        if (next + 1 >= end)
        {
            break;
        }
        offset = next + 2;
    }

    return {};
}


// Uses the installed OpenRGB PawnIO library and signed LpcIO module.
class PawnSession
{
    using OpenFn = HRESULT(WINAPI*)(PHANDLE);
    using LoadFn = HRESULT(WINAPI*)(HANDLE, const UCHAR*, SIZE_T);
    using ExecFn = HRESULT(WINAPI*)(HANDLE, PCSTR, const ULONG64*, SIZE_T, PULONG64, SIZE_T, PSIZE_T);
    using CloseFn = HRESULT(WINAPI*)(HANDLE);
public:
    ~PawnSession()
    {
        if (entered) { ULONG64 exitArgs[] = {0x2E, 0xAA}; Call("ioctl_pio_outb", exitArgs, 2); }
        if (handle && closeFn) closeFn(handle);
        if (locked) ReleaseMutex(mutex);
        if (mutex) CloseHandle(mutex);
    }
    bool Open(QString* error)
    {
        error_ = error;
        mutex = CreateMutexW(nullptr, FALSE, L"Global\\Access_ISABUS.HTP.Method");
        if (!mutex) { *error = QStringLiteral("Could not open the shared Super I/O lock (%1).").arg(GetLastError()); return false; }
        const DWORD wait = WaitForSingleObject(mutex, 3000);
        if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED) { *error = QStringLiteral("Super I/O is busy. Close AI Suite or hardware monitoring tools and retry."); return false; }
        locked = true;
        const QString dir = QCoreApplication::applicationDirPath();
        library.setFileName(dir + QStringLiteral("/PawnIOLib.dll"));
        if (!library.load()) { *error = library.errorString(); return false; }
        auto openFn = reinterpret_cast<OpenFn>(library.resolve("pawnio_open"));
        auto loadFn = reinterpret_cast<LoadFn>(library.resolve("pawnio_load"));
        execFn = reinterpret_cast<ExecFn>(library.resolve("pawnio_execute"));
        closeFn = reinterpret_cast<CloseFn>(library.resolve("pawnio_close"));
        if (!openFn || !loadFn || !execFn || !closeFn) { *error = QStringLiteral("PawnIOLib is missing required functions."); return false; }
        QFile file(dir + QStringLiteral("/LpcIO.bin"));
        if (!file.open(QIODevice::ReadOnly)) { *error = QStringLiteral("OpenRGB LpcIO.bin module is missing or unreadable."); return false; }
        const QByteArray blob = file.readAll();
        if (blob.isEmpty()) { *error = QStringLiteral("LpcIO.bin is empty."); return false; }
        HRESULT status = openFn(&handle);
        if (FAILED(status)) return Failed("pawnio_open", status);
        status = loadFn(handle, reinterpret_cast<const UCHAR*>(blob.constData()), static_cast<SIZE_T>(blob.size()));
        if (FAILED(status)) return Failed("pawnio_load LpcIO", status);
        const ULONG64 slot[] = {0}; // The 0x2E / 0x2F slot.
        return Call("ioctl_select_slot", slot, 1);
    }
    bool Enter()
    {
        const ULONG64 unlock[] = {0x2E, 0x87};
        // Attempt exit even if only the first unlock succeeds.
        entered = true;
        return Call("ioctl_pio_outb", unlock, 2) && Call("ioctl_pio_outb", unlock, 2);
    }
    bool Write(BYTE reg, BYTE value)
    {
        const ULONG64 args[] = {reg, value};
        return Call("ioctl_superio_outb", args, 2);
    }
    bool Read(BYTE reg, BYTE* value)
    {
        const ULONG64 args[] = {reg}; ULONG64 result = 0; SIZE_T count = 0;
        const HRESULT status = execFn(handle, "ioctl_superio_inb", args, 1, &result, 1, &count);
        if (FAILED(status)) return Failed("ioctl_superio_inb", status);
        if (count != 1 || result > 255) { *error_ = QStringLiteral("PawnIO returned an invalid register value."); return false; }
        *value = static_cast<BYTE>(result); return true;
    }
private:
    bool Failed(const char* operation, HRESULT status)
    {
        *error_ = QStringLiteral("PawnIO %1 failed (0x%2).").arg(QString::fromLatin1(operation)).arg(static_cast<quint32>(status), 8, 16, QLatin1Char('0'));
        return false;
    }
    bool Call(const char* operation, const ULONG64* input, SIZE_T count)
    {
        SIZE_T written = 0;
        const HRESULT status = execFn(handle, operation, input, count, nullptr, 0, &written);
        return SUCCEEDED(status) || Failed(operation, status);
    }
    QLibrary library;
    HANDLE mutex = nullptr, handle = nullptr;
    bool locked = false, entered = false;
    ExecFn execFn = nullptr;
    CloseFn closeFn = nullptr;
    QString* error_ = nullptr;
};
}

bool AsioSuperIo::IsSupportedBoard(QString* product)
{
    const QString detected = ReadBaseboardProduct().trimmed();
    if (product)
    {
        *product = detected;
    }

    return detected.compare(QStringLiteral("PRIME H610M-K D4"), Qt::CaseInsensitive) == 0;
}

bool AsioSuperIo::Apply(const LightingSettings& settings, QString* error)
{
    QString localError;
    QString* resultError = error ? error : &localError;
    resultError->clear();

    if (!IsSupportedBoard())
    {
        *resultError = QStringLiteral("This control is restricted to the ASUS PRIME H610M-K D4.");
        return false;
    }

    LightingEncoding encoding;
    if (!EncodeLighting(settings, &encoding, resultError)) return false;
    PawnSession io;
    if (!io.Open(resultError) || !io.Enter()) return false;
    BYTE previousLdn = 0;
    if (!io.Read(SIO_LDN_REGISTER, &previousLdn)) return false;
    // SIORGB.dll 0x421244: LDN12 ED |= E0, then LDN13 FF mode.
    // OFF is mode 1; static is mode 0. FC controls single/multiple color,
    // and the vendor OFF path does not touch FC at all.
    BYTE pinConfig = 0, modeBefore = 0;
    auto apply = [&]() -> bool {
        if (!io.Write(SIO_LDN_REGISTER, 0x12) || !io.Read(0xED, &pinConfig)
            || !io.Write(0xED, static_cast<BYTE>(pinConfig | 0xE0))
            || !io.Write(SIO_LDN_REGISTER, ASUS_LED_LDN)
            || !io.Read(ASUS_LED_MODE_REGISTER, &modeBefore)) return false;
        const BYTE requestedMode = static_cast<BYTE>(((modeBefore | 1) & 0xF9) | (encoding.mode << 1));
        if (!io.Write(ASUS_LED_MODE_REGISTER, requestedMode)) return false;
        if (settings.mode != LightingSettings::Off)
        {
            // Encode all eight color stages in the vendor nibble order.
            for (BYTE reg = 0xF0; reg <= 0xFB; ++reg)
                if (!io.Write(reg, encoding.palette[reg - 0xF0])) return false;
            if (!io.Write(0xFC, encoding.single) || !io.Write(SIO_LDN_REGISTER, 0x15)) return false;
            // Vendor defaults in 0x420F54 and fixed timing bytes in 0x4214D6.
            const BYTE timing[][2] = {
                {0xF8,encoding.speed},{0xF3,encoding.timing[0]},{0xF4,encoding.timing[1]},{0xF9,encoding.timing[2]},{0xFA,encoding.timing[3]},
                {0xF0,0xFF},{0xF1,0x7F},{0xF2,0},{0xF5,0x17},
                {0xF6,0},{0xF7,0x0F},{0xFB,0xFF}
            };
            for (const auto& pair : timing)
                if (!io.Write(pair[0], pair[1])) return false;
            if (!io.Write(SIO_LDN_REGISTER, ASUS_LED_LDN)) return false;
        }
        BYTE actual = 0;
        if (!io.Read(ASUS_LED_MODE_REGISTER, &actual)) return false;
        if ((actual & 7) != (requestedMode & 7))
        {
            *resultError = QStringLiteral("LED mode was not confirmed (read 0x%1).").arg(actual, 2, 16, QLatin1Char('0'));
            return false;
        }
        return true;
    };
    const bool completed = apply();
    const QString originalError = *resultError;
    const bool restored = io.Write(SIO_LDN_REGISTER, previousLdn);
    if (!completed) { *resultError = originalError; return false; }
    return restored;
}
