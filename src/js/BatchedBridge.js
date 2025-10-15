/**
 * BatchedBridge - Mini React Native 批量桥接器
 *
 * 这是 React Native Bridge 系统的初始化和配置模块，负责：
 * 1. 创建和配置 MessageQueue 实例
 * 2. 将 Bridge 接口暴露给全局环境
 * 3. 处理 Native 模块的注册和配置
 * 4. 建立 JavaScript 和 Native 之间的通信通道
 *
 * 设计原则：
 * - 严格遵循 React Native BatchedBridge 的接口设计
 * - 保持与 RN 的初始化流程一致
 * - 简化实现细节，专注核心通信功能
 */

// 引入 MessageQueue（在实际环境中会通过模块加载）
// 由于我们在 JavaScript 环境中运行，这里直接使用全局引用
let MessageQueue;
if (typeof require !== 'undefined') {
  // Node.js 环境
  MessageQueue = require('./MessageQueue.js');
} else if (typeof global !== 'undefined' && global.MessageQueue) {
  // 全局环境
  MessageQueue = global.MessageQueue;
} else {
  console.error('[BatchedBridge] MessageQueue not found');
}

/**
 * BatchedBridge - 批量桥接器类
 *
 * 这是 JavaScript 和 Native 通信的统一入口点
 */
class BatchedBridge {
  constructor() {
    // 创建核心消息队列实例
    this._messageQueue = new MessageQueue();

    // 模块注册表
    this._nativeModules = {};
    this._nativeModuleInfo = {};

    // 初始化标志
    this._isInitialized = false;

    console.log('[BatchedBridge] Initialized');
  }

  /**
   * 注册 Native 模块
   * 这是 Native 端向 JavaScript 端注册可调用模块的接口
   *
   * @param {Object} nativeModules 原生模块配置对象
   * @param {Array} moduleInfo 模块信息数组 [moduleName, moduleID, methodNames, constants]
   */
  registerCallableModule(nativeModules, moduleInfo) {
    console.log('[BatchedBridge] Registering callable modules:', Object.keys(nativeModules));

    // 存储模块信息
    this._nativeModules = { ...this._nativeModules, ...nativeModules };

    // 处理模块信息
    if (moduleInfo && Array.isArray(moduleInfo)) {
      moduleInfo.forEach((info, moduleID) => {
        if (info && info[0]) {
          const moduleName = info[0];
          const methods = info[2] || [];
          const constants = info[3] || {};

          this._nativeModuleInfo[moduleName] = {
            moduleID: moduleID,
            methods: methods,
            constants: constants
          };

          console.log(`[BatchedBridge] Registered module: ${moduleName} (ID: ${moduleID})`);
        }
      });
    }
  }

  /**
   * 获取 Native 模块
   * 提供给 JavaScript 代码调用 Native 模块的接口
   *
   * @param {string} moduleName 模块名称
   * @returns {Object} 模块代理对象
   */
  require(moduleName) {
    const moduleInfo = this._nativeModuleInfo[moduleName];

    if (!moduleInfo) {
      console.error(`[BatchedBridge] Module not found: ${moduleName}`);
      return null;
    }

    // 创建模块代理对象
    const moduleProxy = {};

    // 添加常量
    Object.assign(moduleProxy, moduleInfo.constants);

    // 为每个方法创建代理函数
    moduleInfo.methods.forEach((methodName, methodID) => {
      moduleProxy[methodName] = (...args) => {
        // 分离回调函数和普通参数
        let onFail = null;
        let onSucc = null;
        let params = args;

        // RN 约定：最后两个参数如果是函数，则为回调
        if (args.length >= 2 &&
            typeof args[args.length - 2] === 'function' &&
            typeof args[args.length - 1] === 'function') {
          onFail = args[args.length - 2];
          onSucc = args[args.length - 1];
          params = args.slice(0, -2);
        } else if (args.length >= 1 && typeof args[args.length - 1] === 'function') {
          // 只有一个回调函数，默认为成功回调
          onSucc = args[args.length - 1];
          params = args.slice(0, -1);
        }

        console.log(`[BatchedBridge] Calling ${moduleName}.${methodName}`, params);

        // 通过 MessageQueue 发送调用
        this._messageQueue.callNativeMethod(
          moduleInfo.moduleID,
          methodID,
          params,
          onFail,
          onSucc
        );
      };
    });

    return moduleProxy;
  }

  /**
   * 刷新队列
   * Native 端调用此方法获取待处理的调用队列
   *
   * @returns {Array} 消息队列 [moduleIds, methodIds, params, callbackIds]
   */
  flushedQueue() {
    return this._messageQueue.flushedQueue();
  }

  /**
   * 执行回调并刷新队列
   * Native 端执行完异步操作后，通过此方法回调 JavaScript
   *
   * @param {number} callbackID 回调ID
   * @param {Array} args 回调参数
   * @returns {Array} 新的消息队列
   */
  invokeCallbackAndReturnFlushedQueue(callbackID, args) {
    return this._messageQueue.invokeCallbackAndReturnFlushedQueue(callbackID, args);
  }

  /**
   * 调用 JavaScript 函数
   * Native 端主动调用 JavaScript 函数的接口
   *
   * @param {string} module 模块名称
   * @param {string} method 方法名称
   * @param {Array} args 参数数组
   */
  callFunctionReturnFlushedQueue(module, method, args) {
    console.log(`[BatchedBridge] Native calling JS: ${module}.${method}`, args);

    try {
      // 查找并执行目标函数
      if (global[module] && typeof global[module][method] === 'function') {
        global[module][method].apply(null, args);
      } else {
        console.error(`[BatchedBridge] JS method not found: ${module}.${method}`);
      }
    } catch (error) {
      console.error(`[BatchedBridge] Error calling JS method: ${module}.${method}`, error);
    }

    // 返回执行过程中可能产生的新调用
    return this.flushedQueue();
  }

  /**
   * 获取事件循环运行时间
   */
  getEventLoopRunningTime() {
    return this._messageQueue.getEventLoopRunningTime();
  }

  /**
   * 初始化 Bridge
   * 设置全局 Bridge 实例和相关函数
   */
  initialize() {
    if (this._isInitialized) {
      console.warn('[BatchedBridge] Already initialized');
      return;
    }

    // 将自己设置为全局 Bridge 实例
    global.__fbBatchedBridge = this;

    // 设置 Native 调用 JavaScript 的全局函数
    global.__fbBatchedBridgeConfig = {
      callFunctionReturnFlushedQueue: this.callFunctionReturnFlushedQueue.bind(this),
      invokeCallbackAndReturnFlushedQueue: this.invokeCallbackAndReturnFlushedQueue.bind(this),
      flushedQueue: this.flushedQueue.bind(this)
    };

    this._isInitialized = true;
    console.log('[BatchedBridge] Bridge initialized and available globally');
  }

  /**
   * 获取 Bridge 状态（调试用）
   */
  getStatus() {
    return {
      isInitialized: this._isInitialized,
      moduleCount: Object.keys(this._nativeModules).length,
      registeredModules: Object.keys(this._nativeModuleInfo),
      queueStatus: this._messageQueue.getQueueStatus()
    };
  }
}

// 创建全局 Bridge 实例
const batchedBridge = new BatchedBridge();

// 自动初始化
batchedBridge.initialize();

// 导出供其他模块使用
if (typeof module !== 'undefined' && module.exports) {
  module.exports = batchedBridge;
} else if (typeof global !== 'undefined') {
  global.BatchedBridge = batchedBridge;
}

// 兼容性：支持 window 环境
if (typeof window !== 'undefined') {
  window.BatchedBridge = batchedBridge;
  window.__fbBatchedBridge = batchedBridge;
}

console.log('[BatchedBridge] BatchedBridge.js loaded - RN compatible bridge implementation');