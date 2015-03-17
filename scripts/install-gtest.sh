
source scripts/common.sh

function install_gtest(){
    if [ -d "$DEPENDENCIES_DIR/gtest" ]; then
        echo " -- gtest already installed"
        return 0
    fi

    gtest_version=1.7.0
    gtest_dir=gtest-$gtest_version
    gtest_zip=$gtest_dir.zip
    wget "https://googletest.googlecode.com/files/$gtest_zip"
    unzip "$gtest_zip"
    rm "$gtest_zip"
    mv "$gtest_dir" "$DEPENDENCIES_DIR/gtest"
}

export -f install_gtest
