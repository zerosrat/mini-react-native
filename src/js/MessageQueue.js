/**
 * MessageQueue - Mini React Native 消息队列实现
 *
 * 这是 React Native Bridge 通信的核心 JavaScript 组件，严格遵循 RN MessageQueue.js 的设计。
 * 负责 JavaScript 与 Native 之间的异步消息传递和回调管理。
 *
 * 核心功能：
 * 1. 消息队列管理 - 维护待发送到 Native 的调用队列
 * 2. 回调管理 - 处理异步调用的成功/失败回调
 * 3. 模块注册 - 管理 Native 模块的延迟加载
 * 4. 批量处理 - 优化多个调用的批量传输
 *
 * 严格的 RN 兼容性：
 * - 消息格式：[moduleIds, methodIds, params, callbackIds]
 * - 回调机制：双回调模式（onFail, onSucc）
 * - 模块注册：延迟加载机制
 * - 队列处理：批量刷新机制
 */

class MessageQueue {
  constructor() {
    // === 核心数据结构 ===

    // 消息队列：等待发送到 Native 的调用
    // 格式严格遵循 RN 标准：[moduleIds, methodIds, params, callbackIds]
    this._queue = [[], [], [], []]; // [moduleIDs, methodIDs, params, callbackIDs]

    // 回调管理
    this._callbackID = 0;           // 回调ID生成器
    this._callbacks = {};           // 存储所有待执行的回调函数

    // 模块注册表
    this._lazyCallableModules = {}; // 延迟加载的模块
    this._modules = {};             // 已实例化的模块

    // 模块和方法映射表（与 Native 端保持一致）
    this._moduleTable = {};         // 模块名 -> 模块ID 的映射
    this._methodTable = {};         // 方法名 -> 方法ID 的映射（按模块分组）

    // 性能和调试
    this._isInCallback = false;     // 标记是否正在执行回调
    this._debugEnabled = true;      // 调试模式标志

    console.log('[MessageQueue] Initialized with RN-compatible structure');
  }

  /**
   * 注册延迟加载模块
   * 这是 RN 的标准模块注册方式，模块只在第一次使用时才实例化
   *
   * @param {string} moduleName 模块名称
   * @param {function} factory 模块工厂函数，返回模块实例
   */
  registerLazyCallableModule(moduleName, factory) {
    this._lazyCallableModules[moduleName] = factory;

    if (this._debugEnabled) {
      console.log(`[MessageQueue] Registered lazy module: ${moduleName}`);
    }
  }

  /**
   * 直接注册已实例化的模块（非延迟加载）
   * 用于系统级或核心模块的直接注册
   *
   * @param {string} moduleName 模块名称
   * @param {Object} module 模块实例
   */
  registerCallableModule(moduleName, module) {
    this._modules[moduleName] = module;

    if (this._debugEnabled) {
      console.log(`[MessageQueue] Registered direct module: ${moduleName}`);
    }
  }

  /**
   * 调用 Native 模块方法（核心方法）
   * 这是 JavaScript 调用 Native 的主要入口，严格遵循 RN 的调用约定
   *
   * @param {number} moduleID Native 模块ID
   * @param {number} methodID Native 方法ID
   * @param {Array} params 方法参数数组
   * @param {function} onFail 失败回调函数
   * @param {function} onSucc 成功回调函数
   */
  enqueueNativeCall(moduleID, methodID, params, onFail, onSucc) {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Calling native method - Module: ${moduleID}, Method: ${methodID}`);
    }

    // 验证参数
    if (typeof moduleID !== 'number' || typeof methodID !== 'number') {
      console.error('[MessageQueue] Invalid moduleID or methodID');
      if (onFail) onFail('Invalid module or method ID');
      return;
    }

    // 生成回调ID并注册回调函数
    let callbackID = null;
    if (onFail || onSucc) {
      callbackID = this._callbackID++;
      this._callbacks[callbackID] = {
        onFail: onFail,
        onSucc: onSucc
      };
    }

    // 将调用添加到队列中
    // 严格遵循 RN 的队列格式：[moduleIds, methodIds, params, callbackIds]
    this._queue[0].push(moduleID);    // moduleIds
    this._queue[1].push(methodID);    // methodIds
    this._queue[2].push(params || []); // params
    this._queue[3].push(callbackID);  // callbackIds

    if (this._debugEnabled) {
      console.log(`[MessageQueue] Queued call - Queue length: ${this._queue[0].length}`);
    }

    // 立即刷新队列（简化版实现，实际 RN 会做批量优化）
    this._flushQueue();
  }

  /**
   * 获取并清空待处理队列
   * 这是 Native 端获取 JavaScript 待执行调用的标准接口
   *
   * @returns {Array} 格式：[moduleIds, methodIds, params, callbackIds]
   */
  flushedQueue() {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Flushing queue with ${this._queue[0].length} calls`);
    }

    // 返回当前队列并创建新的空队列
    const queue = this._queue;
    this._queue = [[], [], [], []];

    return queue;
  }

  /**
   * Native 调用 JavaScript 函数的标准接口
   * Native 端通过此方法执行 JS 模块中的方法
   *
   * @param {string} module 模块名称
   * @param {string} method 方法名称
   * @param {Array} args 参数数组
   * @returns {Array} 执行后的消息队列
   */
  callFunctionReturnFlushedQueue(module, method, args) {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Calling JS function - Module: ${module}, Method: ${method}`);
    }

    this._isInCallback = true;

    try {
      // 获取模块实例
      const moduleInstance = this.getCallableModule(module);
      if (!moduleInstance) {
        console.error(`[MessageQueue] Module not found: ${module}`);
        return this.flushedQueue();
      }

      // 检查方法是否存在
      if (typeof moduleInstance[method] !== 'function') {
        console.error(`[MessageQueue] Method not found: ${module}.${method}`);
        return this.flushedQueue();
      }

      // 执行方法
      moduleInstance[method].apply(moduleInstance, args || []);

    } catch (error) {
      console.error(`[MessageQueue] Error calling ${module}.${method}:`, error);
    } finally {
      this._isInCallback = false;
    }

    // 返回在函数执行过程中可能产生的新调用
    return this.flushedQueue();
  }

  /**
   * Native 调用 JavaScript 函数并返回结果和队列
   * 用于需要同步返回值的场景
   *
   * @param {string} module 模块名称
   * @param {string} method 方法名称
   * @param {Array} args 参数数组
   * @returns {Array} [result, flushedQueue] 结果和消息队列
   *
   * TODO: 低优先级功能，暂时不实现
   * 当前架构主要基于异步通信，同步返回值不是核心需求
   */
  // callFunctionReturnResultAndFlushedQueue(module, method, args) {
  //   // 实现将在后续版本中添加
  // }

  /**
   * 执行回调并返回新的队列
   * Native 端在执行完异步操作后，通过此方法执行 JavaScript 回调
   *
   * @param {number} callbackID 回调ID
   * @param {Array} args 回调参数
   * @returns {Array} 新的消息队列（如果回调中产生了新的调用）
   */
  invokeCallbackAndReturnFlushedQueue(callbackID, args) {
    if (this._debugEnabled) {
      console.log(`[MessageQueue] Invoking callback: ${callbackID}`);
    }

    this._isInCallback = true;

    try {
      this._invokeCallback(callbackID, args);
    } catch (error) {
      console.error('[MessageQueue] Error invoking callback:', error);
    } finally {
      this._isInCallback = false;
    }

    // 返回在回调执行过程中可能产生的新调用
    return this.flushedQueue();
  }

  /**
   * 获取事件循环运行时间
   * 用于性能监控，简化实现
   *
   * @returns {number} 运行时间（毫秒）
   */
  getEventLoopRunningTime() {
    // 简化实现，返回固定值
    // 实际 RN 会计算真实的事件循环时间
    return Date.now();
  }

  // === 私有方法 ===

  /**
   * 刷新消息队列到 Native
   * 调用 Native 端的 nativeFlushQueueImmediate 函数
   *
   * @private
   */
  _flushQueue() {
    if (this._queue[0].length === 0) {
      return; // 队列为空，无需刷新
    }

    if (typeof nativeFlushQueueImmediate === 'function') {
      // 获取当前队列
      const queue = this.flushedQueue();

      if (this._debugEnabled) {
        console.log('[MessageQueue] Flushing to native:', {
          moduleIds: queue[0],
          methodIds: queue[1],
          params: queue[2],
          callbackIds: queue[3]
        });
      }

      // 调用 Native 函数，传递标准的 RN 消息格式
      nativeFlushQueueImmediate(
        queue[0], // moduleIds
        queue[1], // methodIds
        queue[2], // params
        queue[3]  // callbackIds
      );
    } else {
      console.error('[MessageQueue] nativeFlushQueueImmediate not available');
    }
  }

  /**
   * 执行单个回调
   *
   * @param {number} callbackID 回调ID
   * @param {Array} args 回调参数
   * @private
   */
  _invokeCallback(callbackID, args) {
    const callback = this._callbacks[callbackID];

    if (!callback) {
      console.error(`[MessageQueue] Callback ${callbackID} not found`);
      return;
    }

    // 清理回调引用，避免内存泄漏
    delete this._callbacks[callbackID];

    try {
      // RN 的回调约定：第一个参数是错误，后续参数是结果
      if (args && args[0] != null) {
        // 有错误，调用失败回调
        if (callback.onFail) {
          callback.onFail(args[0]);
        }
      } else {
        // 成功，调用成功回调
        if (callback.onSucc) {
          const result = args ? args.slice(1) : [];
          callback.onSucc.apply(null, result);
        }
      }
    } catch (error) {
      console.error('[MessageQueue] Error in callback execution:', error);
    }
  }

  /**
   * 获取已注册的模块实例
   * 如果是延迟模块且未实例化，会触发实例化
   *
   * @param {string} moduleName 模块名称
   * @returns {Object} 模块实例
   */
  getCallableModule(moduleName) {
    // 检查是否已实例化
    if (this._modules[moduleName]) {
      return this._modules[moduleName];
    }

    // 延迟加载模块
    const factory = this._lazyCallableModules[moduleName];
    if (factory) {
      this._modules[moduleName] = factory();
      if (this._debugEnabled) {
        console.log(`[MessageQueue] Lazy loaded module: ${moduleName}`);
      }
      return this._modules[moduleName];
    }

    console.error(`[MessageQueue] Module not found: ${moduleName}`);
    return null;
  }

  // === 调试和状态查询方法 ===

  /**
   * 获取当前队列状态（调试用）
   */
  getQueueStatus() {
    return {
      queueLength: this._queue[0].length,
      callbackCount: Object.keys(this._callbacks).length,
      moduleCount: Object.keys(this._modules).length,
      lazyModuleCount: Object.keys(this._lazyCallableModules).length,
      isInCallback: this._isInCallback
    };
  }

  /**
   * 清空所有队列和回调（测试用）
   */
  clearAll() {
    this._queue = [[], [], [], []];
    this._callbacks = {};
    console.log('[MessageQueue] Cleared all queues and callbacks');
  }
}

// 导出 MessageQueue 类供其他模块使用
if (typeof module !== 'undefined' && module.exports) {
  module.exports = MessageQueue;
} else if (typeof global !== 'undefined') {
  global.MessageQueue = MessageQueue;
}

// 在全局作用域中注册（用于测试）
if (typeof window !== 'undefined') {
  window.MessageQueue = MessageQueue;
}

console.log('[MessageQueue] MessageQueue.js loaded - RN compatible implementation');