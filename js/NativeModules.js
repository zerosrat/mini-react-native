/**
 * NativeModules - JavaScript 端的 Native 模块代理
 * 提供简洁的 API 来调用 Native 方法
 */

const MessageQueue = require('./MessageQueue');

class NativeModules {
  constructor() {
    this._messageQueue = global.__MessageQueue || new MessageQueue();
    this._modules = {};
    
    console.log('[NativeModules] 初始化完成');
  }

  /**
   * 注册 Native 模块
   * @param {string} moduleName - 模块名
   * @param {Object} moduleConfig - 模块配置
   */
  registerModule(moduleName, moduleConfig) {
    const { constants = {}, methods = [] } = moduleConfig;
    
    // 注册到 MessageQueue
    this._messageQueue.registerModule([
      moduleName,
      constants,
      methods,
      [], // promiseMethods
      []  // syncMethods  
    ]);

    // 创建模块代理对象
    const moduleProxy = { ...constants };
    
    methods.forEach((methodName, index) => {
      moduleProxy[methodName] = (...args) => {
        return new Promise((resolve, reject) => {
          this._messageQueue.callNativeMethod(
            moduleName,
            methodName,
            args,
            resolve,
            reject
          );
        });
      };
    });

    this._modules[moduleName] = moduleProxy;
    
    console.log(`[NativeModules] 注册模块代理: ${moduleName}`, {
      methods: methods.length,
      constants: Object.keys(constants).length
    });

    return moduleProxy;
  }

  /**
   * 获取模块
   * @param {string} moduleName - 模块名
   */
  getModule(moduleName) {
    return this._modules[moduleName] || null;
  }

  /**
   * 获取所有已注册的模块
   */
  getAllModules() {
    return { ...this._modules };
  }

  /**
   * 创建一个便捷的调用方法
   * @param {string} moduleName - 模块名
   * @param {string} methodName - 方法名
   * @param {Array} args - 参数
   */
  async callMethod(moduleName, methodName, ...args) {
    const module = this._modules[moduleName];
    if (!module) {
      throw new Error(`模块未找到: ${moduleName}`);
    }

    const method = module[methodName];
    if (typeof method !== 'function') {
      throw new Error(`方法未找到: ${moduleName}.${methodName}`);
    }

    return await method(...args);
  }

  /**
   * 批量注册模块 (模拟 RN 的批量注册)
   * @param {Object} moduleConfigs - 模块配置对象
   */
  registerModules(moduleConfigs) {
    Object.entries(moduleConfigs).forEach(([moduleName, config]) => {
      this.registerModule(moduleName, config);
    });
  }

  /**
   * 获取调试信息
   */
  getDebugInfo() {
    return {
      modules: Object.keys(this._modules),
      messageQueue: this._messageQueue.getDebugInfo()
    };
  }
}

// 创建全局实例 
const nativeModules = new NativeModules();

// 模拟注册一些常用的 Native 模块
nativeModules.registerModules({
  // 设备信息模块
  DeviceInfo: {
    constants: {
      DEVICE_TYPE: 'mobile',
      OS_VERSION: '14.0'
    },
    methods: ['getDeviceId', 'getBatteryLevel', 'getNetworkState']
  },
  
  // 存储模块  
  AsyncStorage: {
    methods: ['getItem', 'setItem', 'removeItem', 'clear', 'getAllKeys']
  },
  
  // 网络请求模块
  NetworkManager: {
    constants: {
      TIMEOUT: 30000
    },
    methods: ['request', 'upload', 'download', 'cancelRequest']
  },

  // 原生 UI 模块
  UIManager: {
    methods: ['showAlert', 'showToast', 'showLoading', 'hideLoading']
  }
});

// 导出模块和便捷访问方式
module.exports = {
  NativeModules: nativeModules,
  
  // 便捷访问各个模块
  DeviceInfo: nativeModules.getModule('DeviceInfo'),
  AsyncStorage: nativeModules.getModule('AsyncStorage'), 
  NetworkManager: nativeModules.getModule('NetworkManager'),
  UIManager: nativeModules.getModule('UIManager'),
  
  // 工具方法
  registerModule: (name, config) => nativeModules.registerModule(name, config),
  callNativeMethod: (module, method, ...args) => nativeModules.callMethod(module, method, ...args),
  getDebugInfo: () => nativeModules.getDebugInfo()
};
