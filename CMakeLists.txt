cmake_minimum_required(VERSION 3.10)
if(WIN32)
  set(CMAKE_TOOLCHAIN_FILE "C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(App)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)  

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main5.cpp
    ${PROJECT_SOURCE_DIR}/src/camera.cpp
    ${PROJECT_SOURCE_DIR}/src/model.cpp
    ${PROJECT_SOURCE_DIR}/src/shader.cpp
    ${PROJECT_SOURCE_DIR}/src/cshader.cpp
    ${PROJECT_SOURCE_DIR}/src/util.cpp
    ${PROJECT_SOURCE_DIR}/src/bvh.cpp
    ${PROJECT_SOURCE_DIR}/external/glad/src/gl.c
)

find_package(OpenGL REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/external/glad/include
    ${PROJECT_SOURCE_DIR}/external/tinygltf
)

target_compile_definitions(${PROJECT_NAME} PRIVATE SOURCE_DIR="${PROJECT_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
)
