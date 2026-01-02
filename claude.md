# Mini React Native - Claude 开发指导

## 🎯 项目定位与原则

### 最小可行实现（MVP）指导原则

本项目是一个 **学习型的最小可行实现**，旨在通过构建简化版 React Native 来深度理解其核心原理。

**核心原则：**

1. **理解原理优先** > 完美实现
   - 重点是掌握 Bridge 通信、JS 引擎集成、视图渲染等核心机制
   - 不追求生产级的健壮性和性能优化
   - 代码可读性和教学价值优先于执行效率

2. **最小可行功能** > 功能完整性
   - 实现核心功能的最简版本，能跑通整个流程即可
   - 先建立端到端的通信，再逐步完善细节
   - 避免过度设计和提前优化

3. **快速迭代验证** > 一次性完美
   - 分阶段递增式开发，每个阶段都有可运行的成果
   - 尽快建立反馈循环，验证设计思路
   - 允许后续重构和改进

## 🔧 开发策略

### 质量标准

- **功能性**: 核心功能能正常工作，覆盖主要使用场景
- **稳定性**: 基本的错误处理，避免崩溃，但不追求完美的异常处理
- **性能**: 满足基本可用性即可，不做深度性能优化
- **代码质量**: 清晰易懂，有基础注释，便于学习和理解

### 平台优先级

1. **优先**: macOS (开发和测试便利)
2. **次要**: iOS (移动端验证)
3. **可选**: Android (跨平台完整性)

### 技术栈简化

- **JavaScript 引擎**: 使用系统 JavaScriptCore，避免复杂的引擎配置
- **构建系统**: 简单的 CMake + Makefile，不追求复杂的构建优化
- **测试策略**: 手动测试 + 简单的自动化验证，不追求完整的测试覆盖

## 🏗️ 架构约束

### 基于 React Native 老架构

本项目严格基于 **React Native 传统 Bridge 架构**（RN 0.60 之前的经典架构），不是新的 JSI/TurboModules 架构。

具体的，桥架构版本参考 RN [v0.57.8](https://github.com/facebook/react-native/blob/0.57-stable/Libraries/BatchedBridge/MessageQueue.js)，这个版本只包含 Bridge 代码不包含 JSI 代码，便于学习和参考。

## 关键项目路径

### 核心源码

- `src/common/` - 跨平台核心代码（JSCExecutor、模块系统）
- `src/js/` - JavaScript 实现（MessageQueue、NativeModule 等）
- `src/macos/` - macOS 平台代码
- `src/ios/` - iOS 平台代码
- `src/android/` - Android 平台代码

### 构建输出

- `build/` - macOS 构建目录
- `build_ios/` - iOS 构建目录
- `dist/bundle.js` - JavaScript 打包文件

### 测试和示例
- `examples/` - 测试用例和示例代码

### 关键配置

- `CMakeLists.txt` - CMake 构建配置
- `Makefile` - 构建自动化脚本
- `package.json` - Node.js 依赖
- `rollup.config.js` - JavaScript 打包配置

## 命令

### 构建命令

```bash
make build              # 编译项目（默认包含 JS 构建）
make js-build           # 仅构建 JavaScript bundle
make js-watch           # 监听 JS 文件并自动重建
make clean              # 清理所有构建文件
make js-clean           # 仅清理 JavaScript 构建文件
make rebuild            # 完全清理重建
make configure          # 仅配置 CMake
```

### iOS 构建命令

```bash
make ios-build          # 为 iOS 模拟器构建
make ios-configure      # 仅配置 iOS 构建
```

### macos 测试命令

```bash
make test               # 运行所有测试（基础、模块、集成、性能）
make test-basic         # 仅基础功能测试
make test-module        # 仅模块框架测试
make test-integration   # 仅集成测试
make test-performance   # 仅性能测试
make ios-test           # 运行所有 iOS 测试
make ios-test-deviceinfo # 仅运行 iOS DeviceInfo 测试
```

### iOS 测试

`docs/iOS_TESTING.md`

### 构建系统详情

- 构建类型：基于 CMake（3.15+）
- 编译器：Clang++（C++17 标准）
- 构建目录：`build/`（macOS）、`build_ios/`（iOS）
- 并行构建：自动使用系统 CPU 核心数
- 部署目标：macOS 10.15+、iOS 12.0+

## 关键文档索引

- 路线图：`docs/ROADMAP.md`
- 阶段一规划：`docs/PHASE1_PLAN.md`
- 阶段二规划：`docs/PHASE2_PLAN.md`

## 📝 文档输出

### 期望文档

1. **技术博客**: 深度解析文章
2. **视频演示**: 关键功能的演示视频
3. **学习笔记**: 开发过程中的技术收获

---

**记住：这是一个学习项目，不是生产项目。保持简单，专注核心，快速迭代！**