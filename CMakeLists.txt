cmake_minimum_required(VERSION 3.16)
project(SQLEngine VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set LLVM definitions
add_definitions(${LLVM_DEFINITIONS})

# Include LLVM headers
include_directories(${LLVM_INCLUDE_DIRS})

# Link against LLVM libraries
llvm_map_components_to_libnames(llvm_libs 
    core 
    executionengine 
    interpreter 
    mc 
    support 
    nativecodegen
    orcjit
    mcjit
    x86asmparser
    x86codegen
    x86desc
    x86info
)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)

# Create main executable
add_executable(sql_engine
    main.cpp
)

target_link_libraries(sql_engine
    sql_engine_lib
    ${llvm_libs}
)

# Set compiler flags
target_compile_options(sql_engine PRIVATE
    -Wall -Wextra -O2
)