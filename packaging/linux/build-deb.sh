#!/usr/bin/env bash
# Build a self-contained .deb with desktop shortcut + icons.
# Usage: packaging/linux/build-deb.sh <version> [build-dir]
set -euo pipefail

APP_NAME="graber"
VERSION="${1:?version required}"
BUILD_DIR="${2:-build}"
PKG_DIR="package-root"
LIB_DIR="${PKG_DIR}/usr/lib/${APP_NAME}"
BIN_PATH="${LIB_DIR}/${APP_NAME}-bin"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT"

test -x "${BUILD_DIR}/${APP_NAME}" || { echo "Missing ${BUILD_DIR}/${APP_NAME}"; exit 1; }

rm -rf "${PKG_DIR}"
mkdir -p "${PKG_DIR}/DEBIAN"
mkdir -p "${PKG_DIR}/usr/bin"
mkdir -p "${LIB_DIR}/platforms"
mkdir -p "${PKG_DIR}/usr/share/applications"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/16x16/apps"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/32x32/apps"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/48x48/apps"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/64x64/apps"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/128x128/apps"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/256x256/apps"
mkdir -p "${PKG_DIR}/usr/share/icons/hicolor/512x512/apps"
mkdir -p "${PKG_DIR}/usr/share/pixmaps"

cp "${BUILD_DIR}/${APP_NAME}" "${BIN_PATH}"
chmod 755 "${BIN_PATH}"

# Qt platform plugins
QT_PLUGIN_PATH="$(qtpaths6 --plugin-dir 2>/dev/null || qtpaths --plugin-dir 2>/dev/null || true)"
if [ -z "${QT_PLUGIN_PATH}" ] || [ ! -d "${QT_PLUGIN_PATH}/platforms" ]; then
  for candidate in /usr/lib/x86_64-linux-gnu/qt6/plugins /usr/lib/qt6/plugins; do
    if [ -d "${candidate}/platforms" ]; then
      QT_PLUGIN_PATH="${candidate}"
      break
    fi
  done
fi
if [ -d "${QT_PLUGIN_PATH}/platforms" ]; then
  cp -a "${QT_PLUGIN_PATH}/platforms/"*.so "${LIB_DIR}/platforms/" 2>/dev/null || true
fi

bundle_deps() {
  local target="$1"
  ldd "${target}" 2>/dev/null | awk '/=>/ {print $3}' | while read -r lib; do
    [ -f "$lib" ] || continue
    case "$(basename "$lib")" in
      libc.so*|libpthread.so*|libdl.so*|libm.so*|librt.so*|ld-linux*|libstdc++.so*|libgcc_s.so*|libresolv.so*|libnsl.so*)
        ;;
      *)
        cp -L -n "$lib" "${LIB_DIR}/" 2>/dev/null || true
        ;;
    esac
  done
}
bundle_deps "${BIN_PATH}"
for plugin in "${LIB_DIR}"/platforms/*.so; do
  [ -f "$plugin" ] && bundle_deps "$plugin"
done

if command -v patchelf >/dev/null 2>&1; then
  patchelf --set-rpath '$ORIGIN' "${BIN_PATH}"
  for f in "${LIB_DIR}"/*.so* "${LIB_DIR}"/platforms/*.so*; do
    [ -f "$f" ] && patchelf --set-rpath '$ORIGIN:$ORIGIN/..' "$f" 2>/dev/null || true
  done
fi

# PATH launcher
cat > "${PKG_DIR}/usr/bin/${APP_NAME}" << 'LAUNCHER'
#!/bin/bash
DIR="/usr/lib/graber"
export QT_PLUGIN_PATH="${DIR}/platforms:${DIR}"
export LD_LIBRARY_PATH="${DIR}${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "${DIR}/graber-bin" "$@"
LAUNCHER
chmod 755 "${PKG_DIR}/usr/bin/${APP_NAME}"

# Desktop entry → app menu
if [ -f packaging/linux/graber.desktop ]; then
  cp packaging/linux/graber.desktop "${PKG_DIR}/usr/share/applications/${APP_NAME}.desktop"
else
  cat > "${PKG_DIR}/usr/share/applications/${APP_NAME}.desktop" << DESKTOP
[Desktop Entry]
Type=Application
Name=Clipboard Graber
Comment=Capture clipboard into Markdown notes
Exec=graber
Icon=graber
Terminal=false
Categories=Office;Utility;
StartupNotify=true
StartupWMClass=graber
DESKTOP
fi
chmod 644 "${PKG_DIR}/usr/share/applications/${APP_NAME}.desktop"

# Icons: prefer project multi-size PNGs, fall back to ImageMagick convert
install_icon() {
  local size="$1"
  local dest="${PKG_DIR}/usr/share/icons/hicolor/${size}x${size}/apps/${APP_NAME}.png"
  if [ -f "resources/icons/app-${size}.png" ]; then
    cp -f "resources/icons/app-${size}.png" "${dest}"
    return 0
  fi
  if command -v convert >/dev/null 2>&1 && [ -f resources/icons/app-1024.png ]; then
    convert resources/icons/app-1024.png -resize "${size}x${size}" "${dest}"
    return 0
  fi
  return 1
}

for sz in 16 32 48 64 128 256 512; do
  install_icon "$sz" || true
done

# pixmaps fallback (48 or 256)
if [ -f "${PKG_DIR}/usr/share/icons/hicolor/48x48/apps/${APP_NAME}.png" ]; then
  cp -f "${PKG_DIR}/usr/share/icons/hicolor/48x48/apps/${APP_NAME}.png" \
        "${PKG_DIR}/usr/share/pixmaps/${APP_NAME}.png"
elif [ -f "${PKG_DIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png" ]; then
  cp -f "${PKG_DIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png" \
        "${PKG_DIR}/usr/share/pixmaps/${APP_NAME}.png"
fi

strip --strip-unneeded "${BIN_PATH}" 2>/dev/null || true
for lib in "${LIB_DIR}"/*.so* "${LIB_DIR}"/platforms/*.so*; do
  [ -f "$lib" ] && strip --strip-unneeded "$lib" 2>/dev/null || true
done

cat > "${PKG_DIR}/DEBIAN/control" << CTRL
Package: ${APP_NAME}
Version: ${VERSION}
Architecture: amd64
Maintainer: Developer <emran.blue.120@gmail.com>
Section: utils
Priority: optional
Description: Clipboard Graber — capture clipboard into Markdown notes
 Fully self-contained Qt6 desktop app with application-menu shortcut and icons.
CTRL

cat > "${PKG_DIR}/DEBIAN/postinst" << 'POSTINST'
#!/bin/sh
set -e
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database -q /usr/share/applications || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -q /usr/share/icons/hicolor 2>/dev/null || true
fi
exit 0
POSTINST
chmod 755 "${PKG_DIR}/DEBIAN/postinst"
chmod 755 "${PKG_DIR}/DEBIAN"

OUT="${APP_NAME}_${VERSION}_amd64.deb"
dpkg-deb -Zxz -z9 --build "${PKG_DIR}" "${OUT}"
ls -lh "${OUT}"
echo "Built ${OUT}"
