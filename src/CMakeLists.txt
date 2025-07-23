# [1] 设置应用程序名称 ------------------------------------------------
set(APP_NAME DataHandler)
string(TOUPPER ${APP_NAME} UPPER_APP_NAME)

# [2] 输出目录配置 -----------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# [3] Qt 编译设置 -----------------------------------------------------
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# [4] 查找 Qt5 组件 ---------------------------------------------------
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(zel CONFIG REQUIRED)
find_package(xhlanguage CONFIG REQUIRED)
find_package(xlnt CONFIG REQUIRED)

# [5] UI 搜索路径 -----------------------------------------------------
list(APPEND CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/res/ui)

# [6] 查找源文件/资源 --------------------------------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/*.hpp
)

file(GLOB RESOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/res/rc/*)

# [7] 添加可执行文件 ---------------------------------------------------
add_executable(${APP_NAME} ${SOURCES} ${RESOURCES}) #debug
# add_executable(${APP_NAME} WIN32 ${SOURCES} ${RESOURCES}  ${MY_VERSIONINFO_RC}) # release

# [8] 包含 Qt 的私有头（如果有需要，比如自定义窗口样式）--------------
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

# [9] 添加宏定义 -------------------------------------------------------
target_compile_definitions(${APP_NAME} PUBLIC
    NOLFS
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

# [10] 链接 Qt 库 ------------------------------------------------------
target_link_libraries(${APP_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    fmt::fmt
    zel::zel
    xhlanguage::xhlanguage
    xlnt::xlnt
    openssl/VC/libcrypto32MD
    openssl/VC/libssl32MD
    Crypt32
    Winscard
)

# [11] Qt 翻译处理 -----------------------------------------------------
# set(TRANSLATIONS
#     res/translations/zh_CN.ts
#     res/translations/en_US.ts
# )

# # 使用 qt5_add_translation 自动将 .ts 转换成 .qm
# qt5_create_translation(QM_FILES ${SOURCES} ${TRANSLATIONS})


# # 拷贝 .qm 文件到可执行文件目录
# foreach(QM ${QM_FILES})
#     add_custom_command(TARGET ${APP_NAME} POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             ${QM}
#             $<TARGET_FILE_DIR:${APP_NAME}>
#         COMMENT "Copying translation file: ${QM}"
#     )
# endforeach()

