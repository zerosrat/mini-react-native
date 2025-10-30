/**
 * NativeModule.js - React Native 兼容的模块系统
 *
 * 基于 React Native 官方 NativeModules.js 实现，严格遵循官方的模块生成逻辑。
 * 支持三种方法类型：'async', 'promise', 'sync'
 *
 * 参考：React Native v0.57.8 NativeModules.js
 *
 * 设计原则：
 * - 与 React Native 的 NativeModules 系统完全兼容
 * - 使用官方的 genModule 和 genMethod 函数逻辑
 * - 支持标准的 ModuleConfig 格式：[name, constants, methods, promiseMethods, syncMethods]
 */

'use strict';

// 获取 BatchedBridge（适配多种加载方式）
const BatchedBridge = (function() {
  if (typeof require !== 'undefined') {
    return require('./BatchedBridge');
  } else if (typeof global !== 'undefined' && global.BatchedBridge) {
    return global.BatchedBridge;
  } else if (typeof global !== 'undefined' && global.__fbBatchedBridge) {
    return global.__fbBatchedBridge;
  } else {
    console.error('[NativeModule] BatchedBridge not found');
    return null;
  }
})();

// 类型定义 (对应官方的 MethodType)
const MethodType = {
  ASYNC: 'async',
  PROMISE: 'promise',
  SYNC: 'sync'
};

/**
 * 辅助函数：检查数组是否包含指定值
 * 对应官方的 arrayContains 函数
 */
function arrayContains(array, value) {
  return array && array.indexOf(value) !== -1;
}

/**
 * 从错误数据创建错误对象
 * 对应官方的 createErrorFromErrorData 函数
 */
function createErrorFromErrorData(errorData) {
  const {message, ...extraErrorInfo} = errorData || {};
  const error = new Error(message);
  error.framesToPop = 1;
  return Object.assign(error, extraErrorInfo);
}

/**
 * 简化版的 invariant 检查函数
 * 对应官方的 invariant 函数
 */
function invariant(condition, message) {
  if (!condition) {
    throw new Error(`Invariant Violation: ${message}`);
  }
}

/**
 * 生成单个方法的代理函数
 * 对应官方的 genMethod 函数
 *
 * @param {number} moduleID - 模块ID
 * @param {number} methodID - 方法ID
 * @param {string} type - 方法类型：'async', 'promise', 'sync'
 * @returns {Function} 生成的方法代理函数
 */
function genMethod(moduleID, methodID, type) {
  let fn = null;

  if (type === MethodType.PROMISE) {
    // Promise 方法：返回 Promise 对象
    fn = function(...args) {
      return new Promise((resolve, reject) => {
        BatchedBridge.enqueueNativeCall(
          moduleID,
          methodID,
          args,
          data => resolve(data),
          errorData => reject(createErrorFromErrorData(errorData))
        );
      });
    };
  } else if (type === MethodType.SYNC) {
    // 同步方法：使用 nativeCallSyncHook
    fn = function(...args) {
      if (typeof global.nativeCallSyncHook !== 'function') {
        invariant(
          false,
          'Calling synchronous methods on native modules is not supported. ' +
          'Consider providing alternative methods to expose this method in debug mode.'
        );
      }
      return global.nativeCallSyncHook(moduleID, methodID, args);
    };
  } else {
    // 异步方法：支持双回调模式 (error callback + success callback)
    fn = function(...args) {
      const lastArg = args.length > 0 ? args[args.length - 1] : null;
      const secondLastArg = args.length > 1 ? args[args.length - 2] : null;
      const hasSuccessCallback = typeof lastArg === 'function';
      const hasErrorCallback = typeof secondLastArg === 'function';

      // React Native 的双回调约定：如果有错误回调，必须也有成功回调
      if (hasErrorCallback) {
        invariant(
          hasSuccessCallback,
          'Cannot have a non-function arg after a function arg.'
        );
      }

      const onSuccess = hasSuccessCallback ? lastArg : null;
      const onFail = hasErrorCallback ? secondLastArg : null;
      const callbackCount = (hasSuccessCallback ? 1 : 0) + (hasErrorCallback ? 1 : 0);
      const actualArgs = args.slice(0, args.length - callbackCount);

      BatchedBridge.enqueueNativeCall(
        moduleID,
        methodID,
        actualArgs,
        onFail,
        onSuccess
      );
    };
  }

  // 添加类型标识，便于调试
  fn.type = type;
  return fn;
}

/**
 * 生成模块代理对象
 * 对应官方的 genModule 函数
 *
 * @param {Array} config - 模块配置 [name, constants, methods, promiseMethods, syncMethods]
 * @param {number} moduleID - 模块ID
 * @returns {Object|null} 生成的模块信息 {name, module}
 */
function genModule(config, moduleID) {
  if (!config) {
    return null;
  }

  const [moduleName, constants, methods, promiseMethods, syncMethods] = config;

  // 验证模块名称（简化版的前缀检查）
  invariant(
    !moduleName.startsWith('RCT') && !moduleName.startsWith('RK'),
    `Module name prefixes should've been stripped by the native side but wasn't for ${moduleName}`
  );

  // 如果没有常量和方法，返回延迟加载的模块信息
  if (!constants && !methods) {
    return {name: moduleName};
  }

  const module = {};

  // 生成所有方法
  if (methods) {
    methods.forEach((methodName, methodID) => {
      const isPromise = arrayContains(promiseMethods, methodID);
      const isSync = arrayContains(syncMethods, methodID);

      // 确保方法不能既是 Promise 又是同步
      invariant(
        !isPromise || !isSync,
        'Cannot have a method that is both async and a sync hook'
      );

      const methodType = isPromise ? MethodType.PROMISE :
                        isSync ? MethodType.SYNC :
                        MethodType.ASYNC;

      module[methodName] = genMethod(moduleID, methodID, methodType);
    });
  }

  // 添加常量到模块对象
  if (constants) {
    Object.assign(module, constants);
  }

  // 在开发模式下创建调试信息
  if (typeof global.__DEV__ !== 'undefined' && global.__DEV__) {
    // 简化版的调试信息记录
    console.log(`[NativeModule] Created module: ${moduleName} with methods:`, methods);
  }

  return {name: moduleName, module};
}

/**
 * 延迟加载模块 (简化实现)
 * 对应官方的 loadModule 函数
 */
function loadModule(name, moduleID) {
  // 简化实现：在我们的环境中，所有模块配置都是预先注入的
  // 在真实的 React Native 中，这里会调用 global.nativeRequireModuleConfig
  console.warn(`[NativeModule] Lazy loading not implemented for module: ${name}`);
  return null;
}

// 创建 NativeModules 对象
let NativeModules = {};

// 模块初始化逻辑 (对应官方的模块初始化部分)
function initializeNativeModules() {
  const bridgeConfig = global.__fbBatchedBridgeConfig;

  invariant(
    bridgeConfig,
    '__fbBatchedBridgeConfig is not set, cannot invoke native modules'
  );

  // 处理所有远程模块配置
  const remoteModuleConfig = bridgeConfig.remoteModuleConfig || [];

  remoteModuleConfig.forEach((config, moduleID) => {
    // 生成模块信息
    const info = genModule(config, moduleID);
    if (!info) {
      return;
    }

    if (info.module) {
      // 直接注册模块
      NativeModules[info.name] = info.module;
    } else {
      // 对于延迟加载的模块，使用 getter (简化实现)
      Object.defineProperty(NativeModules, info.name, {
        get: () => loadModule(info.name, moduleID),
        enumerable: true,
        configurable: true
      });
    }
  });

  console.log('[NativeModule] Initialized', remoteModuleConfig.length, 'native modules');
}

// 导出全局函数供 Native 端调用 (对应官方的 global.__fbGenNativeModule)
global.__fbGenNativeModule = genModule;

// 自动初始化模块系统
if (typeof global !== 'undefined' && global.__fbBatchedBridgeConfig) {
  try {
    initializeNativeModules();
  } catch (error) {
    console.error('[NativeModule] Failed to initialize native modules:', error.message);
  }
}

// 提供兼容的接口
const NativeModuleInterface = {
  /**
   * 获取指定模块
   * @param {string} moduleName - 模块名称
   * @returns {Object|null} 模块对象
   */
  get(moduleName) {
    return NativeModules[moduleName] || null;
  },

  /**
   * 获取所有模块
   * @returns {Object} 所有模块的映射对象
   */
  getAll() {
    return NativeModules;
  },

  /**
   * 手动初始化模块系统
   */
  initialize() {
    if (global.__fbBatchedBridgeConfig) {
      initializeNativeModules();
    } else {
      console.warn('[NativeModule] Cannot initialize: __fbBatchedBridgeConfig not available');
    }
  },

  // 导出内部函数供调试使用
  _genModule: genModule,
  _genMethod: genMethod,
  _arrayContains: arrayContains,
  _createErrorFromErrorData: createErrorFromErrorData
};

// 导出模块
if (typeof module !== 'undefined' && module.exports) {
  module.exports = NativeModuleInterface;
} else if (typeof global !== 'undefined') {
  global.NativeModules = NativeModuleInterface;
}

// 兼容性：支持 window 环境
if (typeof window !== 'undefined') {
  window.NativeModules = NativeModuleInterface;
}