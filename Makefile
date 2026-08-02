# Cross-platform Makefile for Graber (thin wrapper around the Qt6/CMake build)
.PHONY: all clean distclean rebuild reconfigure run

BUILD_DIR   := build
BUILD_TYPE  ?= Release
JOBS        ?= $(shell nproc 2>/dev/null || echo 1)

all:
	@if [ ! -d "$(BUILD_DIR)" ] || [ ! -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ; \
	fi
	@cmake --build $(BUILD_DIR) -- -j$(JOBS)
	@cp -f $(BUILD_DIR)/graber ./graber

# Force a fresh `cmake` configure step (e.g. after editing CMakeLists.txt in a
# way the generated build system doesn't pick up on its own, or after
# switching BUILD_TYPE).
reconfigure:
	@cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

clean:
	@if [ -d "$(BUILD_DIR)" ]; then cmake --build $(BUILD_DIR) --target clean; fi
	@rm -f ./graber

distclean:
	@rm -rf $(BUILD_DIR) backup graber CMakeCache.txt CMakeFiles cmake_install.cmake graber_autogen .qt

rebuild: distclean all

run: all
	@./graber
