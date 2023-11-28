# 添加 doctest 预处理宏
add_definitions(-D DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

# 添加 fmt 预处理宏
add_definitions(-D FMT_HEADER_ONLY)

add_executable(test-factory test_factory.cpp)
target_link_libraries(test-factory
PUBLIC

)

# # 寻找Qt5库
# find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)



# add_executable(test-thread test_thread.cpp)
# target_link_libraries(test-thread
# PUBLIC
#     Qt5::Core
#     Qt5::Gui
#     Qt5::Widgets
# )