# Publishing to GitHub

Public repository: [TheCrimsonborn/OpenRGB-ASUS-Lighting](https://github.com/TheCrimsonborn/OpenRGB-ASUS-Lighting).

Version 0.5.0 ships two independent plugins. Attach both standalone DLLs and their separate Windows x64 ZIPs to the release, together with the matching source ZIP and SHA256SUMS.txt. A user can install either plugin without the other.

Build and validate with scripts/Build.ps1 -RunChecks, commit the matching source, create a new version tag, and push main plus that tag. Generate the source archive from the tagged commit. Do not replace previous release tags or assets when publishing a new version.

The supported runtime is Windows x64 OpenRGB 1.1 with MSVC Qt 6.8.3. Keep this compatibility information in release notes.
