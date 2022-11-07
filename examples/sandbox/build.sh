#! /usr/bin/sh

source ../../scripts/color_support.sh

CC=gcc
CFLAGS="-Wall -Wextra -std=c11 -O0 -ggdb"

INCLUDE_PATH="-I include/ -I ../../include"
SRC_PATH="src/"
LIBS="-lm -L ../../build/bin -lfinch"

BUILD_PATH="build/"
BIN_PATH=$BUILD_PATH"/bin/"
BIN="sandbox"

BUILD_DEFINES="-DFINCH_LOGGING"

RUN_TREE_PATH="run_tree/"

SRC=$(find $SRC_PATH -name '*.c' | sort -k 1nr | cut -f2-)
OBJ=$(echo $SRC | sed "s|\.c|\.o|g" | sed "s|$SRC_PATH|$BUILD_PATH|g")

clean()
{
    echo -e "${BOLD}${RED}Removing:${NORMAL} $BIN_PATH"
    rm -rf $BIN_PATH
    echo -e "${BOLD}${RED}Removing:${NORMAL} $BUILD_PATH"
    rm -rf $BUILD_PATH
    echo -e "${BOLD}${RED}Removing:${NORMAL} $RUN_TREE_PATH"
    rm -rf $RUN_TREE_PATH
}

build_finch()
{
    pushd ../../ > /dev/null
    echo -e "${STANDOUT}${BOLD}WORKING DIRECTORY:${NORMAL}${STANDOUT} $(pwd)${NORMAL}"
    ./scripts/build.sh
    popd > /dev/null
    echo -e "${STANDOUT}${BOLD}WORKING DIRECTORY:${NORMAL}${STANDOUT} $(pwd)${NORMAL}"
}

clean_finch()
{
    pushd ../../ > /dev/null
    echo -e "${STANDOUT}${BOLD}WORKING DIRECTORY:${NORMAL}${STANDOUT} $(pwd)${NORMAL}"
    ./scripts/build.sh --clean
    popd > /dev/null
    echo -e "${STANDOUT}${BOLD}WORKING DIRECTORY:${NORMAL}${STANDOUT} $(pwd)${NORMAL}"
    
}

create_directories()
{
    echo -e "${BOLD}${GREEN}Creating:${NORMAL} $BUILD_PATH"
    mkdir -p $BUILD_PATH
    echo -e "${BOLD}${GREEN}Creating:${NORMAL} $BIN_PATH"
    mkdir -p $BIN_PATH
    echo -e "${BOLD}${GREEN}Creating:${NORMAL} $RUN_TREE_PATH"
    mkdir -p $RUN_TREE_PATH
}

compile_sources()
{
    # Compile C files
    CNT=$(echo $SRC | wc -w)
    for i in `seq 1 $CNT`; do
        SRC_FILE=$(echo $SRC | cut -d\  -f$i)
        OBJ_FILE=$(echo $OBJ | cut -d\  -f$i)
        echo -e "${BOLD}${BLUE}Compiling:${NORMAL} $SRC_FILE -> $OBJ_FILE"
        $CC $CFLAGS -c $INCLUDE_PATH $SRC_FILE $BUILD_DEFINES -o $OBJ_FILE
    done
}

link_program()
{
    # Link program
    echo -e "${BOLD}${MAGENTA}Linking:${NORMAL} $OBJ -> $BIN_PATH/$BIN"
    $CC $CFLAGS $INCLUDE_PATH -o $BIN_PATH/$BIN $OBJ $LIBS
}

make_run_tree()
{
    echo -e "${BOLD}${YELLOW}Copying:${NORMAL} $BIN_PATH/$BIN -> $RUN_TREE_PATH/$BIN"
    cp $BIN_PATH/$BIN $RUN_TREE_PATH/$BIN
    echo -e "${BOLD}${YELLOW}Copying:${NORMAL} ../../build/bin/libfinch.so -> $RUN_TREE_PATH/libfinch.so"
    cp ../../build/bin/libfinch.so $RUN_TREE_PATH/libfinch.so
}

#
# Main part of build script starts here
#

if test "$1" == '--clean'; then
    echo -e "${BOLD}${STANDOUT}${RED}CLEANING PROJECT${NORMAL}"
    clean | sed "s|^|    |g"
    echo ""
    
    echo -e "${BOLD}${STANDOUT}${RED}CLEANING FINCH${NORMAL}"
    clean_finch | sed "s|^|    |g"
    
    exit 0
fi

echo -e "${STANDOUT}${BOLD}BUILDING FINCH${NORMAL}"
build_finch | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${GREEN}CREATING DIRECTORIES${NORMAL}"
create_directories | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${BLUE}COMPILING SOURCES${NORMAL}"
compile_sources | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${MAGENTA}LINKING PROGRAM${NORMAL}"
link_program | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${YELLOW}MAKING RUN TREE${NORMAL}"
make_run_tree | sed "s|^|    |g"

if test "$1" == '--run'; then
    pushd run_tree > /dev/null
    echo -e "Running $BIN..."
    ./$BIN
    popd > /dev/null
fi
