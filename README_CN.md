# Mini React Native

> **中文文档 | [English](README.md)**

[![License](https://img.shields.io/badge/license-Not%20Yet%20Specified-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20iOS%20%7C%20Android-lightgrey.svg)]()
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

**首个从零实现 React Native 的完整教学项目。**

这是一个 React Native 最小实现项目，深入探索从传统 Bridge 架构到现代 JSI 架构的核心机制。为想要真正理解跨平台技术内幕的开发者提供帮助。

---

## 为什么做这个项目

虽然 React Native 被广泛使用，但在深入理解其内部实现方面存在明显的资源空白：

- **官方文档**专注于使用方法，而非实现细节
- **现有教程**很少深入到源码级别的解析
- **React Native 源码**复杂度高，学习曲线陡峭（JS 开发者对 C++、OC、Java 不熟悉导致路线更加陡峭）

**本项目填补了这一空白**，通过完整的从零实现，配合每一步的教学文档。

## 核心特色

✅ **渐进式学习** - 4 个阶段，从 Bridge 到 JSI，每个阶段都可独立运行  
✅ **真实实现** - 真实的 JavaScriptCore 集成，兼容 RN 设计，非模拟代码  
✅ **完整文档** - 详细的架构分析和实现说明，理解每个设计决策

---

## 快速开始

### 环境要求

**系统要求**：

- macOS 10.15+
- Xcode Command Line Tools（内置了 make 和 clang）
- CMake 3.15+（brew install cmake）
- 支持 C++17 的编译器

### 构建运行

```bash
# 克隆项目
git clone https://github.com/yourusername/mini-react-native.git
cd mini-react-native

# 构建并运行测试
make test-deviceinfo

# 预期输出：
# ✓ 模块注册成功
# ✓ getSystemVersion: macOS 14.0
# ✓ getModel: MacBookPro18,1
```

### 项目结构

```
mini-react-native/
├── src/common/          # 跨平台核心代码
│   ├── bridge/          # JSCExecutor - JS 引擎集成
│   ├── modules/         # 模块注册和管理
│   └── utils/           # JSON 序列化等工具
├── src/js/              # JavaScript 端实现
│   ├── MessageQueue.js  # RN 兼容消息队列
│   └── NativeModule.js  # 模块代理
├── src/macos/           # 平台特定实现
└── examples/            # 测试用例
```

### 常见问题

**问题**：`JavaScriptCore framework not found`
```bash
# 解决方案：确保已安装 Xcode Command Line Tools
xcode-select --install
```

**问题**：`CMake version too old`
```bash
# 解决方案：通过 Homebrew 安装最新 CMake
brew install cmake
```

**问题**：C++ 标准相关的构建错误
```bash
# 解决方案：确保编译器支持 C++17
clang++ --version  # 应该是 10.0+
```

---

## 技术路线图

### 阶段 1：Bridge 通信 ✅

**目标**：完成 JS ↔ Native 双向消息传递

- [x] 真实 JavaScriptCore 的 JSCExecutor
- [x] RN 兼容的 MessageQueue
- [x] Native 函数注入
- [x] 类型转换系统
- [x] 完整的模块系统

**学习价值**：理解所有 RN 通信的基础

### 阶段 2：JavaScript 引擎深度集成（3-4 周）

**目标**：优化 JS 引擎集成 + 跨平台支持

- [ ] iOS 支持
- [ ] 内存管理优化
- [ ] Chrome DevTools 集成
- [ ] Android 平台支持
- [ ] 热重载基础

**学习价值**：掌握 JS 引擎内部机制和多平台架构

### 阶段 3：视图渲染系统（4-5 周）

**目标**：实现 React 组件 → 原生视图渲染

- [ ] Shadow Tree（虚拟 DOM）实现
- [ ] Yoga 布局引擎集成
- [ ] 基础组件（View、Text、Image）
- [ ] 事件系统（触摸、手势）
- [ ] Diff 算法和增量更新

**学习价值**：理解 React 如何渲染到原生 UI

### 阶段 4：新架构迁移（3-4 周）

**目标**：实现 JSI + TurboModules + Fabric

- [ ] JSI 同步调用
- [ ] TurboModules 懒加载
- [ ] C++ 对象直接暴露给 JS
- [ ] 性能对比：Bridge vs JSI
- [ ] 迁移指南

**学习价值**：体验 React Native 的未来

---

**⭐ 觉得有帮助？给个 Star 支持一下！**

你的支持是我继续前进的动力~
