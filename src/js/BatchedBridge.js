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
if (typeof require !== 'undefined') {
  // Node.js 环境
  MessageQueueClass = require('./MessageQueue.js');
} else if (typeof global !== 'undefined' && global.MessageQueue) {
  // 全局环境
  MessageQueueClass = global.MessageQueue;
} else if (typeof MessageQueue !== 'undefined') {
  // 当前作用域中已经存在 MessageQueue
  MessageQueueClass = MessageQueue;
} else {
  console.error('[BatchedBridge] MessageQueue not found');
}

// 创建 MessageQueue 实例作为 BatchedBridge（官方 RN 方式）
const BatchedBridge = new MessageQueueClass();

// 设置全局 Bridge 实例（官方 RN 方式）
// Wire up the batched bridge on the global object so that we can call into it.
Object.defineProperty(global, '__fbBatchedBridge', {
  configurable: true,
  value: BatchedBridge,
});

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