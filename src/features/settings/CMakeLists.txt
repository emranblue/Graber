add_library(graber_settings STATIC
    settings_manager.cpp
)

target_include_directories(graber_settings PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_settings PUBLIC
    Qt6::Core
    graber_utilities
    graber_shortcuts
)
