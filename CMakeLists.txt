cmake_minimum_required(VERSION 3.10)
project(InterstellarBlackHole)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Базовые зависимости
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Eigen3 REQUIRED)

# Исполняемый файл
add_executable(interstellar_blackhole
    src/main.cpp
    src/BlackHole.cpp
    src/Renderer.cpp
    src/PhysicsEngine.cpp
    src/Camera.cpp
    src/ShaderManager.cpp
)

# Линковка
target_link_libraries(interstellar_blackhole
    ${OPENGL_LIBRARIES}
    glfw
    GLEW::GLEW
    Eigen3::Eigen
    pthread
    dl
)

# Включаемые директории
target_include_directories(interstellar_blackhole PRIVATE 
    src
    ${Eigen3_INCLUDE_DIRS}
)

# Флаги оптимизации
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(interstellar_blackhole PRIVATE -O3)
else()
    target_compile_options(interstellar_blackhole PRIVATE -g)
endif()
