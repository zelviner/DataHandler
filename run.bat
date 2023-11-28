@echo off
setlocal

rem 避免中文乱码
chcp 65001

rem 检查是否提供了可执行文件名称参数
if "%1"=="" (
    echo.
    echo 请提供可执行文件名称作为参数。例如： .\run.bat hello
    goto :eof
)

rem 设置 CMake 和编译目录
set BUILD_DIR=build
set EXECUTABLE_PATH=bin
set EXECUTABLE_NAME=%1.exe


rem 设置 Visual Studio 环境变量
set VS_PATH="D:\Development\Microsoft Visual Studio\VC\Auxiliary\Build\vcvars32.bat"
if not exist %VS_PATH% (
    set VS_PATH="C:\Development\Microsoft Visual Studio\VC\Auxiliary\Build\vcvars32.bat"
)
call %VS_PATH%

rem 检查编译目录是否存在，如果存在则删除，然后创建
if not exist %BUILD_DIR% (
    mkdir %BUILD_DIR%
) else (
    rem 第二个参数为 rebuild 表示删除原有的编译目录
    if "%2"=="rebuild" (
        rmdir /s /q %BUILD_DIR%
        mkdir %BUILD_DIR%
    )
)


rem 进入编译目录
cd %BUILD_DIR%

rem 使用 CMake 生成项目文件
"cmake" -G "Ninja" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

rem 使用 Ninja 进行编译
ninja

rem 如果编译成功，运行生成的可执行文件
if %ERRORLEVEL%==0 (
    rem 清空屏幕
    cls

    rem 进入可执行文件所在目录
    cd ..\%EXECUTABLE_PATH%
    echo 执行结果：
    echo ===========================================================================
    echo.
    %EXECUTABLE_NAME%
    echo.
    echo ===========================================================================
) else (
    echo.
    echo Build failed!
    echo.
)

endlocal