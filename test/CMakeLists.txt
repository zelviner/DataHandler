# 添加 doctest 预处理宏
add_definitions(-D DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

# 添加 fmt 预处理宏
add_definitions(-D FMT_HEADER_ONLY)

# add_executable(test-factory test_factory.cpp)
# target_link_libraries(test-factory
# PUBLIC

# )

add_executable(test test.cpp)
target_link_libraries(test
PUBLIC

)

