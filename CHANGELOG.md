# Changelog

## 0.5.0

- Ships two independently installable plugins and DLLs in one repository.
- Added Corsair VOID ELITE USB (1B1C:0A56) LED On/Off controls.
- Both ASUS Lighting and VOID Headset appear inside the Devices sidebar.
- Added headset target/packet/UI checks without hardware writes.

## 0.4.1

- Moved ASUS Lighting into the Devices sidebar using OpenRGB's plugin page placement API.
- Retained the existing lighting controls and hardware protocol.

## 0.4.0

- Static, Blinking, Fading, Strobing, and OFF modes.
- Color stages and mode-specific speed selection.
- JSON profiles and OpenRGB profile serialization.
- Input validation and checks that send no hardware commands.
- Uses OpenRGB's existing PawnIO Super I/O module.

## Development history

Earlier local prototypes established the MSVC/Qt compatibility requirements, replaced denied ASUS AsIO3 access with PawnIO, and corrected OFF control to the FF mode field. The user confirmed version 0.4.0 works on the target board.
