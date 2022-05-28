cmake_minimum_required(VERSION 3.0)
project(LOCC LANGUAGES C
        DESCRIPTION "Count lines of code (LOC) in a C project")
add_executable(locc src/main.c)
target_compile_definitions(locc
                            PUBLIC DEBUG ARRAY_STORAGE)

if(EXISTS "${CMAKE_SOURCE_DIR}/test")
        enable_testing()
        function(locc_test depth)
                add_test(NAME "lvl${depth}"
                        COMMAND locc . "-d${depth}"
                        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test")
                        # set(res 200*${depth})
                        math(EXPR res "200*${depth}")
                set_tests_properties("lvl${depth}" PROPERTIES PASS_REGULAR_EXPRESSION "C: ${res}")
        endfunction()
        locc_test(1)
        locc_test(2)
        locc_test(3)
        locc_test(4)
        locc_test(5)
        locc_test(6)
        locc_test(7)
endif()
