# iOS 测试指南

本文档描述如何在 iOS 模拟器上测试 Mini React Native 的功能。

## 🍎 iOS 测试脚本

项目提供了一个便捷的 iOS 测试脚本 `test_ios.sh`，可以在 iOS 模拟器中运行各种测试。

### 快速开始

```bash
# 运行所有 iOS 测试
./test_ios.sh all

# 运行特定测试
./test_ios.sh deviceinfo

# 查看帮助
./test_ios.sh help
```

### 使用 Makefile

```bash
# 仅构建 iOS 版本
make ios-sim-build
```

## 📋 可用测试

| 测试类型 | 命令 | 描述 |
|---------|------|------|
| **基础功能** | `./test_ios.sh basic` | 验证 JSCExecutor 基础功能 |
| **DeviceInfo** | `./test_ios.sh deviceinfo` | 测试 iOS 设备信息获取和性能 |
| **模块框架** | `./test_ios.sh module` | 验证模块注册和调用机制 |
| **集成测试** | `./test_ios.sh integration` | 完整 JavaScript ↔ Native 通信 |
| **全部测试** | `./test_ios.sh all` | 运行所有测试 |

## 🔧 环境要求

### 必需软件
- **Xcode** (完整版本，包含 iOS SDK)
- **iOS 模拟器** (通过 Xcode 安装)
- **CMake** 3.15+
- **Node.js** (用于 JavaScript 构建)

### 验证环境
```bash
# 检查 Xcode 和 iOS SDK
xcodebuild -showsdks | grep iOS

# 列出可用的 iOS 模拟器
xcrun simctl list devices available | grep iPhone
```

## 📱 模拟器管理

### 使用默认模拟器
脚本默认使用 `iPhone 16 Pro` 模拟器。

### 使用自定义模拟器
```bash
# 列出可用模拟器
./test_ios.sh list

# 在指定模拟器上运行测试
./test_ios.sh deviceinfo "iPhone 15 Pro"
```

### 手动管理模拟器
```bash
# 启动模拟器
xcrun simctl boot "iPhone 16 Pro"

# 关闭模拟器
xcrun simctl shutdown "iPhone 16 Pro"

# 查看模拟器状态
xcrun simctl list devices
```

## 📊 测试结果解读

### DeviceInfo 测试示例
```
=== iOS DeviceInfo Module Test ===

2. Testing DeviceInfo methods directly...
   UniqueId: F67EDB44-DA12-45E3-800D-800BBB7F1FC6
   SystemVersion: 18.5.0
   DeviceId: x86_64

5. Performance Results:
   Bridge call duration: 17.776 ms
   ⚠️ Performance slower than expected (>= 10ms)
```

### 结果说明
- **UniqueId**: iOS 模拟器的唯一标识符 (每次重新生成)
- **SystemVersion**: iOS 系统版本 (模拟器版本)
- **DeviceId**: 设备架构 (`x86_64` 为模拟器, `arm64` 为真机)
- **Performance**: Bridge 调用性能 (模拟器通常较慢)

## 🔍 性能基准

| 平台 | Bridge 调用耗时 | 状态 |
|------|---------------|------|
| **macOS** | ~2ms | ✅ 优秀 |
| **iOS 模拟器** | ~18ms | ⚠️ 可接受 |
| **iOS 真机** | ~5ms | ✅ 良好 (预期) |

> **注意**: iOS 模拟器性能较慢是正常的，真机性能会明显更好。

## 🐛 常见问题

### 1. 模拟器启动失败
```bash
# 检查可用模拟器
./test_ios.sh list

# 手动启动模拟器
xcrun simctl boot "iPhone 16 Pro"
```

### 2. iOS SDK 不可用
```bash
# 检查 Xcode 安装
xcode-select -p

# 安装 iOS SDK (通过 Xcode)
open /Applications/Xcode.app
```

### 3. 构建失败
```bash
# 清理并重新构建
make clean
make ios-sim-build
```

### 4. JavaScript bundle 错误
```bash
# 重新构建 JavaScript
make js-build

# 或完整重建
make ios-test
```

## 🚀 高级用法

### 自定义测试脚本
```bash
# 创建自定义测试
cat > my_ios_test.sh << 'EOF'
#!/bin/bash
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer

# 启动模拟器
xcrun simctl boot "iPhone 16 Pro"

# 运行自定义测试
xcrun simctl spawn "iPhone 16 Pro" ./build_ios_sim/my_test.app/my_test

# 关闭模拟器
xcrun simctl shutdown "iPhone 16 Pro"
EOF

chmod +x my_ios_test.sh
```

### 性能分析
```bash
# 使用 Instruments 进行性能分析
xcrun instruments -t "Time Profiler" -D trace.trace ./build_ios_sim/test_ios_deviceinfo.app/test_ios_deviceinfo
```

### 批量测试
```bash
# 在多个模拟器上运行测试
for sim in "iPhone 15" "iPhone 16" "iPhone 16 Pro"; do
    echo "Testing on $sim..."
    ./test_ios.sh deviceinfo "$sim"
done
```

## 📝 测试报告

### 生成测试报告
```bash
# 运行测试并保存输出
./test_ios.sh all > ios_test_report.txt 2>&1

# 查看报告
cat ios_test_report.txt
```

### 自动化 CI/CD
```yaml
# GitHub Actions 示例
- name: Run iOS Tests
  run: |
    make ios-sim-build
    ./test_ios.sh all
```

## 🎯 下一步

- 在真实 iOS 设备上测试
- 添加更多设备信息测试
- 实现 iOS 特定功能测试
- 集成性能监控

---

**相关文档:**
- [构建指南](../README.md)
- [Phase 2 计划](PHASE2_PLAN.md)
- [技术路线图](ROADMAP.md)