cmake_minimum_required(VERSION 3.28)

project(webminc)

add_executable(${PROJECT_NAME} src/main.c)

add_subdirectory(webmin)