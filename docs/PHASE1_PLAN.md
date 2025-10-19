# 阶段1：Bridge 通信机制 - 详细实施计划

## 🎯 阶段目标

实现完整的 JavaScript ↔ Native 双向通信机制，这是理解 React Native 核心原理的基础环节。

**成功标准：**

- ✅ JavaScript 可以调用 Native 方法并获得正确返回值
- ✅ Native 可以向 JavaScript 发送事件和数据
- ✅ 错误处理机制正常工作
- ✅ 通信性能达到可接受水平
- ✅ 建立真实的 JavaScriptCore 执行环境和通信机制

## 📋 详细任务分解

### 任务1: 构建 RN 风格的 JSCExecutor

**目标：** 按照 React Native JSCExecutor 的设计思路，构建兼容的 JavaScript 执行环境

**架构约束：**

- 严格遵循 RN JSCExecutor 的接口设计和生命周期管理
- JavaScript 上下文管理方式与 RN 保持一致
- Native 函数注入机制模仿 RN 的实现模式

#### 子任务 1.1: 项目基础架构搭建 (0.5天)

- [x] **跨平台构建系统设置**
  - 配置 CMake 构建脚本支持 macOS (优先)
  - 建立统一的头文件和库文件管理
  - 创建基础目录结构

- [x] **基础工程文件创建**
  - 创建 macOS 项目文件
  - 配置编译选项和依赖管理
  - 建立测试框架基础

#### 子任务 1.2: JavaScriptCore 环境配置 (1天)

- [x] **macOS JSC 环境配置** (优先实现)
  - 链接系统 JavaScriptCore.framework
  - 创建跨平台的 JSC 接口抽象层
  - 验证 JSC API 可用性

- [x] **基础验证**
  - 编写简单的 "Hello World" JS 执行测试
  - 确保构建系统正常工作

#### 子任务 1.3: JSCExecutor 核心实现 (1天)

- [x] **JSCExecutor 类设计 (参照 RN 接口)**

  ```cpp
  // 参照 React Native JSCExecutor 设计
  class JSCExecutor {
  public:
      void loadApplicationScript(const std::string& script, const std::string& sourceURL);
      void setJSExceptionHandler(std::function<void(const std::string&)> handler);
      void callJSFunction(const std::string& module, const std::string& method, const std::string& arguments);
      void installGlobalFunction(const std::string& name, JSObjectCallAsFunctionCallback callback);
      void destroy();
  };
  ```

- [x] **JavaScript 上下文管理 (RN 风格)**
  - 按照 RN 的方式创建和配置 JSContext
  - 实现 RN 风格的全局对象注入 (global, **DEV**, console 等)
  - 异常处理机制与 RN 保持一致

- [x] **数据类型转换 (RN 兼容)**
  - 实现与 RN 一致的 JSValue ↔ JSON 转换
  - 支持 RN 标准的参数序列化格式

#### 子任务 1.4: RN 标准 Native 函数注入 (0.5天)

- [x] **RN Bridge 关键函数注入**
  - 实现 `nativeFlushQueueImmediate(moduleIds, methodIds, params, callbacks)`
  - 实现 `__nativeCallSyncHook(moduleId, methodId, args)`
  - 实现 `__nativeLoggingHook(level, message)`
  - 确保函数签名与 RN 完全一致

- [x] **RN 风格异常处理**
  - 实现 RN 标准的 JavaScript 异常处理机制
  - 错误信息格式与 RN 保持一致

### 任务2: RN 兼容的 MessageQueue 实现

**目标：** 严格按照 React Native MessageQueue.js 的设计实现兼容的消息队列机制

**架构约束：**

- 消息队列格式必须与 RN 完全一致：`[moduleIds, methodIds, params, callbackIds]`
- 回调管理机制遵循 RN 的实现逻辑
- 模块注册和方法映射方式与 RN 保持一致

#### 子任务 2.1: RN 标准 MessageQueue 实现 (1.5天)

- [x] **MessageQueue 类 (严格按照 RN 设计)**

  ```javascript
  // 完全参照 React Native MessageQueue.js
  class MessageQueue {
    constructor()                    // 初始化队列和回调表
    registerLazyCallableModule(name, factory) // 注册延迟加载模块
    enqueueNativeCall(moduleID, methodID, params, onFail, onSucc) // JavaScript → Native 调用
    callFunctionReturnFlushedQueue(module, method, args)       // Native → JavaScript 调用: 执行 JS 模块中的方法
    invokeCallbackAndReturnFlushedQueue(cbID, args) // Native → JavaScript 调用: 执行回调并返回队列
    flushedQueue()                  // 获取并清空待处理队列
    getEventLoopRunningTime()       // 获取事件循环运行时间
  }
  ```

- [x] **RN 标准消息队列格式**
  - 严格实现 `[moduleIds, methodIds, params, callbackIds]` 格式
  - 回调ID生成和管理与 RN 完全一致
  - 模块注册表结构与 RN 保持一致

#### 子任务 2.2: RN 标准 Bridge 集成 (1天)

- [x] **RN Bridge 函数完整实现**
  - 完整实现 `nativeFlushQueueImmediate(moduleIds, methodIds, params, callbacks)`
  - 消息格式处理与 RN 完全一致
  - 支持 RN 标准的批量调用机制

- [x] **RN 兼容回调机制**
  - 实现与 RN 一致的回调ID管理
  - 支持成功/失败双回调模式
  - 回调执行时机与 RN 保持一致

#### 子任务 2.3: 基础调试支持 (0.5天)

- [x] **简单的调试功能**
  - 基础的调用日志
  - 错误信息的格式化输出

### 任务3: RN 兼容的 Native 模块系统

**目标：** 严格按照 React Native 的 NativeModule 机制实现兼容的模块系统

**架构约束：**

- 模块注册方式必须与 RN 的 NativeModule 机制一致
- 方法导出和调用方式遵循 RN 的模式 (反射机制)
- 事件系统按照 RN 的 RCTEventEmitter 思路实现

#### 子任务 3.1: RN 标准模块框架 (1天)

- [ ] **NativeModule 基类设计 (参照 RN 接口)**

  ```cpp
  // 严格参照 React Native NativeModule 设计
  class NativeModule {
  public:
      virtual std::string getName() const = 0;
      virtual std::vector<std::string> getMethods() const = 0;
      virtual std::map<std::string, std::string> getConstants() const = 0;
      virtual void invoke(const std::string& methodName, const std::string& args,
                         std::function<void(const std::string&)> callback) = 0;
      virtual bool supportsWebWorkers() const { return false; }
      virtual ~NativeModule() = default;
  };
  ```

- [ ] **RN 兼容模块注册机制**
  - 模块ID分配方式与 RN 一致
  - 方法ID映射表符合 RN 标准
  - 支持 RN 风格的模块配置导出

#### 子任务 3.2: RN 风格 DeviceInfo 模块 (1.5天)

- [ ] **DeviceInfoModule 类 (参照 RN DeviceInfo)**
  - 继承 NativeModule 基类
  - 实现 RN 标准的模块配置导出
  - 方法调用方式与 RN DeviceInfo 保持一致

- [ ] **RN 兼容的设备信息方法 (macOS 优先)**

  ```cpp
  // 参照 react-native-device-info 的接口
  void getUniqueId(std::function<void(const std::string&)> callback);
  void getSystemVersion(std::function<void(const std::string&)> callback);
  void getModel(std::function<void(const std::string&)> callback);
  std::map<std::string, std::string> getConstants(); // 返回设备常量
  ```

#### 子任务 3.3: RN 风格事件系统 (0.5天)

- [ ] **RCTEventEmitter 风格事件推送**
  - 实现与 RN RCTEventEmitter 一致的事件分发机制
  - 支持 RN 标准的事件订阅/取消订阅
  - 事件数据格式与 RN 保持一致

### 任务4: 基础集成测试

**目标：** 验证基础通信功能正常工作

#### 子任务 4.1: 端到端通信验证 + iOS支持 (1.5天)

- [ ] **基础功能测试** (0.5天)
  - JavaScript 调用 DeviceInfo 模块方法
  - 验证返回值的正确性
  - 测试基础错误处理

- [ ] **iOS 平台适配** (0.5天)
  - 配置 iOS 构建环境
  - 验证 JSCExecutor 在 iOS 上的运行
  - 测试 Bridge 通信在 iOS 平台的兼容性

- [ ] **事件系统测试** (0.25天)
  - 触发一个 Native 事件
  - 验证 JavaScript 端接收
  - 确保数据传递正确

- [ ] **简单性能验证** (0.25天)
  - 测量基础调用延迟 (目标 < 10ms)
  - 验证内存使用基本稳定

## 🔧 技术要点和难点

### 关键技术挑战

1. **JavaScriptCore 集成复杂性**
   - 不同平台的 JSC 版本差异
   - 内存管理和生命周期控制
   - 多线程环境下的安全性

2. **数据转换的性能开销**
   - JSON 序列化/反序列化的优化
   - 大数据量传输的处理
   - 类型安全的保证

3. **异步调用的复杂性**
   - 回调 ID 的管理和清理
   - 异常情况下的状态恢复
   - 线程安全的保证

4. **跨平台兼容性**
   - iOS 和 Android 的 API 差异
   - 构建系统的统一管理
   - 调试工具的平台适配

### 最佳实践

1. **错误处理策略**
   - 分层的错误处理机制
   - 详细的错误日志记录
   - 优雅的降级处理

2. **性能优化原则**
   - 减少不必要的数据拷贝
   - 批量处理提高效率
   - 智能的缓存策略

3. **代码组织结构**
   - 清晰的模块划分
   - 统一的接口设计
   - 完善的文档注释

## 📝 验收标准

### RN 兼容性验收 (新增)

- [ ] **消息格式兼容**: 消息队列格式与 RN 完全一致 `[moduleIds, methodIds, params, callbackIds]`
- [ ] **模块接口兼容**: DeviceInfo 模块的接口与 react-native-device-info 保持一致
- [ ] **JavaScript 接口兼容**: 能够运行标准的 RN Bridge 调用代码
- [ ] **事件系统兼容**: 事件的注册、监听、分发行为与 RN 一致
- [ ] **回调机制兼容**: 成功/失败回调的处理方式与 RN 保持一致
- [ ] **异常处理兼容**: JavaScript 异常处理机制与 RN 保持一致

### 功能验收

- [ ] JavaScript 可以成功调用所有 DeviceInfo 模块方法
- [ ] 所有方法都能返回正确的数据格式
- [ ] 设备信息变化事件可以正确推送到 JavaScript
- [ ] 错误情况得到正确处理和提示

### 性能验收

- [ ] 单次调用延迟 < 10ms (在现代设备上)
- [ ] 支持 100+ 并发调用而不出现错误
- [ ] 内存使用稳定，无明显泄漏
- [ ] CPU 使用率在合理范围内

### 质量验收

- [ ] 代码通过静态分析检查
- [ ] 所有关键路径都有日志记录
- [ ] 异常情况都有对应的测试用例
- [ ] 文档完整且准确

## 🎯 输出成果

### 1. 代码成果

- 完整可运行的 Bridge 通信实现
- 真实的 iOS 和 Android Native 模块
- 完善的测试用例和验证脚本

### 2. 文档成果

- 详细的实现文档和设计说明
- 性能测试报告和分析
- 与 React Native 的对比分析

### 3. 博客文章

- **《从零实现 React Native (1): Bridge 通信原理与实现》**
  - Bridge 架构的深度解析
  - MessageQueue 机制的实现细节
  - 性能优化的思路和方法
  - 与官方实现的对比分析

## ⏱️ 时间规划 (MVP 版本)

| 任务 | 原计划 | MVP调整后 | 主要变化 | 依赖关系 |
|------|---------|-----------|----------|----------|
| ✅ 任务1: 从零构建 JSCExecutor | 7-10 天 | **3天** | 最小可行实现 + AI协助 | 无 |
| 任务2: MessageQueue 最小实现 | 4-6 天 | **3天** | 简化架构，专注核心通信 | 依赖任务1 |
| 任务3: 最小 Native 模块系统 | 5-7 天 | **3天** | 一个 DeviceInfo 示例模块 | 依赖任务1 |
| 任务4: 基础集成测试 + iOS支持 | 3-4 天 | **1.5天** | 基础功能验证 + iOS平台适配 | 依赖任务2,3 |

**MVP总计: 10.5天** (相比原计划的 19-27天，缩短 45-61%)

### 📋 MVP 执行建议

**Week 1: 快速原型** (3-4天)

- JSCExecutor 基础实现 (macOS环境)
- 最简 MessageQueue 实现
- 建立第一个 JS ↔ C++ 调用

**Week 2: 核心功能** (4-5天)

- 完善 MessageQueue 通信机制
- DeviceInfo 模块实现 (macOS平台)
- Native 到 JS 事件推送

**Week 2: 验证测试** (1.5-2天)

- 端到端集成测试
- iOS 平台适配验证
- 基础性能验证
- 文档和演示准备

### 🎯 MVP 成功标准

- [x] **核心通信**: JS 能调用 Native 方法并获得返回值
- [x] **事件系统**: Native 能向 JS 发送事件
- [x] **示例模块**: 至少一个完整的 DeviceInfo 模块
- [x] **基础性能**: 调用延迟 < 10ms
- [x] **稳定性**: 基础错误处理，无明显内存泄漏

## 🚀 下一步计划

完成阶段1后，将进入阶段2（JavaScript 引擎深度集成），重点关注：

- JavaScript 引擎的性能优化
- 调试工具的集成
- Android 平台支持 (2-3天)
- 热重载机制的实现
- 错误处理的完善

## 📱 跨平台支持规划

### ✅ macOS 支持 (已完成)
- 完整实现并测试通过
- 使用系统 JavaScriptCore.framework
- 作为主要开发和验证平台

### 🍎 iOS 支持 (任务4期间)
- **实施时间**: 任务4期间 (0.5天)
- **技术复杂度**: 低
- **主要工作**:
  - 构建配置调整
  - 平台特定代码路径验证
  - 共享 JavaScriptCore.framework

### 🤖 Android 支持 (阶段2期间)
- **实施时间**: 阶段2 (2-3天)
- **技术复杂度**: 中高
- **主要工作**:
  - JNI 集成
  - JSC 移植版本适配
  - 构建系统完善
  - 平台特定优化

这将为后续的视图渲染系统打下坚实的基础。
