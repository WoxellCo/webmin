cmake_minimum_required(VERSION 3.28)

project(webminc)

add_executable(${PROJECT_NAME} src/main.c)

add_subdirectory(webmin)
target_link_libraries(${PROJECT_NAME} PRIVATE webmin)