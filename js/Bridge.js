/**
 * Bridge - JavaScript 和 Native 通信的桥梁
 * 封装了底层的 MessageQueue，提供更高级的 API
 */

const MessageQueue = require('./MessageQueue');

class Bridge {
  constructor() {
    this._messageQueue = global.__MessageQueue || new MessageQueue();
    this._eventListeners = {};
    this._isInitialized = false;
    
    console.log('[Bridge] 初始化开始');
    this._initialize();
  }

  /**
   * 初始化 Bridge
   */
  _initialize() {
    // 在全局对象上挂载 Bridge 方法，供 Native 调用
    if (typeof global !== 'undefined') {
      global.__bridgeInvokeCallback = this._invokeCallback.bind(this);
      global.__bridgeCallFunction = this._callFunction.bind(this);
      global.__bridgeFlushQueue = this._flushQueue.bind(this);
    }

    this._isInitialized = true;
    console.log('[Bridge] 初始化完成');
    
    // 触发初始化完成事件
    this._emit('bridgeReady');
  }

  /**
   * 调用 Native 方法
   * @param {string} moduleName - 模块名
   * @param {string} methodName - 方法名
   * @param {Array} args - 参数
   * @param {Function} callback - 回调函数
   */
  callNative(moduleName, methodName, args = [], callback = null) {
    if (!this._isInitialized) {
      console.error('[Bridge] Bridge 尚未初始化');
      return;
    }

    console.log(`[Bridge] 调用 Native: ${moduleName}.${methodName}`, args);

    return new Promise((resolve, reject) => {
      this._messageQueue.callNativeMethod(
        moduleName, 
        methodName, 
        args,
        (result) => {
          console.log(`[Bridge] Native 调用成功: ${moduleName}.${methodName}`, result);
          callback && callback(null, result);
          resolve(result);
        },
        (error) => {
          console.error(`[Bridge] Native 调用失败: ${moduleName}.${methodName}`, error);
          callback && callback(error);
          reject(error);
        }
      );
    });
  }

  /**
   * 注册事件监听器
   * @param {string} eventName - 事件名
   * @param {Function} listener - 监听器函数
   */
  addEventListener(eventName, listener) {
    if (!this._eventListeners[eventName]) {
      this._eventListeners[eventName] = [];
    }
    this._eventListeners[eventName].push(listener);
    
    console.log(`[Bridge] 注册事件监听器: ${eventName}`);
  }

  /**
   * 移除事件监听器
   * @param {string} eventName - 事件名  
   * @param {Function} listener - 监听器函数
   */
  removeEventListener(eventName, listener) {
    if (!this._eventListeners[eventName]) return;
    
    const index = this._eventListeners[eventName].indexOf(listener);
    if (index > -1) {
      this._eventListeners[eventName].splice(index, 1);
      console.log(`[Bridge] 移除事件监听器: ${eventName}`);
    }
  }

  /**
   * 触发事件
   * @param {string} eventName - 事件名
   * @param {any} data - 事件数据
   */
  _emit(eventName, data = null) {
    const listeners = this._eventListeners[eventName];
    if (!listeners) return;

    console.log(`[Bridge] 触发事件: ${eventName}`, data);
    
    listeners.forEach(listener => {
      try {
        listener(data);
      } catch (error) {
        console.error(`[Bridge] 事件监听器执行错误: ${eventName}`, error);
      }
    });
  }

  /**
   * Native 调用 JavaScript 回调 (供 Native 调用)
   * @param {number} callbackID - 回调 ID
   * @param {Array} args - 参数
   */
  _invokeCallback(callbackID, args) {
    console.log(`[Bridge] Native 调用回调: ${callbackID}`, args);
    return this._messageQueue.invokeCallbackAndReturnFlushedQueue(callbackID, args);
  }

  /**
   * Native 调用 JavaScript 函数 (供 Native 调用)
   * @param {string} moduleName - 模块名
   * @param {string} methodName - 方法名
   * @param {Array} args - 参数
   */
  _callFunction(moduleName, methodName, args) {
    console.log(`[Bridge] Native 调用 JS 函数: ${moduleName}.${methodName}`, args);
    
    // 特殊处理事件分发
    if (moduleName === 'RCTEventEmitter' && methodName === 'receiveEvent') {
      const [eventName, eventData] = args;
      this._emit(eventName, eventData);
      return this._messageQueue.flushedQueue();
    }
    
    return this._messageQueue.callFunctionReturnFlushedQueue(moduleName, methodName, args);
  }

  /**
   * 刷新消息队列 (供 Native 调用)
   */
  _flushQueue() {
    return this._messageQueue.flushedQueue();
  }

  /**
   * 获取 Bridge 状态
   */
  getStatus() {
    return {
      isInitialized: this._isInitialized,
      eventListeners: Object.keys(this._eventListeners),
      messageQueue: this._messageQueue.getDebugInfo()
    };
  }

  /**
   * 发送事件到 Native
   * @param {string} eventName - 事件名
   * @param {any} data - 事件数据
   */
  sendEventToNative(eventName, data) {
    return this.callNative('EventEmitter', 'emit', [eventName, data]);
  }

  /**
   * 注册 JavaScript 模块供 Native 调用
   * @param {string} moduleName - 模块名
   * @param {Object} moduleObject - 模块对象
   */
  registerJSModule(moduleName, moduleObject) {
    global[moduleName] = moduleObject;
    console.log(`[Bridge] 注册 JS 模块: ${moduleName}`, Object.keys(moduleObject));
  }

  /**
   * 创建一个双向通信的通道
   * @param {string} channelName - 通道名
   */
  createChannel(channelName) {
    const channel = {
      // 发送消息到 Native
      send: (data) => this.callNative('ChannelManager', 'receiveMessage', [channelName, data]),
      
      // 监听来自 Native 的消息
      onMessage: (callback) => this.addEventListener(`channel_${channelName}`, callback),
      
      // 关闭通道
      close: () => {
        this.callNative('ChannelManager', 'closeChannel', [channelName]);
        delete this._eventListeners[`channel_${channelName}`];
      }
    };

    console.log(`[Bridge] 创建通信通道: ${channelName}`);
    return channel;
  }
}

// 全局单例
const bridge = new Bridge();

// 等待 Bridge 就绪的 Promise
const bridgeReady = new Promise((resolve) => {
  if (bridge._isInitialized) {
    resolve(bridge);
  } else {
    bridge.addEventListener('bridgeReady', () => resolve(bridge));
  }
});

module.exports = {
  Bridge,
  bridge,
  bridgeReady
};
