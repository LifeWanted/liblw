#! /bin/bash

source scripts/common.sh
source scripts/install-all.sh

mkdir -p $BUILD_DIR         2>/dev/null
mkdir -p $DEPENDENCIES_DIR  2>/dev/null
mkdir -p $BIN_DIR           2>/dev/null

if install_all; then
    echo " -- All components installed."
else
    echo " !! Failed to install required components." 1>&2
fi

# And then run gyp
$GYP_EXE liblw.gyp --depth=. --generator-output=$BUILD_DIR -Goutput_dir=$BUILD_DIR
