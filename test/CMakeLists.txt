function(add_test_executable name)
    file(GLOB_RECURSE files ${name}/*.cpp)
    add_executable(${name}-test ${files})
    target_include_directories(${name}-test 
    PUBLIC
    )
    target_link_libraries(${name}-test
    PUBLIC
        GTest::gtest_main
        ${ARGN}
    )
    add_test(
        NAME ${name}-test
        COMMAND ${name}-test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction(add_test_executable name)

find_package(GTest REQUIRED)

enable_testing()


# 添加测试
add_test_executable(xlnt xlntd)
add_test_executable(time)
