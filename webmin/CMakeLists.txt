cmake_minimum_required(VERSION 3.28)

project(webmin)

set(CMAKE_C_STANDARD           99)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(CMAKE_C_EXTENSIONS        OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#add_library(${PROJECT_NAME} STATIC src/webmin.c src/html-min.c) #src/js-min.c 
add_executable(${PROJECT_NAME}c main.c src/webmin.c src/html-min.c)

#target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME}c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)