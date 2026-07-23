add_library(graber_utilities STATIC
    logger.cpp
    file_handler.cpp
    slug_generator.cpp
)

target_include_directories(graber_utilities PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_utilities PUBLIC
    Qt6::Core
)
