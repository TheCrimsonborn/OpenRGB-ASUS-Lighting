# Publishing to GitHub

Create an empty GitHub repository with the visibility you want. This local repository has no remote and has not been published.

From this folder, connect the repository and push:

```powershell
git remote add origin <your-GitHub-repository-URL>
git push -u origin main
git push origin v0.4.0
```

For a v0.4.0 release, upload the Windows x64 binary ZIP and matching source ZIP from `dist/` together. The binary package includes the license and dependency notices. `SHA256SUMS.txt` records the standalone plugin DLL hash. Build outputs and binaries are intentionally excluded from the source commit.

The supported application is OpenRGB 1.1 with the MSVC Qt 6.8.3 runtime. Describe that compatibility in the release notes; the DLL is not a universal plugin for every OpenRGB build.
