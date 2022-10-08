#! /usr/bin/sh

source scripts/color_support.sh

CC=gcc
CFLAGS="-Wall -Wextra -std=c11 -O0 -ggdb"

INCLUDE_PATH="-I include/"
SOURCE_PATH="src/"
LIBS="$(pkg-config --cflags --libs x11) -lm -ldl"

BUILD_PATH="build/"
BIN_PATH=$BUILD_PATH"/bin/"
BIN="finch"

SOURCES=$(find $SOURCE_PATH -name '*.c' | sort -k 1nr | cut -f2-)
OBJECTS=$(echo $SOURCES | sed "s|\.c|\.o|g" | sed "s|$SOURCE_PATH|$BUILD_PATH|g")
MODULES=$(echo $OBJECTS | sed "s|$BUILD_PATH||g" | sed "s|/[^/]*\.o||g" \
              | xargs -n1 | sort -u | xargs)

clean()
{
    echo -e "${BOLD}${RED}Removing:${NORMAL} $BIN_PATH"
    rm -rf $BIN_PATH
    echo -e "${BOLD}${RED}Removing:${NORMAL} $BUILD_PATH"
    rm -rf $BUILD_PATH
}

create_directories()
{
    echo -e "${BOLD}${GREEN}Creating:${NORMAL} $BUILD_PATH"
    mkdir -p $BUILD_PATH
    for MODULE in $MODULES; do
        MODULE_PATH=$BUILD_PATH/$MODULE
        echo -e "${BOLD}${GREEN}Creating:${NORMAL} $MODULE_PATH"
        mkdir -p $MODULE_PATH
    done
    echo -e "${BOLD}${GREEN}Creating:${NORMAL} $BIN_PATH"
    mkdir -p $BIN_PATH
}

compile_sources()
{
    # Compile C files
    CNT=$(echo $SOURCES | wc -w)
    for i in `seq 1 $CNT`; do
        SOURCE_FILE=$(echo $SOURCES | cut -d\  -f$i)
        OBJECT_FILE=$(echo $OBJECTS | cut -d\  -f$i)
        echo -e "${BOLD}${BLUE}Compiling:${NORMAL} $SOURCE_FILE -> $OBJECT_FILE"
        $CC $CFLAGS -c $INCLUDE_PATH -fpic $SOURCE_FILE -o $OBJECT_FILE
    done
}

link_modules()
{
    for MODULE in $MODULES; do
        MODULE_PATH=$BUILD_PATH/$MODULE
        MODULE_OBJECTS=$(find $MODULE_PATH -maxdepth 1 -name '*.o' | sort -k 1nr | cut -f2- | tr '\n' ' ')
        LIB_NAME="lib$(echo $MODULE | sed 's|/|-|g').a"
        echo -e "${BOLD}${MAGENTA}Linking:${NORMAL} $MODULE_OBJECTS -> $MODULE_PATH/$LIB_NAME"
        ar rcs $MODULE_PATH/$LIB_NAME $MODULE_OBJECTS
    done
}

link_library()
{
    MODULE_LIBS=$(find $BUILD_PATH -name '*.a' | sort -k 1nr | cut -f2- | tr '\n' ' ')
    LIB_NAME="lib$BIN.so"
    echo -e "${BOLD}${MAGENTA}Linking:${NORMAL} $MODULE_LIBS -> $BIN_PATH/$LIB_NAME"
    $CC $CFLAGS -shared -o $BIN_PATH/$LIB_NAME -Wl,--whole-archive $MODULE_LIBS -Wl,--no-whole-archive $LIBS
}

if test "$1" == '--clean'; then
    clean
    exit 0
fi

echo -e "${STANDOUT}${BOLD}${GREEN}CREATING DIRECTORIES${NORMAL}"
create_directories | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${GREEN}COMPILING SOURCES${NORMAL}"
compile_sources | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${MAGENTA}LINKING MODULES${NORMAL}"
link_modules | sed "s|^|    |g"
echo ""

echo -e "${STANDOUT}${BOLD}${MAGENTA}LINKING LIBRARY${NORMAL}"
link_library | sed "s|^|    |g"
echo ""
