#! /usr/bin/sh

CC=gcc
CFLAGS="-Wall -Wextra -std=c11" 

INCLUDE_PATH="include/"
SRC_PATH="src/"
LIBS="$(pkg-config --cflags --libs x11)"

BUILD_PATH="build/"
BIN_PATH=$BUILD_PATH"/bin/"
BIN="finch"

SRC=$(find $SRC_PATH -name '*.c' | sort -k 1nr | cut -f2-)
OBJ=$(echo $SRC | sed "s|\.c|\.o|g" | sed "s|$SRC_PATH|$BUILD_PATH|g")

clean()
{
    echo "Removing: $BIN"
    rm -rf $BIN
    echo "Removing: $BIN_PATH"
    rm -rf $BIN_PATH
    echo "Removing: $BUILD_PATH"
    rm -rf $BUILD_PATH
    exit 0    
}

create_directories()
{
    # Creating: $BUILD_PATH
    echo "Creating: $BUILD_PATH"
    mkdir -p $BUILD_PATH
    echo "Creating: $BIN_PATH"
    mkdir -p $BIN_PATH
}

compile_c()
{
    # Compile C files
    CNT=$(echo $SRC | wc -w)
    for i in `seq 1 $CNT`; do
        SRC_FILE=$(echo $SRC | cut -d\  -f$i)
        OBJ_FILE=$(echo $OBJ | cut -d\  -f$i)
        echo "Compiling: $SRC_FILE -> $OBJ_FILE"
        $CC $CFLAGS -c -I $INCLUDE_PATH $SRC_FILE -o $OBJ_FILE
    done
}

link_program()
{
    # Link program
    echo "Linking: $OBJ -> $BIN_PATH/$BIN"
    $CC $CFLAGS -I $INCLUDE_PATH -o $BIN_PATH/$BIN $OBJ $LIBS
}

create_symlink()
{
    # Symlink to program
    echo "Symlinking: $BIN -> $BIN_PATH/$BIN"
    if test -L $BIN; then
        rm $BIN
    fi
    ln -s $BIN_PATH/$BIN $BIN
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
create_symlink
