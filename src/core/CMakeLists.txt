add_library(graber_core STATIC
    application_state.cpp
    section_definitions.cpp
)

target_include_directories(graber_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_core PUBLIC
    Qt6::Core
    graber_utilities
)
