#! /bin/bash

BUILD_DIR=build
DEPENDENCIES_DIR=external

mkdir $DEPENDENCIES_DIR 2>/dev/null

# Instal gyp if needed.
if [ "`which gyp`" == "" ]; then
    git clone https://chromium.googlesource.com/external/gyp
    cd gyp
    sudo python setup.py install
    cd ..
else
    echo " -- gyp already installed"
fi

# Fetch gtest
if [ ! -d "$DEPENDENCIES_DIR/gtest" ]; then
    gtest_version=1.7.0
    gtest_dir=gtest-$gtest_version
    gtest_zip=$gtest_dir.zip
    wget "https://googletest.googlecode.com/files/$gtest_zip"
    unzip "$gtest_zip"
    rm "$gtest_zip"
    mv "$gtest_dir" "$DEPENDENCIES_DIR/gtest"
else
    echo " -- gtest already installed"
fi

# Then get libuv
if [ ! -d "$DEPENDENCIES_DIR/libuv" ]; then
    libuv_version=1.4.2
    libuv_dir=libuv-$libuv_version
    libuv_tar=v$libuv_version.tar.gz
    wget "https://github.com/libuv/libuv/archive/$libuv_tar"
    tar -xzf "$libuv_tar"
    rm "$libuv_tar"
    mv "$libuv_dir/uv.gyp" "$libuv_dir/uv.gypi"
    mv "$libuv_dir" "$DEPENDENCIES_DIR/libuv"
else
    echo " -- libuv already installed"
fi

# And then run gyp
gyp liblw.gyp --depth=. --generator-output=$BUILD_DIR -Goutput_dir="`pwd`/$BUILD_DIR"
