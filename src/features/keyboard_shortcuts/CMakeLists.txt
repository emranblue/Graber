add_library(graber_shortcuts STATIC
    shortcut_manager.cpp
)

target_include_directories(graber_shortcuts PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_shortcuts PUBLIC
    Qt6::Core
    Qt6::Widgets
    graber_utilities
)
