# Source files for the SQL engine library
set(SQL_ENGINE_SOURCES
    lexer.cpp
    parser.cpp
    ast.cpp
    storage.cpp
    query_engine.cpp
    llvm_codegen.cpp
)

# Create the SQL engine library
add_library(sql_engine_lib STATIC ${SQL_ENGINE_SOURCES})

target_include_directories(sql_engine_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(sql_engine_lib
    ${llvm_libs}
)