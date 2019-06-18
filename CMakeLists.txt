project(filesort LANGUAGES CXX)
set(SOURCES custom_printf.cpp)
add_executable(filesort ${SOURCES})
target_compile_options(filesort PRIVATE -Wall -Wconversion)