cmake_minimum_required(VERSION 3.16)
project(ClipboardGraber LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# X11 is needed on Linux for system-wide (global) keyboard shortcuts, so the
# app's hotkeys keep working even while another window has focus.
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# Tell CMake to use the Qt MOC, UIC, and RCC compilers
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/repositories
    ${CMAKE_CURRENT_SOURCE_DIR}/formatters
    ${CMAKE_CURRENT_SOURCE_DIR}/monitors
    ${CMAKE_CURRENT_SOURCE_DIR}/shortcuts
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# On Windows, this embeds resources/icons/app.ico as the .exe's own icon
# (Explorer/taskbar/shortcuts). Harmless no-op on other platforms.
if(WIN32)
    set(GRABER_WIN_RC resources/graber.rc)
endif()

# Create the executable
add_executable(graber
    main.cpp
    resources.qrc
    ${GRABER_WIN_RC}

    # Interfaces
    interfaces/IClipboardMonitor.h
    interfaces/IDocumentFormatter.h
    interfaces/INoteRepository.h
    interfaces/ISectionRepository.h
    interfaces/INoteService.h
    interfaces/IServiceRegistry.h
    interfaces/IAction.h
    interfaces/IWizardFeature.h

    # Core
    core/Types.h

    # Services
    services/NoteService.h
    services/NoteService.cpp
    services/ServiceRegistry.h
    services/ServiceRegistry.cpp
    services/ActionRegistry.h
    services/ActionRegistry.cpp
    services/FeatureManager.h
    services/FeatureManager.cpp

    # Repositories
    repositories/NoteRepository.h
    repositories/NoteRepository.cpp
    repositories/IniSectionRepository.h
    repositories/IniSectionRepository.cpp

    # Formatters
    formatters/MarkdownDocumentFormatter.h
    formatters/MarkdownDocumentFormatter.cpp
    formatters/MarkdownUtils.h
    formatters/MarkdownUtils.cpp

    # Monitors
    monitors/ClipboardMonitor.h
    monitors/ClipboardMonitor.cpp

    # Shortcuts
    shortcuts/ShortcutManager.h
    shortcuts/ShortcutManager.cpp

    # UI
    ui/ClipboardGrabber.h
    ui/ClipboardGrabber.cpp
    ui/ClipboardGrabberUI.h
    ui/ClipboardGrabberUI.cpp
    ui/HeadingSelectDialog.h
    ui/HeadingSelectDialog.cpp
    ui/ShortcutsSettingsDialog.h
    ui/ShortcutsSettingsDialog.cpp
    ui/ExportNoteWizard.h
    ui/ExportNoteWizard.cpp

    # Utils
    utils/Utils.h
    utils/Utils.cpp
)

# Link the executable to the Qt libraries
target_link_libraries(graber PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

if(UNIX AND NOT APPLE)
    target_include_directories(graber PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(graber PRIVATE ${X11_LIBRARIES})
endif()

# Post-build step to copy executable to backup folders
if(UNIX)
    if(DEFINED ENV{HOME})
        add_custom_command(TARGET graber POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/backup"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:graber> "${CMAKE_SOURCE_DIR}/backup/graber"
            COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/GraberNotes/backup"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:graber> "$ENV{HOME}/GraberNotes/backup/graber"
            COMMENT "Backing up the executable to project backup/ and ~/GraberNotes/backup/"
        )
    else()
        add_custom_command(TARGET graber POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/backup"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:graber> "${CMAKE_SOURCE_DIR}/backup/graber"
            COMMENT "Backing up the executable to project backup/"
        )
    endif()
endif()
