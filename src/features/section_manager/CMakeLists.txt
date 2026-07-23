add_library(graber_section_manager STATIC
    section_manager.cpp
)

target_include_directories(graber_section_manager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_section_manager PUBLIC
    Qt6::Core
    Qt6::Widgets
    graber_utilities
    graber_core
)
