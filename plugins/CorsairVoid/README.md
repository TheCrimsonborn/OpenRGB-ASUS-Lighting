# Corsair VOID ELITE USB Lighting

Independent OpenRGB 1.1 / Plugin API 5 extension for Windows x64, MSVC, and Qt 6.8.3.

Open **Devices** and select the headset icon for **Corsair VOID ELITE USB**. Choose **On** or **Off** in the native Mode control. Its device details appear under Information. A diagnostic page with explicit buttons is available under Settings. Enabling the LEDs restores the headset's existing lighting effect; this plugin does not select colors or effects.

The plugin targets only VID 1B1C, PID 0A56, USB interface 3, usage page FFC5, usage 0001. Enumeration must yield exactly one matching interface. Each button resolves the current device path, opens the interface, submits one three-byte LED payload, and closes it. Windows may pad the payload to the device's output report size. The host may apply device defaults or saved profiles during registration/startup. Physical LED state is not queried.

The user confirmed LED OFF physically works on the connected headset. The ON packet follows the same protocol, but physical ON behavior has not yet been confirmed. Protocol reference: https://github.com/Sapd/HeadsetControl/blob/master/lib/devices/corsair_void_rich.hpp (setLights).

Install OpenRGBCorsairVoidLighting.dll into %APPDATA%\OpenRGB\plugins with OpenRGB closed. Keep OpenRGB's existing hidapi-hotplug.dll beside OpenRGB.exe. Reopen OpenRGB. The ASUS Lighting page is a separate plugin.

Configure CMake with OPENRGB_SOURCE_DIR pointing to the complete OpenRGB 1.1 source tree, CMAKE_PREFIX_PATH pointing to the MSVC Qt 6.8.3 kit, and use MSVC x64. Enable ASUS_LIGHTING_BUILD_CHECKS to build VoidChecks. VoidChecks verifies exact interface targeting, packet bytes, Devices page placement, UI creation and unloading without invoking HID writes.

GPL-2.0-or-later; see LICENSE. OpenRGB interface and HIDAPI headers are supplied externally at build time. No vendor or HIDAPI binaries are distributed here.
