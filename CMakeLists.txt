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

# 通用源文件 (跨平台)
set(COMMON_SOURCES
    src/common/bridge/JSCExecutor.cpp
    src/common/modules/ModuleRegistry.cpp
    src/common/modules/NativeModule.cpp
    src/common/utils/JSONParser.cpp
)

# 平台特定源文件
if(APPLE)
    # 根据具体平台选择源文件
    if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
        set(PLATFORM_SOURCES
            src/ios/modules/deviceinfo/DeviceInfoModule.mm
        )
    else()
        # macOS
        set(PLATFORM_SOURCES
            src/macos/modules/deviceinfo/DeviceInfoModule.mm
        )
    endif()
    # 设置 Objective-C++ 编译标志
    set_source_files_properties(${PLATFORM_SOURCES}
        PROPERTIES COMPILE_FLAGS "-fobjc-arc")
elseif(ANDROID)
    message(STATUS "Building for Android with API level: ${ANDROID_PLATFORM}")

    # 使用 Maven JSC 库 (org.webkit:android-jsc)
    # 库由 Gradle 管理，我们只需要设置库名
    set(JAVASCRIPTCORE_LIB "jsc")
    message(STATUS "Using JSC library: ${JAVASCRIPTCORE_LIB}")

    # 使用项目内置的 JSC 头文件
    set(JSC_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/jsc-headers")

    # 验证关键头文件存在
    if(EXISTS "${JSC_INCLUDE_DIR}/JavaScriptCore/JSBase.h")
        message(STATUS "Found JSC headers: ${JSC_INCLUDE_DIR}")
        include_directories(${JSC_INCLUDE_DIR})
    else()
        message(FATAL_ERROR "JSC headers not found at: ${JSC_INCLUDE_DIR}")
    endif()

    # Android 特定源文件
    set(PLATFORM_SOURCES
        src/android/bridge/JSCExecutorAndroid.cpp
        src/android/modules/deviceinfo/DeviceInfoModule.cpp
        src/android/jni/com_minirn_JSCExecutor.cpp
    )
endif()

# 所有源文件
set(ALL_SOURCES
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
)

# 创建动态库（用于 Android JNI）
add_library(mini_react_native SHARED ${ALL_SOURCES})

# 平台特定配置
if(APPLE)
    # macOS/iOS 配置

    # 在 macOS 中查找 JavaScriptCore 框架
    # 可能在这个路径：/System/Library/Frameworks/JavaScriptCore.framework
    find_library(JAVASCRIPTCORE_FRAMEWORK JavaScriptCore)
    if(NOT JAVASCRIPTCORE_FRAMEWORK)
        message(FATAL_ERROR "JavaScriptCore framework not found")
    endif()

    # 查找并链接 Foundation 框架 (所有 Apple 平台共用)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    if(NOT FOUNDATION_FRAMEWORK)
        message(FATAL_ERROR "Foundation framework not found")
    endif()

    # 平台特定框架
    if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
        # iOS 特定框架
        find_library(UIKIT_FRAMEWORK UIKit)
        if(NOT UIKIT_FRAMEWORK)
            message(FATAL_ERROR "UIKit framework not found")
        endif()
        set(PLATFORM_FRAMEWORKS ${UIKIT_FRAMEWORK})
    else()
        # macOS 特定框架
        find_library(IOKIT_FRAMEWORK IOKit)
        if(NOT IOKIT_FRAMEWORK)
            message(FATAL_ERROR "IOKit framework not found")
        endif()
        set(PLATFORM_FRAMEWORKS ${IOKIT_FRAMEWORK})
    endif()

    # 链接所需框架
    target_link_libraries(mini_react_native
        ${JAVASCRIPTCORE_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${PLATFORM_FRAMEWORKS}
    )

    # 平台特定部署目标设置
    if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
        message(STATUS "Building for iOS with deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
        message(STATUS "Building for macOS with deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()

elseif(ANDROID)
    # Android 配置
    message(STATUS "Configuring Android build with JSC library")

    # 使用 intl 变体时，JSC 库由 Gradle 自动管理，不需要手动链接
    message(STATUS "Using JSC intl variant - library will be included by Gradle")

    # 链接 Android 特定库（不包括 JSC）
    # 添加 undefined symbols 引用，让动态链接器在运行时解析
    target_link_libraries(mini_react_native
        android
        log
        -Wl,--undefined
        -Wl,--allow-shlib-undefined
    )

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

# 模块框架测试可执行文件
add_executable(test_module_framework examples/test_module_framework.cpp)
target_include_directories(test_module_framework PRIVATE src)
target_link_libraries(test_module_framework mini_react_native)

# 集成测试可执行文件（使用打包后的 JavaScript bundle）
if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    # iOS版本需要包含Objective-C helper文件
    add_executable(test_integration examples/test_integration.cpp examples/ios_bundle_helper.m)
else()
    # 其他平台只需要C++文件
    add_executable(test_integration examples/test_integration.cpp)
endif()
target_include_directories(test_integration PRIVATE src)
target_link_libraries(test_integration mini_react_native)

# 性能测试可执行文件（轻量级性能检查）
add_executable(test_performance examples/test_performance.cpp)
target_include_directories(test_performance PRIVATE src)
target_link_libraries(test_performance mini_react_native)

# iOS 特定配置：复制 JavaScript bundle 到应用包
if(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
    # 为 test_integration 添加 bundle.js 资源复制
    add_custom_command(TARGET test_integration POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/dist/bundle.js"
        "$<TARGET_FILE_DIR:test_integration>/bundle.js"
        COMMENT "Copying JavaScript bundle to iOS app package"
    )

    # 为 test_integration 添加测试脚本复制
    add_custom_command(TARGET test_integration POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/examples/scripts/test_deviceinfo.js"
        "$<TARGET_FILE_DIR:test_integration>/test_deviceinfo.js"
        COMMENT "Copying test script to iOS app package"
    )
endif()


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