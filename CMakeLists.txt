cmake_minimum_required(VERSION 4.1)
project(RandomRaypath LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RayClientRenderer)

target_compile_options(RayClientRenderer PRIVATE -fno-exceptions -fno-rtti)

#GLM
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/glm)

add_library(glm_mod STATIC)
target_link_libraries(glm_mod PUBLIC glm::glm)
target_compile_options(glm_mod PUBLIC -fno-exceptions -fno-rtti)

set(GLM_CPPM "${CMAKE_SOURCE_DIR}/third_party/glm/glm.cppm")

target_sources(glm_mod
        PUBLIC
            FILE_SET glm_modules TYPE CXX_MODULES
            BASE_DIRS "${CMAKE_SOURCE_DIR}/third_party/glm"
            FILES "${GLM_CPPM}"
)

# toml++
target_include_directories(RayClientRenderer PRIVATE ${CMAKE_SOURCE_DIR}/third_party/tomlplusplus/include)
target_compile_definitions(RayClientRenderer PRIVATE TOML_EXCEPTIONS=0)

file(GLOB_RECURSE CORE_IMPL CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE CORE_MODULES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.mpp)

target_sources(RayClientRenderer
        PRIVATE
            ${CORE_IMPL}
        PUBLIC
            FILE_SET ray_modules TYPE CXX_MODULES
            BASE_DIRS "${CMAKE_SOURCE_DIR}/src"
            FILES ${CORE_MODULES}
)

target_link_libraries(RayClientRenderer PRIVATE glm_mod)
target_link_libraries(RayClientRenderer PRIVATE stdc++exp)