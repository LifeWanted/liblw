
source scripts/common.sh

export GYP_DIR=$DEPENDENCIES_DIR/gyp

function install_gyp(){
    if [ -d "$GYP_DIR" ] && is_executable "$GYP_DIR/gyp"; then
        echo " -- gyp already installed"
        return 0
    fi

    git clone https://chromium.googlesource.com/external/gyp "$GYP_DIR"
    cd "$GYP_DIR"
    python setup.py build
}

export -f install_gyp
