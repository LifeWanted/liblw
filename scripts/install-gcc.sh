
source scripts/common.sh

function has_gcc(){
    exe_exists gcc && exe_exists g++
}

function has_right_gcc_version(){
    local gcc_49x=`gcc --version | head -1 | grep '4\.9\.[[:digit:]]'`
    if [ "$gcc_49x" ]; then
        return 0
    else
        return 1
    fi
}

function get_gcc_version(){
    local gcc_version_match='[[:digit:]]\.[[:digit:]]\.[[:digit:]]'
    local gcc_version=`gcc --version | grep -o "$gcc_version_match" | head -1`
    echo $gcc_version | grep -o '[[:digit:]]\.[[:digit:]]'
}

function install_gcc_apt(){
    if ! has_gcc; then
        sudo apt-get install --yes gcc g++
    fi

    if ! has_right_gcc_version; then
        gcc_version=$(get_gcc_version)

        sudo apt-add-repository --yes ppa:ubuntu-toolchain-r/test
        sudo apt-get update
        sudo apt-get install --yes gcc-4.9 g++-4.9
        sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$gcc_version 40 --slave /usr/bin/g++ g++ /usr/bin/g++-$gcc_version
        sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.9
        sudo update-alternatives --auto gcc
    fi
}

function install_gcc_yum(){
    if ! has_gcc; then
        sudo yum install gcc gcc-c++
    fi

    if ! has_right_gcc_version; then
        echo " !! Cannot install gcc 4.9 on this platform (do not know how)" >&2
        exit 1
    fi
}

function install_gcc(){
    if exe_exists apt-get; then
        install_gcc_apt
    else
        install_gcc_yum
    fi
}

export -f install_gcc
