add_library(graber_toc STATIC
    toc_generator.cpp
)

target_include_directories(graber_toc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_toc PUBLIC
    Qt6::Core
    graber_utilities
    graber_core
)
