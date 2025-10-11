cmake_minimum_required(VERSION 3.16)

set(TARGET ecs_test)

set(GTEST_ROOT third_party/googletest)

add_subdirectory(${GTEST_ROOT})

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "*.cpp")
add_executable(${TARGET} ${SRC})

enable_testing()
target_include_directories(${TARGET}
    PRIVATE include
    PRIVATE ${GTEST_ROOT}/include
)
target_link_libraries(${TARGET}
    PRIVATE GTest::gtest_main
    PRIVATE ecs
)

add_test(NAME ${TARGET} COMMAND ${TARGET})
