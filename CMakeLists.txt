cmake_minimum_required(VERSION 4.1)
project(RandomRaypath)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RayClientRenderer)

add_compile_options(RayClientRenderer PRIVATE -fno-exceptions -fno-rtti)

# Add libraries include
target_include_directories(RayClientRenderer PRIVATE ${CMAKE_SOURCE_DIR}/third_party/tomlplusplus/include)
target_compile_definitions(RayClientRenderer PRIVATE TOML_EXCEPTIONS=0) # exceptions are not accepted

file(GLOB_RECURSE CORE_IMPL CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE CORE_MODULES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.mpp
)

target_sources(RayClientRenderer
        PRIVATE
        ${CORE_IMPL}
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${CORE_MODULES}
)

target_link_libraries(RayClientRenderer PRIVATE "-lstdc++exp")