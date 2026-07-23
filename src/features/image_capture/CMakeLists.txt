add_library(graber_image_capture STATIC
    image_capturer.cpp
)

target_include_directories(graber_image_capture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_image_capture PUBLIC
    Qt6::Core
    Qt6::Gui
    graber_utilities
)
