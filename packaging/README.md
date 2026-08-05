# Packaging

## Linux (.deb)

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./packaging/linux/build-deb.sh 1.0.0 build
sudo dpkg -i graber_1.0.0_amd64.deb
```

Requires: `patchelf`, Qt6 plugins on the build machine (bundled into the deb).

## Windows (portable zip)

Built by GitHub Actions (`.github/workflows/build.yml`):

1. CMake + MSVC + Qt 6.5
2. `windeployqt` into `windows-package/`
3. Copies `graber.ico` + `Create-Desktop-Shortcut.ps1`
4. Zip artifact

Locally after a Release build:

```cmd
windeployqt --dir dist build\Release\graber.exe
copy packaging\windows\Create-Desktop-Shortcut.ps1 dist\
copy resources\icons\app.ico dist\graber.ico
```

## CI

Push to `main`/`master` or tag `v*` → Linux deb + Windows zip artifacts.  
Tags / workflow_dispatch also create a GitHub Release.
