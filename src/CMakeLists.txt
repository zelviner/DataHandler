# 设置应用程序名
set(APP_NAME DataHandler)

# 设置二进制文件输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 设置Qt5
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 寻找Qt5库
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

# 设置UI文件搜索路径
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/res/ui)

# 查找源文件
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

# 添加资源文件
file(GLOB RESOURCES ${CMAKE_SOURCE_DIR}/res/rc/*)

# 添加可执行文件
add_executable(${APP_NAME}  ${SOURCES} ${RESOURCES}) #debug
# add_executable(${APP_NAME} WIN32 ${SOURCES} ${RESOURCES}  ${MY_VERSIONINFO_RC}) # release

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# 添加编译时的宏定义
target_compile_definitions(${APP_NAME}
PUBLIC
    NOLFS  # 可能用于禁用某些与LFS（Large File Storage）相关的功能
    _CRT_SECURE_NO_WARNINGS  # 禁用对不安全函数的警告
    _WINSOCK_DEPRECATED_NO_WARNINGS  # 禁用对已弃用Winsock功能的警告
)

# 为target添加头文件
target_include_directories(${APP_NAME}
PUBLIC
)

# 为target添加库文件目录
target_link_directories(${APP_NAME}
PUBLIC
)

find_package(xlnt CONFIG REQUIRED)

# 为target添加需要链接的共享库
TARGET_LINK_LIBRARIES(${APP_NAME}
PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    # Qt5::Network
    Winscard
    VC/libcrypto32MD
    VC/libssl32MD
    Crypt32   
    ws2_32
    zel
    xhlanguage
    xlnt::xlnt
)

# 获取 target 链接的库
get_target_property(TARGET_LIBRARIES ${APP_NAME} LINK_LIBRARIES)

# 遍历链接的库，查找对应的 DLL 文件并复制到输出目录
foreach(lib IN LISTS TARGET_LIBRARIES)
    # 从::分割出库名，防止没有::的库名被误处理
    string(REPLACE "::" ";" lib_list ${lib})
    list(GET lib_list 0 lib_name)

    # 定义可能的 DLL 文件路径
    set(lib_dll "${VCPKG_LIB_DIR}/bin/${lib_name}.dll")
    set(libd_dll "${VCPKG_LIB_DIR}/bin/${lib_name}d.dll")

    # 检查并复制 DLL 文件
    foreach(dll_file ${lib_dll} ${libd_dll})
        if(EXISTS ${dll_file})
            message(STATUS "Copying ${dll_file} to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
            file(COPY ${dll_file} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
            break()  # 一旦找到存在的 DLL，就退出循环
        endif()
    endforeach()
endforeach()