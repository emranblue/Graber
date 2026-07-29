# Cross-platform Makefile for ClipboardGraber
.PHONY: all clean distclean rebuild run

BUILD_DIR = build

all: $(BUILD_DIR)/Makefile
	@cmake --build $(BUILD_DIR)
	@cp -f $(BUILD_DIR)/graber ./graber

$(BUILD_DIR)/Makefile: CMakeLists.txt
	@cmake -B $(BUILD_DIR) -S .

clean:
	@if [ -d "$(BUILD_DIR)" ]; then cmake --build $(BUILD_DIR) --target clean; fi
	@rm -f ./graber

distclean:
	@rm -rf $(BUILD_DIR) backup graber CMakeCache.txt CMakeFiles cmake_install.cmake graber_autogen .qt

rebuild: distclean all

run: all
	@./graber
