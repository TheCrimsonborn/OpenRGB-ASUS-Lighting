# Third-party notices

- **OpenRGB**: GPL-2.0-or-later. Plugin interface and JSON headers come from the separately supplied source tree at build time. Reference commit: `8e16de27858cf2be6af54a33ce003df74cb9fd3e`. [Source](https://gitlab.com/CalcProgrammer1/OpenRGB). No OpenRGB sources or headers are vendored here.
- **Qt 6.8.3**: used through the developer's MSVC kit and the host runtime. Qt has module-specific commercial and open-source licensing options. No Qt binaries are included. [Licensing information](https://doc.qt.io/qt-6/licensing.html).
- **nlohmann/json**: MIT. Used through the OpenRGB dependency headers. [Source and license](https://github.com/nlohmann/json).
- **PawnIO / PawnIO.Modules**: this plugin uses the PawnIOLib.dll and signed LpcIO.bin already installed with OpenRGB. The library and module source interfaces informed the implementation. PawnIOLib and module sources are LGPL-2.1-or-later; consult upstream for each component's license. No PawnIO binaries are included. [Library](https://github.com/namazso/PawnIO), [modules](https://github.com/namazso/PawnIO.Modules).
- **ASUS Light Control**: proprietary software inspected locally to understand the board's register behavior. No ASUS software, drivers, firmware, logos, or binaries are included.

- **HIDAPI**: the VOID plugin consumes external headers and dynamically loads the HIDAPI DLL supplied with OpenRGB. HIDAPI is offered under GPLv3/BSD/original HIDAPI license options; no HIDAPI binaries or headers are vendored here. [Source](https://github.com/libusb/hidapi).
- **HeadsetControl**: the known VOID LED packet and target interface were identified from its Corsair VOID implementation. No HeadsetControl source files are bundled. [Protocol reference](https://github.com/Sapd/HeadsetControl/blob/master/lib/devices/corsair_void_rich.hpp).
