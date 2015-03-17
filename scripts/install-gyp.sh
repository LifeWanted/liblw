
source scripts/common.sh

function install_gyp(){
    if is_executable "$BIN_DIR/gyp"; then
        echo " -- gyp already installed"
        return 0
    fi

    git clone https://chromium.googlesource.com/external/gyp $DEPENDENCIES_DIR/gyp
    cd $DEPENDENCIES_DIR/gyp
    python setup.py build
    cp gyp "$BIN_DIR/gyp"
    cp gyp_main.py "$BIN_DIR/gyp_main.py"
}

export -f install_gyp
