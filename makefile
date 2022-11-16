CC ?= gcc

CFLAGS_COMMON = -Wall -Wextra -std=c11 $(shell pkg-config --cflags x11 vulkan)
CFLAGS_DEBUG = -O0 -ggdb
CFLAGS_RELEASE = -O2

DEFINES_COMMON = -DFINCH_LINUX
DEFINES_DEBUG = -DFINCH_LOGGING
DEFINES_RELEASE = 

BIN = libfinch.so

SRC_PATH = src
INCL_PATH = -I include/

SRC = $(shell find $(SRC_PATH) -name '*.c' | sort -k 1nr | cut -f2-)
OBJ = $(SRC:$(SRC_PATH)/%.c=$(BUILD_PATH)/%.o)

LIBS = $(shell pkg-config --libs x11 vulkan) -lm

.PHONY: default_target
default_target: debug

.PHONY: debug
debug: export CFLAGS = ${CFLAGS_COMMON} ${CFLAGS_DEBUG}
debug: export DEFINES = ${DEFINES_COMMON} ${DEFINES_DEBUG}
debug: export BUILD_PATH = build/debug
debug: dirs
	@$(MAKE) all

.PHONY: release
release: export CFLAGS = ${CFLAGS_COMMON} ${CFLAGS_RELEASE}
release: export DEFINES = ${DEFINES_COMMON} ${DEFINES_RELEASE}
release: export BUILD_PATH = build/release
release: dirs
	@$(MAKE) all

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
	$(CC) $(CFLAGS) -shared $(OBJ) -o $@ $(LIBS)

$(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
	@# @echo "Compiling: $< -> $@"
	$(CC) $(CFLAGS) $(DEFINES) $(INCL_PATH) -c -fpic $< -o $@
