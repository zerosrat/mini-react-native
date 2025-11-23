/**
 * Mini React Native JavaScript Bundle Entry Point
 *
 * 这是 Mini React Native 的 JavaScript 模块系统入口文件。
 * 负责按正确的顺序加载和初始化所有核心模块。
 *
 * 加载顺序至关重要：
 * 0. Console - 控制台实现（必须最先加载）
 * 1. MessageQueue - 核心通信队列
 * 2. BatchedBridge - 桥接器（依赖 MessageQueue）
 * 3. NativeModule - 原生模块系统（依赖 BatchedBridge）
 * 4. DeviceInfo - 具体的原生模块（依赖 NativeModule）
 */

// 0. 首先加载 console 实现（在任何 console.log 调用之前）
const console = require('./console')

console.log('[MiniReactNative] Starting JavaScript module system initialization...')

// 1. 加载核心通信模块
const MessageQueue = require('./MessageQueue')
console.log('[MiniReactNative] MessageQueue loaded')

// 2. 加载并初始化桥接器
const BatchedBridge = require('./BatchedBridge')
console.log('[MiniReactNative] BatchedBridge loaded and __fbBatchedBridge set')

// 3. 加载原生模块系统
const NativeModules = require('./NativeModule')
console.log('[MiniReactNative] NativeModule system loaded')

// 4. 加载具体的模块
const DeviceInfo = require('./DeviceInfo')
console.log('[MiniReactNative] DeviceInfo module loaded')

// 将关键模块暴露到全局环境，保持与原有系统的兼容性
// 这样 C++ 端可以继续使用 global.__fbBatchedBridge 等接口
if (typeof global !== 'undefined') {
  // 确保 BatchedBridge 已经设置了 __fbBatchedBridge
  if (!global.__fbBatchedBridge) {
    global.__fbBatchedBridge = BatchedBridge
  }

  // 设置 MessageQueue 为全局可访问（便于测试）
  global.MessageQueue = MessageQueue

  // 设置 NativeModules 为全局可访问
  global.NativeModules = NativeModules

  // 设置 DeviceInfo 为全局可访问（便于测试）
  global.DeviceInfo = DeviceInfo

  console.log('[MiniReactNative] Global objects set up successfully')
}

// 导出主要接口供外部使用
module.exports = {
  MessageQueue,
  BatchedBridge,
  NativeModules,
  DeviceInfo,

  // 提供版本信息
  version: '1.0.0',

  // 提供状态查询方法
  getStatus() {
    return {
      messageQueueReady: !!MessageQueue,
      batchedBridgeReady: !!BatchedBridge && !!global.__fbBatchedBridge,
      nativeModulesReady: !!NativeModules,
      deviceInfoReady: !!DeviceInfo,
      bridgeConfigReady: !!global.__fbBatchedBridgeConfig
    }
  }
}

console.log('[MiniReactNative] JavaScript bundle loaded successfully!')
console.log('[MiniReactNative] System status:', module.exports.getStatus())