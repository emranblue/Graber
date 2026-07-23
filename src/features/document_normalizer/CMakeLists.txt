add_library(graber_normalizer STATIC
    normalizer.cpp
)

target_include_directories(graber_normalizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_normalizer PUBLIC
    Qt6::Core
    graber_utilities
    graber_core
)
