#! /usr/bin/sh

CC=gcc
CFLAGS="-Wall -Wextra -std=c11 -O2"

INCLUDE_PATH="include/finch/"
SRC_PATH="src/finch"
LIBS="$(pkg-config --cflags --libs x11) -lm -ldl"

BUILD_PATH="build/finch"
BIN_PATH=$BUILD_PATH"/bin/"
BIN="finch"

SRC=$(find $SRC_PATH -name '*.c' | sort -k 1nr | cut -f2-)
OBJ=$(echo $SRC | sed "s|\.c|\.o|g" | sed "s|$SRC_PATH|$BUILD_PATH|g")

clean()
{
    echo -e "Removing:\t$BIN"
    rm -rf $BIN
    echo -e "Removing:\t$BIN_PATH"
    rm -rf $BIN_PATH
    echo -e "Removing:\t$BUILD_PATH"
    rm -rf $BUILD_PATH
    exit 0    
}

create_directories()
{
    # Creating: $BUILD_PATH
    echo -e "Creating:\t$BUILD_PATH"
    mkdir -p $BUILD_PATH
    echo -e "Creating:\t$BIN_PATH"
    mkdir -p $BIN_PATH
}

compile_c()
{
    # Compile C files
    CNT=$(echo $SRC | wc -w)
    for i in `seq 1 $CNT`; do
        SRC_FILE=$(echo $SRC | cut -d\  -f$i)
        OBJ_FILE=$(echo $OBJ | cut -d\  -f$i)
        echo -e "Compiling:\t$SRC_FILE -> $OBJ_FILE"
        $CC $CFLAGS -c -I $INCLUDE_PATH $SRC_FILE -o $OBJ_FILE
    done
}

link_program()
{
    # Link program
    echo -e "Linking:\t$OBJ -> $BIN_PATH/$BIN"
    $CC $CFLAGS -I $INCLUDE_PATH -o $BIN_PATH/$BIN $OBJ $LIBS
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
