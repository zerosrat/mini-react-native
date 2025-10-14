# Mini React Native 学习笔记

## 📚 核心概念解析

### JSCExecutor - JavaScript 执行器

**JSCExecutor** 是 React Native 中负责执行 JavaScript 代码的核心组件。它在整个 React Native 架构中扮演着至关重要的角色。

#### 主要职责

1. **JavaScript 环境管理** - 创建和维护 JavaScript 执行上下文
   - 初始化 JavaScriptCore 环境
   - 管理 JavaScript 全局对象和作用域
   - 控制 JavaScript 执行环境的生命周期

2. **代码执行** - 执行 JavaScript 代码（应用代码、框架代码）
   - 加载和执行应用的 JavaScript 代码
   - 执行 React Native 框架代码
   - 处理异步代码执行和调度

3. **Bridge 函数注入** - 向 JavaScript 环境注入 Native 通信函数
   - 注入关键的 Bridge 通信函数（如 `__nativeFlushQueuedReactWork`）
   - 提供 JavaScript 调用 Native 的接口
   - 建立双向通信的桥梁

4. **异常处理** - 捕获和处理 JavaScript 执行错误
   - 捕获 JavaScript 运行时错误
   - 格式化错误信息并传递给 Native
   - 提供错误恢复和调试支持

#### 在 React Native 架构中的位置

```
┌─────────────────────────────────────┐
│           JavaScript                │
│  ┌─────────────┐  ┌─────────────┐   │
│  │ App Logic   │  │ MessageQueue│   │
│  └─────────────┘  └─────────────┘   │
└─────────────┬───────────────────────┘
              │
    ┌─────────▼────────┐
    │   JSCExecutor    │  ◄── 我们要实现的核心组件
    └─────────┬────────┘
              │
┌─────────────▼───────────────────────┐
│           Native (C++)              │
│  ┌─────────────┐  ┌─────────────┐   │
│  │NativeModule │  │Platform APIs│   │
│  └─────────────┘  └─────────────┘   │
└─────────────────────────────────────┘
```

## 🎯 实现目标

我们的目标是实现一个简化但功能完整的 JSCExecutor，它能够：
- 创建真实的 JavaScript 执行环境
- 执行 JavaScript 代码
- 支持 JavaScript 调用 Native 函数
- 处理基本的错误情况

## 📝 学习进度记录

### 阶段1：Bridge 通信机制

#### ✅ 第一步：项目基础架构搭建 (已完成)

**完成的工作：**

1. **项目目录结构设计**
   ```
   mini-react-native/
   ├── src/bridge/          # Bridge 通信核心
   │   ├── JSCExecutor.h    # JavaScript 执行器头文件
   │   └── JSCExecutor.cpp  # JavaScript 执行器实现
   ├── examples/            # 测试和示例
   │   └── test_basic.cpp   # 基础功能验证程序
   ├── docs/                # 文档和学习笔记
   ├── CMakeLists.txt       # CMake 构建配置
   ├── Makefile            # 简化的构建脚本
   └── .gitignore          # Git 忽略配置
   ```

2. **JSCExecutor 类设计完成**
   - 遵循 React Native JSCExecutor 的接口设计
   - 实现了核心的生命周期管理方法
   - 包含完整的 JavaScriptCore 集成逻辑
   - 详细的注释和文档说明

3. **构建系统配置**
   - CMake 配置支持 macOS JavaScriptCore.framework
   - Makefile 提供便捷的开发命令
   - 跨平台支持框架（当前专注 macOS）

4. **基础测试程序**
   - 验证 JavaScript 环境创建
   - 测试基本的代码执行功能
   - 验证 Bridge 函数注入
   - 包含错误处理测试

**关键技术点学习：**

- **JavaScriptCore API 使用**: 学会了如何创建 JavaScript 上下文、执行代码、处理异常
- **C++ 与 JavaScript 互操作**: 理解了类型转换、函数注入、回调机制
- **React Native Bridge 设计模式**: 掌握了 Bridge 函数的命名规范和调用约定
- **跨平台构建配置**: 学会了 CMake 的框架链接和平台特定配置

---

*备注：本文档会随着实现过程不断更新，记录重要的技术决策和学习心得*