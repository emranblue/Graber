cmake_minimum_required(VERSION 3.16)
project(ClipboardGraber LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Tell CMake to use the Qt MOC, UIC, and RCC compilers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Create the executable
add_executable(graber main.cpp resources.qrc)

# Link the executable to the Qt libraries
target_link_libraries(graber PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Post-build step to copy executable to backup folders (Linux/Unix only)
if(UNIX)
    add_custom_command(TARGET graber POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/backup"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:graber> "${CMAKE_SOURCE_DIR}/backup/graber"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/GraberNotes/backup"
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:graber> "$ENV{HOME}/GraberNotes/backup/graber"
        COMMENT "Backing up the executable to project backup/ and ~/GraberNotes/backup/"
    )
endif()
