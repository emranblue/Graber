add_library(graber_clipboard STATIC
    clipboard_monitor.cpp
)

target_include_directories(graber_clipboard PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_clipboard PUBLIC
    Qt6::Core
    Qt6::Gui
    graber_utilities
)
