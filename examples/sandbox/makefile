CC ?= gcc

CFLAGS_COMMON = -Wall -Wextra -std=c11
CFLAGS_DEBUG = -O0 -ggdb
CFLAGS_RELEASE = -O2

DEFINES_COMMON = -DFINCH_LINUX
DEFINES_DEBUG = -DFINCH_LOGGING
DEFINES_RELEASE = 

BIN = sandbox

SRC_PATH = src
INCL_PATH = -I include/ -I ../../include/

SRC = $(shell find $(SRC_PATH) -name '*.c' | sort -k 1nr | cut -f2-)
OBJ = $(SRC:$(SRC_PATH)/%.c=$(BUILD_PATH)/%.o)

.PHONY: default_target
default_target: debug

.PHONY: run_tree
run_tree: $(SHADERS_BIN)
	mkdir -p $(BUILD_PATH)/run_tree/
	cp -r assets/ $(BUILD_PATH)/run_tree/
	cp ${BUILD_PATH}/${BIN} ${BUILD_PATH}/run_tree/
	cp ../../build/${RELEASE_MODE}/libfinch.so ${BUILD_PATH}/run_tree/

.PHONY: debug
debug: export CFLAGS = ${CFLAGS_COMMON} ${CFLAGS_DEBUG}
debug: export DEFINES = ${DEFINES_COMMON} ${DEFINES_DEBUG}
debug: export BUILD_PATH = build/debug
debug: export LIBS := -L ../../build/debug/ -lfinch
debug: export RELEASE_MODE := debug
debug: dirs
	@$(MAKE) -C ../../ debug
	@$(MAKE) all
	@$(MAKE) run_tree

.PHONY: release
release: export CFLAGS = ${CFLAGS_COMMON} ${CFLAGS_RELEASE}
release: export DEFINES = ${DEFINES_COMMON} ${DEFINES_RELEASE}
release: export BUILD_PATH = build/release
release: export LIBS := -L ../../build/release/ -lfinch
release: export RELEASE_MODE := release
release: dirs
	@$(MAKE) -C ../../ release
	@$(MAKE) all
	@$(MAKE) run_tree

.PHONY: dirs
dirs:
	@echo "Creating directories"
	@mkdir -p $(dir $(OBJ))
	@mkdir -p $(BUILD_PATH)

.PHONY: clean
clean:
	@echo "Deleting directories"
	@$(RM) -rf build/

.PHONY: all
all: $(BUILD_PATH)/$(BIN)

$(BUILD_PATH)/$(BIN): $(OBJ)
	@# @echo "Linking: $^ -> $@"
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LIBS)

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
	@# @echo "Compiling: $< -> $@"
	$(CC) $(CFLAGS) $(DEFINES) $(INCL_PATH) -c $< -o $@
