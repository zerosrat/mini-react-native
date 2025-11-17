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

// 使用 CommonJS require 导入 MessageQueue
const MessageQueue = require('./MessageQueue')
console.log('[BatchedBridge] MessageQueue imported via require:', typeof MessageQueue)

// 创建 MessageQueue 实例作为 BatchedBridge（官方 RN 方式）
const BatchedBridge = new MessageQueue()

// 设置全局 Bridge 实例（官方 RN 方式）
// Wire up the batched bridge on the global object so that we can call into it.
Object.defineProperty(global, '__fbBatchedBridge', {
  configurable: true,
  value: BatchedBridge,
})

// 使用 CommonJS 导出
module.exports = BatchedBridge

console.log('[BatchedBridge] BatchedBridge.js loaded - CommonJS module')
