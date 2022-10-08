cmake_minimum_required(VERSION 3.16)
project(PROJECT)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(util/shader)
add_subdirectory(util/renderer)


ADD_EXECUTABLE(PROJECT
        source/main.cpp)


target_link_libraries(
        PROJECT LINK_PUBLIC
        SHADER RENDERER

        -lGLEW -lGL -lGLU -lglut -lpthread -lm)


file(GLOB MY_SHADERS
        "source/shaders/*.tesc"
        "source/shaders/*.tese"
        "source/shaders/*.vert"
        "source/shaders/*.frag"
        "source/shaders/*.geom"
        "source/shaders/*.comp")
foreach(shader ${MY_SHADERS})
    configure_file(${shader} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach()
