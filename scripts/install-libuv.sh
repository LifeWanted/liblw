
source scripts/common.sh

function install_libuv(){
    if [ -d "$DEPENDENCIES_DIR/libuv" ]; then
        echo " -- libuv already installed"
        return 0
    fi

    libuv_version=1.7.0
    libuv_dir=libuv-$libuv_version
    libuv_tar=v$libuv_version.tar.gz
    wget "https://github.com/libuv/libuv/archive/$libuv_tar"
    tar -xzf "$libuv_tar"
    rm "$libuv_tar"
    mv "$libuv_dir/uv.gyp" "$libuv_dir/uv.gypi"
    mv "$libuv_dir" "$DEPENDENCIES_DIR/libuv"
}

export -f install_libuv
