#!/usr/bin/env bash
# rm -fr build dist
VERSION=0.1.0
NAME="Raiju-tools"
DIST_NAME="Raiju-Flasher"

pyinstaller --log-level=DEBUG \
            --noconfirm \
            --windowed \
            build-on-mac.spec

# https://github.com/sindresorhus/create-dmg
create-dmg "dist/$NAME.app"
mv "$NAME $VERSION.dmg" "dist/$DIST_NAME.dmg"
