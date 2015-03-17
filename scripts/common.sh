
export BASE_DIR=`pwd`
export BUILD_DIR=$BASE_DIR/build
export DEPENDENCIES_DIR=$BASE_DIR/external
export BIN_DIR=$DEPENDENCIES_DIR/bin

function exe_exists(){
    command -v "$1" >/dev/null
}

function is_executable(){
    [ -f "$1" ] && [ -x "$1" ]
}

export -f exe_exists
