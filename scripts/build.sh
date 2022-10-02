#! /usr/bin/sh

BUILD_PATH="build/"
RUN_TREE_PATH="run_tree/"

DEPENDENCIES="sandbox finch"

clean()
{
    echo -e "Removing:\t$BUILD_PATH"
    rm -rf $BUILD_PATH
    echo -e "Removing:\t$RUN_TREE_PATH"
    rm -rf $RUN_TREE_PATH
    exit 0    
}

create_directories()
{
    echo -e "Creating:\t$BUILD_PATH"
    mkdir -p $BUILD_PATH
    echo -e "Creating:\t$RUN_TREE_PATH"
    mkdir -p $RUN_TREE_PATH
    echo ""
}

build_dependencies()
{
    for DEPENDENCY in $DEPENDENCIES; do
        echo -e "Building:\t$DEPENDENCY"
        ./scripts/build_$DEPENDENCY.sh
        echo ""
    done
}

make_runtree()
{
    for DEPENDENCY in $DEPENDENCIES; do
        FILE=$(find build/$DEPENDENCY/bin/ -name *$DEPENDENCY*)
        BASENAME=$(basename $FILE)
        echo -e "Copying:\t$FILE -> $RUN_TREE_PATH/$BASENAME"
        cp $FILE $FILE.copy
        mv $FILE.copy $RUN_TREE_PATH/$BASENAME
    done
}

#
# Main part of build script starts here
#

if test "$1" == "--clean"; then
    ./scripts/build_finch.sh --clean
    ./scripts/build_sandbox.sh --clean
    clean
fi

create_directories
build_dependencies
make_runtree

if test "$1" == "--run"; then
    ./scripts/run.sh
fi
