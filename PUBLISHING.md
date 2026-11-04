# Publishing to GitHub

The public repository is [TheCrimsonborn/OpenRGB-ASUS-Lighting](https://github.com/TheCrimsonborn/OpenRGB-ASUS-Lighting). The initial [v0.4.0 release](https://github.com/TheCrimsonborn/OpenRGB-ASUS-Lighting/releases/tag/v0.4.0) contains the standalone DLL, Windows x64 package, matching source package, and SHA-256 checksums.

The prepared local repository already has `origin` configured. Push subsequent commits and new release tags from this folder:

```powershell
git push -u origin main
git push origin <new-release-tag>
```

For a v0.4.0 release, upload the Windows x64 binary ZIP and matching source ZIP from `dist/` together. The binary package includes the license and dependency notices. `SHA256SUMS.txt` records the standalone plugin DLL hash. Build outputs and binaries are intentionally excluded from the source commit.

The supported application is OpenRGB 1.1 with the MSVC Qt 6.8.3 runtime. Describe that compatibility in the release notes; the DLL is not a universal plugin for every OpenRGB build.
