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
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/panels
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/dialogs
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# On Windows, this embeds resources/icons/app.ico as the .exe's own icon
# (Explorer/taskbar/shortcuts). Harmless no-op on other platforms.
if(WIN32)
    set(GRABER_WIN_RC resources/graber.rc)
endif()

# On Windows, WIN32 marks this as a GUI (non-console) application so no
# extra blank console window is spawned alongside the Qt UI at startup.
if(WIN32)
    set(GRABER_WIN32_FLAG WIN32)
endif()

# Create the executable
add_executable(graber
    ${GRABER_WIN32_FLAG}
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
    repositories/NoteRepository_Subjects.cpp
    repositories/NoteRepository_Content.cpp
    repositories/NoteRepository_Append.cpp
    repositories/NoteRepository_Diagram.cpp
    repositories/NoteRepository_Heading.cpp
    repositories/IniSectionRepository.h
    repositories/IniSectionRepository.cpp

    # Formatters
    formatters/MarkdownDocumentFormatter.h
    formatters/MarkdownDocumentFormatter.cpp
    formatters/MarkdownUtils.h
    formatters/MarkdownUtils.cpp
    formatters/DiagramTemplates.h
    formatters/DiagramTemplates.cpp

    # Monitors
    monitors/ClipboardMonitor.h
    monitors/ClipboardMonitor.cpp

    # Shortcuts
    shortcuts/ShortcutManager.h
    shortcuts/ShortcutManager.cpp

    # UI — controller (split for token/modularity) + thin compositor
    ui/ClipboardGrabber.h
    ui/ClipboardGrabber.cpp
    ui/controllers/ClipboardGrabber_Actions.cpp
    ui/controllers/ClipboardGrabber_Capture.cpp
    ui/controllers/ClipboardGrabber_Navigation.cpp
    ui/controllers/ClipboardGrabber_Headings.cpp
    ui/ClipboardGrabberUI.h
    ui/ClipboardGrabberUI.cpp

    # UI Panels (modular visual blocks)
    ui/panels/StatusPanel.h
    ui/panels/StatusPanel.cpp
    ui/panels/SubjectPanel.h
    ui/panels/SubjectPanel.cpp
    ui/panels/CapturePanel.h
    ui/panels/CapturePanel.cpp
    ui/panels/HeadingPanel.h
    ui/panels/HeadingPanel.cpp
    ui/panels/ControlsBar.h
    ui/panels/ControlsBar.cpp

    # UI Dialogs
    ui/dialogs/HeadingSelectDialog.h
    ui/dialogs/HeadingSelectDialog.cpp
    ui/dialogs/ShortcutsSettingsDialog.h
    ui/dialogs/ShortcutsSettingsDialog.cpp
    ui/dialogs/ExportNoteWizard.h
    ui/dialogs/ExportNoteWizard.cpp

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
