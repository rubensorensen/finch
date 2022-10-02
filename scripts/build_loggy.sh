#! /usr/bin/sh

CC=gcc
CFLAGS="-Wall -Wextra -std=c11 -O2 -ggdb"

INCLUDE_PATH="include/loggy"
SRC_PATH="src/loggy"
LIBS=""

BUILD_PATH="build/loggy"
BIN_PATH=$BUILD_PATH"/bin/"
BIN="loggy"

SRC=$(find $SRC_PATH -name '*.c' | sort -k 1nr | cut -f2-)
OBJ=$(echo $SRC | sed "s|\.c|\.o|g" | sed "s|$SRC_PATH|$BUILD_PATH|g")

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
    echo -e "${bold}${red}Removing:${normal}  \t$BIN"
    rm -rf $BIN
    echo -e "${bold}${red}Removing:${normal}  \t$BIN_PATH"
    rm -rf $BIN_PATH
    echo -e "${bold}${red}Removing:${normal}  \t$BUILD_PATH"
    rm -rf $BUILD_PATH
    exit 0    
}

create_directories()
{
    echo -e "${bold}${green}Creating:${normal}  \t$BUILD_PATH"
    mkdir -p $BUILD_PATH
    echo -e "${bold}${green}Creating:${normal}  \t$BIN_PATH"
    mkdir -p $BIN_PATH
}

compile_c()
{
    # Compile C files
    CNT=$(echo $SRC | wc -w)
    for i in `seq 1 $CNT`; do
        SRC_FILE=$(echo $SRC | cut -d\  -f$i)
        OBJ_FILE=$(echo $OBJ | cut -d\  -f$i)
        echo -e "${bold}${yellow}Compiling:${normal} \t$SRC_FILE -> $OBJ_FILE"
        $CC $CFLAGS -c -I $INCLUDE_PATH -fpic $SRC_FILE -o $OBJ_FILE
    done
}

link_program()
{
    # Link program
    echo -e "${bold}${cyan}Linking:${normal}   \t$OBJ -> $BIN_PATH/lib$BIN.so"
    $CC $CFLAGS -shared -I $INCLUDE_PATH -o $BIN_PATH/lib$BIN.so $OBJ $LIBS
}

#
# Main part of build script starts here
#

if test "$1" == '--clean'; then
    clean
fi

create_directories
compile_c
link_program
