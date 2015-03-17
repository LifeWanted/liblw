
export BASE_DIR=`pwd`
export BUILD_DIR=$BASE_DIR/build
export DEPENDENCIES_DIR=$BASE_DIR/external
export BIN_DIR=$DEPENDENCIES_DIR/bin

export GYP_EXE=$DEPENDENCIES_DIR/bin/gyp

function exe_exists(){
    command -v "$1" >/dev/null
}

function is_executable(){
    [ -f "$1" ] && [ -x "$1" ]
}

if exe_exists apt-get; then
    export LW_APT=1
    export LW_INSTALL=apt-get install
elif exe_exists yum; then
    export LW_YUM=1
    export LW_INSTALL=yum install
fi

export -f exe_exists
