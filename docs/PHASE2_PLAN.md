# 阶段2：跨平台支持 - 详细实施计划

## 🎯 阶段目标

实现完整的跨平台支持（macOS + iOS + Android），建立统一的构建和测试流程，这是 Mini React Native 走向完整性的关键步骤。

**成功标准：**

- ✅ iOS 平台完整支持，与 macOS 共享 JavaScriptCore.framework
- ✅ Android 平台完整支持，通过 JNI 集成 JavaScriptCore
- ✅ 三平台统一的构建、测试和验证流程
- ✅ DeviceInfo 模块在所有平台正常工作
- ✅ 性能保持在可接受水平（< 10ms 调用延迟）

## 📋 详细任务分解

### 任务1: iOS 平台支持完成

**目标：** 完成 iOS 平台的完整适配，利用与 macOS 共享的 JavaScriptCore.framework

**架构约束：**

- 复用现有的 JSCExecutor 实现，最小化平台差异
- 利用 iOS 和 macOS 共享的 JavaScriptCore.framework
- 保持与 macOS 版本的 API 一致性

#### 子任务 1.1: iOS 构建系统配置 (0.25天)

- [ ] **Xcode 项目配置**
  - 创建 iOS target 配置
  - 配置 JavaScriptCore.framework 链接
  - 设置最低 iOS 版本支持（iOS 12.0+）

- [ ] **CMake 构建适配**
  - 添加 iOS 平台检测逻辑
  - 配置 iOS 特定的编译选项
  - 确保与 macOS 构建的一致性

#### 子任务 1.2: iOS 平台验证 (0.25天)

- [ ] **基础功能验证**
  - JSCExecutor 在 iOS 上的初始化验证
  - Bridge 通信功能测试
  - DeviceInfo 模块在 iOS 上的运行验证

- [ ] **iOS 特定 API 实现**
  - iOS 版本的设备信息获取
  - 确保 DeviceInfo 返回正确的 iOS 设备数据

### 任务2: Android 平台集成

**目标：** 实现 Android 平台的完整支持，通过 JNI 集成 JavaScriptCore

**架构约束：**

- 使用 JSC 的移植版本（如 facebook/android-jsc）
- 通过 JNI 桥接 C++ 和 Java/Kotlin 代码
- 保持与 iOS/macOS 的 API 一致性

#### 子任务 2.1: Android 构建系统搭建 (1.5天)

- [ ] **Gradle 项目结构**
  - 创建 Android 项目基础结构
  - 配置 CMake 与 Gradle 的集成
  - 设置 NDK 构建配置

- [ ] **JavaScriptCore 移植版本集成**
  - 集成 facebook/android-jsc 或类似的 JSC 移植版本
  - 配置 JSC 库的链接和依赖
  - 验证 JSC API 在 Android 上的可用性

- [ ] **JNI 桥接层设计**
  ```cpp
  // JNI 接口设计
  extern "C" {
      JNIEXPORT void JNICALL Java_com_minirn_JSCExecutor_initializeContext(JNIEnv* env, jobject obj);
      JNIEXPORT void JNICALL Java_com_minirn_JSCExecutor_loadScript(JNIEnv* env, jobject obj, jstring script);
      JNIEXPORT void JNICALL Java_com_minirn_JSCExecutor_destroyContext(JNIEnv* env, jobject obj);
  }
  ```

#### 子任务 2.2: Android Native 层实现 (1天)

- [ ] **JSCExecutor Android 适配**
  - 适配现有的 JSCExecutor 到 Android 环境
  - 处理 Android 特定的内存管理
  - 确保线程安全（Android 主线程 vs Native 线程）

- [ ] **Android DeviceInfo 模块**
  - 实现 Android 版本的设备信息获取
  - 通过 JNI 调用 Android API
  - 返回正确的 Android 设备数据

#### 子任务 2.3: Android Java 层实现 (0.5天)

- [ ] **Java/Kotlin 封装层**
  - 创建 JSCExecutor 的 Java 封装类
  - 实现 DeviceInfo 的 Android API 调用
  - 建立 Java 到 Native 的调用桥梁

- [ ] **Android 应用示例**
  - 创建简单的 Android 应用示例
  - 集成 Mini React Native 库
  - 验证基础功能正常工作

### 任务3: 跨平台测试与验证

**目标：** 建立统一的跨平台测试流程，确保三平台的一致性

#### 子任务 3.1: 统一测试套件 (0.5天)

- [ ] **跨平台测试脚本**
  - 创建统一的测试脚本，支持三平台
  - 自动化构建和测试流程
  - 集成到 Makefile 中

- [ ] **一致性验证**
  - 确保 DeviceInfo 在三平台返回正确格式的数据
  - 验证 Bridge 通信的性能一致性
  - 测试错误处理的平台一致性

#### 子任务 3.2: 文档和示例完善 (0.5天)

- [ ] **跨平台构建文档**
  - 详细的三平台构建指南
  - 依赖安装和环境配置说明
  - 常见问题和解决方案

- [ ] **平台差异说明**
  - 记录三平台的技术差异
  - API 调用的平台特定实现
  - 性能特征对比

## 🔧 技术要点和难点

### Android 平台的主要挑战

1. **JNI 集成复杂性**
   - C++ 和 Java 之间的数据类型转换
   - 内存管理和生命周期控制
   - 异常处理的跨语言传递

2. **JavaScriptCore 移植版本**
   - 不同 JSC 移植版本的 API 差异
   - 性能和稳定性的权衡
   - 版本兼容性管理

3. **Android 构建系统**
   - CMake 和 Gradle 的集成配置
   - NDK 版本兼容性
   - 多架构支持（arm64-v8a, armeabi-v7a）

### iOS 平台的技术要点

1. **框架共享优势**
   - 与 macOS 共享 JavaScriptCore.framework
   - 代码复用率高，适配工作量小
   - API 一致性好

2. **平台特定优化**
   - iOS 内存限制的考虑
   - 后台执行的限制
   - App Store 审核要求

### 跨平台一致性保证

1. **API 抽象层设计**
   - 统一的 NativeModule 接口
   - 平台特定实现的封装
   - 错误处理的标准化

2. **测试策略**
   - 自动化的跨平台测试
   - 性能基准的平台对比
   - 功能一致性验证

## 📝 验收标准

### 功能验收

- [ ] **iOS 平台验收**
  - JSCExecutor 在 iOS 上正常初始化和运行
  - DeviceInfo 模块返回正确的 iOS 设备信息
  - Bridge 通信性能 < 10ms

- [ ] **Android 平台验收**
  - 通过 JNI 成功集成 JavaScriptCore
  - DeviceInfo 模块返回正确的 Android 设备信息
  - 构建系统支持多架构（arm64-v8a, armeabi-v7a）

- [ ] **跨平台一致性验收**
  - 同一份 JavaScript 代码在三平台运行结果一致
  - DeviceInfo API 在三平台返回相同格式的数据
  - 错误处理行为在三平台保持一致

### 性能验收

- [ ] **基础性能指标**
  - 单次调用延迟 < 10ms（所有平台）
  - 内存使用稳定，无明显泄漏
  - 应用启动时间在可接受范围内

- [ ] **跨平台性能对比**
  - 记录三平台的性能基准数据
  - 分析平台间的性能差异
  - 确保没有明显的性能退化

### 质量验收

- [ ] **代码质量**
  - 通过静态分析检查（所有平台）
  - 代码覆盖率达到基本要求
  - 文档完整且准确

- [ ] **构建系统**
  - 三平台的自动化构建成功
  - 统一的测试脚本正常运行
  - CI/CD 流程可以正常执行

## 🎯 输出成果

### 1. 代码成果

- **完整的三平台支持**
  - macOS（已完成）+ iOS + Android 的完整实现
  - 统一的 API 接口和一致的行为
  - 跨平台的构建和测试系统

- **平台特定实现**
  - iOS 的 JavaScriptCore.framework 集成
  - Android 的 JNI + JSC 移植版本集成
  - 各平台的 DeviceInfo 模块实现

### 2. 文档成果

- **跨平台构建指南**
  - 详细的环境配置说明
  - 三平台的构建步骤
  - 常见问题和解决方案

- **技术架构文档**
  - 跨平台架构设计说明
  - 平台差异和处理策略
  - 性能对比和分析报告

### 3. 博客文章

- **《从零实现 React Native (2): 跨平台架构实现》**
  - 跨平台技术挑战和解决方案
  - iOS 和 Android 平台的适配经验
  - JavaScriptCore 在不同平台的集成方式
  - 构建系统的统一设计思路

## ⏱️ 时间规划

| 任务 | 预估时间 | 主要工作内容 | 依赖关系 |
|------|---------|-------------|----------|
| 任务1: iOS 平台支持 | **0.5天** | Xcode 配置 + 功能验证 | 无 |
| 任务2: Android 平台集成 | **3天** | JNI + JSC 移植 + 构建系统 | 无 |
| 任务3: 跨平台测试验证 | **1天** | 统一测试 + 文档完善 | 依赖任务1,2 |

**总计: 4.5天** (相比原 ROADMAP 的 3-4周，大幅简化和聚焦)

### 📋 执行建议

**Day 1: iOS 快速适配**
- 上午：iOS 构建系统配置
- 下午：功能验证和测试

**Day 2-4: Android 深度集成**
- Day 2：构建系统和 JSC 集成
- Day 3：JNI 桥接和 Native 层实现
- Day 4：Java 层封装和应用示例

**Day 5: 跨平台验证**
- 上午：统一测试套件运行
- 下午：文档完善和成果整理

## 🚀 下一步计划

完成阶段2后，将进入阶段3（视图渲染系统），重点关注：

- Shadow Tree 虚拟 DOM 机制
- Yoga 布局引擎集成
- 基础组件实现（View, Text, Image）
- 事件系统的完善

这将标志着从基础通信向完整 UI 系统的重要跃进。

## 📱 跨平台支持总结

### ✅ macOS 支持 (Phase 1 已完成)
- 完整实现并测试通过
- 使用系统 JavaScriptCore.framework
- 作为主要开发和验证平台

### 🍎 iOS 支持 (本阶段)
- **实施时间**: 0.5天
- **技术复杂度**: 低
- **主要优势**: 与 macOS 共享 JavaScriptCore.framework

### 🤖 Android 支持 (本阶段)
- **实施时间**: 3天
- **技术复杂度**: 中高
- **主要挑战**: JNI 集成 + JSC 移植版本适配

这将为后续的视图渲染系统提供坚实的跨平台基础。