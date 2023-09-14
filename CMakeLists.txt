cmake_minimum_required(VERSION 3.11)
project(6502ASM VERSION 0.0.1 LANGUAGES CXX)

#
# For getting Catch2 sources
#
Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# Add External ANTLR 4 CPP .cmake file to fetch ANTLR from github and to compile it
#
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/antlr/cmake)

#
# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MT flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
set(ANTLR4_TAG 4.13.0)
set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/4.13.0.zip)
include(ExternalAntlr4Cpp)

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/antlr/antlr-4.13.0-complete.jar)

# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

# Generate lexer/parser/listener classes out of the grammar file
antlr_target(MOS6502Parser MOS6502.g4
            OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated/antlr4
            LEXER
            PARSER
            LISTENER
            )

# Include the static antlr runtime folders and generated lexer, parser, listener classes
include_directories(${ANTLR4_INCLUDE_DIRS})
include_directories(${ANTLR_MOS6502Parser_OUTPUT_DIR})

message(STATUS "OUTDIR=${ANTLR_MOS6502Parser_OUTPUT_DIR}")
message(STATUS "OUTFILES=${ANTLR_MOS6502Parser_CXX_OUTPUTS}")

#
# 6502 Assembler binary
#
add_executable(ASM6502
    src/main.cpp
    src/ASM6502.cpp
    src/listener/MOS6502Listener.cpp
    src/listener/CodeLine.cpp
    src/listener/MemBlocks.cpp
    ${ANTLR_MOS6502Parser_CXX_OUTPUTS}
    )    

target_link_libraries(ASM6502 PRIVATE antlr4_static)

#
# Tests
#
add_executable(ASM6502Test 
    test/MOS6502AssemblerTest.cpp
    test/MOS6502ErrorTest.cpp
    test/MOS6502TestHelper.cpp
    src/ASM6502.cpp
    src/listener/MOS6502Listener.cpp
    src/listener/CodeLine.cpp
    src/listener/MemBlocks.cpp
    ${ANTLR_MOS6502Parser_CXX_OUTPUTS}
    )

# we include only the src folder to force #including our MOS6502Listener.h, not the generated one
target_include_directories(ASM6502Test PRIVATE 
    ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(ASM6502Test PRIVATE antlr4_static)
target_link_libraries(ASM6502Test PRIVATE Catch2::Catch2WithMain)
add_test(NAME ASM6502Test COMMAND ASM6502Test)
