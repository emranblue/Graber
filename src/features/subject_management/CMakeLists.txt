add_library(graber_subject_management STATIC
    subject_manager.cpp
)

target_include_directories(graber_subject_management PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(graber_subject_management PUBLIC
    Qt6::Core
    graber_utilities
    graber_core
)
