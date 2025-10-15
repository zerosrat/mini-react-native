# Mini React Native - CMake 构建配置
#
# 这个文件配置了整个项目的构建过程，包括：
# 1. 编译器设置和标准
# 2. JavaScriptCore 框架链接
# 3. 源文件组织和目标创建
# 4. 平台特定的配置

cmake_minimum_required(VERSION 3.15)
project(MiniReactNative)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项（由 CMAKE_BUILD_TYPE 变量决定）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # Debug 和 Release 模式都会应用的编译标志
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0") # Debug 模式下特有的编译标志
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2") # Release 模式下特有的编译标志

# 包含目录
include_directories(src)

# 源文件
set(BRIDGE_SOURCES
    src/bridge/JSCExecutor.cpp
)

set(MODULE_SOURCES
    # src/modules/DeviceInfo.cpp  # 下一步添加
)

set(UTIL_SOURCES
    # src/utils/Logger.cpp        # 下一步添加
)

# 所有源文件
set(ALL_SOURCES
    ${BRIDGE_SOURCES}
    ${MODULE_SOURCES}
    ${UTIL_SOURCES}
)

# 创建静态库
add_library(mini_react_native STATIC ${ALL_SOURCES})

# 平台特定配置
if(APPLE)
    # macOS/iOS 配置

    # 在 macOS 中查找 JavaScriptCore 框架
    # 可能在这个路径：/System/Library/Frameworks/JavaScriptCore.framework
    find_library(JAVASCRIPTCORE_FRAMEWORK JavaScriptCore)
    if(NOT JAVASCRIPTCORE_FRAMEWORK)
        message(FATAL_ERROR "JavaScriptCore framework not found")
    endif()

    # 链接 JavaScriptCore 框架
    target_link_libraries(mini_react_native ${JAVASCRIPTCORE_FRAMEWORK})

    # macOS 特定设置
    if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        message(STATUS "Building for macOS with deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()

    # iOS 特定设置 (暂时注释，专注 macOS)
    # if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    #     set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
    #     message(STATUS "Building for iOS with deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    # endif()

elseif(ANDROID)
    # Android 配置 (未来实现)
    message(STATUS "Android build configuration - TODO")

elseif(WIN32)
    # Windows 配置 (未来实现)
    message(STATUS "Windows build configuration - TODO")

else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# 创建测试可执行文件
add_executable(mini_rn_test examples/test_basic.cpp)
target_include_directories(mini_rn_test PRIVATE src)
target_link_libraries(mini_rn_test mini_react_native)

# MessageQueue 测试可执行文件
add_executable(test_messagequeue examples/test_messagequeue.cpp)
target_include_directories(test_messagequeue PRIVATE src)
target_link_libraries(test_messagequeue mini_react_native)

# 安装配置（make install 时才会执行）
# 安装静态库到 /usr/local/lib 下
install(TARGETS mini_react_native
        ARCHIVE DESTINATION lib)
# 安装头文件到 /usr/local/include/mini_react_native 下
install(DIRECTORY src/
        DESTINATION include/mini_react_native
        FILES_MATCHING PATTERN "*.h")

# 编译信息
message(STATUS "=== Mini React Native Build Configuration ===")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "==============================================")