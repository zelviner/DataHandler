# 最低版本
cmake_minimum_required(VERSION 3.20.2) 

# 设置项目名称
project(DataHandler)

# 采用C++14标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置安装路径
set(ZEL_VENDOR $ENV{ZEL_HOME}/vendor)
set(CMAKE_INSTALL_PREFIX ${ZEL_VENDOR}/${PROJECT_NAME}/${CMAKE_BUILD_TYPE})

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
    add_compile_options(/wd4273)
    # Specify MSVC UTF-8 encoding   
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")    
endif()

# 设置 vcpkg
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(VCPKG_INDTALLED $ENV{ZEL_HOME}/pkg/vcpkg/installed/x64-windows)
    file(GLOB VCPKG_SHARED_LIBS ${VCPKG_INDTALLED}/share/*)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(VCPKG_INDTALLED $ENV{ZEL_HOME}/pkg/vcpkg/installed/x86-windows)
    file(GLOB VCPKG_SHARED_LIBS ${VCPKG_INDTALLED}/share/*)
endif()
list(APPEND CMAKE_PREFIX_PATH ${VCPKG_SHARED_LIBS})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VCPKG_LIB_DIR ${VCPKG_INDTALLED}/debug)
else()
    set(VCPKG_LIB_DIR ${VCPKG_INDTALLED})
endif()

# 设置三方库的安装路径, 搜索路径, 链接路径
file(GLOB VENDOR_DIRS ${ZEL_VENDOR}/*/${CMAKE_BUILD_TYPE})
file(GLOB VENDOR_INCLUDE_DIRS ${ZEL_VENDOR}/*/${CMAKE_BUILD_TYPE}/include)
file(GLOB VENDOR_LIB_DIRS ${ZEL_VENDOR}/*/${CMAKE_BUILD_TYPE}/lib)
list(APPEND CMAKE_PREFIX_PATH ${VENDOR_DIRS})
include_directories(${VENDOR_INCLUDE_DIRS})
link_directories(${VENDOR_LIB_DIRS})

# 添加子工程
add_subdirectory(src)
add_subdirectory(test)