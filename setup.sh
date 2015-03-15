#! /bin/bash

mkdir external
cd external

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
if [ ! -d gtest ]; then
    wget https://googletest.googlecode.com/files/gtest-1.7.0.zip
    unzip gtest-1.7.0.zip
    mv gtest-1.7.0 gtest
    rm gtest-1.7.0.zip
else
    echo " -- gtest already installed"
fi

# Then get libuv
if [ ! -d libuv ]; then
    git clone --branch v1.4.2 git@github.com:libuv/libuv
    mv libuv/uv.gyp libuv/uv.gypi
else
    echo " -- libuv already installed"
fi

cd ..

# And then run gyp
gyp liblw.gyp --depth=. --generator-output=build/
