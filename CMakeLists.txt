cmake_minimum_required(VERSION 3.28)
project(StarshipWorkstation LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(main src/main.cpp src/DesktopGame.cpp)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics SFML::Audio)

# Ensure UTF-8 source encoding for Chinese UI strings (MSVC / MinGW)
if(MSVC)
    target_compile_options(main PRIVATE /utf-8)
else()
    target_compile_options(main PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# Always copy assets folder to output directory during build, even if source code hasn't changed
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:main>/assets"
)
add_dependencies(main CopyAssets)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
