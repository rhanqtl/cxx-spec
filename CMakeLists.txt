cmake_minimum_required(VERSION 3.28)

project(cxx-spec LANGUAGES CXX)

include(cmake/FindDeps.cmake)

include_directories(include)

enable_testing()

add_subdirectory(examples)
