cmake_minimum_required(VERSION 3.0)
cmake_policy(VERSION 3.0...3.18.4)

project(OpenCL-template)

add_executable(OpenCL-template src/main.cpp)

find_package(OpenCLHeaders REQUIRED)
find_package(OpenCLICDLoader REQUIRED)
find_package(OpenCLHeadersCpp REQUIRED)

target_link_libraries(OpenCL-template PRIVATE OpenCL::Headers OpenCL::OpenCL OpenCL::HeadersCpp)