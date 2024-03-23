# 添加 doctest 预处理宏
add_definitions(-D DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

# 添加 fmt 预处理宏
add_definitions(-D FMT_HEADER_ONLY)


add_executable(test test.cpp)
target_link_libraries(test
PUBLIC

)

add_executable(ftp-test ftp.cpp)

# 为target添加头文件
target_include_directories(ftp-test
PUBLIC
    "$ENV{CPATH}/pkg/zel-win/include"
    "$ENV{CPATH}/pkg/openssl/include"
)

# 为target添加库文件目录
target_link_directories(ftp-test
PUBLIC
    "$ENV{CPATH}/pkg/zel-win/lib"
    "$ENV{CPATH}/pkg/openssl/lib/VC/static"
)

# 为target添加需要链接的共享库
TARGET_LINK_LIBRARIES(ftp-test
PRIVATE
    "libcrypto32MD.lib"
    "libssl32MD.lib"
    "Crypt32.lib"   
    "ws2_32.lib"
    "zel-win.lib"
)


target_compile_definitions(ftp-test
PUBLIC
    NOLFS
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
)