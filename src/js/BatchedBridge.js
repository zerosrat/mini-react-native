/**
 * BatchedBridge - Mini React Native 批量桥接器
 *
 * 严格遵循 React Native 官方 BatchedBridge 实现，负责：
 * 1. 创建 MessageQueue 实例作为通信桥梁
 * 2. 将 Bridge 接口暴露给全局环境
 * 3. 建立 JavaScript 和 Native 之间的通信通道
 *
 * 设计原则：
 * - 严格遵循 React Native BatchedBridge 的接口设计
 * - 保持与 RN 的初始化流程一致
 * - 简化实现，专注核心通信功能
 */

// MessageQueue 获取逻辑（适配无打包工具环境）
var MessageQueueClass;
console.log('[BatchedBridge] Debug - require:', typeof require);
console.log('[BatchedBridge] Debug - global:', typeof global);
console.log('[BatchedBridge] Debug - global.MessageQueue:', typeof global.MessageQueue, global.MessageQueue);
console.log('[BatchedBridge] Debug - MessageQueue in scope:', typeof MessageQueue);

// 在嵌入式环境中，优先使用 global.MessageQueue，因为 require 可能不可靠
if (typeof global !== 'undefined' && global.MessageQueue) {
  // 全局环境
  console.log('[BatchedBridge] Using global.MessageQueue');
  MessageQueueClass = global.MessageQueue;
} else if (typeof MessageQueue !== 'undefined') {
  // 当前作用域中已经存在 MessageQueue
  console.log('[BatchedBridge] Using scope MessageQueue');
  MessageQueueClass = MessageQueue;
} else if (typeof require !== 'undefined') {
  // Node.js 环境 - 最后尝试 require
  console.log('[BatchedBridge] Using require for MessageQueue');
  MessageQueueClass = require('./MessageQueue.js');
} else {
  console.error('[BatchedBridge] MessageQueue not found');
}

// 调试信息
console.log('[BatchedBridge] MessageQueueClass type:', typeof MessageQueueClass);
console.log('[BatchedBridge] MessageQueueClass:', MessageQueueClass);

// 创建 MessageQueue 实例作为 BatchedBridge（官方 RN 方式）
const BatchedBridge = new MessageQueueClass();

// 设置全局 Bridge 实例（官方 RN 方式）
// Wire up the batched bridge on the global object so that we can call into it.
Object.defineProperty(global, '__fbBatchedBridge', {
  configurable: true,
  value: BatchedBridge,
});

// 自动初始化 NativeModules 系统
if (typeof require !== 'undefined') {
  try {
    const NativeModules = require('./NativeModule.js');

    // 延迟初始化，确保模块配置已经注入
    setTimeout(() => {
      if (global.__fbBatchedBridgeConfig) {
        console.log('[BatchedBridge] Initializing NativeModules system...');
        NativeModules.initialize();

        // 将 NativeModules 暴露到全局，方便测试和调试
        global.NativeModules = NativeModules;

        console.log('[BatchedBridge] NativeModules system initialized');
      } else {
        console.log('[BatchedBridge] Module config not ready, NativeModules will initialize on first use');
      }
    }, 0);

  } catch (error) {
    console.log('[BatchedBridge] NativeModules not available:', error.message);
  }
}

// 导出模块
if (typeof module !== 'undefined' && module.exports) {
  module.exports = BatchedBridge;
} else if (typeof global !== 'undefined') {
  global.BatchedBridge = BatchedBridge;
}

// 兼容性：支持 window 环境
if (typeof window !== 'undefined') {
  window.BatchedBridge = BatchedBridge;
  window.__fbBatchedBridge = BatchedBridge;
}

console.log('[BatchedBridge] BatchedBridge.js loaded - RN compatible bridge implementation');