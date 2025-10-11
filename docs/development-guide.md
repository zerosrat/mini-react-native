# 开发指南

## 快速开始

### 环境要求

- Node.js 16+
- iOS: Xcode 12+, iOS 11+
- Android: Android Studio, API Level 21+
- C++11 支持的编译器

### 项目结构

```
mini-rn/
├── js/                     # JavaScript Bridge 层
│   ├── MessageQueue.js     # 消息队列核心
│   ├── NativeModules.js    # Native 模块代理
│   └── Bridge.js           # 高级 Bridge API
├── cpp/                    # C++ Bridge 层
│   ├── JSCExecutor.h/.cpp  # JavaScript 执行器
│   └── NativeModule.h/.cpp # Native 模块基类
├── ios/                    # iOS 平台实现
│   ├── DeviceInfoModule.h  # iOS 模块头文件
│   └── DeviceInfoModule.mm # iOS 模块实现
├── android/                # Android 平台实现
│   ├── DeviceInfoModule.h  # Android 模块头文件
│   ├── DeviceInfoModule.cpp# Android 模块实现
│   └── DeviceInfoHelper.java # Java 辅助类
├── examples/               # 使用示例
└── docs/                   # 文档
```

## 创建自定义 Native 模块

### 1. 定义模块接口

```cpp
// MyCustomModule.h
#include "../cpp/NativeModule.h"

class MyCustomModule : public bridge::NativeModule {
public:
    MyCustomModule();
    
    // 实现基类方法
    std::map<std::string, std::string> getConstants() override;
    std::vector<bridge::NativeMethodDescriptor> getMethods() override;
    std::string callMethod(int methodId, const std::vector<std::string>& arguments,
                          std::function<void(const std::string&, const std::string&)> callback) override;

private:
    // 具体方法实现
    void myMethod(const std::vector<std::string>& args,
                  std::function<void(const std::string&, const std::string&)> callback);
};
```

### 2. 实现模块功能

```cpp
// MyCustomModule.cpp
#include "MyCustomModule.h"

// 注册模块
REGISTER_NATIVE_MODULE(MyCustomModule, "MyCustomModule")

MyCustomModule::MyCustomModule() : NativeModule("MyCustomModule") {
    log("info", "MyCustomModule 初始化");
}

std::map<std::string, std::string> MyCustomModule::getConstants() {
    std::map<std::string, std::string> constants;
    constants["VERSION"] = "1.0.0";
    constants["PLATFORM"] = "example";
    return constants;
}

std::vector<bridge::NativeMethodDescriptor> MyCustomModule::getMethods() {
    return {
        bridge::NativeMethodDescriptor("myMethod", {}, true, false)
    };
}

std::string MyCustomModule::callMethod(int methodId, const std::vector<std::string>& arguments,
                                      std::function<void(const std::string&, const std::string&)> callback) {
    switch (methodId) {
        case 0: // myMethod
            myMethod(arguments, callback);
            break;
        default:
            if (callback) {
                callback("方法未找到", "");
            }
            break;
    }
    return "";
}

void MyCustomModule::myMethod(const std::vector<std::string>& args,
                             std::function<void(const std::string&, const std::string&)> callback) {
    // 实现具体逻辑
    std::string result = "Hello from Native!";
    
    if (callback) {
        callback("", "\"" + result + "\"");
    }
}
```

### 3. JavaScript 端使用

```javascript
// 注册模块
const nativeModules = require('./js/NativeModules');
nativeModules.registerModule('MyCustomModule', {
    constants: {
        VERSION: '1.0.0',
        PLATFORM: 'example'
    },
    methods: ['myMethod']
});

// 使用模块
const MyCustomModule = nativeModules.getModule('MyCustomModule');

async function useCustomModule() {
    try {
        const result = await MyCustomModule.myMethod();
        console.log('结果:', result);
    } catch (error) {
        console.error('错误:', error);
    }
}
```

## 平台特定实现

### iOS 实现

```objc
// iOS 特定代码
@interface MyCustomModuleHelper : NSObject
+ (NSString*)getPlatformSpecificInfo;
@end

@implementation MyCustomModuleHelper
+ (NSString*)getPlatformSpecificInfo {
    return [[UIDevice currentDevice] name];
}
@end

// 在 C++ 中调用
std::string MyCustomModule::getPlatformInfo() {
    return [[MyCustomModuleHelper getPlatformSpecificInfo] UTF8String];
}
```

### Android 实现

```cpp
// Android JNI 调用
std::string MyCustomModule::getPlatformInfo() {
    if (!m_env || !m_context) return "";
    
    jclass helperClass = m_env->FindClass("com/example/MyCustomModuleHelper");
    if (!helperClass) return "";
    
    jmethodID method = m_env->GetStaticMethodID(helperClass, "getPlatformInfo", 
        "(Landroid/content/Context;)Ljava/lang/String;");
    
    if (method) {
        jstring result = (jstring)m_env->CallStaticObjectMethod(helperClass, method, m_context);
        if (result) {
            const char* chars = m_env->GetStringUTFChars(result, nullptr);
            std::string info(chars);
            m_env->ReleaseStringUTFChars(result, chars);
            m_env->DeleteLocalRef(result);
            return info;
        }
    }
    
    m_env->DeleteLocalRef(helperClass);
    return "";
}
```

```java
// MyCustomModuleHelper.java
package com.example;

import android.content.Context;

public class MyCustomModuleHelper {
    public static String getPlatformInfo(Context context) {
        return android.os.Build.MODEL;
    }
}
```

## 事件系统

### 1. 发送事件到 JavaScript

```cpp
// 在 Native 模块中发送事件
void MyCustomModule::sendCustomEvent(const std::string& data) {
    Json::Value eventData;
    eventData["message"] = data;
    eventData["timestamp"] = std::time(nullptr);
    
    Json::StreamWriterBuilder builder;
    std::string jsonData = Json::writeString(builder, eventData);
    
    sendEventToJS("customEvent", jsonData);
}
```

### 2. JavaScript 端监听事件

```javascript
// 监听自定义事件
const { bridge } = require('./js/Bridge');

bridge.addEventListener('customEvent', (data) => {
    console.log('收到自定义事件:', data);
});
```

## 异步操作最佳实践

### 1. Promise 化异步方法

```javascript
// 模块方法自动 Promise 化
const result = await MyModule.asyncMethod(param1, param2);
```

### 2. 错误处理

```javascript
try {
    const result = await MyModule.riskyMethod();
    console.log('成功:', result);
} catch (error) {
    console.error('失败:', error.message);
}
```

### 3. 超时处理

```javascript
function callWithTimeout(promise, timeout = 5000) {
    return Promise.race([
        promise,
        new Promise((_, reject) =>
            setTimeout(() => reject(new Error('操作超时')), timeout)
        )
    ]);
}

// 使用
try {
    const result = await callWithTimeout(MyModule.slowMethod());
} catch (error) {
    console.error('调用失败或超时:', error.message);
}
```

## 性能优化

### 1. 批量操作

```javascript
// 避免
for (const item of items) {
    await MyModule.processItem(item);
}

// 推荐
const promises = items.map(item => MyModule.processItem(item));
const results = await Promise.all(promises);
```

### 2. 缓存策略

```javascript
class CachedModule {
    constructor(module) {
        this.module = module;
        this.cache = new Map();
    }
    
    async getWithCache(key) {
        if (this.cache.has(key)) {
            return this.cache.get(key);
        }
        
        const result = await this.module.expensiveMethod(key);
        this.cache.set(key, result);
        return result;
    }
}
```

### 3. 内存管理

```cpp
// C++ 中及时清理资源
class MyModule : public bridge::NativeModule {
public:
    ~MyModule() {
        // 清理资源
        cleanup();
    }
    
private:
    void cleanup() {
        // 释放持有的资源
    }
};
```

## 调试和测试

### 1. 调试技巧

```javascript
// 启用详细日志
const DEBUG = true;

function debugLog(...args) {
    if (DEBUG) {
        console.log('[DEBUG]', ...args);
    }
}

// 在调用前后添加日志
async function callWithLogging(module, method, ...args) {
    debugLog(`调用 ${module}.${method}`, args);
    const startTime = Date.now();
    
    try {
        const result = await module[method](...args);
        const duration = Date.now() - startTime;
        debugLog(`${module}.${method} 成功 (${duration}ms)`, result);
        return result;
    } catch (error) {
        const duration = Date.now() - startTime;
        debugLog(`${module}.${method} 失败 (${duration}ms)`, error);
        throw error;
    }
}
```

### 2. 单元测试

```javascript
// test/module.test.js
const assert = require('assert');
const { NativeModules } = require('../js/NativeModules');

describe('MyCustomModule', () => {
    let module;
    
    before(() => {
        module = NativeModules.getModule('MyCustomModule');
    });
    
    it('should return correct constants', () => {
        assert.equal(module.VERSION, '1.0.0');
        assert.equal(module.PLATFORM, 'example');
    });
    
    it('should call myMethod successfully', async () => {
        const result = await module.myMethod();
        assert.equal(result, 'Hello from Native!');
    });
});
```

### 3. 集成测试

```javascript
// test/integration.test.js
describe('Bridge Integration', () => {
    it('should handle multiple concurrent calls', async () => {
        const promises = Array(10).fill().map((_, i) =>
            MyModule.method(i)
        );
        
        const results = await Promise.all(promises);
        assert.equal(results.length, 10);
    });
    
    it('should handle errors gracefully', async () => {
        try {
            await MyModule.methodThatFails();
            assert.fail('应该抛出错误');
        } catch (error) {
            assert(error instanceof Error);
        }
    });
});
```

## 部署和分发

### 1. 编译配置

```json
// package.json
{
  "scripts": {
    "build:ios": "xcodebuild -project ios/BridgeDemo.xcodeproj",
    "build:android": "cd android && ./gradlew assembleRelease",
    "test": "jest",
    "lint": "eslint js/ --ext .js"
  }
}
```

### 2. CMake 配置 (Android)

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.4.1)

add_library(
    bridge
    SHARED
    cpp/JSCExecutor.cpp
    cpp/NativeModule.cpp
    android/DeviceInfoModule.cpp
)

target_link_libraries(
    bridge
    jsc
    log
)
```

### 3. Xcode 项目配置 (iOS)

```
// 在 Xcode 中添加文件到项目
- cpp/JSCExecutor.cpp
- cpp/NativeModule.cpp  
- ios/DeviceInfoModule.mm

// 链接框架
- JavaScriptCore.framework
- Foundation.framework
- UIKit.framework
```

## 常见问题和解决方案

### 1. 内存泄漏

**问题**: 回调函数未被及时清理
**解决**: 在模块销毁时清理所有回调

```cpp
void MyModule::destroy() {
    // 清理所有待执行的回调
    m_callbacks.clear();
    NativeModule::destroy();
}
```

### 2. 线程安全

**问题**: 多线程访问共享资源
**解决**: 使用适当的同步机制

```cpp
#include <mutex>

class ThreadSafeModule : public bridge::NativeModule {
private:
    std::mutex m_mutex;
    std::map<std::string, std::string> m_data;
    
public:
    void setData(const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_data[key] = value;
    }
};
```

### 3. JavaScript 异常

**问题**: JavaScript 代码执行出错
**解决**: 添加异常捕获和错误报告

```javascript
function safeCall(fn, ...args) {
    try {
        return fn(...args);
    } catch (error) {
        console.error('JavaScript 执行错误:', error);
        // 发送错误报告
        ErrorReporter.report(error);
        return null;
    }
}
```

这个开发指南提供了创建自定义 Native 模块的完整流程，包括最佳实践、性能优化和常见问题的解决方案。通过遵循这些指导原则，你可以构建高质量、高性能的 React Native Bridge 模块。
