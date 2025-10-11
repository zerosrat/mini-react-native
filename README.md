# React Native 从零实现 - 教学项目

这是一个从零到一实现 React Native 核心机制的教学项目，深度解析 JavaScript 和 Native 代码之间的通信原理，为理解现代跨平台技术打下坚实基础。

## 🎯 项目目标

- **深度理解**: 透彻掌握 React Native Bridge 和 JSI 架构的工作原理
- **实战教学**: 通过完整的代码实现学习跨平台技术的核心机制
- **技术进阶**: 从传统 Bridge 架构到新 JSI 架构的完整演进
- **社区贡献**: 填补国内 React Native 从零实现教程的空白

## 📋 项目规划

本项目采用分阶段渐进式开发，每个阶段都是完整可运行的实现：

- **🔄 [完整技术路线图](docs/roadmap.md)** - 查看 5 个阶段的详细规划
- **⚡ [阶段1实施计划](docs/phase1-bridge-plan.md)** - 当前阶段的具体任务分解

### 当前状态：阶段1 - Bridge 通信机制
**目标**: 完成 JavaScript ↔ Native 双向通信的真实实现
**进度**: 基础架构已完成，正在实现真实的 JSC 集成

## 🏗️ 架构概览

```
┌─────────────────────────────────────┐
│           JavaScript Layer         │  ← React Native 应用层
│        (React Components)          │
├─────────────────────────────────────┤
│         JavaScript Bridge          │  ← 消息队列和模块代理
│        (MessageQueue)              │
├─────────────────────────────────────┤
│           C++ Bridge Layer         │  ← JavaScript 引擎和通信桥梁
│         (JSCExecutor)              │
├─────────────────────────────────────┤
│          Native Module Layer       │  ← 平台原生功能实现
│        (iOS/Android)               │
└─────────────────────────────────────┘
```

## 🔄 通信流程

### JS → Native 调用
1. JavaScript 调用 `NativeModules.ModuleName.methodName()`
2. `MessageQueue` 将调用信息序列化为 JSON 并加入队列
3. 通过 C++ `JSCExecutor` 传递到 Native 端
4. Native 端执行对应方法并返回结果

### Native → JS 事件推送
1. Native 模块触发事件
2. 通过 C++ Bridge 调用 JavaScript 函数
3. JavaScript 端触发相应的事件监听器

## 📁 项目结构

```
mini-rn/
├── js/                     # JavaScript Bridge 层
│   ├── MessageQueue.js     # 消息队列核心实现
│   ├── NativeModules.js    # Native 模块代理
│   └── Bridge.js           # 高级 Bridge API
├── cpp/                    # C++ Bridge 层
│   ├── JSCExecutor.h/.cpp  # JavaScript 执行器
│   └── NativeModule.h/.cpp # Native 模块基类
├── ios/                    # iOS 平台实现
│   ├── DeviceInfoModule.h/.mm  # iOS 设备信息模块
│   └── (其他 iOS 模块...)
├── android/                # Android 平台实现
│   ├── DeviceInfoModule.h/.cpp # Android 设备信息模块
│   ├── DeviceInfoHelper.java   # Java 辅助类
│   └── (其他 Android 模块...)
├── examples/               # 使用示例
│   ├── basic-usage.js      # 基本用法示例
│   └── bridge-demo.html    # 交互式演示页面
├── docs/                   # 详细文档
│   ├── roadmap.md              # 完整技术路线图
│   ├── phase1-bridge-plan.md   # 阶段1详细计划
│   ├── architecture-guide.md   # 架构详解
│   └── development-guide.md    # 开发指南
├── Makefile               # 构建系统
├── package.json           # 项目配置
└── index.js               # 主入口文件
```

## 🚀 快速开始

### 1. 环境要求

- Node.js 16+
- C++11 支持的编译器
- iOS: Xcode 12+, iOS 11+ (可选)
- Android: Android Studio, API Level 21+ (可选)

### 2. 安装和运行

```bash
# 克隆项目
git clone <repository-url>
cd mini-rn

# 安装依赖
npm install

# 运行基础演示
node index.js

# 或者启动交互式演示
make demo
```

### 3. 交互式演示

打开浏览器访问 `examples/bridge-demo.html` 或运行 `make demo`，体验完整的 Bridge 通信演示。

## 📖 核心组件详解

### JavaScript 层
- **MessageQueue**: 管理调用队列、回调注册、模块注册
- **NativeModules**: 提供 Promise 化的 Native 模块代理
- **Bridge**: 高级 API，包含事件系统和双向通信

### C++ 层  
- **JSCExecutor**: JavaScript 引擎管理、代码执行、数据转换
- **NativeModule**: 定义 Native 模块接口和基础功能

### Native 层
- **iOS**: 使用 Objective-C++ 实现，集成 iOS 系统 API
- **Android**: 使用 JNI 调用 Java 代码，集成 Android 系统 API

## 🛠️ 开发命令

```bash
make all        # 完整构建和测试
make build      # 编译 C++ 代码  
make test       # 运行测试
make demo       # 启动演示服务器
make clean      # 清理构建文件
make help       # 查看所有命令
```

## 📚 示例代码

### 基本 Native 调用
```javascript
const { DeviceInfo } = require('./js/NativeModules');

// 获取设备信息
const deviceId = await DeviceInfo.getDeviceId();
const batteryLevel = await DeviceInfo.getBatteryLevel();
console.log('设备 ID:', deviceId);
console.log('电池电量:', batteryLevel);
```

### 事件监听
```javascript
const { bridge } = require('./js/Bridge');

// 监听电池电量变化
bridge.addEventListener('batteryLevelChanged', (data) => {
    console.log('电池电量变化:', data);
});

// 开始电池监控
await DeviceInfo.startBatteryMonitoring();
```

### 双向通信通道
```javascript
// 创建通信通道
const channel = bridge.createChannel('dataSync');

// 监听消息
channel.onMessage((data) => {
    console.log('收到 Native 消息:', data);
});

// 发送消息
channel.send({ message: 'Hello Native!' });
```

## 🔍 学习路径

### 新手入门
1. **了解项目规划**: [技术路线图](docs/roadmap.md) - 理解整体学习目标
2. **阅读架构文档**: [架构详解](docs/architecture-guide.md) - 掌握核心概念
3. **运行基础示例**: `examples/basic-usage.js` - 快速体验功能

### 深入学习
4. **当前阶段详解**: [阶段1计划](docs/phase1-bridge-plan.md) - 了解具体实现步骤
5. **体验交互演示**: `examples/bridge-demo.html` - 可视化理解通信流程
6. **阅读开发指南**: [开发指南](docs/development-guide.md) - 参与开发实践

### 进阶实践
7. **创建自定义模块**: 参考 `DeviceInfoModule` 实现
8. **性能分析和优化**: 使用项目提供的调试工具
9. **架构演进**: 跟随项目从 Bridge 到 JSI 的完整迁移过程

## 📊 与新架构对比

| 特性 | 传统 Bridge | JSI 新架构 |
|------|------------|-----------|
| 通信方式 | 异步消息队列 | 同步直接调用 |
| 序列化 | JSON 序列化 | 直接对象传递 |
| 性能 | 相对较慢 | 更快 |
| 类型安全 | 运行时检查 | 编译时检查 |
| 学习难度 | 适中 | 较高 |

## 🎓 教育价值

通过这个项目，你将学到：

- React Native 底层通信机制
- JavaScript 引擎集成
- 跨平台架构设计
- 异步编程模式
- C++ 与移动平台集成
- 性能优化策略

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进这个教学项目！

## 📄 许可证

MIT License - 详见 LICENSE 文件

## 🚀 技术影响力目标

这个项目致力于：

- **填补技术空白**: 提供国内首个完整的 React Native 从零实现教程
- **深度技术洞察**: 不仅仅是表面的 API 使用，而是深入到架构设计和性能优化
- **实战价值**: 通过真实可运行的代码帮助开发者理解复杂的技术概念
- **社区贡献**: 为中文技术社区提供高质量的学习资源

### 适合的学习者

- 💻 **高级前端开发者**: 希望深入理解跨平台技术原理
- 🏗️ **移动端架构师**: 需要进行技术选型和架构决策
- ⚡ **React Native 深度用户**: 遇到性能瓶颈需要底层优化
- 🔬 **技术爱好者**: 对现代软件架构和性能优化感兴趣

---

**📚 学习提示**: 这是一个渐进式的教学项目，建议按照阶段顺序学习。每个阶段都包含完整的理论解析、代码实现和实践验证，确保深度理解每一个技术细节。
