#! /usr/bin/sh

BUILD_PATH="build/"
RUN_TREE_PATH="run_tree/"

DEPENDENCIES="loggy sandbox finch"

# Test for color support
ncolors=$(tput colors)
if test -n "$ncolors" && test $ncolors -ge 8; then
    bold="$(tput bold)"
    underline="$(tput smul)"
    standout="$(tput smso)"
    normal="$(tput sgr0)"
    black="$(tput setaf 0)"
    red="$(tput setaf 1)"
    green="$(tput setaf 2)"
    yellow="$(tput setaf 3)"
    blue="$(tput setaf 4)"
    magenta="$(tput setaf 5)"
    cyan="$(tput setaf 6)"
    white="$(tput setaf 7)"
fi

clean()
{
    echo -e "${underline}${bold}${red}Removing:${normal} \t$BUILD_PATH"
    # echo -e "Removing:\t$BUILD_PATH"
    rm -rf $BUILD_PATH
    echo -e "${underline}${bold}${red}Removing:${normal} \t$RUN_TREE_PATH"
    rm -rf $RUN_TREE_PATH
    exit 0    
}

create_directories()
{
    echo -e "${underline}${bold}${green}Creating:${normal} \t$BUILD_PATH"
    mkdir -p $BUILD_PATH
    echo -e "${underline}${bold}${green}Creating:${normal} \t$RUN_TREE_PATH"
    mkdir -p $RUN_TREE_PATH
    echo ""
}

build_dependencies()
{
    for DEPENDENCY in $DEPENDENCIES; do
        echo -e "${standout}${bold}${green}Building:${normal} \t$DEPENDENCY"
        ./scripts/build_$DEPENDENCY.sh | sed 's/^/  /g'
        echo ""
    done
}

make_runtree()
{
    for DEPENDENCY in $DEPENDENCIES; do
        FILE=$(find build/$DEPENDENCY/bin/ -name *$DEPENDENCY*)
        BASENAME=$(basename $FILE)
        echo -e "${underline}${bold}${magenta}Copying:${normal}  \t$FILE -> $RUN_TREE_PATH/$BASENAME"
        cp $FILE $FILE.copy
        mv $FILE.copy $RUN_TREE_PATH/$BASENAME
    done
}

#
# Main part of build script starts here
#

if test "$1" == "--clean"; then
    echo -e "${bold}${standout}${yellow}CLEANING BUILD${normal}"
    ./scripts/build_finch.sh --clean
    ./scripts/build_sandbox.sh --clean
    ./scripts/build_loggy.sh --clean
    clean
    exit 0
fi

create_directories
build_dependencies
make_runtree

if test "$1" == "--run"; then
    ./scripts/run.sh
fi
