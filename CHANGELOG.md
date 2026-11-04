# Changelog

## 0.4.0

- Static, Blinking, Fading, Strobing, and OFF modes.
- Color stages and mode-specific speed selection.
- JSON profiles and OpenRGB profile serialization.
- Input validation and checks that send no hardware commands.
- Uses OpenRGB's existing PawnIO Super I/O module.

## Development history

Earlier local prototypes established the MSVC/Qt compatibility requirements, replaced denied ASUS AsIO3 access with PawnIO, and corrected OFF control to the FF mode field. The user confirmed version 0.4.0 works on the target board.
