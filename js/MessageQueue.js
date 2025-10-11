/**
 * MessageQueue - React Native Bridge 的核心消息队列
 * 负责管理 JavaScript 和 Native 之间的通信
 */

class MessageQueue {
  constructor() {
    // 存储待发送的调用队列
    this._callQueue = [];
    this._callIDs = [];
    this._callbackCount = 1;
    this._callbacks = [];
    
    // 模块注册表
    this._moduleTable = {};      // 模块名 -> 模块ID 的映射
    this._moduleConfigs = {};    // 模块名 -> 模块配置 的映射
    this._methodTable = {};
    
    // 调用 ID 计数器
    this._callID = 0;
    this._moduleIDCounter = 0;   // 模块 ID 计数器
    
    this._debugInfo = {};
    
    console.log('[MessageQueue] 初始化完成');
  }

  /**
   * 注册 Native 模块
   * @param {Array} config - 模块配置 [模块名, 常量, 方法列表, promise方法列表, 同步方法列表]
   */
  registerModule(config) {
    const [moduleName, constants, methods, promiseMethods, syncMethods] = config;
    
    // 分配唯一的模块 ID
    const moduleID = this._moduleIDCounter++;
    this._moduleTable[moduleName] = moduleID;
    this._moduleConfigs[moduleName] = { constants, methods, promiseMethods, syncMethods };
    this._methodTable[moduleName] = {};
    
    methods.forEach((methodName, methodIndex) => {
      this._methodTable[moduleName][methodName] = methodIndex;
    });
    
    console.log(`[MessageQueue] 注册模块: ${moduleName}`, {
      moduleID,
      methods: methods.length,
      constants: Object.keys(constants || {}).length
    });
  }

  /**
   * 调用 Native 方法
   * @param {string} moduleName - 模块名
   * @param {string} methodName - 方法名  
   * @param {Array} args - 参数
   * @param {Function} onSuccess - 成功回调
   * @param {Function} onFail - 失败回调
   */
  callNativeMethod(moduleName, methodName, args, onSuccess, onFail) {
    const moduleID = this._getModuleID(moduleName);
    const methodID = this._getMethodID(moduleName, methodName);
    
    if (moduleID === null || methodID === null) {
      const error = `找不到模块方法: ${moduleName}.${methodName}`;
      console.error(`[MessageQueue] ${error}`);
      onFail && onFail(new Error(error));
      return;
    }

    // 生成调用 ID
    const callID = this._callID++;
    let callbackID = null;
    
    // 注册回调函数
    if (onSuccess || onFail) {
      callbackID = this._callbackCount;
      this._callbacks[callbackID] = {onSuccess, onFail};
      args = args.concat([callbackID, callbackID]);
      this._callbackCount++;
    }

    // 添加到调用队列
    this._callQueue[0] = moduleID;
    this._callQueue[1] = methodID; 
    this._callQueue[2] = args;
    this._callIDs.push(callID);

    this._debugInfo[callID] = {
      module: moduleName,
      method: methodName,
      args: args.slice(0, callbackID ? -2 : args.length), // 移除回调ID
      timestamp: Date.now(),
      callbackID
    };

    console.log(`[MessageQueue] 调用 Native 方法: ${moduleName}.${methodName}`, {
      callID,
      moduleID,
      methodID,
      args: args.slice(0, callbackID ? -2 : args.length),
      callbackID
    });

    // 立即发送到 Native (模拟异步调用)
    this._flushQueue();
  }

  /**
   * 获取并清空消息队列 (Native 调用)
   */
  flushedQueue() {
    const queue = this._callQueue.slice();
    const callIDs = this._callIDs.slice();
    
    this._callQueue = [];
    this._callIDs = [];
    
    console.log(`[MessageQueue] 队列被刷新`, { 
      calls: callIDs.length,
      queue: queue.length > 0 ? queue : null 
    });
    
    return queue.length > 0 ? [queue] : null;
  }

  /**
   * 调用 JavaScript 回调 (Native 调用)
   * @param {number} callbackID - 回调 ID
   * @param {Array} args - 回调参数
   */
  invokeCallbackAndReturnFlushedQueue(callbackID, args) {
    const callback = this._callbacks[callbackID];
    
    if (!callback) {
      console.warn(`[MessageQueue] 未找到回调函数: ${callbackID}`);
      return this.flushedQueue();
    }

    console.log(`[MessageQueue] 执行回调`, { callbackID, args });

    try {
      if (args.length > 0 && args[0] !== null) {
        // 有错误，调用失败回调
        callback.onFail && callback.onFail(new Error(args[0]));
      } else {
        // 成功，调用成功回调
        callback.onSuccess && callback.onSuccess(args[1]);
      }
    } catch (error) {
      console.error(`[MessageQueue] 回调执行错误:`, error);
    }

    // 清除回调
    delete this._callbacks[callbackID];
    
    return this.flushedQueue();
  }

  /**
   * 调用 JavaScript 方法 (Native 调用)
   * @param {string} moduleName - 模块名
   * @param {string} methodName - 方法名
   * @param {Array} args - 参数
   */
  callFunctionReturnFlushedQueue(moduleName, methodName, args) {
    console.log(`[MessageQueue] Native 调用 JS 方法: ${moduleName}.${methodName}`, args);

    try {
      // 这里可以扩展支持 JS 模块的方法调用
      // 例如事件分发、状态更新等
      const jsModule = global[moduleName];
      if (jsModule && typeof jsModule[methodName] === 'function') {
        jsModule[methodName].apply(jsModule, args);
      } else {
        console.warn(`[MessageQueue] JS 模块方法未找到: ${moduleName}.${methodName}`);
      }
    } catch (error) {
      console.error(`[MessageQueue] JS 方法调用错误:`, error);
    }

    return this.flushedQueue();
  }

  /**
   * 模拟发送队列到 Native
   */
  _flushQueue() {
    // 在真实环境中，这里会调用 C++ Bridge
    setTimeout(() => {
      const queue = this.flushedQueue();
      if (queue) {
        console.log(`[MessageQueue] 模拟发送到 Native:`, queue);
        // 模拟 Native 处理和回调
        this._simulateNativeResponse(queue);
      }
    }, 10);
  }

  /**
   * 模拟 Native 响应
   */
  _simulateNativeResponse(queue) {
    setTimeout(() => {
      // 从队列中提取回调ID
      if (queue && queue.length > 0) {
        const [moduleID, methodID, args] = queue[0];
        // 回调ID通常在参数的最后两位
        if (args && args.length >= 2) {
          const callbackID = args[args.length - 2]; // 成功回调ID
          if (this._callbacks[callbackID]) {
            this.invokeCallbackAndReturnFlushedQueue(callbackID, [null, '模拟 Native 响应数据']);
          }
        }
      }
    }, 100);
  }

  /**
   * 获取模块 ID
   */
  _getModuleID(moduleName) {
    return this._moduleTable[moduleName] !== undefined
      ? this._moduleTable[moduleName] : null;
  }

  /**
   * 获取方法 ID  
   */
  _getMethodID(moduleName, methodName) {
    const methodTable = this._methodTable[moduleName];
    return methodTable && methodTable[methodName] !== undefined 
      ? methodTable[methodName] : null;
  }

  /**
   * 获取调试信息
   */
  getDebugInfo() {
    return {
      callQueue: this._callQueue,
      callIDs: this._callIDs,
      callbacks: Object.keys(this._callbacks),
      modules: Object.keys(this._moduleTable),
      debugInfo: this._debugInfo
    };
  }
}

// 全局单例
if (typeof global !== 'undefined') {
  global.__MessageQueue = global.__MessageQueue || new MessageQueue();
}

module.exports = MessageQueue;
