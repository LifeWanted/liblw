
source scripts/install-gcc.sh
source scripts/install-gyp.sh
source scripts/install-gtest.sh
source scripts/install-libuv.sh

function install_all()(
    set -e
    install_gcc
    install_gyp
    install_gtest
    install_libuv
)

export -f install_all
